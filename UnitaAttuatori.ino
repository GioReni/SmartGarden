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


#define BLYNK_PRINT Serial
#include <BlynkSimpleEsp8266.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

WiFiUDP Udp;
unsigned int localUdpPort = 4240;                                         //cambiare con porta del proprio NodeMcu UnitaAttuatori
char incomingPacket[UDP_TX_PACKET_MAX_SIZE];

IPAddress ip(192, 168, 1, 36);                                            //cambiare con indirizzo IP che si vuole assegnare a questo NodeMcu (UnitaAttuatori)
IPAddress gateway(192, 168, 1, 254);                                      //cambiare con gateway della propria rete
IPAddress subnet(255, 255, 255, 0);                                       //cambiare con subnet della propria rete

const char* ssid = "ArduinoAfternoon";                                            //cambiare con ssid della propria Wi-Fi
const char* password = "***"; 	                                          //cambiare con password della propria Wi-Fi

char * indirizzoUnitaDiControllo = "192.168.1.35";                        //cambiare con indirizzo IP del proprio NodeMcu UnitaDiControllo
int portaUnitaDiControllo = 4210;                                         //cambiare con porta del proprio NodeMcu UnitaDiControllo

int lucePianta = D0;
int relaisPompa = D1;
int relaisElettrovalvola = D2;

//variabile spegni acqua
char spegniAcqua[] = "Water";
//variabile spegni luce
char spegniLuce[] = "Light";

int tempoInizioIrr = 0;
int tempoDurataIrr = 0;
boolean innaffia = false;
boolean illumina = false;
int tempoInizioIll = 0;
int tempoDurataIll = 0;

char auth [] = "****";


//########################################################################################### SETUP ###########################################################################################
void setup() {
  Blynk.begin(auth, ssid, password);
  //settaggio pin luce pianta
  pinMode(lucePianta, OUTPUT);
  digitalWrite(lucePianta, LOW); //setta a OFF la luce, inizialmente

  //settaggio pin pompa
  pinMode(relaisPompa, OUTPUT);
  digitalWrite(relaisPompa, HIGH); //setta a OFF la pompa, inizialmente

  //settaggio pin elettrovalvola
  pinMode(relaisElettrovalvola, OUTPUT);
  digitalWrite(relaisElettrovalvola, HIGH); //setta a OFF l'elettrovalvola, inizialmente
  
  Serial.begin(9600);
  Serial.println();
  Serial.printf("Connecting to %s ", ssid);
  //WiFi.mode(WIFI_STA);
  WiFi.config(ip, gateway, subnet);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" connected");

  //Serial.print("Indirizzo IP UnitaAttuatori: ");
  //Serial.pritnln(WiFi.localIP());
  Udp.begin(localUdpPort);
  Serial.printf("Now listening at IP %s, UDP port %d\n", WiFi.localIP().toString().c_str(), localUdpPort);
}


//######################################################################################### FUNCTIONS #########################################################################################
void sendPacket(char pacchetto [8], char * indirizzo, int porta) {
  Udp.beginPacket(indirizzo, porta);
  Udp.write(pacchetto);
  Udp.endPacket();
}


//########################################################################################### MAIN ###########################################################################################
void loop() {
  Blynk.run();
  //attesa richiesta scatto relais
  String ricevuta;
  int packetSize = Udp.parsePacket();
  if (packetSize > 0) {
   //receive incoming UDP packets
   //Serial.printf("Received %d bytes from %s, port %d\n", packetSize, Udp.remoteIP().toString().c_str(), Udp.remotePort());
   Udp.read(incomingPacket, packetSize);
   Serial.printf("UDP packet contents: %s\n", incomingPacket);
   ricevuta = String(incomingPacket);
    
   //HIGH su relais pompa + elettrovalvola, la spegne
   //LOW su relais luce, la spegne
   if (ricevuta.indexOf("L0") != -1) {
      digitalWrite(lucePianta, LOW);
      illumina = false;
      tempoDurataIll = 0;
      Blynk.notify("L'impianto di irrigazione e di illuminazione si sono arrestati per STOP o PIOGGIA");
   }
   if (ricevuta.indexOf("P0") != -1) {
      digitalWrite(relaisPompa, HIGH);
      innaffia = false;
      tempoDurataIrr = 0;
   }
   if (ricevuta.indexOf("E0") != -1) {
      digitalWrite(relaisElettrovalvola, HIGH);
      innaffia = false;
      tempoDurataIrr = 0;
   }
   else if ((ricevuta.indexOf("P1") != -1) && innaffia == false) {
      innaffia = true;
      tempoInizioIrr = millis();
      tempoDurataIrr = tempoInizioIrr + (ricevuta.substring(ricevuta.indexOf("P1")+2).toInt()*1000);
      digitalWrite(relaisPompa, LOW);
      Blynk.notify("L'impianto di irrigazione si e' acceso");
    }
    else if ((ricevuta.indexOf("E1") != -1) && innaffia == false) {
      innaffia = true;
      tempoInizioIrr = millis();
      tempoDurataIrr = tempoInizioIrr + (ricevuta.substring(ricevuta.indexOf("E1")+2).toInt()*1000);
      digitalWrite(relaisElettrovalvola, LOW);
      Blynk.notify("L'impianto di irrigazione si e' acceso");
    }
    else if (ricevuta.indexOf("L1") != -1 && illumina == false) {
      illumina = true;
      tempoInizioIll = millis();
      tempoDurataIll = tempoInizioIll + (ricevuta.substring(ricevuta.indexOf("L1")+2).toInt()*1000);
      digitalWrite(lucePianta, HIGH);
      Blynk.notify("L'impianto di illuminazione si e' acceso");
    }
    else if (ricevuta.indexOf("E") != -1 && innaffia == true) {
      digitalWrite(relaisPompa, HIGH);
      digitalWrite(relaisElettrovalvola, LOW);
    }
    else if (ricevuta.indexOf("P") != -1 && innaffia == true) {
      digitalWrite(relaisPompa, LOW);
      digitalWrite(relaisElettrovalvola, HIGH);
    }

  } //chiusura if(packetSize > 0)

    if (tempoDurataIrr != 0 && millis() > tempoDurataIrr) {
      digitalWrite(relaisPompa, HIGH);
      digitalWrite(relaisElettrovalvola, HIGH);
      innaffia = false;
      sendPacket(spegniAcqua, indirizzoUnitaDiControllo, portaUnitaDiControllo);
      tempoDurataIrr = 0;
      Blynk.notify("L'impianto di irrigazione si e' spento automaticamente");
    }
    if(tempoDurataIll != 0 && millis() > tempoDurataIll) {
      digitalWrite(lucePianta, LOW);
      illumina = false;
      sendPacket(spegniLuce, indirizzoUnitaDiControllo, portaUnitaDiControllo);
      tempoDurataIll = 0;
      Blynk.notify("L'impianto di illuminazione si e' spento automaticamente");
    }
    
}
