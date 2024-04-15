#define VERSION "Accendino_v2 v1.5 del 06/04/2024"

#include <OneWire.h>
#include <DallasTemperature.h>
#include "CTBot.h"
#include "DHT.h"
#include "secret.h"
#define DHTPIN D3     
#define DHTTYPE DHT11  
#define TEMP_PIN D8
#define RELE_PIN D13
#define HUM "hum"
#define ON "on"
#define OFF "off"
#define STATUS "status"
#define TEMP "temp"


CTBot myBot;
CTBotInlineKeyboard pulsante;
String ssid = WIFI_SSID;    
String pass = WIFI_PASSWORD; 
String token = TELEGRAM_TOKEN; 

int check_time;
int msg_time;   

OneWire oneWire(TEMP_PIN); 
DallasTemperature sensors(&oneWire); 

DHT dht(DHTPIN, DHTTYPE);
TBMessage msg;
int status=0;

void setup() {
  dht.begin();
	Serial.begin(9600);
	myBot.wifiConnect(ssid, pass);
	myBot.setTelegramToken(token);

	//test connection
	if (myBot.testConnection())
		Serial.println("\ntestConnection OK");
	else
		Serial.println("\ntestConnection NOK");

  pulsante.addButton("Umidità", HUM, CTBotKeyboardButtonQuery);
  pulsante.addButton("Temperatura", TEMP, CTBotKeyboardButtonQuery);
  pulsante.addRow();
  pulsante.addButton("Accendi la caldaia", ON, CTBotKeyboardButtonQuery);
  pulsante.addButton("Spegni la caldaia", OFF, CTBotKeyboardButtonQuery);
  pulsante.addRow();
  pulsante.addButton("Stato accensione", STATUS, CTBotKeyboardButtonQuery);

  Serial.println("Sto eseguendo la versione "+ String(VERSION));
  
  sensors.begin();
  pinMode(RELE_PIN,OUTPUT);
  check_time=millis();
  msg_time=millis();
  myBot.sendMessage(EMILIANO_ID,("Ciao, sono di nuovo online e pronto a ricevere comandi."));
}



void loop() {

	
  
  //check connection 30 minutes
  if(millis()>check_time+1800000){
    Serial.println("CONTROLLO LA CONNESSIONE");
    check_time=millis();  
    if (!myBot.testConnection()){
      myBot.wifiConnect(ssid, pass);
    }
  }

  if(millis()>msg_time+30000){ //check every 30 second, otherwise consume 500 mb every 4 days
    msg_time=millis();
    //if there is new message
    if (myBot.getNewMessage(msg)) {

      String text=msg.text;
      text.toLowerCase();
      
      if (msg.callbackQueryData.equals(HUM)) {              
        float h = readHum();
        myBot.sendMessage(msg.sender.id,("L'umidità è al "+String(h)+"%"));
        sendButtons(msg); 
      }

      else if (msg.callbackQueryData.equals(TEMP)) {
        float temp=readTemp();
        myBot.sendMessage(msg.sender.id,("La temperatura è di "+String(temp)+"° C"));
        sendButtons(msg); 
      }
    
      else if (msg.callbackQueryData.equals(ON)){
        turnON();
        myBot.sendMessage(msg.sender.id,("Ho acceso la caldaia"));
        sendButtons(msg); 
      }

      else if (msg.callbackQueryData.equals(OFF)){
        turnOFF();
        myBot.sendMessage(msg.sender.id,("Ho spento la caldaia"));
        sendButtons(msg); 
      }

      else if (msg.callbackQueryData.equals(STATUS)){
        if(status==0){
          myBot.sendMessage(msg.sender.id,("La caldaia è spenta"));
        }
        else{
          myBot.sendMessage(msg.sender.id,("La caldaia è accesa"));
        }
        sendButtons(msg); 
      }

      else if (text.equals("accendi")){
        turnON();
        myBot.sendMessage(msg.sender.id,("Ho acceso la caldaia"));
        sendButtons(msg);
      }

      else if (text.equals("spegni")){
        turnOFF();
        myBot.sendMessage(msg.sender.id,("Ho spento la caldaia"));
        sendButtons(msg); 
      }

      else {                                                    
        sendButtons(msg);  
      }

    }
  }
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
  String reply = (String)"Benvenuto/a "+msg.sender.firstName+", questo è quello che posso fare:";
  myBot.sendMessage(msg.sender.id, reply ,pulsante);    
}
	
