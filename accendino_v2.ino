#define VERSION "v1.2 13/01/2024"

#include <OneWire.h>
#include <DallasTemperature.h>
#include "CTBot.h"
#include "DHT.h"
#define DHTPIN D3     
#define DHTTYPE DHT11  
#define TEMP_PIN D8
#define RELE_PIN D13
#define HUM "hum"
#define ON "on"
#define OFF "off"
#define STATUS "status"
#define TEMP "temp"
#define EMILIANO_ID 0000


CTBot myBot;
CTBotInlineKeyboard pulsante;
String ssid = "ssid";    
String pass = "password"; 
String token = "token"; 
int check_time;   

OneWire oneWire(TEMP_PIN); 
DallasTemperature sensors(&oneWire); 

DHT dht(DHTPIN, DHTTYPE);
TBMessage msg;
int status=0;

void setup() {
  dht.begin();
	Serial.begin(115200);
	Serial.println("Sto eseguendo la versione "+ String(VERSION));
  // connect the ESP8266
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
  
  sensors.begin();
  pinMode(RELE_PIN,OUTPUT);
  check_time=millis();
  myBot.sendMessage(EMILIANO_ID,("Ciao, sono pronto a ricevere comandi."));
}



void loop() {

	TBMessage msg;
  
  //check connection every hour
  if(millis()-3600000>check_time){
    check_time=millis();  
    if (!myBot.testConnection()){
      myBot.wifiConnect(ssid, pass);
    }
  }

	//if there is new message
	if (myBot.getNewMessage(msg)) {
    
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
      digitalWrite(RELE_PIN, HIGH);
      myBot.sendMessage(msg.sender.id,("Accendo la caldaia..."));
      status=1;
      sendButtons(msg); 
    }

    else if (msg.callbackQueryData.equals(OFF)){
      digitalWrite(RELE_PIN, LOW);
      myBot.sendMessage(msg.sender.id,("Spengo la caldaia..."));
      status=0;
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

    else {                                                    
      sendButtons(msg);  
    }

  }
}


//utility functions
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
	
