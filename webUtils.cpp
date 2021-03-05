/*
  HelloServerBearSSL - Simple HTTPS server example

  This example demonstrates a basic ESP8266WebServerSecure HTTPS server
  that can serve "/" and "/inline" and generate detailed 404 (not found)
  HTTP respoinses.  Be sure to update the SSID and PASSWORD before running
  to allow connection to your WiFi network.

  Adapted by Earle F. Philhower, III, from the HelloServer.ino example.
  This example is released into the public domain.
*/
// #include <ESP8266WebServerSecure.h>
#include <ESP8266WebServer.h>
#include <Ticker.h>
#include <LittleFS.h>
#include "fileUtils.h"
#include "imuUtils.h"

const unsigned int serverPort = 8080;

// BearSSL::ESP8266WebServerSecure server(serverPort);
ESP8266WebServer server(serverPort);
#define SSE_MAX_CHANNELS 8  // in this simplified example, only eight SSE clients subscription allowed
struct SSESubscription {
  IPAddress clientIP;
  WiFiClient client;
  Ticker keepAliveTimer;
} subscription[SSE_MAX_CHANNELS];
uint8_t subscriptionCount = 0;

static const char serverCert[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIIDSzCCAjMCCQD2ahcfZAwXxDANBgkqhkiG9w0BAQsFADCBiTELMAkGA1UEBhMC
VVMxEzARBgNVBAgMCkNhbGlmb3JuaWExFjAUBgNVBAcMDU9yYW5nZSBDb3VudHkx
EDAOBgNVBAoMB1ByaXZhZG8xGjAYBgNVBAMMEXNlcnZlci56bGFiZWwuY29tMR8w
HQYJKoZIhvcNAQkBFhBlYXJsZUB6bGFiZWwuY29tMB4XDTE4MDMwNjA1NDg0NFoX
DTE5MDMwNjA1NDg0NFowRTELMAkGA1UEBhMCQVUxEzARBgNVBAgMClNvbWUtU3Rh
dGUxITAfBgNVBAoMGEludGVybmV0IFdpZGdpdHMgUHR5IEx0ZDCCASIwDQYJKoZI
hvcNAQEBBQADggEPADCCAQoCggEBAPVKBwbZ+KDSl40YCDkP6y8Sv4iNGvEOZg8Y
X7sGvf/xZH7UiCBWPFIRpNmDSaZ3yjsmFqm6sLiYSGSdrBCFqdt9NTp2r7hga6Sj
oASSZY4B9pf+GblDy5m10KDx90BFKXdPMCLT+o76Nx9PpCvw13A848wHNG3bpBgI
t+w/vJCX3bkRn8yEYAU6GdMbYe7v446hX3kY5UmgeJFr9xz1kq6AzYrMt/UHhNzO
S+QckJaY0OGWvmTNspY3xCbbFtIDkCdBS8CZAw+itnofvnWWKQEXlt6otPh5njwy
+O1t/Q+Z7OMDYQaH02IQx3188/kW3FzOY32knER1uzjmRO+jhA8CAwEAATANBgkq
hkiG9w0BAQsFAAOCAQEAnDrROGRETB0woIcI1+acY1yRq4yAcH2/hdq2MoM+DCyM
E8CJaOznGR9ND0ImWpTZqomHOUkOBpvu7u315blQZcLbL1LfHJGRTCHVhvVrcyEb
fWTnRtAQdlirUm/obwXIitoz64VSbIVzcqqfg9C6ZREB9JbEX98/9Wp2gVY+31oC
JfUvYadSYxh3nblvA4OL+iEZiW8NE3hbW6WPXxvS7Euge0uWMPc4uEcnsE0ZVG3m
+TGimzSdeWDvGBRWZHXczC2zD4aoE5vrl+GD2i++c6yjL/otHfYyUpzUfbI2hMAA
5tAF1D5vAAwA8nfPysumlLsIjohJZo4lgnhB++AlOg==
-----END CERTIFICATE-----
)EOF";

