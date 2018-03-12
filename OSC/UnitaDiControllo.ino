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

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <LiquidCrystal_I2C.h>
#include <SimpleTimer.h>
#include <SPI.h>
#include <BlynkSimpleEsp8266.h>
#include <OSCBundle.h>
#include <OSCBoards.h>


LiquidCrystal_I2C lcd(0x3F, 20, 4);

char auth[] = "***"; //Blynk app

const byte MAX_MSG_SIZE PROGMEM=150;
byte packetBuffer[MAX_MSG_SIZE];
WiFiUDP Udp;


IPAddress ip(192, 168, 1, 35);                     //cambiare con indirizzo IP che si vuole assegnare a questo NodeMcu (UnitaDiControllo)
IPAddress gateway(192, 168, 1, 254);               //cambiare con gateway della propria rete
IPAddress subnet(255, 255, 255, 0);                //cambiare con subnet della propria rete

const char* ssid = "OuterRim";                     //cambiare con ssid della propria Wi-Fi
const char* password = "****";               	   //cambiare con password della propria Wi-Fi
IPAddress outIp(192, 168, 1, 36);                  //cambiare con indirizzo IP del NodeMcu UnitaAttuatori

int tempoIrrPuls = 10;                            //cambiare a piacere se si vuole modificare tempo (in sec.) innaffiata con pulsante
int tempoIrrAuto = 300; //300 sec <--> 5 min      //cambiare a piacere se si vuole modificare tempo (in sec.) innaffiata automatica
int tempoLucePuls = 10;                           //cambiare a piacere se si vuole modificare tempo (in sec.) illuminazione con pulsante
int tempoLuceAuto = 300;                          //cambiare a piacere se si vuole modificare tempo (in sec.) illuminazione automatica


//pin LED pioggia
int pinGiallo = D5;
int pinBlu = D6;

//pin LED serbatoio
int pinVerde =  D4;
int pinRosso = D7;

//pin bottone lucePianta
int bottoneLuce = D8;

//pin bottone pompa
int bottonePompa = D0;

//pin bottone STOP
int bottoneStop = A0;

//variabile pioggia (0 --> NON piove)
int pioggia = 0;
//variabile serbatoio (0 --> vuoto)
int serbatoio = 0;
//variabile umidita' terra
int umiditaTerra;
//variabile temperatura terra
double temperaturaTerra;
//variabile temperatura aria
int temperatura;
//variabile luce env.
int luce;
//variabile umidita aria
int umidita;
//variabile tipo irrigatore
char irrigatore [] = "P";
//variabile pulsante irrigazione temporanea
int letturaPulsanteAcqua = LOW;
//variabile pulsante illuminazione temporanea
int letturaPulsanteLuce = LOW;
//variabile pulsante STOP
int letturaPulsanteStop = 0;


//####################################################################################### FUNCTIONS 1 #######################################################################################
void printIllOff(OSCMessage &msg, int addrOffset) {
  lcd.setCursor(0,3);
  lcd.print("Luce:");
  lcd.print("OFF   ");
}

void printAcquaOff(OSCMessage &msg, int addrOffset) {
  lcd.setCursor(11,3);
  lcd.print("Acqua:");
  lcd.print("OFF");
}

void printIllIrrOff() {
  lcd.setCursor(0,3);
  lcd.print("Luce:");
  lcd.print("OFF   ");
  lcd.setCursor(11,3);
  lcd.print("Acqua:");
  lcd.print("OFF");
}


//########################################################################################### SETUP ###########################################################################################
void setup() {
  //pin LED pioggia
  pinMode(pinGiallo, OUTPUT);
  pinMode(pinBlu, OUTPUT);

  //pin bottoni STOP, IRRIGAZIONE, LUCE
  pinMode(bottonePompa, INPUT);
  pinMode(bottoneStop, INPUT);
  pinMode(bottoneLuce, INPUT);
  
  //pin LED serbatoio
  pinMode(pinVerde, OUTPUT);
  pinMode(pinRosso, OUTPUT);

  Blynk.begin(auth, ssid, password);
  Serial.begin(9600);
  WiFi.mode(WIFI_STA);
  WiFi.config(ip, gateway, subnet);
  WiFi.begin(ssid, password);
  Serial.println();
  Serial.printf("Connecting to %s ", ssid);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" connected");

  Serial.printf("Now listening at IP %s", WiFi.localIP().toString().c_str());
  Udp.begin(7400);
  
  //settaggio display LCD
  lcd.begin(20,4);
  lcd.init();
  lcd.backlight();

  printIllIrrOff();
}


//######################################################################################## FUNCTIONS 2 ########################################################################################
void printTemp(int i) {
  lcd.setCursor(0,0);
  lcd.print("Temp:");
  lcd.print(String(i));
  lcd.print((char)223);
  lcd.print("C  ");
}

