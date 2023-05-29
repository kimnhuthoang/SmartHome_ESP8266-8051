
//Được viết bới Kim Nhựt Hoàng - 2002190238 - 10DHDT1 - Trường Đại Học CNTP TP.HCM//
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <EEPROM.h>
#include <ArduinoJson.h>
#include <UniversalTelegramBot.h>
#include <DHT.h>
#include "SmartConfigPage.h"

ESP8266WebServer webServer(80);
const char* ssid_ap = "ESP_SmartConfig";
const char* pass_ap = "88888888";

// Initialize Telegram BOT
#define BOTtoken "5533976849:AAHTQm6IDro03WF8WpAyxUSvqUusR1HFD_Q" 
#define CHAT_ID "1338702332"

X509List cert(TELEGRAM_CERTIFICATE_ROOT);
WiFiClientSecure telegram;
UniversalTelegramBot bot(BOTtoken, telegram);

String ssid,pass;
unsigned long led_time = 0;
unsigned long time_checkconnect = 0;
unsigned long TimeSend_TempHum = 0;
unsigned int time_toggle = 2000;
bool config_state = false;
bool flag_led = false;

// Thông tin về MQTT Broker
#define MQTT_Server "broker.hivemq.com"   // Thay bằng thông tin của bạn
#define MQTT_User "nhuthoang07"           //Thay bằng user va pass của bạn
#define MQTT_Pass "hoang123"
#define Client_id "ESP8266_nhuthoang0159HuFi"
const uint16_t MQTT_Port = 1883;          //Port của CloudMQTT

WiFiClient espClient;
PubSubClient client(espClient);

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP,"2.vn.pool.ntp.org",7*3600);

DHT dht(2,DHT11);

#define EEP_Checked_Start 200
#define EEP_Checked_Stop  201

#define EEP_hour_start 202
#define EEP_min_start  203
#define EEP_hour_stop  204
#define EEP_min_stop   205

#define EEP_StateTime_D1 206
#define EEP_StateTime_D2 207
#define EEP_StateTime_D3 208
#define EEP_StateTime_Q1 209
#define EEP_StateTime_Q2 210
#define EEP_StateTime_Q3 211

#define EEP_CheckedTemp_ON  212
#define EEP_CheckedTemp_OFF 213
#define EEP_Temp_ON  214
#define EEP_Temp_OFF 215

#define EEP_StateTemp_Q1 216
#define EEP_StateTemp_Q2 217
#define EEP_StateTemp_Q3 218

unsigned long time_delayConnect = 0, Wait_ONOFF = 0, Wait_ONOFF1 = 0;
byte tt_d1,tt_d2,tt_d3,tt_q1,tt_q2,tt_q3;
byte check_startTime,check_stopTime,check_OnTemp,check_OffTemp;
byte Hour_Start, Min_Start, Hour_Stop, Min_Stop, TempON, TempOFF;
bool TimeOn_done = true, TimeOff_done = true,TempOn_done = true, TempOff_done = true;
String inputString = "";         // a String to hold incoming data
bool stringComplete = false;  // whether the string is complete

