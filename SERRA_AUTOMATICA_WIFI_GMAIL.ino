//   Serra autonoma con sensore di temperatura e umidità (DHT11), igrometro, fotoresistore, display LCD 20x4, sensore livello acqua e relè.
#include <ESP8266WiFi.h>
#include "Gsender.h"

const char* SSID = "Franco";
const char* PASS = "Fiwoldiois01$";
char server[] = "smtp.gmail.com";
String messaggio;

WiFiClient client;

  
const int valore_limite = 27; //Valore dell'igrometro al quale il relay sarà ON
int contatore=0;
byte ret=0;

//DHT11 Sensor:
#include "DHT.h"
#define DHTPIN 15     // Sensore collegato al PIN 8 (15 è 8...)
#define DHTTYPE DHT11   // DHT 11
DHT dht(DHTPIN, DHTTYPE);

void setup(){
  //CONNESSIONE WIFI
  Serial.begin(115200);
  delay(10);
  Serial.println("");
  Serial.println("");
  Serial.print("Connecting ");
  Serial.println(SSID);
  WiFi.begin(SSID, PASS);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi Connected");
  Serial.print("IP sessione: ");
  Serial.println(WiFi.localIP());
  //FINE CONNESSIONE

// PIN 9 al Relè - IN1
//	pinMode(8, OUTPUT);

Serial.println("Serra Autonoma di Franco Avino");
  
// Avvio sensore DHT11  
	dht.begin();
}


void loop(){
// Lettura umidità e temperatura del sensore DHT11
int umidita = dht.readHumidity();
int temperatura = dht.readTemperature();

  Serial.print("Temperatura ambiente: ");
  Serial.print(temperatura);
  Serial.println("°C");
  Serial.print("Umidità: ");
  Serial.print(umidita);
  Serial.println("%");
	
// Igrometro
int igro = analogRead(A0); // Legge il valore analogico
int umdtrr = 0; // Variabile umidità suolo
umdtrr = map (igro, 100, 990, 100, 0); // converto il valore analogico in percentuale
Serial.print("Umidità del terreno: ");
Serial.print(umdtrr);
Serial.println("%"); //Stampa a schermo il valore

if (umdtrr <= valore_limite){
	digitalWrite(16,HIGH); // Attiva Relè 1
	
    messaggio="Umidità terreno: " +String(umdtrr)+"% <br>Umidità ambiente: "+String(umidita)+"% <br> Temperatura ambiente: "+String(temperatura) +"°C";
    Gsender *gsender = Gsender::Instance();    // Getting pointer to class instance
    String subject = "INNAFFIO";
    if(gsender->Subject(subject)->Send("iz4tow@gmail.com", messaggio)) {
        Serial.println("Message send.");
    } else {
        Serial.print("Error sending message: ");
        Serial.println(gsender->getError());
    }
	
	
}else{
  digitalWrite(16,LOW); // Spegni Relè 1
}

delay(1000);
contatore++;
Serial.println(contatore);
if (contatore>3600){
  contatore=0;
  
  	messaggio="Umidità terreno: " +String(umdtrr)+"% <br>Umidità ambiente: "+String(umidita)+"% <br> Temperatura ambiente: "+String(temperatura) +"°C";
    Gsender *gsender = Gsender::Instance();    // Getting pointer to class instance
    String subject = "RIEPILOGO";
    if(gsender->Subject(subject)->Send("iz4tow@gmail.com", messaggio)) {
        Serial.println("Message send.");
    } else {
        Serial.print("Error sending message: ");
        Serial.println(gsender->getError());
    }
}
}





void alarm(){
  tone(3,850,700);  // Imposta tono acustico su PIN 3
  delay(150);       // Aspetta per 0,15 secondi
  tone(3,850,700);  // Imposta tono acustico su PIN 3
  delay(150);       // Aspetta per 0,15 secondi
}

void flash() {
  digitalWrite(4, HIGH);   // Imposta il LED su ON
  delay(1000);               // Aspetta per 1 secondo
  digitalWrite(4, LOW);    // Impostiamo il LED su OFF
  delay(1000);               // Aspetta per 1 secondo
}



