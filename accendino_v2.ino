#define VERSION "Accendino_v2 v3.1 del 19/09/2025 con AsyncTelegram2"

#include <AsyncTelegram2.h>
#include <ESP8266WiFi.h>
#include <time.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "secret.h"

#define USE_CLIENTSSL false
#define MYTZ "CET+1CEST,M3.5.0,M10.5.0/3"

#define RELE_PIN D13
#define ON "on"
#define OFF "off"
#define STATUS "status"
#define PUFFER_TEMP "puffer"
#define LEVEL "level"
#define PUFFER_PIN D12
#define TRIGGER_PIN D4
#define ECHO_PIN D2
#define HEIGHT_TANK 100 //cm
#define MSG_INTERVAL 0 //30000 //check every 30 second, otherwise consume 500 mb every 4 days
#define CHK_INTERVAL 1800000 //30 minutes


BearSSL::WiFiClientSecure client;
BearSSL::Session   session;
BearSSL::X509List  certificate(telegram_cert);

const char* ssid= WIFI_SSID;    
const char* pass= WIFI_PASSWORD; 
const char* token = TELEGRAM_TOKEN; 

AsyncTelegram2 myBot(client);
InlineKeyboard kbd;
int wifi_status = WL_IDLE_STATUS;

int check_time;
int msg_time;   


OneWire puffer(PUFFER_PIN);
DallasTemperature puffer_sensor(&puffer);

TBMessage msg;
int status=0;

void setup() {
	Serial.begin(9600);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  
  client.setSession(&session);
  client.setTrustAnchors(&certificate);
  client.setBufferSizes(1024, 1024);

  myBot.setUpdateTime(20000);
  myBot.setTelegramToken(token);

  kbd.addButton("Accendi la caldaia", ON, KeyboardButtonQuery);
  kbd.addButton("Spegni la caldaia", OFF, KeyboardButtonQuery);
  kbd.addRow();
  kbd.addButton("Stato accensione", STATUS, KeyboardButtonQuery);
  kbd.addButton("Temperatura puffer", PUFFER_TEMP, KeyboardButtonQuery);
  kbd.addRow();
  kbd.addButton("Livello serbatoio", LEVEL, KeyboardButtonQuery);

  puffer_sensor.begin();
  pinMode(RELE_PIN,OUTPUT);
  pinMode(TRIGGER_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(PUFFER_PIN, INPUT);
  check_time=millis();
  msg_time=millis();
  myBot.sendTo(EMILIANO_ID,("Ciao, sono di nuovo online e pronto a ricevere comandi."));
  Serial.println("Sto eseguendo la versione "+ String(VERSION));
}



void loop() {
  
  if(millis()>check_time+CHK_INTERVAL){
    Serial.println(F("Controllo la connessione"));
    check_time=millis();  
    wifi_status=WiFi.status();
    if(wifi_status!=WL_CONNECTED){
      Serial.println(F("Connessione assente, provo a riconettermi..."));
      wifi_status = WiFi.begin(ssid, pass);
      delay(5000);
    }
  }

  if(millis()>msg_time+MSG_INTERVAL){
    msg_time=millis();
    if (myBot.getNewMessage(msg)) {

      String text=msg.text;
      text.toLowerCase();
    
      if (msg.callbackQueryData.equals(ON) || text.equals("accendi")){
        if(status==1){
          myBot.sendMessage(msg,("La caldaia è già accesa"));
          sendButtons(msg);
        }
        else{
          turnON();
          myBot.sendMessage(msg,("Ho acceso la caldaia"));
          sendButtons(msg); 
        }
      }

      else if (msg.callbackQueryData.equals(OFF) || text.equals("spegni")){
        if(status==0){
          myBot.sendMessage(msg,("La caldaia è già spenta"));
          sendButtons(msg);
        }
        else{
          turnOFF();
          myBot.sendMessage(msg,("Ho spento la caldaia"));
          sendButtons(msg);
        }
      }

      else if (msg.callbackQueryData.equals(STATUS)){
        if(status==0){
          myBot.sendMessage(msg,("La caldaia è spenta"));
        }
        else{
          myBot.sendMessage(msg,("La caldaia è accesa"));
        }
        sendButtons(msg); 
      }

      else if(msg.callbackQueryData.equals(PUFFER_TEMP)){
        float pufferTemp = readPufferTemp();
        myBot.sendMessage(msg,("La temperatura del puffer è: "+String(pufferTemp)+"° C"));
        sendButtons(msg); 
      }

      else if(msg.callbackQueryData.equals(LEVEL)){
        int capacity=readCapacity();
        myBot.sendMessage(msg,("Il serbatoio è al "+String(capacity)+"%"));
        sendButtons(msg); 
      }

      else {                                                    
        sendButtons(msg);  
      }
      msg.callbackQueryData = "";
    }
  } 
}


void turnON(){
  digitalWrite(RELE_PIN, HIGH);
  status=1;
}

void turnOFF(){
  digitalWrite(RELE_PIN, LOW);
  status=0;
}

float readPufferTemp(){
  puffer_sensor.requestTemperatures();
  return puffer_sensor.getTempCByIndex(0);
}

void sendButtons(TBMessage msg){
  myBot.sendMessage(msg, "Benvenuto/a questo è quello che posso fare:" ,kbd);    
}

float readCapacity(){
  digitalWrite(TRIGGER_PIN, LOW);
  digitalWrite(TRIGGER_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIGGER_PIN, LOW);

  long time = pulseIn(ECHO_PIN, HIGH);
  long distance = time/58.31;
  return 105-((distance*100)/(HEIGHT_TANK)); //105 for the minimum distance of the sensor 
}

	