void printErrTemp() {
  lcd.setCursor(0,0);
  lcd.print("Temp:N.D.  ");
}

void printHum(int i) {
  lcd.setCursor(11,0);
  lcd.print("Umid:");
  lcd.print(String(i));
  lcd.print("% ");
}

void printErrHum() {
  lcd.setCursor(11,0);
  lcd.print("Umid:N.D.");
}

void printTempTerra(double d) {
  lcd.setCursor(0,1);
  lcd.print("Temp Terra:");
  lcd.print(String(d));
  lcd.print((char)223);
  lcd.print("C");
}

void printErrTempTerra() {
  lcd.setCursor(0,1);
  lcd.print("Temp Terra:N.D.     ");
}

void printUmidTerra(int i) {
  lcd.setCursor(0,2);
  if(i==1) {
     lcd.print("Terreno Umido ");
   }
   else if(i==0) {
     lcd.print("Terreno Arido ");
   }
}

void printErrUmidTerra() {
  lcd.setCursor(0,2);
  lcd.print("Errore lettura");
}

void ledPioggia(int i) {
  if(i==1) {
     digitalWrite(pinBlu, HIGH);
     analogWrite(pinGiallo, LOW);
   }
   else if(i==0) {
     digitalWrite(pinBlu, LOW);
     analogWrite(pinGiallo, HIGH);
   }
}

void ledErrPioggia() {
  digitalWrite(pinBlu, HIGH);
  analogWrite(pinGiallo, HIGH);
}

void ledSerbatoio(int i) {
  if(i==0) {
    digitalWrite(pinVerde, LOW);
    digitalWrite(pinRosso, HIGH);
   }
   else {
     digitalWrite(pinVerde, HIGH);
     digitalWrite(pinRosso, LOW);
   }
}

void ledErrSerbatoio() {
  digitalWrite(pinRosso, HIGH);
  digitalWrite(pinVerde, HIGH);
}

void printIllOn() {
  lcd.setCursor(0,3);
  lcd.print("Luce:");
  lcd.print("ON    ");
}

void printAcquaOn() {
  lcd.setCursor(11,3);
  lcd.print("Acqua:");
  lcd.print("ON ");
}

void leggiTemperatura(OSCMessage &msg, int addrOffset) {
  if (msg.isString(0) == true) {
    printErrTemp();
  }
  else {
    temperatura = msg.getInt(0);
    Serial.print("Temperatura aria:");
    Serial.println(temperatura);
    printTemp(temperatura);
  }
}

void leggiUmidita(OSCMessage &msg, int addrOffset) {
  if (msg.isString(0) == true) {
    printErrHum();
  }
  else {
    umidita = msg.getInt(0);
    Serial.print("Umidita aria:");
    Serial.println(umidita);
    printHum(umidita);
  }
}

void leggiTemperaturaTerra(OSCMessage &msg, int addrOffset) {
  if (msg.isString(0) == true) {
    printErrTempTerra();
  }
  else {
    temperaturaTerra = msg.getDouble(0);
    Serial.print("Temperatura terra:");
    Serial.println(temperaturaTerra);
    printTempTerra(temperaturaTerra);
  }
}

void leggiPioggia(OSCMessage &msg, int addrOffset) {
  if (msg.isString(0) == true) {
    ledErrPioggia();
  }
  else {
    pioggia = msg.getInt(0);
    ledPioggia(pioggia);
    Serial.print("Pioggia: ");
    Serial.println(pioggia);
  }
}

void leggiLuce(OSCMessage &msg, int addrOffset) {
  if (msg.isString(0) == false) {
    luce = msg.getInt(0);
    Serial.print("Luce: ");
    Serial.println(luce);
  }
}

void leggiUmiditaTerra(OSCMessage &msg, int addrOffset) {
  if (msg.isString(0) == true) {
    printErrUmidTerra();
  }
  else {
    umiditaTerra = msg.getInt(0);
    printUmidTerra(umiditaTerra);
    Serial.print("Umidita Terra: ");
    Serial.println(umiditaTerra);
  }
}

void leggiLivelloSerbatoio(OSCMessage &msg, int addrOffset) {
  if (msg.isString(0) == true) {
    ledErrSerbatoio();
  }
  else {
    serbatoio = msg.getInt(0);
    ledSerbatoio(serbatoio);
    Serial.print("Livello serbatoio: ");
    Serial.println(serbatoio);
  }
}

