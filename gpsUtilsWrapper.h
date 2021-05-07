#ifndef __gps_Utils_Wrapper_h__
#define __gps_Utils_Wrapper_h__

void gpsUtilsWrapper_begin();
void gpsUtilsWrapper_loop();
void gpsUtilsWrapper_toggle_debug_print();
float gpsUtilsWrapper_get_latitude(int index);
float gpsUtilsWrapper_get_longitude(int index);

#endif // def(__gps_Utils_h__)