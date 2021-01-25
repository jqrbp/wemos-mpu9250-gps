#ifndef __gps_Utils_h__
#define __gps_Utils_h__

#include <SoftwareSerial.h>
#include "TinyGPS.h"

const int MAX_SATELLITES = 40;
const int PAGE_LENGTH = 40;

class GPS_UTIL
{
    public:
        char latStr[13];
        char longStr[13];
        unsigned int idx;
        GPS_UTIL(unsigned int _idx, unsigned int _RXPin, unsigned int _TXPin);
        void begin(unsigned int baudRate);
        void update(void);
        void toggle_print_raw(void);
    private:
        // The TinyGPS++ object
        TinyGPSPlus gps;
        SoftwareSerial gpsSerial;
        bool print_raw_flag = false;

        char c = 0;
        String rawGPSData = "";

        bool latLongValid = false;
        uint32_t latLongValidTime = 0;
        const uint32_t latLongValidTimeOut = 10000;

        const uint32_t displayInfoTimeLimit = 1000;
        uint32_t displayInfoTime = 0;

        TinyGPSCustom satNumber[4]; // to be initialized later
        TinyGPSCustom elevation[4];
        TinyGPSCustom azimuth[4];
        TinyGPSCustom snr[4];
        TinyGPSCustom totalGPGSVMessages; // $GPGSV sentence, first element
        TinyGPSCustom messageNumber;      // $GPGSV sentence, second element
        TinyGPSCustom satsInView;         // $GPGSV sentence, third element
        bool anyChanges = false;
        unsigned long linecount = 0;
        bool gps_flag = true;

        void displayInfo();
        // void gpsSatelliteTracker();
        // void gpsSatelliteElevPrinter();
        void IntPrint(int n, int len);
        void TimePrint();
        void printHeader();
};


#endif // def(__gps_Utils_h__)