void receiveOSC() {
    //Serial.println("osc...");
    OSCBundle bundleINSensori;
    int size;
    if((size = Udp.parsePacket())>0) {
        Serial.println("received");
        Serial.printf("Received %d bytes from %s, port %d\n", size, Udp.remoteIP().toString().c_str(), Udp.remotePort());
 
        Udp.read(packetBuffer,size);
        bundleINSensori.fill(packetBuffer,size);
 
        if(!bundleINSensori.hasError()) {
          Serial.println("Routing");
          bundleINSensori.route("/T", leggiTemperatura);
          bundleINSensori.route("/H", leggiUmidita);
          bundleINSensori.route("/TH", leggiTemperaturaTerra);
          bundleINSensori.route("/R", leggiPioggia);
          bundleINSensori.route("/L", leggiLuce);
          bundleINSensori.route("/HE", leggiUmiditaTerra);
          bundleINSensori.route("/S", leggiLivelloSerbatoio);
          bundleINSensori.route("/WOFF", printAcquaOff);
          bundleINSensori.route("/LOFF", printIllOff);
        }
        Udp.flush();
    }
}

void sendOSCMessage(char * nome) {
  OSCMessage msg(nome);
  Udp.beginPacket(outIp, 7400);
  msg.send(Udp);
  Udp.endPacket();
  msg.empty();
}

void sendAccendiAttuatore(int tempo, char * nome) {
  OSCMessage msg(nome);
  msg.add(tempo);
  Udp.beginPacket(outIp, 7400);
  msg.send(Udp);
  Udp.endPacket();
  msg.empty();
}

//lettura pulsante acqua con Blynk app
BLYNK_WRITE(V5) {
  int pinValue = param.asInt();
  if(pinValue==1){
    letturaPulsanteAcqua = HIGH;
  }
}

//lettura pulsante lampada con Blynk app
BLYNK_WRITE(V6) {
  int pinValue = param.asInt();
  if(pinValue==1){
    letturaPulsanteLuce = HIGH;
  }
}

//lettura pulsante stop con Blynk app
BLYNK_WRITE(V7) {
  int pinValue = param.asInt();
  if(pinValue==1){
    letturaPulsanteStop = 1024;
  }
}


//########################################################################################### MAIN ###########################################################################################
void loop() {
  String ricevuta;

  Blynk.run();
  
  //ricezione lettura sensori da UnitaSensori and UnitaAttuatori
  delay(100);

  receiveOSC(); 
  

  //lettura pulsante STOP
  if (letturaPulsanteStop < 1024) {
    letturaPulsanteStop = analogRead(bottoneStop);
  }

  //lettura pulsante pompa
  if (letturaPulsanteAcqua == LOW) {
    letturaPulsanteAcqua = digitalRead(bottonePompa);
  }

  //lettura pulsante luce
  if (letturaPulsanteLuce == LOW) {
    letturaPulsanteLuce = digitalRead(bottoneLuce);
  }

  if (pioggia == 1 || letturaPulsanteStop == 1024) {
    sendOSCMessage("/L0");
    sendOSCMessage("/P0");
    sendOSCMessage("/E0");
    printIllIrrOff();
  }
  else {
    if (serbatoio == 1 && strcmp(irrigatore,"E")==0) {
      strncpy(irrigatore, "P", 3);
      sendOSCMessage("/P");
    }
    else if (serbatoio == 0 && strcmp(irrigatore,"P")==0) {
           strncpy(irrigatore, "E", 3);
           sendOSCMessage("/E"); 
    }

    //pulsante irrigazione premuto <--> HIGH pressed, LOW released
    if (letturaPulsanteAcqua == HIGH) {
      if (strcmp(irrigatore,"P")==0) {
        sendAccendiAttuatore(tempoIrrPuls, "/P1");
      }
      else {
        sendAccendiAttuatore(tempoIrrPuls, "/E1");
      }
      printAcquaOn();
    }
       
    //condizioni di secco
    if ((umiditaTerra == 0) && (temperaturaTerra > 8.0) && (temperatura > 5.0) && (luce > 950) && (umidita < 80)) {
      if (strcmp(irrigatore,"P")==0) {
        sendAccendiAttuatore(tempoIrrAuto, "/P1");
      }
      else {
        sendAccendiAttuatore(tempoIrrAuto, "/E1");
      }
      printAcquaOn(); 
    }
    
    //pulsante luce premuto <--> HIGH pressed, LOW released
    if(letturaPulsanteLuce == HIGH) {
      sendAccendiAttuatore(tempoLucePuls, "/L1");
      printIllOn();
    }

    //condizioni di illuminazione
    if ((temperaturaTerra <= 8.0) && (temperatura <= 5) && (luce >= 950)) {
      sendAccendiAttuatore(tempoLuceAuto, "/L1");
      printIllOn();
    }
  }

  //reset pulsanti Blynk app
  letturaPulsanteAcqua = LOW;
  letturaPulsanteLuce = LOW;
  letturaPulsanteStop = 0;
}