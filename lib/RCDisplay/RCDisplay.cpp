#include "RCDisplay.h"
#include "display_ch.h"


RCDisplay::RCDisplay() {

}

DisplayCh displaych;

const char * speed_name[] = {
  "\xA3\xC1",
  "\xA2\xF1",
  "\xA2\xF2",
  "\xA2\xF3"
};

void RCDisplay::begin() {
  M5.begin();
  M5.Power.begin();
  displaych.loadHzk16();
  displaych.setTextSize(2);
  current_room = 0;
  current_page = PAGE_ROOM;
  for (int i=0;i<ROOM_COUNT;i++) {
    all_info[i*INFO_SIZE] = i;
  }
}

void RCDisplay::set_room_info(int room_id, int hvac_status, int sensor_temp, int set_temp, int fan_speed) {
  if (room_id >= 0 && room_id < ROOM_COUNT) {
    int * info_start = all_info + room_id * INFO_SIZE;
    if (info_start[1] != hvac_status ||
        info_start[2] != sensor_temp ||
        info_start[3] != set_temp ||
        info_start[4] != fan_speed
    ) {
      info_start[1] = hvac_status;
      info_start[2] = sensor_temp;
      info_start[3] = set_temp;
      info_start[4] = fan_speed;
      render();
    }
  }
}

int RCDisplay::get_room_set_temp(int room_id) {
  if (room_id >= 0 && room_id < ROOM_COUNT) {
    int * info_start = all_info + room_id * INFO_SIZE;
    int temp = info_start[3];
    if (temp < 16 || temp > 30) {
      temp = info_start[2];
    }
    return temp;
  }
  return -1;
}

int RCDisplay::get_room_hvac_status(int room_id) {
  if (room_id >= 0 && room_id < ROOM_COUNT) {
    int * info_start = all_info + room_id * INFO_SIZE;
    int temp = info_start[1];
    return temp;
  }
  return -1;
}

void RCDisplay::next_page() {
  if (current_page == PAGE_ROOM) current_page = PAGE_TEMP;
  else if (current_page == PAGE_TEMP) current_page = PAGE_FAN;
  else current_page = PAGE_ROOM;
  render();
}

void RCDisplay::init_page() {
  current_page = PAGE_ROOM;
  render();
}

page_t RCDisplay::page() {
  return current_page;
}

void RCDisplay::next_room() {
  current_room ++;
  if (current_room == ROOM_COUNT) current_room = 0;
  render();
}

int RCDisplay::get_room_id() {
  return current_room;
}

void RCDisplay::render() {
  _render_main(all_info + current_room * INFO_SIZE);
  _render_tail(current_page);
}

void RCDisplay::_render_main(int * info) {
  M5.Lcd.fillRect(0, 0, 320, 32, YELLOW);
  displaych.setTextColor(BLACK, YELLOW);
  displaych.setCursor(32*4, 0, 2);
  displaych.writeHzk(ROOM_NAMES[info[0]]);
  M5.Lcd.fillRect(0, 32, 320, 200, BLACK);
  M5.Lcd.setTextSize(7);
  M5.Lcd.setTextColor(GREEN, BLACK);
  M5.Lcd.drawNumber(info[2], 0+40, 32 + 20);
  if (info[1] != HVAC_OFF) {
    M5.Lcd.drawNumber(info[3], 160+40, 32 + 20);
  }
  displaych.setCursor(16, 160-32-16, 1);
  displaych.setTextColor(GREEN, BLACK);
  displaych.writeHzk("\xB5\xB1\xC7\xB0\xCE\xC2\xB6\xC8");
  if (info[1] != HVAC_OFF) {
    displaych.setCursor(16+160, 160-32-16, 1);
    displaych.setTextColor(GREEN, BLACK);
    displaych.writeHzk("\xC9\xE8\xB6\xA8\xCE\xC2\xB6\xC8");
    displaych.setCursor(16+160, 160-13, 1);
    if (info[1] == HVAC_HEAT) {
      displaych.setTextColor(BLACK, RED);
      displaych.writeHzk("\xBC\xD3\xC8\xC8");
    } else if (info[1] == HVAC_COOL) {
      displaych.setTextColor(BLACK, BLUE);
      displaych.writeHzk("\xD6\xC6\xC0\xE4");      
    }
    
    if (info[4] >= 0 && info[4] <= 3) {
      displaych.setTextColor(YELLOW, BLACK);
      displaych.setCursor(160+32*3, 160-13, 1);
      displaych.writeHzk(speed_name[info[4]]); 
    }
  } 
}

void RCDisplay::_render_tail(page_t page) {
  M5.Lcd.fillRect(0, 240-36, 320, 240, GREEN);
  displaych.setCursor(0, 240-34, 2);
  displaych.setTextColor(GREEN, BLACK);
  displaych.setCursor(32*4, 240-34, 2);
  displaych.writeHzk("\xB9\xA6\xC4\xDC");

  switch (page) {
    case PAGE_ROOM:
      displaych.setCursor(32, 240-34, 2);
      displaych.writeHzk("\xB7\xBF\xBC\xE4");
      displaych.setCursor(32*7, 240-34, 2);
      displaych.writeHzk("\xBF\xAA\xB9\xD8");
      break;
    case PAGE_TEMP:
      displaych.setCursor(32, 240-34, 2);
      displaych.writeHzk("\xBD\xB5\xCE\xC2");
      displaych.setCursor(32*7, 240-34, 2);
      displaych.writeHzk("\xC9\xFD\xCE\xC2");
      break;
    case PAGE_FAN:
      displaych.setCursor(32, 240-34, 2);
      displaych.writeHzk("\xB7\xE7\xCB\xD9");
      displaych.setCursor(32*7, 240-34, 2);
      displaych.writeHzk("\xC4\xA3\xCA\xBD");
      break;
    default:
      break;
  }
}