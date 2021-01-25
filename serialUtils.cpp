#include <stdint.h>
#include "serialUtils.h"
#include "imuUtils.h"
#include "wifiUtils.h"
#include "gpsUtilsWrapper.h"

#define SERIALINLIM 16
static char serialData[SERIALINLIM];
static int data[3];
static uint8_t dataLen;

void serial_begin(unsigned int baudRate) {
  Serial.begin(baudRate);
}

void parseCommand(char* command, uint8_t _sIdx, int* returnValues, uint8_t *valLen)
{
  // parsing state machine
  uint8_t i = _sIdx, j = 0, sign = 0, valid=1;
  char c = 0;
  int temp = 0;

  while(*(command + i) != '\0')
  {
    c = *(command + i);
    if (c == '!') break;
    switch(c)
    {
      case ',':
        returnValues[j++] = sign?-temp:temp;
        sign = 0;
        temp = 0;
        break;
      case '-':
        sign = 1;
        break;
      case '\n':
        break;
      case '\r':
        break;
      default:
        if ((c > 47) && (c < 58)) {
          temp = temp * 10 + (int)c - 48;
        } else {
          valid = 0;
        }
    }
    i++;
    if (i > SERIALINLIM) {
      valid = 0;
      break;
    }
  }

  if (valid) {
    returnValues[j] = sign?-temp:temp;
    *valLen = (uint8_t)j + 1;
  } else *valLen = 0;
}

void serial_loop(void) {
  uint16_t pdata;
  if(Serial.available() > 0)
  {
    Serial.readBytesUntil('!', serialData, 15);
    switch(serialData[0])
    {
      case 'g':
        gpsUtilsWrapper_toggle_debug_print();
        break;
      case 'w':
        print_ip_address();
        break;
      case 'b':
        toggle_broadcast_serial_print_flag();
        break;
      case 'i':
        toggle_serial_debug_imu_flag();
        break;
      case 'a':
        break;
      case 's':
        parseCommand(serialData, 2, data, &dataLen);
        DEBUGPRINT("Data len:");DEBUGPRINTF(dataLen, HEX);
        DEBUGPRINT("d0:");DEBUGPRINTF((uint8_t)data[0], DEC);
        DEBUGPRINT("d1:");DEBUGPRINTF((uint8_t)data[1], DEC);
        DEBUGPRINT("d2:");DEBUGPRINTF((uint8_t)data[2], DEC);
      case 'l':
        break;
    }
    // always echo
    DEBUGPRINTLN(serialData);

    memset(serialData, 0, sizeof(serialData));
  }
}