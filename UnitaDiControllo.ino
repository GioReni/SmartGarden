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
#include <LiquidCrystal_I2C.h>
#include <SimpleTimer.h>

LiquidCrystal_I2C lcd(0x3F, 20, 4);

WiFiUDP Udp;
unsigned int localUdpPort = 4210;                 //cambiare con porta che si vuole assegnare a questo NodeMcu (UnitaDiControllo)
char incomingPacket[UDP_TX_PACKET_MAX_SIZE];

IPAddress ip(192, 168, 1, 13);                    //cambiare con indirizzo IP che si vuole assegnare a questo NodeMcu (UnitaDiControllo)
IPAddress gateway(192, 168, 1, 1);                //cambiare con gateway della propria rete
IPAddress subnet(255, 255, 255, 0);               //cambiare con subnet della propria rete

const char* ssid = "Esp8266";                     //cambiare con ssid della propria Wi-Fi
const char* password = "*****";              	  //cambiare con password della propria Wi-Fi

char * luceOff = "L0";                            //comando spegni luce
char * luceOn = "L1";                             //comando accendi luce
char * pumpOff = "P0";                            //comando spegni pompa
char * pumpOn = "P1";                             //comando accendi pompa
char * evOff = "E0";                              //comando spegni elettrovalvola
char * evOn = "E1";                               //comando accnedi elettrovalvola

int tempoIrrPuls = 10;                            //cambiare a piacere se si vuole modificare tempo (in sec.) innaffiata con pulsante
int tempoIrrAuto = 300; //300 sec <--> 5 min      //cambiare a piacere se si vuole modificare tempo (in sec.) innaffiata automatica
int tempoLucePuls = 10;                           //cambiare a piacere se si vuole modificare tempo (in sec.) illuminazione con pulsante
int tempoLuceAuto = 300;                          //cambiare a piacere se si vuole modificare tempo (in sec.) illuminazione automatica

//indirizzo IP + Porta NodeMcu ATTUATORE
char * indirizzoRelaisGiardino = "192.168.1.15";  //cambiare con indirizzo IP del proprio NodeMcu attuatore
int portaRelaisGiardino = 4240;                   //cambiare con porta del proprio NodeMcu attuatore

//indirizzo IP + Porta NodeMcu SENSORE
char * indirizzoGiardino = "192.168.1.14";        //cambiare con indirizzo IP del proprio NodeMcu sensore
int portaGiardino = 4230;                         //cambiare con porta del proprio NodeMcu sensore

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
//variabile tipo luce
char illuminazione [] = "L";
//variabile pulsante irrigazione temporanea
int letturaPulsanteAcqua;
//variabile pulsante illuminazione temporanea
int letturaPulsanteLuce;
//variabile pulsante STOP
int letturaPulsanteStop;
//variabile comando
char comando [UDP_TX_PACKET_MAX_SIZE];
//variabile per convertire gli interi
char temp [15];
//variabile temporanea numero
int numero;


//######################################################################################### METHODS 1 #########################################################################################
void printIllOff() {
  lcd.setCursor(0,3);
  lcd.print("Luce:");
  lcd.print("OFF   ");
}

void printAcquaOff() {
  lcd.setCursor(11,3);
  lcd.print("Acqua:");
  lcd.print("OFF");
}

void printIllIrrOff() {
  printIllOff();
  printAcquaOff();
}


//########################################################################################### SETUP ###########################################################################################
void setup() {
  //pin LED pioggia
  pinMode(pinGiallo, OUTPUT);
  pinMode(pinBlu, OUTPUT);

  //pin bottoni STOP, IRRIGAZIONE, LUCE
  pinMode(bottonePompa, INPUT_PULLUP);
  pinMode(bottoneStop, INPUT_PULLUP);
  pinMode(bottoneLuce, INPUT);
  
  //pin LED serbatoio
  pinMode(pinVerde, OUTPUT);
  pinMode(pinRosso, OUTPUT);
    
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

  //settaggio display LCD
  lcd.begin(20,4);
  lcd.init();
  lcd.backlight();

  printIllIrrOff();
}


//########################################################################################## METHODS ##########################################################################################
void sendPacket(char pacchetto [8], char * indirizzo, int porta) {
  Udp.beginPacket(indirizzo, porta);
  Udp.write(pacchetto);
  Udp.endPacket();
}

void printTemp(int i) {
  lcd.setCursor(0,0);
  lcd.print("Temp:");
  lcd.print(String(i));
  lcd.print((char)223);
  lcd.print("C");
}

void printHum(int i) {
  lcd.print("  Umid:");
  lcd.print(String(i));
  lcd.print("%");
}

void printTempTerra(double d) {
  lcd.setCursor(0,1);
  lcd.print("Temp Terra:");
  lcd.print(String(d));
  lcd.print((char)223);
  lcd.print("C");
}

