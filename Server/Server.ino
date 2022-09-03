#include <ESPmDNS.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <Preferences.h>

#include "index.h"

//#define FACTORYDEFAULTS

String DeviceName;

String STAssid;
String STApasswd;
String sAPssid;
String sAPpass;
String SERVERIP;
String ALMTIME;
String HTTPuser;
String HTTPpass;

String settingsJSON;
String redirectResponse = "";

const int btnPin = 23;

unsigned long previousMillis = 0;
long interval = 5000; 

int bollenbakstatus = 0;

const char* PARAM_INPUT_SSID = "SSID";
const char* PARAM_INPUT_PASSWD = "PASSWD";
const char* PARAM_INPUT_ALMTIME = "ALMTIME";

AsyncWebServer server(80);

Preferences preferences;


String GenerateDeviceName(const char *name){
  String MACAddress;  
  MACAddress = WiFi.macAddress();

  return name + MACAddress.substring(9,11) + MACAddress.substring(12,14) + MACAddress.substring(15,17);
}

String GetDeviceSettings(){
  String returnString = "";

  preferences.begin("settings", false);
  sAPssid = preferences.getString("sAPssid", "");
  sAPpass = preferences.getString("sAPpass", "");
  STAssid = preferences.getString("STAssid", "");
  STApasswd = preferences.getString("STApass", "");
  ALMTIME = preferences.getString("ALMTIME", "");
  HTTPuser = preferences.getString("HTTPuser", "");
  HTTPpass = preferences.getString("HTTPpass", "");  
  preferences.end();
     
  returnString = "{\"STAssid\": \"" + STAssid + "\"," + 
                  "\"STApasswd\": \"" + STApasswd + "\"," + 
                  "\"ALMTIME\": \"" + ALMTIME + "\"}";

  return returnString;
}

