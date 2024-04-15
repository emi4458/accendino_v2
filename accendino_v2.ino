#define VERSION "Accendino_v2 v2.0 del 15/04/2024 con AsyncTelegram2"

#include <AsyncTelegram2.h>
#include <ESP8266WiFi.h>
#include <time.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "DHT.h"
#include "secret.h"

#define USE_CLIENTSSL false
#define MYTZ "CET+1CEST,M3.5.0,M10.5.0/3"

#define DHTPIN D3     
#define DHTTYPE DHT11  
#define TEMP_PIN D8
#define RELE_PIN D13
#define HUM "hum"
#define ON "on"
#define OFF "off"
#define STATUS "status"
#define TEMP "temp"


BearSSL::WiFiClientSecure client;
BearSSL::Session   session;
BearSSL::X509List  certificate(telegram_cert);

const char* ssid= WIFI_SSID;    
const char* pass= WIFI_PASSWORD; 
const char* token = TELEGRAM_TOKEN; 

//WiFiClient client;
AsyncTelegram2 myBot(client);
InlineKeyboard kbd;
int wifi_status = WL_IDLE_STATUS; //status of connection

int check_time;
int msg_time;   

OneWire oneWire(TEMP_PIN); 
DallasTemperature sensors(&oneWire); 

DHT dht(DHTPIN, DHTTYPE);
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

  kbd.addButton("Umidità", HUM, KeyboardButtonQuery);
  kbd.addButton("Temperatura", TEMP, KeyboardButtonQuery);
  kbd.addRow();
  kbd.addButton("Accendi la caldaia", ON, KeyboardButtonQuery);
  kbd.addButton("Spegni la caldaia", OFF, KeyboardButtonQuery);
  kbd.addRow();
  kbd.addButton("Stato accensione", STATUS, KeyboardButtonQuery);

  sensors.begin();
  dht.begin();
  pinMode(RELE_PIN,OUTPUT);
  check_time=millis();
  //msg_time=millis();
  myBot.sendTo(EMILIANO_ID,("Ciao, sono di nuovo online e pronto a ricevere comandi."));
  Serial.println("Sto eseguendo la versione "+ String(VERSION));
}



void loop() {
  
  //check connection 30 minutes (1800000 ms)
  if(millis()>check_time+1800000){
    Serial.println(F("Controllo la connessione"));
    check_time=millis();  
    wifi_status=WiFi.status();
    if(wifi_status!=WL_CONNECTED){
      Serial.println(F("Connessione assente, provo a riconettermi..."));
      wifi_status = WiFi.begin(ssid, pass);
      delay(5000);
    }
  }

  //if(millis()>msg_time+30){ //check every 30 second, otherwise consume 500 mb every 4 days
    //msg_time=millis();
    //if there is new message
    if (myBot.getNewMessage(msg)) {

      String text=msg.text;
      text.toLowerCase();
      
      if (msg.callbackQueryData.equals(HUM)) {              
        float h = readHum();
        myBot.sendMessage(msg,("L'umidità è al "+String(h)+"%"));
        sendButtons(msg); 
      }

      else if (msg.callbackQueryData.equals(TEMP)) {
        float temp=readTemp();
        myBot.sendMessage(msg,("La temperatura è di "+String(temp)+"° C"));
        sendButtons(msg); 
      }
    
      else if (msg.callbackQueryData.equals(ON)){
        turnON();
        myBot.sendMessage(msg,("Ho acceso la caldaia"));
        sendButtons(msg); 
      }

      else if (msg.callbackQueryData.equals(OFF)){
        turnOFF();
        myBot.sendMessage(msg,("Ho spento la caldaia"));
        sendButtons(msg); 
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

      else if (text.equals("accendi")){
        turnON();
        myBot.sendMessage(msg,("Ho acceso la caldaia"));
        sendButtons(msg);
      }

      else if (text.equals("spegni")){
        turnOFF();
        myBot.sendMessage(msg,("Ho spento la caldaia"));
        sendButtons(msg); 
      }

      else {                                                    
        sendButtons(msg);  
      }

    }
  //}
}

//utility functions
void turnON(){
  digitalWrite(RELE_PIN, HIGH);
  status=1;
}

void turnOFF(){
  digitalWrite(RELE_PIN, LOW);
  status=0;
}

float readHum(){
  return dht.readHumidity();   
}

float readTemp(){
  sensors.requestTemperatures();
  return sensors.getTempCByIndex(0);
}

void sendButtons(TBMessage msg){
  myBot.sendMessage(msg, "Benvenuto/a questo è quello che posso fare:" ,kbd);    
}
	
