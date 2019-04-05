#include <LWiFi.h>
#include <WiFiClient.h>
#include "MCS.h"

// Assign AP ssid / password here
#define _SSID "MakeNTU2019-B-2.4G"
#define _KEY  "lazy_tech"
#define FSR_TRANS 13

// Assign device id / key of your test device
MCSDevice mcs("D1cXN0Cg", "Uc6VrG740S2Fdkp9");

// Assign channel id 
MCSDisplayOnOff FSR("FSR_display");
MCSControllerInteger hour("FSR_switch");

#define FSR_PIN A0

void setup() { 
  // setup Serial output at 9600
  pinMode(FSR_TRANS,OUTPUT);
  Serial.begin(9600);
  
  // setup Wifi connection
  while(WL_CONNECTED != WiFi.status())
  {
    Serial.print("WiFi.begin(");
    Serial.print(_SSID);
    Serial.print(",");
    Serial.print(_KEY);
    Serial.println(")...");
    WiFi.begin(_SSID, _KEY);
  }
  Serial.println("WiFi connected !!");

  // setup MCS connection
  mcs.addChannel(FSR);
  mcs.addChannel(hour);
  while(!mcs.connected())
  {
    Serial.println("MCS.connect()...");
    mcs.connect();
  }
  Serial.println("MCS connected !!");
  
} 
 
void loop() { 
  int fsr_value = analogRead(FSR_PIN); // 讀取 FSR
   
  int FSR_switch;
  if (fsr_value > 2047) { FSR_switch = HIGH; }
  else { FSR_switch = LOW; }
  FSR.set(FSR_switch);
  digitalWrite(FSR_TRANS,FSR_switch);
  Serial.print("Output: "); 
  // LinkIt 7697 ADC spec: 12-bit, 0~2.5V range 
  Serial.println(fsr_value); 
  Serial.print("FSR value is: ");
  Serial.println(digitalRead(FSR_TRANS));
  Serial.println(hour.value());
 
  delay(100); 
}
