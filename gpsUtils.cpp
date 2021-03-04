    /*   
modified on Sep 27, 2020
Modified by MohammedDamirchi from https://github.com/mikalhart/TinyGPSPlus
https://electropeak.com/learn/ 
*/ 
#include "gpsUtils.h"
#include "webUtils.h"
#include "fileUtils.h"

/* 
  From http://aprs.gids.nl/nmea/:
   
  $GPGSV
  
  GPS Satellites in view
  
  eg. $GPGSV,3,1,11,03,03,111,00,04,15,270,00,06,01,010,00,13,06,292,00*74
      $GPGSV,3,2,11,14,25,170,00,16,57,208,39,18,67,296,40,19,40,246,00*74
      $GPGSV,3,3,11,22,42,067,42,24,14,311,43,27,05,244,00,,,,*4D

  1    = Total number of messages of this type in this cycle
  2    = Message number
  3    = Total number of SVs in view
  4    = SV PRN number
  5    = Elevation in degrees, 90 maximum
  6    = Azimuth, degrees from true north, 000 to 359
  7    = SNR, 00-99 dB (null when not tracking)
  8-11 = Information about second SV, same as field 4-7
  12-15= Information about third SV, same as field 4-7
  16-19= Information about fourth SV, same as field 4-7
*/

GPS_UTIL::GPS_UTIL(unsigned int _idx, unsigned int _RXPin, unsigned int _TXPin):gpsSerial(_RXPin, _TXPin) {
  idx = _idx;
    // TinyGPSCustom totalGPGSVMessages(gps, "GPGSV", 1); // $GPGSV sentence, first element
    // TinyGPSCustom messageNumber(gps, "GPGSV", 2);      // $GPGSV sentence, second element
    // TinyGPSCustom satsInView(gps, "GPGSV", 3);         // $GPGSV sentence, third element
}

void GPS_UTIL::begin(unsigned int baudRate) {
    gpsSerial.begin(baudRate);

    Serial.println(F("DeviceExample.ino"));
    Serial.println(F("A simple demonstration of TinyGPS++ with an attached GPS module"));
    Serial.print(F("Testing TinyGPS++ library v. ")); Serial.println(TinyGPSPlus::libraryVersion());
    Serial.println(F("by Mikal Hart"));
    Serial.println();

    // Initialize all the uninitialized TinyGPSCustom objects
    for (int i=0; i<4; ++i)
    {
        satNumber[i].begin(gps, "GPGSV", 4 + 4 * i); // offsets 4, 8, 12, 16
        elevation[i].begin(gps, "GPGSV", 5 + 4 * i); // offsets 5, 9, 13, 17
        azimuth[i].begin(  gps, "GPGSV", 6 + 4 * i); // offsets 6, 10, 14, 18
        snr[i].begin(      gps, "GPGSV", 7 + 4 * i); // offsets 7, 11, 15, 19
    }
}

