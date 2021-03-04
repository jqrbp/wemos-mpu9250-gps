#ifndef __serial_Utils_h__
#define __serial_Utils_h__

#if defined(ARDUINO) && ARDUINO >= 100
  #include "Arduino.h"
#else
  #include "WProgram.h"
#endif

#define SERIAL_DEBUG
#ifdef SERIAL_DEBUG
  #define DEBUGPRINT(x) Serial.print(x)
  #define DEBUGPRINTF(x, y) Serial.print(x, y)
  #define DEBUGPRINTLN(x) Serial.println(x)
#else
  #define DEBUGPRINT(x)
  #define DEBUGPRINTF(x, y)
  #define DEBUGPRINTLN(x)
#endif // SERIAL_DEBUG

void serial_begin(unsigned int baudRate);
void serial_loop(void);

#endif // def(__serial_Utils_h__)