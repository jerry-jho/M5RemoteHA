#include <Arduino.h>
#include <M5Stack.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

#include "RCDisplay.h"
#include "private.h"

RCDisplay rcdisplay;

WiFiMulti wifiMulti;

#define REFRESH_INTERVAL 10000

void setup() {
  Serial.begin(115200);
  rcdisplay.begin();
  wifiMulti.addAP(ssid, password);
}

int post_services(const char * service, const char * id, const char * value_name, const char * value, int ivalue) {
  HTTPClient http;
  http.begin(service);
  http.addHeader("Authorization", token);
  DynamicJsonDocument doc(128);
  doc["entity_id"] = id;
  if (value_name != nullptr) {
    if (value == nullptr) {
      doc[value_name] = ivalue;
    } else {
      doc[value_name] = value;
    }
    
  }
  char myDoc[measureJson(doc) + 1];
  serializeJson(doc, myDoc, measureJson(doc) + 1);
  int httpCode = http.POST(String(myDoc));
  Serial.printf("[HTTP] POST... code: %d\n", httpCode);
  return httpCode;
}

const char * hvac_names[]= {"off","heat","cool"};

void loop(){
  if((wifiMulti.run() == WL_CONNECTED)) {
    for (int room_id=0;room_id<ROOM_COUNT;room_id++) {
      int valid = 0;
      int set_temp;
      int sensor_temp;
      int hvac_mode;
      do {
        HTTPClient http;
        http.begin(String(api_status_address) + room_api_climate_list[room_id]);
        http.addHeader("Authorization", token);
        http.addHeader("Content-Type", "application/json");
        int httpCode = http.GET();
        USE_SERIAL.printf("Room %d\n", room_id);
        if(httpCode > 0) {
          //USE_SERIAL.printf("[HTTP] GET... code: %d\n", httpCode);
          if(httpCode == HTTP_CODE_OK) {
            String payload = http.getString();
            //USE_SERIAL.println(payload);
            DynamicJsonDocument doc(1024);
            deserializeJson(doc, payload);
            valid = 1;
            set_temp = doc["attributes"]["temperature"];
            sensor_temp = doc["attributes"]["current_temperature"];
            String state = doc["state"];
            hvac_mode = 0;
            for (int m=0;m<3;m++) {
              if (state == String(hvac_names[m])) {
                hvac_mode = m;
                break;
              }
            }
          } else {
            USE_SERIAL.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
          }
        }
        http.end();
      } while (0);

      int fan_valid = 0;
      int fan_speed = -1;
      do {
        HTTPClient http;
        http.begin(String(api_status_address) + room_api_fan_list[room_id]);
        http.addHeader("Authorization", token);
        http.addHeader("Content-Type", "application/json");
        int httpCode = http.GET();
        if(httpCode > 0) {
          USE_SERIAL.printf("[HTTP] GET... code: %d\n", httpCode);
          if(httpCode == HTTP_CODE_OK) {
            String payload = http.getString();
            //USE_SERIAL.println(payload);
            DynamicJsonDocument doc(1024);
            deserializeJson(doc, payload);
            fan_valid = 1;
            
            JsonArray array = doc["attributes"]["options"].as<JsonArray>();
            fan_speed = 0;
            String fan_state = doc["state"];
            //USE_SERIAL.printf("fan_state %s\n",fan_state);
            for(JsonVariant v : array) {
              //USE_SERIAL.printf("fan_opt %s\n",v.as<String>());
              if (v.as<String>() == fan_state) break;
              fan_speed ++;
            }
          } else {
            Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
          }
        }
        http.end();
      } while(0);

      if (valid) {
        rcdisplay.set_room_info(room_id, hvac_mode, sensor_temp, set_temp, fan_speed);
      }
    }
    unsigned long start = millis();
    while(1) {
      M5.update();
      
      if (M5.BtnB.wasPressed()) {
        int room_id = rcdisplay.get_room_id();
        int hvac = rcdisplay.get_room_hvac_status(room_id);
        if (hvac != HVAC_OFF) {
          rcdisplay.next_page();
        } else {
          rcdisplay.init_page();
        }
      }
      if (M5.BtnA.wasPressed()) {
        page_t current_page = rcdisplay.page();
        if (current_page == PAGE_ROOM) {
          rcdisplay.next_room();
          int room_id = rcdisplay.get_room_id();
          int hvac = rcdisplay.get_room_hvac_status(room_id);
          rcdisplay.init_page();
        } else if (current_page == PAGE_TEMP) {
          int room_id = rcdisplay.get_room_id();
          int temp = rcdisplay.get_room_set_temp(room_id);
          if (temp != -1) {
            temp -= 1;
            post_services(api_set_temperature_address, room_api_climate_list[room_id], "temperature", nullptr, temp);
            break;
          }
        } else if (current_page == PAGE_FAN) {
          int room_id = rcdisplay.get_room_id();
          post_services(api_set_next_address, room_api_fan_list[room_id], nullptr, nullptr, 0);
          break;
        }
      }
      if (M5.BtnC.wasPressed()) {
        page_t current_page = rcdisplay.page();
        if (current_page == PAGE_ROOM) {
          int room_id = rcdisplay.get_room_id();
          int hvac = rcdisplay.get_room_hvac_status(room_id);
          if (hvac == HVAC_OFF) {
            post_services(api_turn_on_address, room_api_climate_list[room_id], nullptr, nullptr, 0);
            post_services(api_set_hvac_address, room_api_climate_list[room_id], "hvac_mode", "cool", 0);
            break;
          } else {
            post_services(api_turn_off_address, room_api_climate_list[room_id], nullptr, nullptr, 0);
            break;
          }
        } else if (current_page == PAGE_TEMP) {
          int room_id = rcdisplay.get_room_id();
          int temp = rcdisplay.get_room_set_temp(room_id);
          if (temp != -1) {
            temp += 1;
            post_services(api_set_temperature_address, room_api_climate_list[room_id], "temperature", nullptr, temp);
            break;
          }
        } else if (current_page == PAGE_FAN) {
          int room_id = rcdisplay.get_room_id();
          int hvac = rcdisplay.get_room_hvac_status(room_id);
          if (hvac == HVAC_COOL) {
            post_services(api_set_hvac_address, room_api_climate_list[room_id], "hvac_mode", "heat", 0);
            break;
          } else if (hvac == HVAC_HEAT) {
            post_services(api_set_hvac_address, room_api_climate_list[room_id], "hvac_mode", "cool", 0);
            break;
          }
        }    
      }
      if (millis() - start > REFRESH_INTERVAL) {
        break;
      }
      delay(100);
    }
  } else {
    delay(REFRESH_INTERVAL);
  }

}