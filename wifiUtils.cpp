#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <WiFiClient.h>
#include <ESP8266mDNS.h>
#include "webUtils.h"
#include "fileUtils.h"

ESP8266WiFiMulti wifiMulti;

const uint8_t ssidNum = 2;
const char* ssid[] = { "U+Net2DB0", "iptimeM2G"};
const char* ssid_passwd[] = { "5000013954", "imas0315" };

const char* ap_ssid = "myAccessPoint";
const char* ap_password = "myAPPassword";

String wifiInfoTxt = "";
uint32_t wifiInfoTxtTime = 0;
const uint32_t wifiInfoTxtTimeOut = 5000;

void wifi_setup() {
    Serial.print("\r\nConfiguring access point...");
    /* You can remove the password parameter if you want the AP to be open. */
    WiFi.softAP(ap_ssid, ap_password);

    IPAddress myIP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(myIP);

    for (uint8_t i =0; i < ssidNum; i++) {
        wifiMulti.addAP(ssid[i], ssid_passwd[i]);
    }

    if (wifiMulti.run() == WL_CONNECTED) {
        Serial.println("");
        Serial.println("WiFi connected");
        Serial.println("IP address: ");
        Serial.println(WiFi.localIP());
    }

    configTime(3 * 3600, 0, "pool.ntp.org", "time.nist.gov");

    if (MDNS.begin("imasgps")) {
        Serial.println("MDNS responder started");
    }
}

void wifi_loop() {
    static bool wl_connected_info_log_flag = true;
    if (wifiMulti.run() == WL_CONNECTED) {
        if (millis() - wifiInfoTxtTime > wifiInfoTxtTimeOut) {
            wifiInfoTxt = "{\"ip\":\"" + WiFi.localIP().toString() + "\"}\r\n";
            SSEBroadcastTxt(wifiInfoTxt);
            wifiInfoTxtTime = millis();
            if (wl_connected_info_log_flag) {
                appendFile("log.txt", wifiInfoTxt.c_str());
                wl_connected_info_log_flag = false;
            }
        }
    } else {
        wl_connected_info_log_flag = true;
    }
    
    MDNS.update();
}

void print_ip_address(void) {
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
}