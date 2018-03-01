# Progetto SmartGarden - Corso Sistemi Embedded (Università degli Studi di Milano, Anno 2017-2018)

## Autori:
Alessandro Gigliotti, Giovanni Reni 
## Board utilizzate:
NodeMCU ESP8266


### Descrizione generale:
In questo progetto, abbiamo realizzato un sistema di gestione intelligente di un piccolo orto o giardino. È presente un impianto d'irrigazione, basato sul controllo di ciò che accade e quindi di come si modifica, l'ambiente circostante. L'impianto entra infatti autonomamente in azione, quando si verificano determinate condizioni (come secchezza del terreno o assenza di pioggia), tenendo costantemente sotto controllo varie informazioni sull'aria, terra, meteo e sullo stato degli strumenti d'irrigazioni utilizzati. L'irrigazione può entrare in funzione, utilizzando una cisterna che raccoglie l'acqua piovana, oppure tramite un impianto idraulico. È altresì possibile attivare l'irrigazione anche manualmente da un utente tramite un pulsante fisico.
Le informazioni rilevate tramite i sensori in giardino, vengono inviate e visualizzate su un display, attaccato ad un'altra board, posizionato in un luogo chiuso (ad esempio in casa). Affianco al display, ci saranno anche 4 led, che segnalano la presenza/assenza di pioggia, la presenza/assenza di sole, lo stato del serbatoio pieno o vuoto. Ci sono infine 3 pulsanti fisici, posizionati accanto al display, per azionare manualmente l'irrigazione, per accendere una lampada per illuminare e un tasto per arrestare entrambe queste attività.
Inoltre il sistema sfrutta un'applicazione mobile, dalla quale sono consultabili tutte le informazioni lette dai sensori e lo stato della cisterna. Sono presenti inoltre 3 pulsanti virtuali, gemelli di quelli fisici, per azionare gli strumenti del progetto da un dispositivo. Infine l'app invia anche notifiche, relative agli eventi che accadono nel sistema.


### Descrizione dettagliata:
All'esterno, avremo il primo nodeMCU posto in prossimità del giardino. Ad esso, sono attacati tutti i sensori che utilizziamo per recepire le informazioni chiave che andremo ad utilizzare per il funzionamento generale del progetto.
I sensori utilizzati da questo nodeMCU sono:
- Temperatura e umidità dell'aria
- Temperatura e umidità della terra
- Rilevazione della pioggia
- Fotoresistore