void GPS_UTIL::displayInfo()
{
  static bool log_gps_datetime_flag = true;
  if(log_gps_datetime_flag) {
    if (gps.date.isValid() & gps.time.isValid())
    {
      String gpsValidDatetimeStr = "gps datetime:" + String(gps.date.year(),DEC)+"-"+String(gps.date.month(),DEC)+"-"+String(gps.date.day(),DEC)+"U"+String(gps.time.hour(),DEC)+":"+String(gps.time.minute(),DEC)+":"+String(gps.time.second(),DEC)+"\r\n";
      appendFile("log.txt", gpsValidDatetimeStr.c_str());
      log_gps_datetime_flag = false;
    }
  }
  if (gps.location.isValid()) {

    if (!latLongValid) latLongValid = true;
    latLongValidTime = millis();

    if (millis() - displayInfoTime < displayInfoTimeLimit) {
      return;
    }
    
    displayInfoTime = millis();
    
    dtostrf(gps.location.lat(),4,8,latStr);
    dtostrf(gps.location.lng(),4,8,longStr);
    String str = "{\"latitude\":" + String(latStr) + ",\"longitude\":" + String(longStr) + "}";
    SSEBroadcastTxt(str);
    String fileStr = "#" + String(idx, DEC)+";"+String(latStr) + ", " + String(longStr) + ";";

    Serial.print(F("Location: ")); 
    Serial.print(gps.location.lat(), 6);
    Serial.print(F(","));
    Serial.print(gps.location.lng(), 6);

    Serial.print(F("  Date/Time: "));
    if (gps.date.isValid())
    {
      Serial.print(gps.date.month());
      Serial.print(F("/"));
      Serial.print(gps.date.day());
      Serial.print(F("/"));
      Serial.print(gps.date.year());
      fileStr += String(gps.date.year(),DEC)+"-"+String(gps.date.month(),DEC)+"-"+String(gps.date.day(),DEC);
    }
    else
    {
      Serial.print(F("INVALID"));
    }
    fileStr+=";";
    Serial.print(F(" "));
    if (gps.time.isValid())
    {
      if (gps.time.hour() < 10) Serial.print(F("0"));
      Serial.print(gps.time.hour());
      Serial.print(F(":"));
      if (gps.time.minute() < 10) Serial.print(F("0"));
      Serial.print(gps.time.minute());
      Serial.print(F(":"));
      if (gps.time.second() < 10) Serial.print(F("0"));
      Serial.print(gps.time.second());
      Serial.print(F("."));
      if (gps.time.centisecond() < 10) Serial.print(F("0"));
      Serial.print(gps.time.centisecond());
      fileStr+=String(gps.time.hour(),DEC)+":"+String(gps.time.minute(),DEC)+":"+String(gps.time.second(),DEC);
    }
    else
    {
      Serial.print(F("INVALID"));
    }
    fileStr+=";";
    appendFile(R"(/gpslog.txt)", fileStr.c_str() );
    Serial.println();
  } else {
    if (latLongValid) {
      if (millis() - latLongValidTime > latLongValidTimeOut) {
          String str = "{\"latitude\":" + String(latStr) + ",\"longitude\":" + String(longStr) + "}";
          SSEBroadcastTxt(str);
          latLongValidTime = millis();
      }
    }
  }

}

// void GPS_UTIL::gpsSatelliteTracker() {
//   if (totalGPGSVMessages.isUpdated())
//     {
//       for (int i=0; i<4; ++i)
//       {
//         int no = atoi(satNumber[i].value());
//         // Serial.print(F("SatNumber is ")); Serial.println(no);
//         if (no >= 1 && no <= this.MAX_SATELLITES)
//         {
//           sats[no-1].elevation = atoi(elevation[i].value());
//           sats[no-1].azimuth = atoi(azimuth[i].value());
//           sats[no-1].snr = atoi(snr[i].value());
//           sats[no-1].active = true;
//         }
//       }
      
//       int totalMessages = atoi(totalGPGSVMessages.value());
//       int currentMessage = atoi(messageNumber.value());
//       if (totalMessages == currentMessage)
//       {
//         Serial.print(F("Sats=")); Serial.print(gps.satellites.value());
//         Serial.print(F(" Nums="));
//         for (int i=0; i<MAX_SATELLITES; ++i)
//           if (sats[i].active)
//           {
//             Serial.print(i+1);
//             Serial.print(F(" "));
//           }
//         Serial.print(F(" Elevation="));
//         for (int i=0; i<MAX_SATELLITES; ++i)
//           if (sats[i].active)
//           {
//             Serial.print(sats[i].elevation);
//             Serial.print(F(" "));
//           }
//         Serial.print(F(" Azimuth="));
//         for (int i=0; i<MAX_SATELLITES; ++i)
//           if (sats[i].active)
//           {
//             Serial.print(sats[i].azimuth);
//             Serial.print(F(" "));
//           }
        
