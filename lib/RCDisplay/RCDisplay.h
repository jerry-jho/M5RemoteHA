#ifndef RCDISPLAY_H
#define RCDISPLAY_H

#include <Arduino.h>
#include <M5Stack.h>
#include "RCConfig.h"
#define INFO_SIZE 5

#define HVAC_OFF 0
#define HVAC_COOL 2
#define HVAC_HEAT 1

typedef enum {
  PAGE_ROOM, // [ROOM] [FUNC] [ON/OFF]
  PAGE_TEMP, // [DEC]  [FUNC] [INC]
  PAGE_FAN,  // [FAN]  [FUNC] [HVAC]
  PAGE_NONE 
} page_t;

class RCDisplay {
public:
  RCDisplay();
  void begin();
  void next_page();
  void init_page();
  void next_room();
  void set_room_info(int room_id, int hvac_status, int sensor_temp, int set_temp, int fan_speed);
  int get_room_set_temp(int room_id);
  int get_room_hvac_status(int room_id);
  int get_room_id();
  void render();
  page_t page();
private:
  // 0 - room_id
  // 1 - hvac status
  // 2 - sensor_temp
  // 3 - set_temp
  // 4 - fan_speed
  
  void _render_main(int * info);
  void _render_tail(page_t page);
  int all_info[INFO_SIZE * ROOM_COUNT];
  const char ** _room_name;
  int current_room;
  page_t current_page;
};

#endif