static const char serverKey[] PROGMEM =  R"EOF(
-----BEGIN RSA PRIVATE KEY-----
MIIEpQIBAAKCAQEA9UoHBtn4oNKXjRgIOQ/rLxK/iI0a8Q5mDxhfuwa9//FkftSI
IFY8UhGk2YNJpnfKOyYWqbqwuJhIZJ2sEIWp2301OnavuGBrpKOgBJJljgH2l/4Z
uUPLmbXQoPH3QEUpd08wItP6jvo3H0+kK/DXcDzjzAc0bdukGAi37D+8kJfduRGf
zIRgBToZ0xth7u/jjqFfeRjlSaB4kWv3HPWSroDNisy39QeE3M5L5ByQlpjQ4Za+
ZM2yljfEJtsW0gOQJ0FLwJkDD6K2eh++dZYpAReW3qi0+HmePDL47W39D5ns4wNh
BofTYhDHfXzz+RbcXM5jfaScRHW7OOZE76OEDwIDAQABAoIBAQDKov5NFbNFQNR8
djcM1O7Is6dRaqiwLeH4ZH1pZ3d9QnFwKanPdQ5eCj9yhfhJMrr5xEyCqT0nMn7T
yEIGYDXjontfsf8WxWkH2TjvrfWBrHOIOx4LJEvFzyLsYxiMmtZXvy6YByD+Dw2M
q2GH/24rRdI2klkozIOyazluTXU8yOsSGxHr/aOa9/sZISgLmaGOOuKI/3Zqjdhr
eHeSqoQFt3xXa8jw01YubQUDw/4cv9rk2ytTdAoQUimiKtgtjsggpP1LTq4xcuqN
d4jWhTcnorWpbD2cVLxrEbnSR3VuBCJEZv5axg5ZPxLEnlcId8vMtvTRb5nzzszn
geYUWDPhAoGBAPyKVNqqwQl44oIeiuRM2FYenMt4voVaz3ExJX2JysrG0jtCPv+Y
84R6Cv3nfITz3EZDWp5sW3OwoGr77lF7Tv9tD6BptEmgBeuca3SHIdhG2MR+tLyx
/tkIAarxQcTGsZaSqra3gXOJCMz9h2P5dxpdU+0yeMmOEnAqgQ8qtNBfAoGBAPim
RAtnrd0WSlCgqVGYFCvDh1kD5QTNbZc+1PcBHbVV45EmJ2fLXnlDeplIZJdYxmzu
DMOxZBYgfeLY9exje00eZJNSj/csjJQqiRftrbvYY7m5njX1kM5K8x4HlynQTDkg
rtKO0YZJxxmjRTbFGMegh1SLlFLRIMtehNhOgipRAoGBAPnEEpJGCS9GGLfaX0HW
YqwiEK8Il12q57mqgsq7ag7NPwWOymHesxHV5mMh/Dw+NyBi4xAGWRh9mtrUmeqK
iyICik773Gxo0RIqnPgd4jJWN3N3YWeynzulOIkJnSNx5BforOCTc3uCD2s2YB5X
jx1LKoNQxLeLRN8cmpIWicf/AoGBANjRSsZTKwV9WWIDJoHyxav/vPb+8WYFp8lZ
zaRxQbGM6nn4NiZI7OF62N3uhWB/1c7IqTK/bVHqFTuJCrCNcsgld3gLZ2QWYaMV
kCPgaj1BjHw4AmB0+EcajfKilcqtSroJ6MfMJ6IclVOizkjbByeTsE4lxDmPCDSt
/9MKanBxAoGAY9xo741Pn9WUxDyRplww606ccdNf/ksHWNc/Y2B5SPwxxSnIq8nO
j01SmsCUYVFAgZVOTiiycakjYLzxlc6p8BxSVqy6LlJqn95N8OXoQ+bkwUux/ekg
gz5JWYhbD6c38khSzJb0pNXCo3EuYAVa36kDM96k1BtWuhRS10Q1VXk=
-----END RSA PRIVATE KEY-----
)EOF";


const int led = 2;
static String SSE_broadcast_string = "";
static bool SSE_broadcast_flag = false;

static const char TEXT_PLAIN[] PROGMEM = "text/plain";
static const char FS_INIT_ERROR[] PROGMEM = "FS INIT ERROR";
static const char FILE_NOT_FOUND[] PROGMEM = "FileNotFound";

//These are temporary credentials that will only be used if none are found saved in LittleFS.
String login = "admin";
const String realm = "global";
String H1 = "";
String authentication_failed = "User authentication has failed.";
const String file_credentials = R"(/credentials.txt)"; // LittleFS file name for the saved credentials
const String change_creds =  "changecreds";            // Address for a credential change
bool broadcast_serial_print_flag = false;

void SSE_add_char(const char *c) {
  SSE_broadcast_string += c;
}

void set_SSE_broadcast_flag(bool flag) {
  SSE_broadcast_flag = flag;
}

