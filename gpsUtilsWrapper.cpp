#include "gpsUtils.h"

GPS_UTIL gps1(1,12,13);
GPS_UTIL gps2(2,2,15);
uint8_t idx = 0;

void gpsUtilsWrapper_begin(void) {
  gps1.begin(9600);
  gps2.begin(9600);
}

void gpsUtilsWrapper_loop(void) {
  gps1.update();
  gps2.update();
}

void gpsUtilsWrapper_toggle_debug_print(void) {
  gps1.toggle_print_raw();
  gps2.toggle_print_raw();
}

float gpsUtilsWrapper_get_latitude(int index) {
  switch(index) {
    case 1:
      gps1.latitude;
      break;
    case 2:
      gps2.latitude;
      break;
  }
}

float gpsUtilsWrapper_get_longitude(int index) {
  switch(index) {
    case 1:
      gps1.longitude;
      break;
    case 2:
      gps2.longitude;
      break;
  }
}