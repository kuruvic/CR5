/*
 *  WIFI Module for Tinyfarmer
 *  
 *  
 *  References
 *  http://esp8266.github.io/Arduino/versions/2.1.0/doc/libraries.html
 * 
 * button    - GPIO 0   (LOW when switch is pressed)
 * Relay     - GPIO 12  (HIGH to turn on)
 * Green LED - GPIO 13  (LOW to turn on)
 * Spare     - GPIO 14
 * 
 * 전원이 들어오면 LED가 ON, 바로 10번 깜빡임, 그때 버튼 누르면 set mode진입 후 LED OFF
 * setting이 종료되면 LED ON
 * 
 * Client 코넥션이 끊기면 LED OFF, 연결 복구되면 LED ON
 * 
 * - Parameter : hub IP , node ID (device id)
 * 
 * modification
 * 
 * 2017.07.01
 * - add chipid to SSID name
 * 
 * 2017.11.15
 * - secure code added
 * - id , password
 * 
 * 2018.03.28
 * - topic, id, pass removed
 * - char형 으로 고정
 * 
 * 2018.05.04
 * main 함수에 중복 선언된 pubString 삭제
 * 시리얼 스트링 받는 함수 아래와 같은 형태로 될수 있는한 시리얼이벤트 함수 비슷한 형태로 제작 
 * void serialEvent()
    {
        if (Serial.available() > 0)
        {
            pubString = Serial.readStringUntil('\n');
            pubString.toCharArray(message_buff, pubString.length()+1);
            jsonComplete = true;
        }
    }
 * 2018.11.22
 * void reconnect() 함수에 허브와 연결이 끈어지면 {"Err":"1"} 붙으면 {"Err":"0"} 이라고 아두이노에 보냄
 * 
 * 2018.12.13
 * 하트비트 파싱 하여 ID 값 축출 void callback() 함수에서 subString 파싱 후 축출된 ID 비교 후 해당 아이디에 서브스트립트 값 전송
 *
 * 2018.12.14
 * 콜백 함수에서 relayHistoryId, motorCtrlYn, id, val, seq 값을 파싱하여 구조체로 구성한두 아두이노에게 불필요한 ptcd 값을 제외하고 전송  
 *
 */


#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino
#include <ArduinoJson.h>
#include "FS.h"                   //http://esp8266.github.io/Arduino/versions/2.0.0/doc/filesystem.html

//needed for library
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>         //https://github.com/tzapu/WiFiManager
#include <EEPROM.h>
#include <PubSubClient.h>

#define NoErr 0
#define internet 1

//http://www.esp8266.com/viewtopic.php?f=21&t=1388
int button = 0;
int sigLED = 2;

#define   STRUCT_CHAR_ARRAY_SIZE 40

//#define SECURE

WiFiClient espclient;
WiFiClientSecure secureClient;
PubSubClient client(espclient);

char message_buff[200] = {0};
uint16_t port;
uint16_t nodeid;
const char * topic;
long lastMsg = 0;
char ssid[100] ;

char senddata[150];
String subString = "";    
String pubString = "";
String conString = "";
char msg[50];

boolean stringComplete = false; 

bool    jsonComplete = false;

typedef struct
{
  char   TinyFarmerHub[STRUCT_CHAR_ARRAY_SIZE]   = "";//{0};
  char   TinyFarmerHubPort[6]                    = "";//{0};
  char   ID[3] = "";
} Settings;

Settings  settings;

typedef struct protocolSt 
{
      //String relayHistoryId;
      //String motorCtrlYn;
      uint16_t id;
      uint16_t relay;
      uint16_t sel;
      
} _bmProtocolST;

_bmProtocolST stPROTO;

typedef struct SeriallSt 
{
      uint16_t ptCd;
      uint16_t id;
} _bmSeriallST;

_bmSeriallST stSERIAL;

char RX_topic[40];
String RXD ; //= "/tinyhub/command/"+ String(nodeid)+"";

char TX_topic[40] = "/tinyhub/message";
//char tinyID[40] = //"iotmonster";
//char tinyPASSWARD[40] = //"iot@1115(*&^";

