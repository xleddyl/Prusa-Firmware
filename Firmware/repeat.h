/**
 * Based on
 * Marlin 3D Printer Firmware
 * Copyright (c) 2020 MarlinFirmware [https://github.com/MarlinFirmware/Marlin]
 *
 */

#ifndef REPEAT_H
#define REPEAT_H

#include <stdint.h>

#define MAX_REPEAT_NESTING 10
#define ENABLE_GCODE_REPEAT_MARKERS

typedef struct {
  uint32_t sdpos;
  int16_t counter;
} repeat_marker_t;

class Repeat {
  private:
    static repeat_marker_t marker[MAX_REPEAT_NESTING];
    static uint8_t index;
    static bool seenL;
    static uint16_t Lvalue;

  public:
    static void parse(char *p);
    static void reset() { index = 0; }
    static bool is_active() {
      for(uint8_t i = 0; i < index; i++) if (marker[i].counter) return true;
      return false;
    }
    static bool is_command_M808(char * const cmd) { return cmd[0] == 'M' && cmd[1] == '8' && cmd[2] == '0' && cmd[3] == '8' && !(cmd[4] >= '0' && cmd[4] <= '9'); }
    static void early_parse_M808(char * const cmd);
    static void add_marker(const uint32_t sdpos, const uint16_t count);
    static void loop();
    static void cancel();
};

extern Repeat repeat;

#endif // REPEAT_H