//         Serial.print(F(" SNR="));
//         for (int i=0; i<MAX_SATELLITES; ++i)
//           if (sats[i].active)
//           {
//             Serial.print(sats[i].snr);
//             Serial.print(F(" "));
//           }
//         Serial.println();
  
//         for (int i=0; i<MAX_SATELLITES; ++i)
//           sats[i].active = false;
//       }
//     }
// }

// void GPS_UTIL::gpsSatelliteElevPrinter() {
//   if (totalGPGSVMessages.isUpdated())
//     {
//       for (int i=0; i<4; ++i)
//       {
//         int no = atoi(satNumber[i].value());
//         if (no >= 1 && no <= MAX_SATELLITES)
//         {
//           int elev = atoi(elevation[i].value());
//           sats[no-1].active = true;
//           if (sats[no-1].elevation != elev)
//           {
//             sats[no-1].elevation = elev;
//             anyChanges = true;
//           }
//         }
//       }
      
//       int totalMessages = atoi(totalGPGSVMessages.value());
//       int currentMessage = atoi(messageNumber.value());
//       if (totalMessages == currentMessage && anyChanges)
//       {
//         if (linecount++ % PAGE_LENGTH == 0)
//           printHeader();
//         TimePrint();
//         for (int i=0; i<MAX_SATELLITES; ++i)
//         {
//           Serial.print(F(" "));
//           if (sats[i].active)
//             IntPrint(sats[i].elevation, 2);
//           else
//             Serial.print(F("   "));
//           sats[i].active = false;
//         }
//         Serial.println();
//         anyChanges = false;
//       }
//     }
// }

void GPS_UTIL::IntPrint(int n, int len)
{
  int digs = n < 0 ? 2 : 1;
  for (int i=10; i<=abs(n); i*=10)
    ++digs;
  while (digs++ < len)
    Serial.print(F(" "));
  Serial.print(n);
  Serial.print(F(" "));
}

void GPS_UTIL::TimePrint()
{
  if (gps.time.isValid())
  {
    if (gps.time.hour() < 10)
      Serial.print(F("0"));
    Serial.print(gps.time.hour());
    Serial.print(F(":"));
    if (gps.time.minute() < 10)
      Serial.print(F("0"));
    Serial.print(gps.time.minute());
    Serial.print(F(":"));
    if (gps.time.second() < 10)
      Serial.print(F("0"));
    Serial.print(gps.time.second());
    Serial.print(F(" "));
  }
  else
  {
    Serial.print(F("(unknown)"));
  }
}

void GPS_UTIL::printHeader()
{
  Serial.println();
  Serial.print(F("Time     "));
  for (int i=0; i<MAX_SATELLITES; ++i)
  {
    Serial.print(F(" "));
    IntPrint(i+1, 2);
  }
  Serial.println();
  Serial.print(F("---------"));
  for (int i=0; i<MAX_SATELLITES; ++i)
    Serial.print(F("----"));
  Serial.println();
}

void GPS_UTIL::update(void) {
  if(!gps_flag) {
      Serial.println(F("No GPS detected: check wiring."));
  }

    // This sketch displays information every time a new sentence is correctly encoded.
  while (gpsSerial.available() > 0) {
    // if (c == 0) SSE_add_char("\"");
    c = gpsSerial.read();
    // Serial.print(c);
    if (c != '\r' && c != '\n' && c > 0) rawGPSData += c;
    if(gps.encode(c)) {
      rawGPSData += "\",\"idx\":"+String(idx)+"}";
      // SSE_add_char(rawGPSData.c_str());
      displayInfo();
      // gpsSatelliteElevPrinter();
        ////        gpsSatelliteTracker();
      // set_SSE_broadcast_flag(true);
      if (print_raw_flag) {
        Serial.println(rawGPSData);
      }
      rawGPSData = "{\"raw\":\"";
    }
  }
  
  if (millis() > 5000 && gps.charsProcessed() < 10)
    gps_flag = false;
}

void GPS_UTIL::toggle_print_raw(void) {
  print_raw_flag = !print_raw_flag;
}