void setup() 
{
  pinMode(0,INPUT_PULLUP);
//  pinMode(LED_BUILTIN, OUTPUT); digitalWrite(LED_BUILTIN,HIGH);
  Serial.begin(9600);
  
  dht.begin();
  delay(1000);
  Serial.println("nd" + String(uint16_t(dht.readTemperature())) + "da" + String(uint16_t(dht.readHumidity())));
  
  configTime(7*3600, 0,"2.vn.pool.ntp.org");
  telegram.setTrustAnchors(&cert); // Add root certificate for api.telegram.org
  delay(200); 
  
  inputString.reserve(200);
  
  EEPROM.begin(512);
  check_startTime = EEPROM.read(EEP_Checked_Start); check_stopTime = EEPROM.read(EEP_Checked_Stop);
  Hour_Start = EEPROM.read(EEP_hour_start); Min_Start = EEPROM.read(EEP_min_start);
  Hour_Stop = EEPROM.read(EEP_hour_stop); Min_Stop = EEPROM.read(EEP_min_stop);
  TempON = EEPROM.read(EEP_Temp_ON); TempOFF = EEPROM.read(EEP_Temp_OFF);
  
  if(Read_EEPROM()){connection_wifi(ssid.c_str(),pass.c_str());timeClient.begin();}
  delay(500);
  if(WiFi.status() == WL_CONNECTED) bot.sendMessage(CHAT_ID,"Smart Home đã kết nối","");
  if(WiFi.status() == WL_CONNECTED) bot.sendMessage(CHAT_ID,"Thông tin chủ Smart Home: Kim Nhựt Hoàng - 2002190238 - 10DHDT1 - DH CNTP TP.HCM","");
//  Serial.println("Đã gửi thông báo đến Telegram");
  
  client.setServer(MQTT_Server, MQTT_Port); // cài đặt server là broker.mqtt-dashboard.com và lắng nghe client ở port 1883
  client.setCallback(callback); // gọi hàm callback để thực hiện các chức năng publish/subcribe
}

void loop() 
{
  // kiểm tra nếu ESP8266 chưa kết nối được thì sẽ thực hiện kết nối lại
  if(WiFi.status() == WL_CONNECTED)
  {
    if(!client.connected()) reconnect();
    client.loop();
  }
  Read_Send_TempHum();
  OnOffDevice_after_TimeSet();
  OnOffDevice_after_TempSet();
  if(config_state == true) webServer.handleClient();
  if(long_press() && config_state == false)
  {
    Smart_config();
  }
  if(long_press() && config_state == true)
  {
    config_state == false;
    connection_wifi(ssid.c_str(),pass.c_str());
  }
  checkConnectionWF(); 
  led_toggle();
}

