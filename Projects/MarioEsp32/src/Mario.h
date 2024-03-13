#include <Notes.h>

uint16_t mario[] = {
    NOTE_E7, NOTE_E7, 0, NOTE_E7, 0, NOTE_C7, NOTE_E7, 0,
    NOTE_G7, 0, 0, 0, NOTE_G6, 0, 0, 0,
    NOTE_C7, 0, 0, NOTE_G6, 0, 0, NOTE_E6, 0,
    0, NOTE_A6, 0, NOTE_B6, 0, NOTE_AS6, NOTE_A6, 0,
    NOTE_G6, NOTE_E7, 0, NOTE_G7, NOTE_A7, 0, NOTE_F7, NOTE_G7,
    0, NOTE_E7, 0, NOTE_C7, NOTE_D7, NOTE_B6, 0, 0,
    NOTE_C7, 0, 0, NOTE_G6, 0, 0, NOTE_E6, 0,
    0, NOTE_A6, 0, NOTE_B6, 0, NOTE_AS6, NOTE_A6, 0,
    NOTE_G6, NOTE_E7, 0, NOTE_G7, NOTE_A7, 0, NOTE_F7, NOTE_G7,
    0, NOTE_E7, 0, NOTE_C7, NOTE_D7, NOTE_B6, 0, 0
};

// Durations for the Super Mario Bros. theme
uint8_t marioDurations[] = {
  8, 8, 8, 8, 8, 8, 8, 8,
  8, 8, 8, 8, 8, 8, 8, 8,
  8, 8, 8, 8, 8, 8, 8, 8,
  8, 8, 8, 8, 8, 8, 8, 8,
  8, 8, 8, 8, 8, 8, 8, 8,
  8, 8, 8, 8, 8, 8, 8, 8,
  8, 8, 8, 8, 8, 8, 8, 8,
  8, 8, 8, 8, 8, 8, 8, 8,
  8, 8, 8, 8, 8, 8, 8, 8,
  8, 8, 8, 8, 8, 8, 8, 8,
  8, 8, 8, 8, 8, 8, 8, 8,
};
  

uint8_t sizeMario = sizeof(mario)/4;