bool isCommandJSON(String _comm)
{
    String jsonString = _comm;
    bool ret = false;

    StaticJsonBuffer<200> jsonBuffer;
    JsonObject& root = jsonBuffer.parseObject(jsonString);

    if (root.success())
    {    
        uint16_t _id = root["id"];
        //String _relayHistoryId = root["relayHistoryId"];
        //String _motorCtrlYn = root["motorCtrlYn"];
        uint16_t _relay = root["relay"];
        uint16_t _sel = root["sel"];
        
        //stPROTO.relayHistoryId = _relayHistoryId;
        //stPROTO.motorCtrlYn = _motorCtrlYn;
        stPROTO.id = _id;
        stPROTO.relay = _relay;
        stPROTO.sel = _sel;
        
        ret = true;
    }

    return ret;
}

bool SerialJSON(String _comm)
{
    String jsonString = _comm;
    bool ret = false;

    StaticJsonBuffer<200> jsonBuffer;
    JsonObject& root = jsonBuffer.parseObject(jsonString);

    if (root.success())
    {
        uint16_t _ptCd = root["ptCd"];
        uint16_t _id = root["id"];

        stSERIAL.ptCd = _ptCd;
        stSERIAL.id = _id; 
            
        ret = true;
    }

    return ret;
}

/////////////////////////////////////////////////////////////////////////////////
void callback(char* topic, byte* payload, unsigned int length) 
{
  for (int i = 0; i < length; i++) {
    //Serial.print((char)payload[i]);
    senddata[i] = (char)payload[i];
  }

  subString = senddata;
  isCommandJSON(subString);

  //conString = "{\"id\":\""+String(stPROTO.id)+"\""; 
  //conString += ",\"relayHistoryId\":\""+String(stPROTO.relayHistoryId)+"\"";
  //conString += ",\"motorCtrlYn\":\""+String(stPROTO.motorCtrlYn)+"\"";
  //conString += ",\"relay\":\""+String(stPROTO.relay)+"\"";
  //conString += ",\"sel\":\""+String(stPROTO.sel)+"\"";
  //conString += "}";
     
  if(stPROTO.id == nodeid)
  {
        Serial.println(subString);
  }   
  
  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '1') {
    digitalWrite(sigLED, LOW);   // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because
    // it is acive low on the ESP-01)
  } else {
    digitalWrite(sigLED, HIGH);  // Turn the LED off by making the voltage HIGH
  }
}

/////////////////////////////////////////////////////////////////////////////////
void setup() {
    // put your setup code here, to run once:
    Serial.begin(9600);

    pinMode(button, INPUT);
    pinMode(sigLED, OUTPUT);

    digitalWrite(sigLED,HIGH);

    EEPROM.begin(512);
    EEPROM.get(0, settings);
    EEPROM.end();

    WiFiManagerParameter tinyfarmer_hub_server("server", "허브 IP", settings.TinyFarmerHub, STRUCT_CHAR_ARRAY_SIZE);
    WiFiManagerParameter tinyfarmer_hub_port("port", "허브 포트번호", settings.TinyFarmerHubPort, 6);
    //WiFiManagerParameter tinyfarmer_RX_topic("RX-topic", "받는 Topic", settings.RXTopic, STRUCT_CHAR_ARRAY_SIZE);
    //WiFiManagerParameter tinyfarmer_TX_topic("TX-topic", "보내는 Topic", settings.TXTopic, STRUCT_CHAR_ARRAY_SIZE);
    WiFiManagerParameter tinyfarmer_ID("ID", "허브ID", settings.ID, STRUCT_CHAR_ARRAY_SIZE);
    //WiFiManagerParameter tinyfarmer_PASSWARD("PASSWORD", "허브 PASSWARD", settings.PASSWORD, STRUCT_CHAR_ARRAY_SIZE);

    //WiFiManager
    //Local intialization. Once its business is done, there is no need to keep it around
    WiFiManager wifiManager;
    //reset saved settings

    if(digitalRead(button) == LOW)
    {
        digitalWrite(sigLED,LOW);
        wifiManager.resetSettings();
    }

    wifiManager.addParameter(&tinyfarmer_hub_server);
    wifiManager.addParameter(&tinyfarmer_hub_port);
    //wifiManager.addParameter(&tinyfarmer_RX_topic);
    //wifiManager.addParameter(&tinyfarmer_TX_topic);
    wifiManager.addParameter(&tinyfarmer_ID);
    //wifiManager.addParameter(&tinyfarmer_PASSWARD);
    
    String ssidS = "BitMoss_" + String(ESP.getChipId());

    ssidS.toCharArray(ssid, ssidS.length());
    wifiManager.autoConnect(ssid);
    
    digitalWrite(sigLED,HIGH);
    
    //if you get here you have connected to the WiFi
    Serial.println("connected...yeey :)");

    strcpy(settings.TinyFarmerHub, tinyfarmer_hub_server.getValue());
    strcpy(settings.TinyFarmerHubPort, tinyfarmer_hub_port.getValue());
    //strcpy(settings.RXTopic, tinyfarmer_RX_topic.getValue());
    //strcpy(settings.TXTopic, tinyfarmer_TX_topic.getValue());
    strcpy(settings.ID, tinyfarmer_ID.getValue());
    //strcpy(settings.PASSWORD, tinyfarmer_PASSWARD.getValue());
    
    EEPROM.begin(512);
    EEPROM.put(0, settings);
    EEPROM.end();

    port = (uint16_t)atoi(settings.TinyFarmerHubPort);
    nodeid = (uint16_t)atoi(settings.ID);

    RXD = "/tinyhub/command/"+ String(nodeid)+"";
    RXD.toCharArray(RX_topic, RXD.length()+1);
    
    client.setServer(settings.TinyFarmerHub, port);
    client.setCallback(callback);

    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP()); 
    Serial.println(settings.TinyFarmerHub);
    
    if (client.connect("arduinoClient3")) {
        
        client.subscribe(RX_topic);
        Serial.println(RX_topic);
        client.publish(TX_topic, ssid);  
    }

    delay(3000);

    String noerr1 = "{\"id\":\"" + String(settings.ID) + "\"}";
    
    for(int i=0;i<2;i++)
    {
       Serial.println(noerr1);
       delay(100);
    }

    delay(1000);
}