void callback(char* topic, byte* payload, unsigned int length) 
{
  String Message,Topic_name;
  Topic_name = String(topic);
//  Serial.print("Message arrived ["); Serial.print(Topic_name); Serial.print("] ");
  for (int i = 0; i < length; i++) 
  {
    Message += (char)payload[i];
  }
//  Serial.println(Message);
  if(Topic_name == "ESP8266/Device")
  {
    StaticJsonDocument<200> doc;
    DeserializationError error = deserializeJson(doc, Message);
    if(error) return;
    byte Den1 = doc["den1"], Den2 = doc["den2"], Den3 = doc["den3"];
    byte Quat1 = doc["quat1"], Quat2 = doc["quat2"], Quat3 = doc["quat3"];
    tt_d1 = Den1; tt_d2 = Den2; tt_d3 = Den3; tt_q1 = Quat1; tt_q2 = Quat2; tt_q3 = Quat3;
    Serial.println("d1" + String(Den1) + "d2" + String(Den2) + "d3" + String(Den3) 
                    + "q1" + String(Quat1) + "q2" + String(Quat2) + "q3" + String(Quat3));
  }
  if(Topic_name == "ESP8266/SettimeOn")
  {
    StaticJsonDocument<200> doc;
    DeserializationError error = deserializeJson(doc, Message);
    if(error) return;
    byte EEP_Checked = doc["Checked"], hour_start = doc["hour"], min_start = doc["min"];
    check_startTime = EEP_Checked; Hour_Start = hour_start; Min_Start = min_start;
    EEPROM.write(EEP_Checked_Start,EEP_Checked); EEPROM.write(EEP_hour_start,hour_start);EEPROM.write(EEP_min_start,min_start);
    EEPROM.commit();
  }
  if(Topic_name == "ESP8266/SettimeOff")
  {
    StaticJsonDocument<200> doc;
    DeserializationError error = deserializeJson(doc, Message);
    if(error) return;
    byte EEP_Checked = doc["Checked"], hour_stop = doc["hour"], min_stop = doc["min"];
    check_stopTime = EEP_Checked; Hour_Stop = hour_stop; Min_Stop = min_stop;
    EEPROM.write(EEP_Checked_Stop,EEP_Checked); EEPROM.write(EEP_hour_stop,hour_stop);EEPROM.write(EEP_min_stop,min_stop);
    EEPROM.commit();
  }
  if(Topic_name == "ESP8266/DeviceSettime")
  {
    StaticJsonDocument<200> doc;
    DeserializationError error = deserializeJson(doc, Message);
    if(error) return;
    byte D1 = doc["Den1"], D2 = doc["Den2"], D3 = doc["Den3"],
         Q1 = doc["Quat1"], Q2 = doc["Quat2"], Q3 = doc["Quat3"];
    EEPROM.write(EEP_StateTime_D1,D1);EEPROM.write(EEP_StateTime_D2,D2);EEPROM.write(EEP_StateTime_D3,D3);
    EEPROM.write(EEP_StateTime_Q1,Q1);EEPROM.write(EEP_StateTime_Q2,Q2);EEPROM.write(EEP_StateTime_Q3,Q3);
    EEPROM.commit();
  }
  if(Topic_name == "ESP8266/TempSetOnOff")
  {
    StaticJsonDocument<200> doc;
    DeserializationError error = deserializeJson(doc, Message);
    if(error) return;
    
    byte Checked_On = doc["CheckedOn"], Checked_Off = doc["CheckedOff"], tempOn = doc["tempOn"], tempOff = doc["tempOff"];
    byte Q1 = doc["quat1"], Q2 = doc["quat2"], Q3 = doc["quat3"];

    TempON = tempOn; TempOFF = tempOff;
    
    EEPROM.write(EEP_CheckedTemp_ON,Checked_On);EEPROM.write(EEP_CheckedTemp_OFF,Checked_Off);
    EEPROM.write(EEP_Temp_ON,tempOn); EEPROM.write(EEP_Temp_OFF,tempOff);
    EEPROM.write(EEP_StateTemp_Q1,Q1);EEPROM.write(EEP_StateTemp_Q2,Q2);EEPROM.write(EEP_StateTemp_Q3,Q3);
    EEPROM.commit();
  }
}
void reconnect() 
{
  if(millis() - time_delayConnect > 5000)
  {
    client.connected(); // lặp cho đến khi được kết nối trở lại
//    Serial.print("Attempting MQTT connection...");
    if (client.connect(Client_id, MQTT_User, MQTT_Pass)) 
    {
      time_delayConnect = 0;
//      Serial.println("connected");
      client.subscribe("ESP8266/Device"); // đăng kí nhận gói tin tại topic ESP8266/Device
      client.subscribe("ESP8266/SettimeOn"); // đăng kí nhận gói tin tại topic ESP8266/SettimeOn
      client.subscribe("ESP8266/SettimeOff"); // đăng kí nhận gói tin tại topic ESP8266/SettimeOff
      client.subscribe("ESP8266/DeviceSettime"); // đăng kí nhận gói tin tại topic ESP8266/DeviceSettime
      client.subscribe("ESP8266/TempSetOnOff"); // đăng kí nhận gói tin tại topic ESP8266/TempSetOnOff
    } 
    else 
    {
      time_delayConnect = millis();
//      Serial.print("failed, rc=");  // in ra màn hình trạng thái của client khi không kết nối được với MQTT broker
//      Serial.print(client.state());
//      Serial.println(" try again in 5 seconds");
    }
  }
}

void SendStatus_to_MQTT()
{
  String JsonData;
  StaticJsonDocument<200> doc;
  doc["den1"] = tt_d1; doc["den2"] = tt_d2; doc["den3"] = tt_d3; 
  doc["quat1"] = tt_q1; doc["quat2"] = tt_q2; doc["quat3"] = tt_q3;
  serializeJson(doc, JsonData);
  client.publish("ESP8266/Device",JsonData.c_str(),true);
}