Queste informazioni verranno costantemente tenute sotto controllo dal primo nodeMCU (file UnitaSensori.ino), che le invierà periodicamente ad un altro nodeMCU (definiamolo d'ora in avanti "controllore"), che sarà in un luogo chiuso, quindi idealmente all'interno della casa.
Le informazioni rilevate, in costante aggiornamento, saranno anche consultabili da un dispaly interno alla casa e dall'applicazione mobile.

Vicino al giardino, sarà presente una cisterna che raccoglie l'acqua piovana, che verrà usata per irrigare il giardino, nel caso in essa sia presente una sufficiente quantità d'acqua. Se la cisterna fosse vuota, il giardino verrà allora irrigato direttamente con l'acqua dell'impianto idraulico casalingo.

Tutte le decisioni, su come e quando irrigare, sono prese quindi dal nodeMCU "controllore" (file UnitaDiControllo) che abbiamo posizionato internamente alla casa e che è in comunicazione con il primo nodeMCU (UnitaSensori) di cui abbiamo parlato sopra, tramite il protocollo UDP, via WI-FI. In base alle informazioni che arrivano al controllore, quest'ultimo deciderà se azionare o meno l'irrigazione, ed in caso positivo, se sfruttare la cisterna o l'impianto idraulico casalingo. 

Abbiamo poi un terzo ed ultimo nodeMCU (UnitaAttuatori), all'esterno, a cui sono attaccati 3 relays:
- Uno per attivare l'elettrovalvola che aziona l'impianto idraulico casalingo per irrigare
- Uno per aprire la valvola dell'acqua della cisterna, per irrigare
- Uno per accendere una lampada per illuminare una pianta (quest'ultimo è azionabile solo manualmente)

Anche questo nodeMCU (UnitaAttuatori), sarà gestito dal controllore, grazie alle informazioni derivanti dal primo nodeMCU (UnitaSensori).

Descriviamo più dettagliamente il nodeMCU "controllore" (UnitaDiControllo), che sta in casa:

Esso ogni n secondi, riceve dal primo nodeMCU che controlla i sensori, i dati che ha letto.
Il controllore riceve ed elabora quindi i dati, e decide se azionare o meno i relays del terzo nodeMCU (UnitaAttuatori).
Attaccati a questo nodeMCU "controllore", abbiamo:
- Uno schermo a LED, sul quale possiamo visualizzare i dati letti dal primo nodeMCU (UnitaSensori), quindi: temperatura e umidità dell'aria, temperatura della terra, l'umidità della terra (terreni arido o umido) e infine lo stato acceso/spento dell'irrigazione e della luce della lampada.
- Quattore led: uno di colore blu, che segnala la presenza (acceso) o assenza (spento) di pioggia (funziona tramite sensore di pioggia), uno di colore giallo, che segnala la presenza (acceso) o assenza (spento) della luce del sole (funziona tramite fotoresistore), uno di colore verde, che si accende solamente quando la cisterna è piena, e uno di colore rosso che si accende solamente quando la cisterna è vuota.
un'altro, che può assumere i colori verde e rosso: verde nel caso la cisterna sia piena d'acqua, e rosso nel caso sia vuota.
- Tre bottoni: essi, fanno scattare i relays del nodeMCU esterno (UnitaAttuatori). Un bottone comanda l'apertura dell'irrigazione, uno l'accensione della lampada e l'ultimo l'arresto di entrambe queste attività.

Infine, abbiamo utilizzato un'applicazione per visualizzare le informazioni lette dal nodeMCU esterno (UnitaSensori), su un dispositivo mobile, sia Android che iOS. L'applicazione si chiama Blynk, ed è scaricabile gratuitamente dal sito https://www.blynk.cc/ e lavora con varie piattaforme, tra cui ESP8266.
Da questa applicazione mobile, una volta installata e impostato il progetto SmartGarden, sono consultabili due diversi tab.
Nel primo, chimato "sensori", sono consultabili tutte le informazioni lette dai sensori: temperatura, umidità dell'aria e temperatura della terra, attraverso delle barre circolari, con al centro il valore rilevato. Inoltre c'è un piccolo led che quando è acceso, ci informa che la cisterna è piena d'acqua.
Nel secondo tab, chiamato "relays", sono invece presenti 3 pulsanti virtuali, che assolvono alle stesse funzionalità dei pulsanti fisici gemelli, attaccati al nodeMCU controllore. Questi 3 pulsanti, una volta premuti, possono quindi far attivare l'irrigazione, accendere la lampada ed uno per eventualmente fermare queste due attività se necessario, direttamente dal proprio dispositivo mobile, sfruttando l'applicazione.
Infine quest'applicazione invia delle utili notifiche a tutti i dispositivi che la installano, se connessi con il progetto, quando si verifica l'accensione dell'impianto di irrigazione o della lampada, o l'eventuale stop di tutte le attività. Sarà così possibile vedere tramite la notifica, ad esempio l'orario a cui si è accesa l'irrigazione in maniera automatica, direttamente dal proprio dispositivo. 

### Lista completa componenti utilizzati:
- 1 Sonda sensore di temperatura waterproof DS18B20
- 1 Rain sensor HL38
- 1 Modulo sensore umidità terra - igrometro YL-69
- 1 KY-018 modulo Fotodiodo
- 1 Elettrovalvola
- 1 Interruttore a galleggiante sensore livello liquido di accio inox RG-1075S
- Pompa acqua 220V
- 1 5V 1-Channel Relay Module
- 1 5V 2-Channel Relay Module
- 3 KY-004 Modulo Pulsante Key
- 1 resistenza da 4.1 KOhms
- 4 KY-016 modulo Led RGB full color
- 1 KY-015 Sensore di temperatura e umidità dell'aria (DHT11)
- 1 Modulo LCD 20x4 I2C/TWI


## Lista librerie utilizzate:
- Blynk Library (app: https://github.com/blynkkk/blynk-library/releases/tag/v0.5.1)
- ESP8266WiFi
- WiFiUdp
- LiquidCrystal_I2C
- SimpleTimer
- SPI
- Adafruit_Sensor
- DHT
- DHT_U
- OneWire
- DallasTemperature
