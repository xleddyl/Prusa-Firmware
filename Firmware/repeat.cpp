/**
 * Based on
 * Marlin 3D Printer Firmware
 * Copyright (c) 2020 MarlinFirmware [https://github.com/MarlinFirmware/Marlin]
 *
 */

#include "repeat.h"
#include "cardreader.h"
#include "Marlin.h"

repeat_marker_t Repeat::marker[MAX_REPEAT_NESTING];
uint8_t Repeat::index;
bool Repeat::seenL;
uint16_t Repeat::Lvalue;

void Repeat::add_marker(const uint32_t sdpos, const uint16_t count) {
  if (index >= MAX_REPEAT_NESTING)
    SERIAL_ECHOLNPGM("!Too many markers.");
  else {
    marker[index].sdpos = sdpos;
    marker[index].counter = count ? count - 1 : -1;
    index++;
    SERIAL_ECHOLNPGM("Add Marker");
  }
}

void Repeat::loop() {
  if (!index)                           // No marker?
    SERIAL_ECHOLNPGM("!No marker set."); //  Inform the user.
  else {
    const uint8_t ind = index - 1;      // Active marker's index
    if (!marker[ind].counter) {         // Did its counter run out?
      SERIAL_ECHOLNPGM("Pass Marker");
      index--;                          //  Carry on. Previous marker on the next 'M808'.
    }
    else {
      card.setIndex(marker[ind].sdpos); // Loop back to the marker.
      if (marker[ind].counter > 0)      // Ignore a negative (or zero) counter.
        --marker[ind].counter;          // Decrement the counter. If zero this 'M808' will be skipped next time.
      SERIAL_ECHOLNPGM("Goto Marker");
    }
  }
}

void Repeat::cancel() { for(uint8_t i = 0; i < index; i++) marker[i].counter = 0; }

void Repeat::early_parse_M808(char * const cmd) {
  if (is_command_M808(cmd)) {
    SERIAL_ECHOLNPGM("M808 found");
    parse(cmd);
    if (seenL) {
      SERIAL_ECHOPGM("Seen L with value: ");
      SERIAL_ECHOLN(Lvalue);
      add_marker(card.get_sdpos(), Lvalue);
    } else {
      SERIAL_ECHOLNPGM("Loop");
      Repeat::loop();
    }
  }
}

void Repeat::parse(char *p) {
  seenL = false;
  Lvalue = 0;

  // Skip spaces
  while (*p == ' ') ++p;

  // Skip the command letter, which in this case is M
  p++;

  // Skip spaces to get the numeric part and skip it
  while (*p == ' ') p++;
  while (*p >= '0' && *p <= '9') p++;

  // Skip all spaces to get to the first argument, or nul
  while (*p == ' ') p++;
  seenL = 'L' == *p;

  if (seenL) {
    p++;
    Lvalue = (uint16_t) (*p ? strtol(p, nullptr, 10) : 0L);
  }
}