void toggle_broadcast_serial_print_flag(void) {
  broadcast_serial_print_flag = !broadcast_serial_print_flag;
}

void SSEBroadcastTxt(String txt) {
  for (uint8_t i = 0; i < SSE_MAX_CHANNELS; i++) {
    if (!(subscription[i].clientIP)) {
      continue;
    }
    String IPaddrstr = IPAddress(subscription[i].clientIP).toString();
    if (subscription[i].client.connected()) {
      if (broadcast_serial_print_flag) {
        Serial.printf_P(PSTR("broadcast txt to client IP %s on channel %d with string: %s\n"),
                        IPaddrstr.c_str(), i, txt.c_str());
      }
      subscription[i].client.printf_P(PSTR("event: event\ndata: %s\n\n"), txt.c_str());
    }
    //  else {
    //   Serial.printf_P(PSTR("SSEBroadcastState - client %s registered on channel %d but not listening\n"), IPaddrstr.c_str(), i);
    // }
  }
}

////////////////////////////////
// Utils to return HTTP codes, and determine content-type

void replyOK() {
  server.send(200, FPSTR(TEXT_PLAIN), "");
}

void replyOKWithMsg(String msg) {
  server.send(200, FPSTR(TEXT_PLAIN), msg);
}

void replyNotFound(String msg) {
  server.send(404, FPSTR(TEXT_PLAIN), msg);
}

void replyBadRequest(String msg) {
  Serial.println(msg);
  server.send(400, FPSTR(TEXT_PLAIN), msg + "\r\n");
}

void replyServerError(String msg) {
  Serial.println(msg);
  server.send(500, FPSTR(TEXT_PLAIN), msg + "\r\n");
}

void handleNotFound(){
  digitalWrite(led, 1);
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i=0; i<server.args(); i++){
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
  digitalWrite(led, 0);
}

void handleCalib() {
  imu_set_calib_flag(true);
  replyOK();
}

void handleGetMagCalib() {
  imu_send_mag_calib();
  replyOK();
}
/*
   Handle a file deletion request
   Operation      | req.responseText
   ---------------+--------------------------------------------------------------
   Delete file    | parent of deleted file, or remaining ancestor
   Delete folder  | parent of deleted folder, or remaining ancestor
*/
void handleFileDelete() {
  if (!isfsOK()) {
    return replyServerError(FPSTR(FS_INIT_ERROR));
  }

  String path = server.arg(0);
  if (path.isEmpty() || path == "/") {
    return replyBadRequest("BAD PATH");
  }

  Serial.println(String("handleFileDelete: ") + path);
  if (!isFileExist(path.c_str())) {
    return replyNotFound(FPSTR(FILE_NOT_FOUND));
  }
  deleteRecursive(path.c_str());

  server.send(200, FPSTR(TEXT_PLAIN), "deleted: " + path);
}

/*
   Read the given file from the filesystem and stream it back to the client
*/
bool handleFileRead(String path) {
  Serial.println(String("handleFileRead: ") + path);
  if (!isfsOK()) {
    replyServerError(FPSTR(FS_INIT_ERROR));
    return true;
  }

  if (path.endsWith("/")) {
    path += "index.html";
  }

  String contentType;
  if (server.hasArg("download")) {
    contentType = F("application/octet-stream");
  } else {
    contentType = mime::getContentType(path);
  }

  if (!isFileExist(path.c_str())) {
    // File not found, try gzip version
    path = path + ".gz";
  }

  if (isFileExist(path.c_str())) {
    File file = LittleFS.open(path, "r");
    if (server.streamFile(file, contentType) != file.size()) {
      Serial.println("Sent less data than expected!");
    }
    file.close();
    return true;
  }

  return false;
}

//This function redirects home
void redirect(){
  String url = "http://" + WiFi.localIP().toString();
  Serial.println("Redirect called. Redirecting to " + url);
  server.sendHeader("Location", url, true);
  Serial.println("Header sent.");
  server.send( 302, "text/plain", ""); // Empty content inhibits Content-length header so we have to close the socket ourselves.
  Serial.println("Empty page sent.");
  server.client().stop(); // Stop is needed because we sent no content length
  Serial.println("Client stopped.");
}

//This function checks whether the current session has been authenticated. If not, a request for credentials is sent.
bool session_authenticated() {
  return true;
  Serial.println("Checking authentication.");
  if (server.authenticateDigest(login,H1)) {
    Serial.println("Authentication confirmed.");
    return true;
  } else  {
    Serial.println("Not authenticated. Requesting credentials.");
    server.requestAuthentication(DIGEST_AUTH,realm.c_str(),authentication_failed);
    redirect();
    return false;
  }
}