void serialEvent() 
{
  while(Serial.available()) 
  {
    char inChar = (char)Serial.read();
    inputString += inChar;
    if(inChar == '\n') stringComplete = true;
  }
  if(stringComplete) 
  {
//    Serial.print(inputString);
    if(inputString == "DOP\n")
    {
      if(WiFi.status() == WL_CONNECTED) bot.sendMessage(CHAT_ID,"Thông báo: Cửa vừa được mở !","");
    }
    if(inputString == "PWOF\n")
    {
      if(WiFi.status() == WL_CONNECTED) bot.sendMessage(CHAT_ID,"Thông báo: Hệ thống mất điện nguồn, chuyển sang nguồn dự phòng","");
    }
    if(inputString == "WR3\n")
    {
      if(WiFi.status() == WL_CONNECTED) bot.sendMessage(CHAT_ID,"Cảnh báo: Mã Pin hoặc thẻ RFID nhập sai 3 lần !","");
    }
    if(inputString[0] == 'T' && inputString[1] == 'T')
    {
      if(inputString[2] == 'd' && inputString[3] == '1' && inputString[4] == '1') tt_d1 = 1; 
      else if(inputString[2] == 'd' && inputString[3] == '1' && inputString[4] == '0') tt_d1 = 0;
      
      if(inputString[2] == 'd' && inputString[3] == '2' && inputString[4] == '1') tt_d2 = 1; 
      else if(inputString[2] == 'd' && inputString[3] == '2' && inputString[4] == '0') tt_d2 = 0;
      
      if(inputString[2] == 'd' && inputString[3] == '3' && inputString[4] == '1') tt_d3 = 1; 
      else if(inputString[2] == 'd' && inputString[3] == '3' && inputString[4] == '0') tt_d3 = 0;
      
      if(inputString[2] == 'q' && inputString[3] == '1' && inputString[4] == '1') tt_q1 = 1; 
      else if(inputString[2] == 'q' && inputString[3] == '1' && inputString[4] == '0') tt_q1 = 0;
      
      if(inputString[2] == 'q' && inputString[3] == '2' && inputString[4] == '1') tt_q2 = 1; 
      else if(inputString[2] == 'q' && inputString[3] == '2' && inputString[4] == '0') tt_q2 = 0;
      
      if(inputString[2] == 'q' && inputString[3] == '3' && inputString[4] == '1') tt_q3 = 1; 
      else if(inputString[2] == 'q' && inputString[3] == '3' && inputString[4] == '0') tt_q3 = 0;
      
      if(WiFi.status() == WL_CONNECTED) SendStatus_to_MQTT();
    }
    inputString = "";
    stringComplete = false;
  }
}

void OnOffDevice_after_TimeSet()
{
  if(millis() - Wait_ONOFF > 5000)
  {
    Wait_ONOFF = millis();
    if(check_startTime == 1 && WiFi.status() == WL_CONNECTED)
    {
      timeClient.update();
      if(timeClient.getMinutes() != Min_Start) TimeOn_done = true;
      if(timeClient.getHours() == Hour_Start && timeClient.getMinutes() == Min_Start && TimeOn_done == true)
      {
        TimeOn_done = false;
        if(EEPROM.read(EEP_StateTime_D1) == 1) tt_d1 = 1;
        if(EEPROM.read(EEP_StateTime_D2) == 1) tt_d2 = 1;
        if(EEPROM.read(EEP_StateTime_D3) == 1) tt_d3 = 1;
        if(EEPROM.read(EEP_StateTime_Q1) == 1) tt_q1 = 1;
        if(EEPROM.read(EEP_StateTime_Q2) == 1) tt_q2 = 1;
        if(EEPROM.read(EEP_StateTime_Q3) == 1) tt_q3 = 1;
        Serial.println("d1" + String(tt_d1) + "d2" + String(tt_d2) + "d3" + String(tt_d3) 
                      + "q1" + String(tt_q1) + "q2" + String(tt_q2) + "q3" + String(tt_q3));
        SendStatus_to_MQTT();
      }
    }
    if(check_stopTime == 1 && WiFi.status() == WL_CONNECTED)
    {
      timeClient.update();
      if(timeClient.getMinutes() != Min_Stop) TimeOff_done = true;
      if(timeClient.getHours() == Hour_Stop && timeClient.getMinutes() == Min_Stop && TimeOff_done == true)
      {
        TimeOff_done = false;
        if(EEPROM.read(EEP_StateTime_D1) == 1) tt_d1 = 0;
        if(EEPROM.read(EEP_StateTime_D2) == 1) tt_d2 = 0;
        if(EEPROM.read(EEP_StateTime_D3) == 1) tt_d3 = 0;
        if(EEPROM.read(EEP_StateTime_Q1) == 1) tt_q1 = 0;
        if(EEPROM.read(EEP_StateTime_Q2) == 1) tt_q2 = 0;
        if(EEPROM.read(EEP_StateTime_Q3) == 1) tt_q3 = 0;
        Serial.println("d1" + String(tt_d1) + "d2" + String(tt_d2) + "d3" + String(tt_d3) 
                      + "q1" + String(tt_q1) + "q2" + String(tt_q2) + "q3" + String(tt_q3));
        SendStatus_to_MQTT();
      }
    }
  }  
}

