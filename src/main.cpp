#include <ESP8266WiFi.h>         

#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>   
#include <EEPROM.h>

#include <WiFiClient.h>
#include <ESP8266HTTPClient.h>

#include <ArduinoOTA.h>

#define ENCODER_USE_INTERRUPTS
#include <Encoder.h>
#define ENCODER_A D3
#define ENCODER_B D4

#define JUMPPERCLICK 10

#define BUTTON D7
#define DEPRELL 3

String urlbase;
String urlauth;
WiFiManager wifiManager;
WiFiManagerParameter radioIpManager("radioip", "IP/host of radio", "192.168.1.250", 100);
char radioIp[100];

Encoder encoder(ENCODER_A, ENCODER_B);

String xmlTakeParam(String inStr,String needParam)
{
  if(inStr.indexOf("<"+needParam+">")>0){
     int CountChar=needParam.length();
     int indexStart=inStr.indexOf("<"+needParam+">");
     int indexStop= inStr.indexOf("</"+needParam+">");  
     return inStr.substring(indexStart+CountChar+2, indexStop);
  }
  return "not found";
}

String connect(String command, String parameter, String value){
  String url = urlbase+command;
  if ( command == "SET" ){
    url += "/"+parameter+urlauth+"&value="+value;
  } else if ( command == "GET" ){
    url += "/"+parameter+urlauth;
  }
  String payload = "";
  WiFiClient wclient;
  HTTPClient http;
  http.begin(wclient, url);
  int httpCode = http.GET();
  // httpCode will be negative on error
  if( httpCode == HTTP_CODE_OK) {
    payload = http.getString();
  } 
  http.end();
  return payload;
}

void getSessionId(){
  urlauth = "?pin=1234";
  //it looks like we don't need this anymore - it always returns with internal server error
  //however, the control works with just the pin
  String payload = connect("CREATE_SESSION", "", "");
  if (payload.length() > 0){
    if ( xmlTakeParam(payload, "status") == "FS_OK" ) {
      urlauth =  "?pin=1234&sid=" +  xmlTakeParam(payload, "sessionId");
    } 
  } 
}

long getLength(){
  long llength= 0;
  String payload = connect("GET", "netRemote.play.info.duration", "");
  if (payload.length() > 0){
    if ( xmlTakeParam(payload, "status") == "FS_OK" ) {
      String slength = xmlTakeParam(payload, "u32");
      llength = slength.toInt();
    }
  }
  return llength;
}

long getPos(){
  long lpos= 0;
  String payload = connect("GET", "netRemote.play.position", "");
  if (payload.length() > 0){
    if ( xmlTakeParam(payload, "status") == "FS_OK" ) {
      String spos = xmlTakeParam(payload, "u32");
      lpos = spos.toInt();
    }
  }
  return lpos;
}

long setPos(long newpos){
  long lpos= 0;
  String payload = connect("SET", "netRemote.play.position", String(newpos));
  if (payload.length() > 0){
    if ( xmlTakeParam(payload, "status") == "FS_OK" ) {
      String spos = xmlTakeParam(payload, "u32");
      lpos = spos.toInt();
    }
  }
  return lpos;
}

long encoderChange = 0;
long pressLength = 0;


long oldPosition  = 0;
unsigned long pressedAt = 0;
bool isPressed = false;

void updateInput(){
  long newPosition = encoder.read();
  if (newPosition != oldPosition) {
    encoderChange = newPosition - oldPosition;
    oldPosition = newPosition;
  }
  
  bool isPressedNew = digitalRead(BUTTON)?false:true;
  if( isPressed!=isPressedNew && (millis()-pressedAt)>DEPRELL ){
    isPressed = isPressedNew;
    if( isPressed ) {
      pressedAt = millis();
    } else {
      pressLength = millis() - pressedAt;
    }
  }
}

void reconnect(){
  urlbase = "http://" + String(radioIp) + "/fsapi/";

  Serial.println("Using url " + urlbase);
  getSessionId();
  Serial.println("Using auth " + urlauth);
}

void saveConfigCallback(){
  strncpy(radioIp, radioIpManager.getValue(), sizeof(radioIp));
  Serial.println("Save IP " + String(radioIp));
  EEPROM.put(0, radioIp);
  EEPROM.commit();
}

void setup() {
  pinMode(BUTTON, INPUT_PULLUP);
  EEPROM.begin(sizeof(radioIp));
  Serial.begin(115200);
  Serial.println("booted");
  wifiManager.addParameter(&radioIpManager);
  wifiManager.setSaveConfigCallback(saveConfigCallback);
  wifiManager.autoConnect("AutoConnectAP");
  EEPROM.get(0, radioIp);
  EEPROM.end();
  Serial.println("IP loaded: " + String(radioIp));

  ArduinoOTA.setHostname ("Hama3100Control");
  ArduinoOTA.begin();
  urlauth = "";
}

void loop() {
  ArduinoOTA.handle();
  updateInput();
  if( urlauth.equals("") || pressLength > 0){
    reconnect();
  }
  if ( encoderChange ) {
      long len = getLength();
      long pos = getPos();
      long newpos = pos + encoderChange*250*JUMPPERCLICK;
      if ( newpos < 0 ){
        newpos = 0;
      } else if ( newpos > len ) {
        newpos = len;
      }
      Serial.println("LEN:"+String(len)+"POS:"+String(pos)+"NPOS"+String(newpos));
      setPos(newpos);
  }

  encoderChange = pressLength = 0;
  delay(100);
}