//This function sends a simple webpage for changing login credentials to the client
void showcredentialpage(){
  Serial.println("Show credential page called.");
  if(!session_authenticated()){
    return;
  }

  Serial.println("Forming credential modification page.");

  String page;
  page = R"(<html>)";

  page+=
  R"(
  <h2>Login Credentials</h2><br>
  <form action=")" + change_creds + R"(" method="post">
  Login:<br>
  <input type="text" name="login"><br>
  Password:<br>
  <input type="password" name="password"><br>
  Confirm Password:<br>
  <input type="password" name="password_duplicate"><br>
  <p><button type="submit" name="newcredentials">Change Credentials</button></p>
  </form><br>
  )"
  ;

  page += R"(</html>)";

  Serial.println("Sending credential modification page.");

  server.send(200, "text/html", page);
}

//Saves credentials to LittleFS
void savecredentials(String new_login, String new_password)
{
  //Set global variables to new values
  login=new_login;
  H1=ESP8266WebServer::credentialHash(new_login,realm,new_password);

  //Save new values to LittleFS for loading on next reboot
  Serial.println("Saving credentials.");
  File f=LittleFS.open(file_credentials,"w"); //open as a brand new file, discard old contents
  if(f){
    Serial.println("Modifying credentials in file system.");
    f.println(login);
    f.println(H1);
    Serial.println("Credentials written.");
    f.close();
    Serial.println("File closed.");
  }
  Serial.println("Credentials saved.");
}

//loads credentials from LittleFS
void loadcredentials()
{
  Serial.println("Searching for credentials.");
  File f;
  f=LittleFS.open(file_credentials,"r");
  if(f){
    Serial.println("Loading credentials from file system.");
    String mod=f.readString(); //read the file to a String
    int index_1=mod.indexOf('\n',0); //locate the first line break
    int index_2=mod.indexOf('\n',index_1+1); //locate the second line break
    login=mod.substring(0,index_1-1); //get the first line (excluding the line break)
    H1=mod.substring(index_1+1,index_2-1); //get the second line (excluding the line break)
    f.close();
  } else {
    String default_login = "admin";
    String default_password = "changeme";
    Serial.println("None found. Setting to default credentials.");
    Serial.println("user:" + default_login);
    Serial.println("password:" + default_password);
    login=default_login;
    H1=ESP8266WebServer::credentialHash(default_login,realm,default_password);
    Serial.println("H1:" + H1);
  }
}

//This function handles a credential change from a client.
void handlecredentialchange() {
  Serial.println("Handle credential change called.");
  if(!session_authenticated()){
    return;
  }

  Serial.println("Handling credential change request from client.");

  String login = server.arg("login");
  String pw1 = server.arg("password");
  String pw2 = server.arg("password_duplicate");

  if(login != "" && pw1 != "" && pw1 == pw2){

    savecredentials(login,pw1);
    server.send(200, "text/plain", "Credentials updated");
    redirect();
  } else {
    server.send(200, "text/plain", "Malformed credentials");
    redirect();
  }
}

void handleRoot() {
  if(!session_authenticated()){
    return;
  }
  if (!isfsOK()) {
    return replyServerError(FPSTR(FS_INIT_ERROR));
  }
  String uri = ESP8266WebServer::urlDecode(server.uri()); // required to read paths with blanks
  if (handleFileRead(uri)) {
    return;
  }
}

void SSEKeepAlive() {
  for (uint8_t i = 0; i < SSE_MAX_CHANNELS; i++) {
    if (!(subscription[i].clientIP)) {
      continue;
    }
    if (!subscription[i].client.connected()) {
    //   Serial.printf_P(PSTR("SSEKeepAlive - client is still listening on channel %d\n"), i);
    //   subscription[i].client.println(F("event: event\ndata: { \"TYPE\":\"KEEP-ALIVE\" }\n"));   // Extra newline required by SSE standard
    // } else {
      Serial.printf_P(PSTR("SSEKeepAlive - client not listening on channel %d, remove subscription\n"), i);
      subscription[i].keepAliveTimer.detach();
      subscription[i].client.flush();
      subscription[i].client.stop();
      subscription[i].clientIP = INADDR_NONE;
      subscriptionCount--;
    }
  }
}

