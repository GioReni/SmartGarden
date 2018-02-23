# Progetto SmartGarden - Corso Sistemi Embedded (Università degli Studi di Milano, Anno 2017-2018)

## Autori:
Alessandro Gigliotti, Giovanni Reni 
## Board utilizzate:
NodeMCU ESP8266


### Descrizione generale:
In questo progetto, abbiamo realizzato un sistema di gestione intelligente di un piccolo orto o giardino, tramite un impianto d'irrigazione, basato sul controllo di ciò che accade e quindi di come si modifica, l'ambiente circostante. L'impianto entra infatti autonomamente in azione, quando si verificano determinate condizioni (come secchezza del terreno o assenza di pioggia), tenendo costantemente sotto controllo varie informazioni sull'aria, terra, meteo e sugli strumenti d'irrigazioni utilizzati. È altresì possibile attivare l'irrigazione anche manualmente da un utente, tramite un'applicazione su dispositivo mobile.


### Descrizione dettagliata:
All'esterno, avremo il primo nodeMCU posto in prossimità del giardino. Ad esso, sono attacati tutti i sensori che utilizziamo per recepire le informazioni chiave che andremo ad utilizzare per il funzionamento generale del progetto.
I sensori utilizzati da questo nodeMCU sono:
- Temperatura e umidità dell'aria
- Temperatura e umidità della terra
- Rilevazione della pioggia
- Fotoresistore

Queste informazioni verranno costantemente tenute sotto controllo dal primo nodeMCU, che le invierà periodicamente ad un altro nodeMCU.
Le informazioni verranno anche usate per definire delle statistiche, che saranno consultabili dall'applicazione mobile.

Vicino al giardino, sarà presente una cisterna che raccoglie l'acqua piovana, che verrà usata per irrigare il giardino, nel caso in essa sia presente una sufficiente quantità d'acqua. Quindi a questo nodeMCU esterno ci sarà attaccato un sensore del livello dell'acqua, posto nella cisterna. Se la cisterna fosse vuota, il giardino verrà irrigato direttamente con l'acqua dell'impianto idraulico casalingo.

Tutte le decisioni, su come e quando irrigare, sono prese da un altro nodeMCU che abbiamo posizionato internamente alla casa (definiamolo d'ora in avanti "controllore") e che è in comunicazione con il primo nodeMCU di cui abbiamo parlato sopra, tramite il protocollo UDP, via WI-FI. In base alle informazioni che arrivano al controllore, quest'ultimo deciderà se azionare o meno l'irrigazione, ed in caso positivo, se azionarla dalla cisterna o dall'impianto casalingo. 

Abbiamo poi un terzo ed ultimo nodeMCU, all'esterno, a cui sono attaccati 3 relè:
- Uno per attivare l'elettrovalvola che aziona l'impianto idraulico casalingo per irrigare
- Uno per aprire la valvola dell'acqua della cisterna, per irrigare
- Uno per accendere una lampada per illuminare una pianta (quest'ultimo azionabile solo manualmente)

Anche questo nodeMCU, sarà gestito dal controllore, grazie alle informazioni derivanti dal primo nodeMCU.

Descriviamo più dettagliamente il nodeMCU "controllore", che sta in casa:

Esso ogni n secondi, chiede al primo nodeMCU che controlla i sensori, i dati che ha letto. Quest'ultimo glieli invia.
Il controllore riceve ed elabora quindi i dati, e decide se azionare o meno i relè del terzo nodeMCU.
Attaccati a questo nodeMCU "controllore", abbiamo:
- Uno schermo a LED, sul quale possiamo visualizzare i dati letti dal primo nodeMCU, quindi temperatura e umidità dell'aria e della terra, e lo stato acceso/spento dell'irrigazione e della luce della lampada.
- Due led: uno che può assumere i colori blu e giallo, nel caso in cui fuori stia piovendo (blu), oppure se ci sia il sole (giallo);
un'altro, che può assumere i colori verde e rosso: verde nel caso la cisterna sia piena d'acqua, e rosso nel caso sia vuota.
- Due bottoni: questi bottoni, fanno scattare i relè del nodeMCU esterno. Un bottone comanda l'apertura dell'irrigazione, l'altro l'accensione della lampada.


L'applicazione che abbiamo utilizzato per visualizzare le informazioni lette dal nodeMCU esterno, su un dispositivo mobile, si chiama Blynk.
Essa è scaricabile dal sito https://www.blynk.cc/ e lavora con varie piattaforme, tra cui ESP8266.

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
- 1 KY-004 Modulo Pulsante Key
- 1 resistenza da 4.1 KOhms
- 1 KY-016 modulo Led RGB full color
- 1 KY-015 Sensore di temperatura e umidità dell'aria (DHT11)
- 1 KY-029 Modulo LED 2 colori 3 millimetri
- 1 Modulo LCD 20x4 I2C/TWI


## Lista librerie utilizzate:
- Blynk Library: https://github.com/blynkkk/blynk-library/releases/tag/v0.5.1
- ...