void OnOffDevice_after_TempSet()
{
  if(millis() - Wait_ONOFF1 > 5000)
  {
    Wait_ONOFF1 = millis();
    uint16_t tempRead = dht.readTemperature();
    if(check_OnTemp == 1 && WiFi.status() == WL_CONNECTED)
    {
      if(tempRead <= TempON) TempOn_done = true;
      if(tempRead >= TempON && TempOn_done == true)
      {
        TempOn_done = false;
        if(EEPROM.read(EEP_StateTime_Q1) == 1) tt_q1 = 1;
        if(EEPROM.read(EEP_StateTime_Q2) == 1) tt_q2 = 1;
        if(EEPROM.read(EEP_StateTime_Q3) == 1) tt_q3 = 1;
        Serial.println("d1" + String(tt_d1) + "d2" + String(tt_d2) + "d3" + String(tt_d3) 
                      + "q1" + String(tt_q1) + "q2" + String(tt_q2) + "q3" + String(tt_q3));
        SendStatus_to_MQTT();
      }
    }
    if(check_OffTemp== 1 && WiFi.status() == WL_CONNECTED)
    {
      if(tempRead != TempOFF) TempOff_done = true;
      if(tempRead == TempOFF && TempOff_done == true)
      {
        TempOff_done = false;
        if(EEPROM.read(EEP_StateTime_Q1) == 1) tt_q1 = 0;
        if(EEPROM.read(EEP_StateTime_Q2) == 1) tt_q2 = 0;
        if(EEPROM.read(EEP_StateTime_Q3) == 1) tt_q3 = 0;
        Serial.println("d1" + String(tt_d1) + "d2" + String(tt_d2) + "d3" + String(tt_d3) 
                      + "q1" + String(tt_q1) + "q2" + String(tt_q2) + "q3" + String(tt_q3));
        SendStatus_to_MQTT();
      }
    }
  }  
}

void Read_Send_TempHum()
{
  static uint16_t last_t = 0,last_h = 0;
  if(millis() - TimeSend_TempHum > 3000)
  {
    TimeSend_TempHum = millis();
    if(WiFi.status() == WL_CONNECTED) timeClient.update();
    uint16_t t = dht.readTemperature();
    uint16_t h = dht.readHumidity();
    if(t < 50 && h < 100)
    {
      last_t = t; last_h = h;
      String JsonData;
      StaticJsonDocument<200> doc;
      doc["temp"] = t; doc["hum"] = h;
      serializeJson(doc, JsonData);
      if(WiFi.status() == WL_CONNECTED) client.publish("ESP8266/Sensor",JsonData.c_str(),true);
      if(WiFi.status() == WL_CONNECTED) Serial.println("nd" + String(t) + "da" + String(h) + "T" + String(timeClient.getHours()) + ":" + String(timeClient.getMinutes()));
      else Serial.println("nd" + String(t) + "da" + String(h));
    }
    else
    { 
      if(WiFi.status() == WL_CONNECTED) Serial.println("nd" + String(last_t) + "da" + String(last_h) + "T" + String(timeClient.getHours()) + ":" + String(timeClient.getMinutes()));
      else Serial.println("nd" + String(last_t) + "da" + String(last_h));
    }
  }
}