/////////////////////////////////////////////////////////////////////////////////
void reconnect()
{
  // Loop until we're reconnected
  if (!client.connected()) 
  {
      //The device will connect at this point, and the mosquitto server will be happy. 
      //It will use the name on the certificate as the client name, and the connection will be encrypted.
      
#ifdef SECURE
      secureClient.connect("arduinoClient3");
      
      if(secureClient.verify("DA 39 A3 EE 5E 6B 4B 0D 32 55 BF EF 95 60 18 90 AF D8 07 09", "mqtt.local")) 
      {
          Serial.println("connection checks out");
          secureClient.stop();
          
          // Attempt to connect
          if (client.connect(ssid, tinyID, tinyPASSWARD)) 
          {
              Serial.println("connected");
              delay(3000);
          } 
          else 
          {
            Serial.print("failed, rc=");
            Serial.print(client.state());
            Serial.println(" try again in 5 seconds");
            // Wait 5 seconds before retrying
            delay(5000);
            //ESP.reset(); // CPU restart
          }
      } 
      else       
      {
            secureClient.stop();
            Serial.println("connection doesn't check out");
      }
#else
      if(client.connect("arduinoClient3"))
      {
          String noerr1 = "{\"id\":\"" + String(settings.ID) + "\"}";
          
          
          RXD = "/tinyhub/command/"+ String(nodeid)+"";
          RXD.toCharArray(RX_topic, RXD.length()+1);

          client.subscribe(RX_topic);
          //Serial.println(RX_topic);
          client.publish(TX_topic, ssid);  
      }
#endif
  }
}

void loop() 
{           
     serialEvent();

     if (!client.connected()) 
     {
         reconnect();
     }   
     
     client.loop(); 
    
     if(jsonComplete == true)
     { 
        if(client.connect("arduinoClient3")){  
            digitalWrite(sigLED,LOW);
            client.publish(TX_topic, message_buff);  
            //Serial.print(message_buff);
            //Serial.println(RX_topic);
            digitalWrite(sigLED,HIGH);  
            jsonComplete = false;
        }    
     }       
}

void serialEvent()
{
  if (Serial.available() > 0)
  {
      pubString = Serial.readStringUntil('\n');
      pubString.toCharArray(message_buff, pubString.length()+1);
      SerialJSON(pubString);
      jsonComplete = true;
  }
}


