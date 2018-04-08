//   Serra autonoma con sensore di temperatura e umidità (DHT11), igrometro, fotoresistore, display LCD 20x4, sensore livello acqua e relè.
#include <ESP8266WiFi.h>
#include "Gsender.h"

const char* SSID = "NETGEAR";
const char* PASS = "";
char server[] = "smtp.gmail.com";
String messaggio;
WiFiServer serverhttp(80);

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
    String ip = WiFi.localIP().toString();
     messaggio="IP sessione: "+ip;
    Gsender *gsender = Gsender::Instance();    // Getting pointer to class instance
    String subject = "INIZIO SESSIONE";
    if(gsender->Subject(subject)->Send("iz4tow@gmail.com", messaggio)) {
        Serial.println("Message send.");
    } else {
        Serial.print("Error sending message: ");
        Serial.println(gsender->getError());
    }

    ///AVVIO SERVER HTTP
      serverhttp.begin();
  Serial.println("Server started");
 
  // Print the IP address
  Serial.print("Use this URL to connect: ");
  Serial.print("http://");
  Serial.print(WiFi.localIP());
  Serial.println("/");

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
//////////////////////////////////////////////////PARTE HTTP SERVER
  // Check if a client has connected
  WiFiClient client = serverhttp.available();
  if (!client) {
    return;
  }
 
  // Wait until the client sends some data
  Serial.println("new client");
  while(!client.available()){
    delay(1);
  }
 
  // Read the first line of the request
  String request = client.readStringUntil('\r');
  Serial.println(request);
  client.flush();


  // Return the response
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println(""); //  do not forget this one
  client.println("<!DOCTYPE HTML>");
  client.println("<html>");
 

  client.println("Temperatura Ambiente: "+String(temperatura)+"&#176;C");
  client.println("<br>Umidit&agrave; Ambiente :"+String(umidita)+"%");
  client.println("<br>Umidit&agrave; Terreno: "+String(umdtrr)+"%");  
  client.println("</html>");
 
  delay(1);
  Serial.println("Client disonnected");
  Serial.println("");
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



