#include <Wire.h>
#include <LRTC.h>
#include <LiquidCrystal_I2C.h>

// FOR MCS
#include <LWiFi.h>
#include <WiFiClient.h>
#include "MCS.h"
#include <LRemote.h>

// FOR MCS
LRemoteButton alarm;
LRemoteLabel label_alarm;
LRemoteSlider slider_minute;
LRemoteSlider slider_hour;
LRemoteCircleButton determine;
LRemoteCircleButton cancel;
LRemoteCircleButton hour_plus;
LRemoteCircleButton hour_minus;
LRemoteCircleButton minute_plus;
LRemoteCircleButton minute_minus;

int alarm_hour = 24;
int alarm_minute = 60;

int set_alarm = 0;

int count_off = 0;
int count_off_2 = 0;
int count_on = 0;

int count_reset = 0;
bool alarm_again = 0;
bool judge = 0;

// Assign AP ssid / password here
#define _SSID "MakeNTU2019-B-2.4G"
#define _KEY  "lazy_tech"

#define ALARM 10
#define FSR_RECEIVE 11

// Assign device id / key of your test device
MCSDevice mcs("D1cXN0Cg", "Uc6VrG740S2Fdkp9");

// Assign channel id 
MCSDisplayInteger Alarm_hour_display("Alarm_hour_display");
MCSDisplayInteger Alarm_minute_display("Alarm_min_display");
MCSDisplayOnOff FSR_display("FSR_display");
MCSDisplayOnOff Oversleep_Alert("Oversleep_Alert");

LiquidCrystal_I2C lcd(0x27);
void setup() {
  // Start the I2C interface
  
  pinMode(2, INPUT);   // control the switch
  pinMode(ALARM, OUTPUT);
  pinMode(FSR_RECEIVE,INPUT);
  Wire.begin();
  LRTC.begin();
  LRTC.set(2019, 3, 31, 12, 29, 10);
  // Start the serial interface
  Serial.begin(115200);
  lcd.begin(16, 2);

  // 閃爍三次
  for(int i = 0; i < 3; i++) {
    lcd.backlight(); // 開啟背光
    delay(250);
    lcd.noBacklight(); // 關閉背光
    delay(250);
  }
  lcd.backlight();

  // 輸出初始化文字
  lcd.setCursor(0, 0); // 設定游標位置在第一行行首
  lcd.print("2019 MakeNTU");
  delay(1000);
  lcd.setCursor(0, 1); // 設定游標位置在第二行行首
  lcd.print("Come on, Maker!");
  

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
  mcs.addChannel(Alarm_hour_display);
  mcs.addChannel(Alarm_minute_display);
  mcs.addChannel(FSR_display);
  mcs.addChannel(Oversleep_Alert);
  
  while(!mcs.connected())
  {
    Serial.println("MCS.connect()...");
    mcs.connect();
  }
  Serial.println("MCS connected !!");

  Alarm_hour_display.set(alarm_hour);
  Alarm_minute_display.set(alarm_minute);
  delay(30); 
  
  Serial.println("Start configuring remote");

  // Initialize GPIO
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, 0);

  // Setup the Remote Control's UI canvas
  LRemote.setName("Alarm_05");
  LRemote.setOrientation(RC_PORTRAIT);
  LRemote.setGrid(3, 5);

  // Add a button
  alarm.setText(" 鬧鐘 ");
  alarm.setPos(0, 0);
  alarm.setSize(1, 1);
  alarm.setColor(RC_PINK);
  LRemote.addControl(alarm);

  // Add a label
  label_alarm.setText("  無  ");
  label_alarm.setPos(1, 0);
  label_alarm.setSize(2, 1);
  label_alarm.setColor(RC_PINK);
  LRemote.addControl(label_alarm);

  // Add a round button
  hour_plus.setText(" 時 + ");
  hour_plus.setPos(0, 1);
  hour_plus.setSize(1, 1);
  hour_plus.setColor(RC_ORANGE);
  LRemote.addControl(hour_plus);

  hour_minus.setText(" 時 - ");
  hour_minus.setPos(0, 2);
  hour_minus.setSize(1, 1);
  hour_minus.setColor(RC_ORANGE);
  LRemote.addControl(hour_minus);

  minute_plus.setText(" 分 + ");
  minute_plus.setPos(1, 1);
  minute_plus.setSize(1, 1);
  minute_plus.setColor(RC_ORANGE);
  LRemote.addControl(minute_plus);

  minute_minus.setText(" 分 - ");
  minute_minus.setPos(1, 2);
  minute_minus.setSize(1, 1);
  minute_minus.setColor(RC_ORANGE);
  LRemote.addControl(minute_minus);

  determine.setText(" 確定 ");
  determine.setPos(2, 1);
  determine.setSize(1, 1);
  determine.setColor(RC_BLUE);
  LRemote.addControl(determine);

  cancel.setText(" 取消 ");
  cancel.setPos(2, 2);
  cancel.setSize(1, 1);
  cancel.setColor(RC_BLUE);
  LRemote.addControl(cancel);

  // Add a slider
  slider_hour.setText(" 時 (0 ~ 23)");
  slider_hour.setPos(0, 3);
  slider_hour.setSize(3, 1);
  slider_hour.setColor(RC_GREEN);
  slider_hour.setValueRange(0, 23, 0);
  LRemote.addControl(slider_hour);

  slider_minute.setText(" 分 (0 ~ 59)");
  slider_minute.setPos(0, 4);
  slider_minute.setSize(3, 1);
  slider_minute.setColor(RC_GREEN);
  slider_minute.setValueRange(0, 59, 0);
  LRemote.addControl(slider_minute);

  // Start broadcasting our remote contoller
  // This method implicitly initialized underlying BLE subsystem
  // to create a BLE peripheral, and then start advertisement on it.
  LRemote.begin();
  Serial.println("begin() returned");

  Oversleep_Alert.set(0);
  delay(8000);
  lcd.clear();
}