void printUmidTerra(int i) {
  lcd.setCursor(0,2);
  if(i==1) {
     lcd.print("Terreno Umido");
   }
   else if(i==0) {
     lcd.print("Terreno Arido");
   }
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

void spegniTutto() {
  sendPacket(luceOff, indirizzoRelaisGiardino, portaRelaisGiardino);
  sendPacket(pumpOff, indirizzoRelaisGiardino, portaRelaisGiardino);
  sendPacket(evOff, indirizzoRelaisGiardino, portaRelaisGiardino);
}

void clearArray(char lista[UDP_TX_PACKET_MAX_SIZE]) {
  for (int i = 0; i< sizeof(lista); i++) {
    lista[i]=(char)0;
  } 
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


//########################################################################################### MAIN ###########################################################################################
void loop() {
  String ricevuta;
  
  //ricezione lettura sensori da UnitaSensori and UnitaAttuatori
  delay(100);
  int packetSize = Udp.parsePacket();
  
  if (packetSize) 
  {
    //Serial.printf("Received %d bytes from %s, port %d\n", packetSize, Udp.remoteIP().toString().c_str(), Udp.remotePort());
    int len = Udp.read(incomingPacket, UDP_TX_PACKET_MAX_SIZE);
    if (len > 0)
    {
      incomingPacket[len] = 0;
      ricevuta = String(incomingPacket);
      //ricezione dati sensori da UnitaSensori
      if (ricevuta.indexOf("T") != -1) {
        temperatura = ricevuta.substring(ricevuta.indexOf("T")+1, ricevuta.indexOf("H")).toInt();
        umidita = ricevuta.substring(ricevuta.indexOf("H")+1, ricevuta.indexOf("TH")).toInt();
        temperaturaTerra = atof(ricevuta.substring(ricevuta.indexOf("TH")+2, ricevuta.indexOf("R")).c_str());
        pioggia = ricevuta.substring(ricevuta.indexOf("R")+1, ricevuta.indexOf("L")).toInt();
        luce = ricevuta.substring(ricevuta.indexOf("L")+1, ricevuta.indexOf("HE")).toInt();
        umiditaTerra = ricevuta.substring(ricevuta.indexOf("HE")+2, ricevuta.indexOf("S")).toInt();
        serbatoio = ricevuta.substring(ricevuta.indexOf("S")+1).toInt();
        printTemp(temperatura);
        printHum(umidita);
        printTempTerra(temperaturaTerra);
        printUmidTerra(umiditaTerra);
        ledPioggia(pioggia);
        ledSerbatoio(serbatoio);
      }

      //notifica di spegnimento acqua da UnitaAttuatori
      else if (ricevuta.indexOf("W") != -1) {
        printAcquaOff();
      }
      //notifica di spegnimento luce da UnitaAttuatori
      else if (ricevuta.indexOf("i") != -1) {
        printIllOff();
      }
    }
     Serial.printf("UDP packet contents: %s\n", incomingPacket);
     clearArray(incomingPacket);
  }

  //lettura pulsante STOP
  letturaPulsanteStop = analogRead(bottoneStop);

  //lettura pulsante pompa
  letturaPulsanteAcqua = digitalRead(bottonePompa);

  //lettura pulsante luce
  letturaPulsanteLuce = digitalRead(bottoneLuce);

  if (pioggia == 1 || letturaPulsanteStop == 1024) {
    spegniTutto();
    printIllIrrOff();
  }
  else {
    if (serbatoio == 1 && strcmp(irrigatore,"E")==0) {
      strncpy(irrigatore, "P", 3);
      sendPacket(irrigatore, indirizzoRelaisGiardino, portaRelaisGiardino); 
    }
    else if (serbatoio == 0 && strcmp(irrigatore,"P")==0) {
           strncpy(irrigatore, "E", 3);
           sendPacket(irrigatore, indirizzoRelaisGiardino, portaRelaisGiardino);  
    }

    //pulsante irrigazione premuto <--> HIGH pressed, LOW released
    if (letturaPulsanteAcqua == HIGH) {
      numero = 1;
      clearArray(temp);
      clearArray(comando);
      strncpy(comando, irrigatore, 1);
      itoa(numero, temp, 10);
      strcat(comando, temp);
      itoa(tempoIrrPuls, temp, 10);
      strcat(comando, temp);
      sendPacket(comando, indirizzoRelaisGiardino, portaRelaisGiardino);
      printAcquaOn();
    }
    
    //condizioni di secco
    if ((umiditaTerra == 0) && (temperaturaTerra > 8.0) && (temperatura > 5.0) && (luce > 950) && (umidita < 80)) {
      numero = 1;
      clearArray(temp);
      clearArray(comando);
      strncpy(comando, irrigatore, 1);
      itoa(numero, temp, 10);
      strcat(comando, temp);
      itoa(tempoIrrAuto, temp, 10);
      strcat(comando, temp);
      sendPacket(comando, indirizzoRelaisGiardino, portaRelaisGiardino);
      printAcquaOn(); 
    }

    //pulsante luce premuto <--> HIGH pressed, LOW released
    if(letturaPulsanteLuce == HIGH) {
      numero = 1;
      clearArray(temp);
      clearArray(comando);
      strncpy(comando, illuminazione, 1);
      itoa(numero, temp, 10);
      strcat(comando, temp);
      itoa(tempoLucePuls, temp, 10);
      strcat(comando, temp);
      sendPacket(comando, indirizzoRelaisGiardino, portaRelaisGiardino);
      printIllOn();
    }

    //condizioni di illuminazione
    if ((temperaturaTerra <= 8.0) && (temperatura <= 5) && (luce >= 950)) {
      numero = 1;
      clearArray(temp);
      clearArray(comando);
      strncpy(comando, illuminazione, 1);
      itoa(numero, temp, 10);
      strcat(comando, temp);
      itoa(tempoLuceAuto, temp, 10);
      strcat(comando, temp);
      sendPacket(comando, indirizzoRelaisGiardino, portaRelaisGiardino);
      printIllOn();
    }
  }
  
}