/*
 *  Copyright 2018 Giovanni Reni & Alessandro Gigliotti
 * 
 * SmartGarden1.0 is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * SmartGarden1.0 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <SimpleTimer.h>
#include <BlynkSimpleEsp8266.h>
#include <OSCBundle.h>
#include <OSCBoards.h>

//Il terminale data del sensore è connesso al D1 del NodeMcu
#define ONE_WIRE_BUS D1
//Imposta la comunicazione oneWire per comunicare con un dispositivo compatibile 
OneWire oneWire(ONE_WIRE_BUS);
//Passaggio oneWire reference alla Dallas Temperature
DallasTemperature sensors(&oneWire);

#define DHTPIN D0
#define DHTTYPE DHT11
DHT_Unified dht(DHTPIN, DHTTYPE);

#define BLYNK_PRINT Serial
WidgetLED serbLED(V4);
char auth[] = "*****";                     							  //auth token Blynk app
BlynkTimer timer;

const byte MAX_MSG_SIZE PROGMEM=150;
byte packetBuffer[MAX_MSG_SIZE];
WiFiUDP Udp;

const char* ssid = "OuterRim";                                        //cambiare con ssid della propria Wi-Fi
const char* password = "***";                                 		  //cambiare con password della propria Wi-Fi
IPAddress ip(192, 168, 1, 34);                                        //cambiare con indirizzo IP che si vuole assegnare a questo NodeMcu (UnitaSensori)                                    
IPAddress gateway(192, 168, 1, 254);                                  //cambiare con gateway della propria rete
IPAddress subnet(255, 255, 255, 0);                                   //cambiare con subnet della propria rete

IPAddress outIp(192, 168, 1, 35);                                     //cambiare con indirizzo IP del proprio NodeMcu UnitaDiControllo
int temperatura = 0;
int umidita = 0;
double temperaturaTerra = 0.0;

OSCBundle bundleSensori;


//######################################################################################## FUNCTIONS 1 ########################################################################################
void myTempEvent() {
  Blynk.virtualWrite(V1, temperatura);
}

void myUmEvent() {
  Blynk.virtualWrite(V2, umidita);
}

void myTempTerraEvent() {
  Blynk.virtualWrite(V3, temperaturaTerra);
}


//########################################################################################### SETUP ###########################################################################################
void setup() {

  //sensore pioggia
  pinMode(D2, INPUT);

  //senore umidita' terreno YL-69
  pinMode(D3, INPUT);

  //sensore luce (fotoresistenza)
  pinMode(A0, INPUT);

  //interruttore a galleggiante
  pinMode(D4, INPUT);
  
  Serial.begin(9600);
  Serial.println();
  Serial.printf("Connecting to %s ", ssid);

  //Connessione con Blynk app
  Blynk.begin(auth, ssid, password);
  
  timer.setInterval(1000L, myTempEvent);
  timer.setInterval(1000L, myUmEvent);
  timer.setInterval(1000L, myTempTerraEvent);

  WiFi.mode(WIFI_STA);
  WiFi.config(ip, gateway, subnet);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("Connected");

  Udp.begin(7400);      //porta default OSC
  Serial.printf("Now listening at IP %s", WiFi.localIP().toString().c_str());

  //sensore temperatura
  dht.begin();
  sensor_t sensore;
  //inizializzazione sensore Temperatura
  dht.temperature().getSensor(&sensore);
  //inizializzazione sensore Umidita'
  dht.humidity().getSensor(&sensore);
}


//######################################################################################## FUNCTIONS 2 ########################################################################################
//[1]Is it raining? yes/no
void getRain() {
  int pioggia = digitalRead(D2);
  if(pioggia==1) {
    bundleSensori.add("/R").add(0);
  }
  else if(pioggia==0) {
    bundleSensori.add("/R").add(1);
  }
  else {
    bundleSensori.add("/R").add("n");
  }
}

//[2]What's the air temperature and the humidity of the air?
void getTempHum() {
  sensors_event_t evento;
  dht.temperature().getEvent(&evento);
  if (isnan(evento.temperature)) {
    bundleSensori.add("/T").add("n");
    Serial.println("Errore nella lettura della temperatura"); 
  }
  else {
    temperatura = evento.temperature;
    bundleSensori.add("/T").add(temperatura);
    Serial.println(temperatura); 
  }
  dht.humidity().getEvent(&evento);
  if (isnan(evento.relative_humidity)) {
    bundleSensori.add("/H").add("n");
    Serial.println("Errore nella lettura dell'umidita"); 
  }
  else {
    umidita = evento.relative_humidity;
    bundleSensori.add("/H").add(umidita);
    Serial.println(umidita); 
  }
}

//[3]What's the temperature of the earth?
void getTempEarth() {
  sensors.requestTemperatures();
  if (sensors.getTempCByIndex(0) == -127) {
    bundleSensori.add("/TH").add("n");
  }
  else {
    temperaturaTerra = sensors.getTempCByIndex(0);
    bundleSensori.add("/TH").add(temperaturaTerra);
  }
}

//[4]What's the env. light?
void getLight() {
  int luce = analogRead(A0);
  if (!isnan(analogRead(A0))) {
    bundleSensori.add("/L").add(luce);
  }
  else {
    bundleSensori.add("/L").add("n");
  }
}

//[5]What's the humidity of the earth?
void getMoisture() {
  int umidTerra = digitalRead(D3);
  if(umidTerra==1) {
    bundleSensori.add("/HE").add(0);
  }
  else if(umidTerra==0) {
    bundleSensori.add("/HE").add(1);
  }
  else {
    bundleSensori.add("/HE").add("n");
  }  
}

//[6]What's the level of the water hole?
void getWaterLevel() {
  int livelloSerbatoio = digitalRead(D4);
  if(livelloSerbatoio==1) {
    bundleSensori.add("/S").add(0);
    serbLED.off(); //Blynk app
  }
  else if(livelloSerbatoio==0) {
    bundleSensori.add("/S").add(1);
    serbLED.on(); //Blynk app
  }
  else {
    bundleSensori.add("/S").add("n");
  }  
}

void sendOSC() {
  getTempHum();
  getTempEarth();
  getRain();
  getLight();
  getMoisture();
  getWaterLevel();
  
  Udp.beginPacket(outIp, 7400);
  bundleSensori.send(Udp);
  Udp.endPacket();
  bundleSensori.empty();
}


//########################################################################################### MAIN ###########################################################################################
void loop() {

  Blynk.run();  //Blynk app
  timer.run(); 
  
  //ogni secondo UnitaSensori invia i propri dati a UnitaDiControllo
  delay(1000);
  sendOSC();
}