String StrReadBollenBak(){

  if(bollenbakstatus == LOW) {      
      Serial.println("Bollenbak LEEG");
      return "LEEG";
  }
  else {
      Serial.println("Bollenbak VOL");
      return "VOL";
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("ESP Booting...");  
  Serial.println("Loading preferences.");
#if defined(FACTORYDEFAULTS)
#pragma message "LOADING FACTORY DEFAULTS"
  preferences.begin("settings", false);
  preferences.putString("sAPssid", GenerateDeviceName("esp"));                     //SoftAP SSID
  preferences.putString("sAPpass", "config98765");                  //SoftAP Default Password
  preferences.putString("STAssid", "REPLACE WITH YOUR SSID");       //SSID to connect to
  preferences.putString("STApass", "REPLACE WITH YOUR PASSWORD");   //Pass
  preferences.putString("SERVERIP", "192.168.1.129");
  preferences.putString("ALMTIME", "15");
  preferences.putString("HTTPuser", "admin");
  preferences.putString("HTTPpass", "admin");
  preferences.end();
#endif
  pinMode(btnPin, INPUT);
  
  settingsJSON = GetDeviceSettings();
  
  DeviceName = sAPssid;
  interval = 1000 * ALMTIME.toInt();

  Serial.print("JSON: ");
  Serial.println(settingsJSON);

  Serial.print("Soft AP SSID: ");
  Serial.println(sAPssid.c_str());

  Serial.print("Soft AP Password : ");
  Serial.println(sAPpass);

  Serial.print("Station SSID: ");
  Serial.println(STAssid);

  Serial.print("Station Password: ");
  Serial.println(STApasswd);
  
  Serial.print("ALARM TIME: ");
  Serial.println(ALMTIME);

  Serial.print("Interval: ");
  Serial.println(interval);

  Serial.print("HTTP user: ");
  Serial.println(HTTPuser);

  Serial.print("HTTP pass: ");
  Serial.println(HTTPpass);

  if (sAPssid != "" && sAPpass != "") {
    WiFi.mode(WIFI_AP_STA);
    WiFi.softAP(sAPssid.c_str(), sAPpass.c_str());

    Serial.print("sAP IP: ");
    Serial.println(WiFi.softAPIP());
  }  

  //Starting mDNS
  Serial.println("Starting mDNS.");
  if(!MDNS.begin(DeviceName.c_str())) {
    Serial.println("Error starting mDNS");
    return;
  }

  Serial.println("mDNS started!");
  Serial.print("mDNS: ");
  Serial.print("http://");
  Serial.print(DeviceName);
  Serial.println(".local");

  //Async Callbacks for webserver
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    if(!request->authenticate(HTTPuser.c_str(), HTTPpass.c_str())) return request->requestAuthentication();
    settingsJSON = GetDeviceSettings();
    //request->send_P(200, "text/html", index_html);
    String idx = index_html;
    String repidx = idx;
    repidx.replace("{{OWNIP}}", WiFi.localIP().toString());
    request->send_P(200, "text/html", repidx.c_str());
  });

  server.on("/bollenbakstatus", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", StrReadBollenBak().c_str());
  });

  server.on("/settings", HTTP_GET, [](AsyncWebServerRequest *request){    
    request->send_P(200, "application/json", settingsJSON.c_str());
  });

  server.on("/reboot", HTTP_GET, [](AsyncWebServerRequest *request){
    redirectResponse = "<!DOCTYPE html><html><head></head><body><script>window.location.replace(\"http://" + WiFi.localIP().toString() + "\");</script></body></html>";
    request->send(200, "text/html", redirectResponse);    
    
    ESP.restart();
  });

  server.on("/get", HTTP_GET, [] (AsyncWebServerRequest *request){
    String HTMLGET_INPUT_STA_SSID;
    String HTMLGET_INPUT_STA_PASSWD;
    String HTMLGET_INPUT_ALMTIME;

    if(request->hasParam(PARAM_INPUT_SSID)) {
      HTMLGET_INPUT_STA_SSID = request->getParam(PARAM_INPUT_SSID)->value();      
      Serial.print("STA_SSID: ");
      Serial.println(HTMLGET_INPUT_STA_SSID);
      preferences.begin("settings", false);
      preferences.putString("STAssid", HTMLGET_INPUT_STA_SSID);
      Serial.println("Netwerk SSID opgeslagen");
      preferences.end();
    }
    
    if(request->hasParam(PARAM_INPUT_PASSWD)) {
      HTMLGET_INPUT_STA_PASSWD = request->getParam(PARAM_INPUT_PASSWD)->value();      
      Serial.print("PASSWD: ");
      Serial.println(HTMLGET_INPUT_STA_PASSWD);
      preferences.begin("settings", false);
      preferences.putString("STApass", HTMLGET_INPUT_STA_PASSWD);
      Serial.println("Netwerk wachtwoord opgeslagen");
      preferences.end();
    }

    if(request->hasParam(PARAM_INPUT_ALMTIME)) {
      HTMLGET_INPUT_ALMTIME = request->getParam(PARAM_INPUT_ALMTIME)->value();      
      Serial.print("Alarmtijd: ");
      Serial.println(HTMLGET_INPUT_ALMTIME);
      preferences.begin("settings", false);
      preferences.putString("ALMTIME", HTMLGET_INPUT_ALMTIME);
      Serial.println("Alarmtijd opgeslagen");
      preferences.end();
    }
  });

  server.begin();
  Serial.println("Server started.");

  if(STAssid != "REPLACE WITH YOUR SSID"){
    WiFi.begin(STAssid.c_str(), STApasswd.c_str());
    while (WiFi.status() != WL_CONNECTED) {
      Serial.println("Connecting to WiFi..");
      delay(1000);    
    }
    Serial.println(WiFi.localIP());
  }  
}

void loop() {
  unsigned long currentMillis = millis();

  if(currentMillis - previousMillis >= interval) {

    bollenbakstatus = digitalRead(btnPin);

    Serial.println(bollenbakstatus);   
    
    previousMillis = currentMillis;
  }
}