bool long_press()
{
  static int lastpress = 0;
  if(millis() - lastpress > 5000 && digitalRead(0) == 0)
  {
    lastpress = 0;
    return true;
  }
  else if(digitalRead(0) == 1) lastpress = millis();
  return false;
}

void Smart_config()
{
    config_state = true; flag_led = true; time_toggle = 200;
//    Serial.println("Smart config start");
//    Serial.println("Configuring access point...");
    WiFi.disconnect();
    WiFi.mode(WIFI_AP);
    WiFi.softAP(ssid_ap, pass_ap);
//    Serial.print("AP IP address: ");
//    Serial.println(WiFi.softAPIP());
    
    webServer.on("/",mainpage);
    webServer.on("/action",handleForm);
    webServer.begin();
//    Serial.println("Server đã chạy");
}

void handleForm() 
{
  ssid = webServer.arg("ssid"); 
  pass = webServer.arg("pass"); 
//  Serial.print("ssid:"); Serial.println(ssid);
//  Serial.print("pass:"); Serial.println(pass);
  mainpage();
  Clear_EEPROM(); Write_EEPROM(ssid,pass);
  delay(1000);
//  if(Read_EEPROM()) Serial.println("Read eeprom done");
  Read_EEPROM();
  connection_wifi(ssid.c_str(),pass.c_str());
}

void connection_wifi(const char *ssid, const char *pass)
{
  int timeout = 0;
  config_state = false;
  webServer.stop();
  WiFi.disconnect();
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid,pass);
//  Serial.print("Đang kết nối với: ");
//  Serial.println(ssid);
  while(timeout < 100)
  {
    if(WiFi.status() == WL_CONNECTED)
    {
//      Serial.println();
//      Serial.print("Đã kết nối với ");
//      Serial.println(ssid);
//      Serial.print("Địa chỉ IP: ");
//      Serial.println(WiFi.localIP());
      return;
    }
    delay(200);
//    Serial.println(".");
    timeout++;
  }
//  Serial.println("Timed out");  
}

void led_toggle()
{
  if(flag_led)
  {
    static bool led_state = false;
    if(millis() - led_time > time_toggle)
    {
//      digitalWrite(LED_BUILTIN,!digitalRead(LED_BUILTIN));
      led_state = !led_state;
      Serial.println("S" + String(led_state));
      led_time = millis(); 
    }
  }
}

void checkConnectionWF()
{
  if(config_state == false)
  {
    if(millis() - time_checkconnect > 5000)
    {
      if(WiFi.status() == WL_CONNECTED) 
      {
        flag_led = true; time_toggle = 2000; 
        client.publish("ESP8266/StatusConnected","1");
      }
      else 
      {
        flag_led = false; digitalWrite(LED_BUILTIN,HIGH);
        Serial.println("S0");
      }
      time_checkconnect = millis();
    }
  }
}

void mainpage()
{
  String s = FPSTR(SmartConfigPage);
  webServer.send(200,"text/html",s);
}

boolean Read_EEPROM()
{
//  Serial.println("Reading EEPROM...");
  if(EEPROM.read(0)!=0)
  {
    ssid = "";
    pass = "";
    for (int i=0; i<32; ++i)
    {
      ssid += char(EEPROM.read(i));
    }
//    Serial.print("SSID: ");
//    Serial.println(ssid);
    for (int i=32; i<96; ++i)
    {
      pass += char(EEPROM.read(i));
    }
//    Serial.print("PASSWORD: ");
//    Serial.println(pass);
    return true;
  }
  else
  {
//    Serial.println("Data wifi not found!");
    return false;
  }
}

void Write_EEPROM(String ssid, String pass)
{
  for (int i = 0; i < ssid.length(); ++i) 
  {
    EEPROM.write(i, ssid[i]);
  }
  for (int i = 0; i < pass.length(); ++i) 
  {
    EEPROM.write(32 + i, pass[i]);
  }
  EEPROM.commit();
//  Serial.println("Writed to EEPROM done!");
}

void Clear_EEPROM()
{
//  Serial.println("Clear EEPROM!");
  for (int i = 0; i < 96; ++i) 
  {
    EEPROM.write(i, 0);           
  }    
}
