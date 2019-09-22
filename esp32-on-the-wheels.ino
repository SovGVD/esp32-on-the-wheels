//#include <Arduino.h>
#include <analogWrite.h>
#include "WiFi.h"
#include "ESPAsyncWebServer.h"
#include "config.h"
#include "public_html.h"
 
IPAddress broadCast;
String ip = "0.0.0.0";

// define pins
int leftMotor1 = 27;
int leftMotor2 = 26;
int rightMotor1 = 25;
int rightMotor2 = 33;

// define motor speed
// motor output speed
int leftMotorS = 0;
int rightMotorS = 0;

// motor speed
int leftMotorValue = 0;
int rightMotorValue = 0;

int incSpeed = 1;
int incEveryCycle = 2;
int watchDog = 0; // some value that increment every loop cycle
bool canUpdate = false;
int speedL = 250;  // 0 to 255
int speedR = 250;  // 0 to 255

bool clientOnline = false; // expected only one client!!!
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

void onWsEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len){
 
  if(type == WS_EVT_CONNECT){ 
    client->text("Ok");
    clientOnline = true;
  } else if (clientOnline && type == WS_EVT_DATA) {
    if (data[0] == 70) {
      // move forward
      leftMotorValue  = speedL * -1;
      rightMotorValue = speedR * -1;
    } else if (data[0] == 66) {
      // move backward
      leftMotorValue  = speedL;
      rightMotorValue = speedR;
    } else {
      leftMotorS      = 0;
      rightMotorS     = 0;
      leftMotorValue  = 0;
      rightMotorValue = 0;
    }
  } else if(type == WS_EVT_DISCONNECT){
    clientOnline = false;
  }
}
 
void setup(){
  Serial.begin(115200);
 
  WiFi.begin(ssid, password);
 
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
  broadCast = WiFi.localIP();
  Serial.println(broadCast);
  ip = String(broadCast[0]) + String(".") + String(broadCast[1]) + String(".") + String(broadCast[2]) + String(".") + String(broadCast[3]);
 
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/html", index_html);
  });
  server.on("/w.js", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "application/x-javascript", scripts_ws_js);
  });
  // Dinamic config
  server.on("/c.js", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "application/x-javascript", "var c={w:'ws://" + ip + "/ws'}");
  });

  ws.onEvent(onWsEvent);
  server.addHandler(&ws);
 
  server.begin();
}
 
void loop(){
  watchDog++;
  if (watchDog > incEveryCycle) {
    watchDog = 0;
    canUpdate = true;
  }
  
  if (!clientOnline) {
    leftMotorS      = 0;
    rightMotorS     = 0;
    leftMotorValue  = 0;
    rightMotorValue = 0;
  }

  if (canUpdate) {
    // update motors speed with increment
    // this should reduce over power consumption issues
    if (leftMotorValue != leftMotorS) {
      Serial.print("L:");
      Serial.print(leftMotorValue);
      if (leftMotorValue > leftMotorS) {
        leftMotorS = leftMotorS + incSpeed;
      } else {
        leftMotorS = leftMotorS - incSpeed;
      }
      Serial.print(" -> ");
      Serial.println(leftMotorS);
    }
    
    if (rightMotorValue != rightMotorS) {
      Serial.print("R:");
      Serial.print(rightMotorValue);
      if (rightMotorValue > rightMotorS) {
        rightMotorS = rightMotorS + incSpeed;
      } else {
        rightMotorS = rightMotorS - incSpeed;
      }
      Serial.print(" -> ");
      Serial.println(rightMotorS);
    }
    canUpdate = false;
  }
    
  if (leftMotorS >= 0) {
    analogWrite(leftMotor1, leftMotorS);
    analogWrite(leftMotor2, 0);
  } else {
    analogWrite(leftMotor1, 0);
    analogWrite(leftMotor2, leftMotorS);
  }

  if (rightMotorS >= 0) {
    analogWrite(rightMotor1, rightMotorS);
    analogWrite(rightMotor2, 0);
  } else {
    analogWrite(rightMotor1, 0);
    analogWrite(rightMotor2, rightMotorS);
  }

}
