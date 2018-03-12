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
#include <OSCBundle.h>
#include <OSCBoards.h>

const byte MAX_MSG_SIZE PROGMEM=100;
byte packetBuffer[MAX_MSG_SIZE];
WiFiUDP Udp;

IPAddress ip(192, 168, 1, 36);                                            //cambiare con indirizzo IP che si vuole assegnare a questo NodeMcu (UnitaAttuatori)
IPAddress gateway(192, 168, 1, 254);                                      //cambiare con gateway della propria rete
IPAddress subnet(255, 255, 255, 0);                                       //cambiare con subnet della propria rete

const char* ssid = "OuterRim";                                            //cambiare con ssid della propria Wi-Fi
const char* password = "***";                                      	  	  //cambiare con password della propria Wi-Fi
IPAddress outIp(192, 168, 1, 35);                                         //cambiare con indirizzo IP del NodeMcu UnitaDiControllo

int lucePianta = D0;
int relaisPompa = D1;
int relaisElettrovalvola = D2;

int tempoInizioIrr = 0;
int tempoDurataIrr = 0;
boolean innaffia = false;
boolean illumina = false;
int tempoInizioIll = 0;
int tempoDurataIll = 0;

char auth [] = "****";

OSCBundle bundleAttuatori;
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
  WiFi.mode(WIFI_STA);
  WiFi.config(ip, gateway, subnet);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" connected");

  Serial.printf("Now listening at IP %s", WiFi.localIP().toString().c_str());
  Udp.begin(7400);
}


//######################################################################################### FUNCTIONS #########################################################################################
//HIGH su relais pompa + elettrovalvola, la spegne
//LOW su relais luce, la spegne

void spegniPompa(OSCMessage &msg, int addrOffset) {
  digitalWrite(relaisPompa, HIGH);
  innaffia = false;
  tempoDurataIrr = 0;
  Serial.println("P0");
}

void spegniElettrovalvola(OSCMessage &msg, int addrOffset) {
  digitalWrite(relaisElettrovalvola, HIGH);
  innaffia = false;
  tempoDurataIrr = 0;
  Serial.println("E0"); 
}

void spegniLuce(OSCMessage &msg, int addrOffset) {
  digitalWrite(lucePianta, LOW);
  illumina = false;
  tempoDurataIll = 0;
  Serial.println("L0"); 
  Blynk.notify("L'impianto di irrigazione e di illuminazione si sono arrestati per STOP o PIOGGIA");
}

void accendiPompa(OSCMessage &msg, int addrOffset) {
  if (innaffia == false) {
      innaffia = true;
      tempoInizioIrr = millis();
      tempoDurataIrr = tempoInizioIrr + (msg.getInt(0)*1000);
      digitalWrite(relaisPompa, LOW);
      Blynk.notify("L'impianto di irrigazione si e' acceso");
      Serial.print("P1");
      Serial.println(msg.getInt(0));
  }
}

void accendiElettrovalvola(OSCMessage &msg, int addrOffset) {
  if (innaffia == false) {
      innaffia = true;
      tempoInizioIrr = millis();
      tempoDurataIrr = tempoInizioIrr + (msg.getInt(0)*1000);
      digitalWrite(relaisElettrovalvola, LOW);
      Blynk.notify("L'impianto di irrigazione si e' acceso");
      Serial.print("E1");
      Serial.println(msg.getInt(0));
  }
}

void accendiLuce(OSCMessage &msg, int addrOffset) {
  if (illumina == false) {
      illumina = true;
      tempoInizioIll = millis();
      tempoDurataIll = tempoInizioIll + (msg.getInt(0)*1000);
      digitalWrite(lucePianta, HIGH);
      Blynk.notify("L'impianto di illuminazione si e' acceso");
      Serial.print("L1");
      Serial.println(msg.getInt(0));
  }
}

void switchToElettr(OSCMessage &msg, int addrOffset) {
  if (innaffia == true) {
      digitalWrite(relaisPompa, HIGH);
      digitalWrite(relaisElettrovalvola, LOW);
      Serial.println("E");
  }
}

void switchToPump(OSCMessage &msg, int addrOffset) {
  if (innaffia == true) {
      digitalWrite(relaisPompa, LOW);
      digitalWrite(relaisElettrovalvola, HIGH);
      Serial.println("P");
  }
}

void receiveOSC() {
    //Serial.println("osc...");
    OSCMessage messageIN;
    int size;
    if( (size = Udp.parsePacket())>0) {
        //Serial.println("received");
        //Serial.printf("Received %d bytes from %s, port %d\n", size, Udp.remoteIP().toString().c_str(), Udp.remotePort());
 
        Udp.read(packetBuffer,size);
        messageIN.fill(packetBuffer,size);
 
        if(!messageIN.hasError()) {
            messageIN.route("/L0", spegniLuce);
            messageIN.route("/P0", spegniPompa);
            messageIN.route("/E0", spegniElettrovalvola);
            messageIN.route("/P1", accendiPompa);
            messageIN.route("/E1", accendiElettrovalvola);
            messageIN.route("/L1", accendiLuce);
            messageIN.route("/E", switchToElettr);
            messageIN.route("/P", switchToPump);
        }
        Udp.flush();
    }
}

void sendOSC() {
  Udp.beginPacket(outIp, 7400);
  bundleAttuatori.send(Udp);
  Udp.endPacket();
  bundleAttuatori.empty();
}


//########################################################################################### MAIN ###########################################################################################
void loop() {
  Blynk.run();
  delay(50);
  receiveOSC();
  
  if (tempoDurataIrr != 0 && millis() > tempoDurataIrr) {
    digitalWrite(relaisPompa, HIGH);
    digitalWrite(relaisElettrovalvola, HIGH);
    innaffia = false;
    bundleAttuatori.add("/WOFF");
    sendOSC();
    tempoDurataIrr = 0;
    Blynk.notify("L'impianto di irrigazione si e' spento automaticamente");
  }
  if(tempoDurataIll != 0 && millis() > tempoDurataIll) {
    digitalWrite(lucePianta, LOW);
    illumina = false;
    bundleAttuatori.add("/LOFF");
    sendOSC();
    tempoDurataIll = 0;
    Blynk.notify("L'impianto di illuminazione si e' spento automaticamente");
  }

}