// SSEHandler handles the client connection to the event bus (client event listener)
// every 60 seconds it sends a keep alive event via Ticker
void SSEHandler(uint8_t channel) {
  WiFiClient client = server.client();
  SSESubscription &s = subscription[channel];
  if (s.clientIP != client.remoteIP()) { // IP addresses don't match, reject this client
    Serial.printf_P(PSTR("SSEHandler - unregistered client with IP %s tries to listen\n"), server.client().remoteIP().toString().c_str());
    return handleNotFound();
  }
  client.setNoDelay(true);
  client.setSync(true);
  // Serial.printf_P(PSTR("SSEHandler - registered client with IP %s is listening\n"), IPAddress(s.clientIP).toString().c_str());
  s.client = client; // capture SSE server client connection
  server.setContentLength(CONTENT_LENGTH_UNKNOWN); // the payload can go on forever
  server.sendContent_P(PSTR("HTTP/1.1 200 OK\nContent-Type: text/event-stream;\nConnection: keep-alive\nCache-Control: no-cache\nAccess-Control-Allow-Origin: *\n\n"));
  s.keepAliveTimer.attach_scheduled(30.0, SSEKeepAlive);  // Refresh time every 30s for demo
}

void handleAll() {
  if(!session_authenticated()){
    return;
  }
  
  const char *uri = server.uri().c_str();
  const char *restEvents = PSTR("/rest/events/");
  if (!strncmp_P(uri, restEvents, strlen_P(restEvents))) {
    uri += strlen_P(restEvents); // Skip the "/rest/events/" and get to the channel number
    unsigned int channel = atoi(uri);
    if (channel < SSE_MAX_CHANNELS) {
      return SSEHandler(channel);
    }
  }

  Serial.printf_P(PSTR("get uri: %s\r\n"), uri);
  
  if (!isfsOK()) {
    return replyServerError(FPSTR(FS_INIT_ERROR));
  }
  String uriStr = ESP8266WebServer::urlDecode(server.uri()); // required to read paths with blanks
  if (handleFileRead(uriStr)) {
    return;
  }

  handleNotFound();
}

void handleSubscribe() {
  if(!session_authenticated()){
    return;
  }
  if (subscriptionCount == SSE_MAX_CHANNELS - 1) {
    return handleNotFound();  // We ran out of channels
  }

  uint8_t channel;
  IPAddress clientIP = server.client().remoteIP();   // get IP address of client
  String SSEurl = F("http://");
  SSEurl += WiFi.localIP().toString();
  SSEurl += F(":");
  SSEurl += serverPort;
  size_t offset = SSEurl.length();
  SSEurl += F("/rest/events/");

  ++subscriptionCount;
  for (channel = 0; channel < SSE_MAX_CHANNELS; channel++) // Find first free slot
    if (!subscription[channel].clientIP) {
      break;
    }
  subscription[channel] = {clientIP, server.client(), Ticker()};
  SSEurl += channel;
  // Serial.printf_P(PSTR("Allocated channel %d, on uri %s\n"), channel, SSEurl.substring(offset).c_str());
  // server.on(SSEurl.substring(offset), std::bind(SSEHandler, &(subscription[channel])));
  // Serial.printf_P(PSTR("subscription for client IP %s: event bus location: %s\n"), clientIP.toString().c_str(), SSEurl.c_str());
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send_P(200, "text/plain", SSEurl.c_str());
}


void web_setup() {
  loadcredentials();

  time_t now = time(nullptr);
  String logtimeStr = "Server Started at: ";
  logtimeStr +=  ctime(&now);
  Serial.println(logtimeStr);
  appendFile(R"(/log.txt)", logtimeStr.c_str());
  // server.getServer().setRSACert(new BearSSL::X509List(serverCert), new BearSSL::PrivateKey(serverKey));

  server.on("/", handleRoot);
  server.on("/creds",showcredentialpage); //for this simple example, just show a simple page for changing credentials at the root
  server.on("/creds" + change_creds,handlecredentialchange); //handles submission of credentials from the client
  server.on(F("/rest/events/subscribe"), handleSubscribe);
  server.on("/delete", handleFileDelete);
  server.on("/calib", handleCalib);
  server.on("/get/magcalib", handleGetMagCalib);
  server.onNotFound(handleAll);

  server.begin();
  Serial.println("HTTPS server and  SSE EventSource started");
}

void web_loop(void){
  server.handleClient();

  if(SSE_broadcast_string.length() > 0 && SSE_broadcast_flag) {
    SSEBroadcastTxt(SSE_broadcast_string);
    SSE_broadcast_flag = false;
    SSE_broadcast_string = "";
  }
}
