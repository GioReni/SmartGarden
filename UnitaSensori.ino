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

//Il terminale data del sensore è connesso al D1 del NodeMcu
#define ONE_WIRE_BUS D1
//Imposta la comunicazione oneWire per comunicare con un dispositivo compatibile 
OneWire oneWire(ONE_WIRE_BUS);
//Passaggio oneWire reference alla Dallas Temperature
DallasTemperature sensors(&oneWire);

#define DHTPIN D0
#define DHTTYPE DHT11
DHT_Unified dht(DHTPIN, DHTTYPE);

const char* ssid = "Esp8266";                                         //cambiare con ssid della propria Wi-Fi
const char* password = "*****";                                  	  //cambiare con password della propria Wi-Fi

WiFiUDP Udp;
unsigned int localUdpPort = 4230;                                     //cambiare con indirizzo IP che si vuole assegnare a questo NodeMcu (UnitaSensori)
char incomingPacket[UDP_TX_PACKET_MAX_SIZE];  
char replyPacekt[UDP_TX_PACKET_MAX_SIZE];

IPAddress ip(192, 168, 1, 14);                                        //cambiare con indirizzo IP che si vuole assegnare a questo NodeMcu (UnitaSensori)                                    
IPAddress gateway(192, 168, 1, 1);                                    //cambiare con gateway della propria rete
IPAddress subnet(255, 255, 255, 0);                                   //cambiare con subnet della propria rete

char * indirizzoUnitaDiControllo = "192.168.1.13";                    //cambiare con indirizzo IP del proprio NodeMcu UnitaDiControllo
int portaUnitaDiControllo = 4210;                                     //cambiare con porta del proprio NodeMcu unita di controllo


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
  WiFi.config(ip, gateway, subnet);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" connected");

  Udp.begin(localUdpPort);
  Serial.printf("Now listening at IP %s, UDP port %d\n", WiFi.localIP().toString().c_str(), localUdpPort);

  //sensore temperatura
  dht.begin();
  sensor_t sensore;
  //inizializzazione sensore Temperatura
  dht.temperature().getSensor(&sensore);
  //inizializzazione sensore Umidita'
  dht.humidity().getSensor(&sensore);
}


//########################################################################################## METHODS ##########################################################################################
//[1]Is it raining? yes/no
void getRain(char lista[UDP_TX_PACKET_MAX_SIZE]) {
  int pioggia = digitalRead(D2);
  if(pioggia==1) {
    strcat(lista, "R0");  //NON piove
  }
  else if(pioggia==0) {
    strcat(lista, "R1"); //piove
  }
  else {
    strcat(lista, "Rn"); //errore
  }
}

//[2]What's the air temperature and the humidity of the air?
void getTempHum(char lista[UDP_TX_PACKET_MAX_SIZE]) {
  sensors_event_t evento;
  dht.temperature().getEvent(&evento);
  char degree[5];
  if (isnan(evento.temperature)) {
    strcat(lista, "Tn");
  }
  else {
    int temperatura = evento.temperature;
    strcat(lista, "T");
    itoa(temperatura, degree, 10);
    strcat(lista, degree);  
  }
  dht.humidity().getEvent(&evento);
  char humidity[5];
  if (isnan(evento.relative_humidity)) {
    strcat(lista, "Hn");
  }
  else {
    int umidita = evento.relative_humidity;
    strcat(lista, "H");
    itoa(umidita, humidity, 10);
    strcat(lista, humidity);
  }
}

//[3]What's the temperature of the earth?
void getTempEarth(char lista[UDP_TX_PACKET_MAX_SIZE]) {
  char tempEarth[10];
  sensors.requestTemperatures();
  if (isnan(sensors.getTempCByIndex(0))) {
    strcat(lista, "THn");
  }
  else {
    double temperaturaTerra = sensors.getTempCByIndex(0);
    strcat(lista, "TH");
    dtostrf(temperaturaTerra, 2, 2, tempEarth);
    strcat(lista, tempEarth);
  }
}

//[4]What's the env. light?
void getLight(char lista[UDP_TX_PACKET_MAX_SIZE]) {
  char light[10];
  int luce = analogRead(A0);
  if (!isnan(analogRead(A0))) {
    strcat(lista, "L");
    itoa(luce, light, 10);
    strcat(lista, light);
  }
  else {
    strcat(lista, "Ln");
  }
}

//[5]What's the humidity of the earth?
void getMoisture(char lista[UDP_TX_PACKET_MAX_SIZE]) {
  int umidTerra = digitalRead(D3);
  if(umidTerra==1) {
    strcat(lista, "HE0");
  }
  else if(umidTerra==0) {
    strcat(lista, "HE1");
  }
  else {
    strcat(lista, "HEn");
  }  
}

//[6]What's the level of the water hole?
void getWaterLevel(char lista[UDP_TX_PACKET_MAX_SIZE]) {
  int livelloSerbatoio = digitalRead(D4);
  if(livelloSerbatoio==1) {
    strcat(lista, "S0");
  }
  else if(livelloSerbatoio==0) {
    strcat(lista, "S1");
  }
  else {
    strcat(lista, "Sn");
  }  
}

void clearArray(char lista[UDP_TX_PACKET_MAX_SIZE]) {
  for (int i = 0; i< sizeof(lista); i++) {
    lista[i]=(char)0;
  } 
}

void sendPacket(char pacchetto [8], char * indirizzo, int porta) {
  Udp.beginPacket(indirizzo, porta);
  Udp.write(pacchetto);
  Udp.endPacket();
}

//########################################################################################### MAIN ###########################################################################################
void loop() {
  //ogni secondo UnitaSensori invia i propri dati a UnitaDiControllo
  delay(1000);
  clearArray(replyPacekt);
  
  //sensore temp. + umid. dell'aria
  getTempHum(replyPacekt);
  
  //sensore temp. della terra
  getTempEarth(replyPacekt);

  //sensore pioggia
  getRain(replyPacekt);

  //sensore fotoresistenza
  getLight(replyPacekt);

  //sensore umidita' terra
  getMoisture(replyPacekt);
  
  //interruttore a galleggiante
  getWaterLevel(replyPacekt);
  
  //invio temperatura dell'aria + umidita' (pacchetto UDP) a UnitaDiControllo
  sendPacket(replyPacekt, indirizzoUnitaDiControllo, portaUnitaDiControllo);
}