//int hour = 24;
//int minute = 60;

void loop() 
{
  LRTC.get();
  char buffer[64];
  Oversleep_Alert.set(0);

  // check if we are connect by some BLE central device, e.g. an mobile app
  if(!LRemote.connected()) {
    Serial.println("waiting for connection");
    delay(100);
  } else {
    // The interval between button down/up an be very short - e.g. a quick tap on the screen.
    // We could lose some event if we delay something like 100ms.
    delay(15);
  }
  
  // Process the incoming BLE write request and translate them to control events
  LRemote.process();

  // Now we poll each control's status
  if(alarm.getValue() == 1){
    //hour = alarm_hour;
    //minute = alarm_minute;
    set_alarm = 1;
    Serial.println("set alarm");
    delay(15);
  }
  
  if(set_alarm > 0)
  {
    Serial.println(String(alarm_hour) + "：" + String(alarm_minute));
    Serial.println("set_alarm");
    LRemote.process();
    if(hour_plus.isValueChanged())
    {
      Serial.println(hour_plus.getValue());
      if (hour_plus.getValue() == 1)
      {
        if(alarm_hour < 23) {alarm_hour += 1;}
        else {alarm_hour = 0;}
        if(alarm_minute > 59) {alarm_minute = 0;}
        Serial.println("hour plus");
      }
      delay(15);
    }
    LRemote.process();
    if(hour_minus.isValueChanged() and hour_minus.getValue() == 1)
    {
      if(alarm_hour > 0 and alarm_hour < 24) {alarm_hour -= 1;}
      else {alarm_hour = 0;}
      if(alarm_minute > 59) {alarm_minute = 0;}
      Serial.println("hour minus");
      delay(15);
    }
    LRemote.process();
    if(minute_plus.isValueChanged() and minute_plus.getValue() == 1)
    {
      if(alarm_minute < 59) {alarm_minute += 1;}
      else {alarm_minute = 0;}
      if(alarm_hour > 23) {alarm_hour = 0;}
      Serial.println("minute plus");
      delay(15);
    }
    LRemote.process();

    if(!LRemote.connected()) {
      Serial.println("waiting for connection");
      delay(100);
    } 
    else {delay(15);}
    
    if(minute_minus.isValueChanged() and minute_minus.getValue() == 1)
    {
      if(alarm_minute > 0 and alarm_minute < 60) {alarm_minute -= 1;}
      else {alarm_minute = 0;}
      if(alarm_hour > 23) {alarm_hour = 0;}
      Serial.println("minute minus");
      delay(15);
    }
    LRemote.process();
    if(slider_hour.isValueChanged())
    {
      alarm_hour = slider_hour.getValue();
      if(alarm_minute > 59) {alarm_minute = 0;}
      Serial.println("slider hour");
      delay(15);
    }
    LRemote.process();
    if(slider_minute.isValueChanged())
    {
      alarm_minute = slider_minute.getValue();
      if(alarm_hour > 23) {alarm_hour = 0;}
      Serial.println("slider minute");
      delay(15);
    }
    
    //if(alarm_hour < 24) {label_alarm.updateText(String(alarm_hour) + "：" + String(alarm_minute));}
    //else {label_alarm.updateText("  無  ");}

    if(!LRemote.connected()) {
      Serial.println("waiting for connection");
      delay(100);
    } 
    else {delay(15);}
    
    LRemote.process();
    if(determine.isValueChanged() and determine.getValue() == 1) 
    {
      //alarm_hour = hour;
      //alarm_minute = minute;
      set_alarm = 0;
      Serial.println("determine");
      delay(15);
    }
    LRemote.process();
    if(cancel.isValueChanged() and cancel.getValue() == 1) 
    {
      alarm_hour = 24;
      alarm_minute = 60;
      set_alarm = 0;
      Serial.println("cancel");
      delay(15);
    }
  }

  if(alarm_hour < 24) 
  {
    label_alarm.updateText(String(alarm_hour) + "：" + String(alarm_minute));
    Serial.println(" 鬧鐘：" + String(alarm_hour) + "：" + String(alarm_minute));
  }
  else 
  {
    label_alarm.updateText("  無  ");
    Serial.println(" 鬧鐘：無 ");
  }
  
  // check if need to re-connect
  while(!mcs.connected())
  {
    Serial.println("re-connect to MCS...");
    mcs.connect();
    if(mcs.connected())
      Serial.println("MCS connected !!");
  }
  Alarm_hour_display.set(alarm_hour);
  Alarm_minute_display.set(alarm_minute);

  lcd.clear();
  lcd.setCursor(0, 0); // 設定游標位置在第一行行首
  
  if (digitalRead(2)==LOW) {
    lcd.print("Time:");
    lcd.setCursor(5, 0);
  
    sprintf(buffer, "%.2ld:%.2ld:%.2ld", LRTC.hour(), LRTC.minute(), LRTC.second());
    lcd.print(buffer);

    lcd.setCursor(3, 1);
    sprintf(buffer, "%.4ld/%.2ld/%.2ld", LRTC.year(), LRTC.month(), LRTC.day());
    lcd.print(buffer);
  }
  else 
  {
    lcd.print("Alarm:");
    lcd.setCursor(6, 0);
    if(alarm_hour != 24 && alarm_minute != 60)
    {
      if(alarm_hour < 10){
        lcd.print("0");
        lcd.setCursor(7, 0);
        lcd.print(alarm_hour);
      }
      else{
        lcd.print(alarm_hour);
      }
      lcd.setCursor(8, 0);
      lcd.print(":");
      lcd.setCursor(9, 0);
      if(alarm_minute < 10){
        lcd.print("0");
        lcd.setCursor(10, 0);
        lcd.print(alarm_minute);
      }
      else{
        lcd.print(alarm_minute);
      }
    }
    lcd.setCursor(0, 1);
    lcd.print("Have a good night !");

      
   }
    // consider whether the clock to alarm or not
    if(((LRTC.hour() ==  alarm_hour) && (LRTC.minute() == alarm_minute) && (count_off != 11)) || (digitalRead(ALARM)==1))
    {
      digitalWrite(ALARM, HIGH);
      Serial.println("******************************ALARM VALUE");
      Serial.println(digitalRead(ALARM));
      if(digitalRead(FSR_RECEIVE)==1){
        count_off = 0;
        count_on = count_on + 1;
      }
      else{
        count_on = 0;
        if(count_off == 10){
          digitalWrite(ALARM, LOW);
          Serial.println("******************************clock turns off******************************");
        }
        count_off = count_off + 1;
      }
    }
    if(((LRTC.hour() !=  alarm_hour) || (LRTC.minute() != alarm_minute)) && (count_on >= 40)){    // count_on >= 40
        alarm_again = 1;
        Oversleep_Alert.set(alarm_again);
        count_on = 0;
    }
    if(((LRTC.hour() !=  alarm_hour) || (LRTC.minute() != alarm_minute)) && (count_off == 11)){
        count_off = 0;
        judge = 1;
    }
    Serial.println("******************************ALARM VALUE");
      Serial.println(digitalRead(ALARM));
    Serial.println("******************************Count off");
    Serial.println(count_off);
    Serial.println("******************************FSR VALUE");
    Serial.println(digitalRead(FSR_RECEIVE));
    if((count_off==11 || judge==1) && digitalRead(FSR_RECEIVE)==1)
    {
      Serial.println("******************************Count off 2");
      Serial.println(count_off_2);
      if(count_off_2==15)         //count_off_2==60
      {
        alarm_again = 1;
        digitalWrite(ALARM, HIGH);
        Oversleep_Alert.set(alarm_again);
      }
      count_off_2 = count_off_2 + 1;
    }
    else {count_off_2 = 0;}
    if(judge==1){
      count_reset = count_reset + 1;
      if(count_reset == 60){        //count_reset == 3600
        judge = 0;
        count_reset = 0;
        alarm_again = 0;
        count_off = 0;
        count_off_2 = 0;
        count_on = 0;
      }
    }
  
  delay(1000);
}
