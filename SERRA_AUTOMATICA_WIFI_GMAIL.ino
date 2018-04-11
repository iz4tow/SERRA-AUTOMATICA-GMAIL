// Load Wi-Fi library
#include <ESP8266WiFi.h>
#include "Gsender.h"
#include <WiFiUdp.h>
#include <TimeLib.h>

//SETTAGGI PER NTP
unsigned int localPort = 2390;      // local port to listen for UDP packets NTP
IPAddress timeServerIP; // time.nist.gov NTP server address
const char* ntpServerName = "0.it.pool.ntp.org";
const int NTP_PACKET_SIZE = 48; // NTP time stamp is in the first 48 bytes of the message
byte packetBuffer[ NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets
WiFiUDP udp;

// Replace with your network credentials
const char* ssid     = "THOUSAND SUNNY";
const char* password = "kingzoro";

// SETTAGGI PER POSTA
char server[] = "smtp.gmail.com";
String messaggio;
String oggetto;
int riepilogo=0;


//SETTAGGI PER HTTP SERVER
// Set web server port number to 80
WiFiServer serverhttp(80);
// Variable to store the HTTP request
String header;

int contatore=0;
const int valore_limite = 27; //Valore dell'igrometro al quale il relay sarà ON
int igro=0; //INDICATORE UMIDITA TERRENO
int umdtrr = 0;
//DHT11 Sensor:
#include "DHT.h"
#define DHTPIN 15     // Sensore collegato al PIN 8 (15 è 8...)
#define DHTTYPE DHT11   // DHT 11
DHT dht(DHTPIN, DHTTYPE);
int umidita;
int temperatura;

#define POMPA 5     // Sensore collegato al PIN 5 (5 è 1...)


void setup() {
  Serial.begin(115200);

 // IPAddress staticip(192,168,2,100);
 // IPAddress netmask(255,255,255,0);
 // IPAddress gateway(192,168,2,1);
 // IPAddress dns(8,8,8,8);
 // WiFi.config(staticip,dns,gateway,netmask); 
  // Connect to Wi-Fi network with SSID and password
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  //SET NTP TIME SETUP
  Serial.println("Starting UDP");
  udp.begin(localPort);//START UDP LISTENER
  Serial.print("Local port: ");
  Serial.println(udp.localPort());
  // wait to see if a reply is available
  ricevintp();  

  
  // Print local IP address and start web server
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  serverhttp.begin();



  //MAIL INIZIO SESSIONE
  String ip = WiFi.localIP().toString();
  messaggio="IP sessione: http://"+ip+"<br>Ora Attuale: "+String(hour()+2)+":"+String(minute())+":"+String(second());
    Gsender *gsender = Gsender::Instance();    // Getting pointer to class instance
    String subject = "INIZIO SESSIONE";
    if(gsender->Subject(subject)->Send("iz4tow@gmail.com", messaggio)){
    Serial.println("Message send.");
    } else {
        Serial.print("Error sending message: ");
        Serial.println(gsender->getError());
   }

   // Avvio sensore DHT11  
 dht.begin();
}




void loop(){
// Lettura umidità e temperatura del sensore DHT11
  umidita =dht.readHumidity();
  temperatura =dht.readTemperature();

  
  //Serial.print("Temperatura ambiente: ");
  //Serial.print(temperatura);
  //Serial.println("°C");
  //Serial.print("Umidità: ");
  //Serial.print(umidita);
  //Serial.println("%");
  
// Igrometro
  if (contatore==1000){ //LA LETTURA DELLA PORTA ANALOGICA INTERFERISCE COL WIFI
    contatore=0;
    igro = analogRead(A0); // Legge il valore analogico
    umdtrr = map (igro, 100, 990, 100, 0); // converto il valore analogico in percentuale
  //  Serial.print("Umidità del terreno: ");  
  //  Serial.print(umdtrr);
  //  Serial.println("%"); //Stampa a schermo il valore
    if (umdtrr <= valore_limite){
      innaffia(umdtrr,umidita,temperatura);
      igro = analogRead(A0);
    }else{
      digitalWrite(POMPA,HIGH); // Spegni Relè 1
    }
  }else{
    contatore++;
  }


  if (minute()==0 and riepilogo==0){ //ogni ora invia il riepilogo
    oggetto="RIEPILOGO";
    mail(umdtrr,umidita,temperatura,oggetto);
    riepilogo=1;
  }
    if (minute()==1 and riepilogo==1){ //ogni ora invia il riepilogo
    riepilogo=0;
  }

 //AGGIORNA ORA OGNI MEZZANOTTE
  if (hour()==0 and minute()==0){ 
    ricevintp();
  }


//////////////////////////////////////////////PARTE WEB SERVER
  WiFiClient client = serverhttp.available();   // Listen for incoming clients

  if (client) {                             // If a new client connects,
    Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected()) {            // loop while the client's connected
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        header += c;
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            
            // turns the GPIOs on and off
            if (header.indexOf("/MAIL") >= 0) {
              Serial.println("MAIL VIA WEB");
              mail(umdtrr,umidita,temperatura,oggetto);
            } 
            if (header.indexOf("/INNAFFIA") >= 0) {
              Serial.println("INNAFFIA");
              innaffia(umdtrr,umidita,temperatura);
            }
            
            // Display the HTML web page
            client.println("<!DOCTYPE html>");
            client.println("<html><body>");
            client.println("Temperatura Ambiente: "+String(temperatura)+"&#176;C");
            client.println("<br>Umidit&agrave; Ambiente: "+String(umidita)+"%");
            client.println("<br>Umidit&agrave; Terreno: "+String(umdtrr)+"%");  
            client.println("<br><a href=\"/MAIL\"\"><button>MAIL DI RIEPILOGO</button></a>");  
            client.println("<a href=\"/INNAFFIA\"\"><button>INNAFFIA</button></a>"); 
            //////////client.println("<br>Ora Aggiornamento: "+String(hour()+2)+":"+String(minute())+":"+String(second()));
            client.println("</body></html>");
            
            // The HTTP response ends with another blank line
            client.println();
            // Break out of the while loop
            break;
          } else { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
}






void mail(int umdtrr,int umidita,int temperatura,String oggetto){
  messaggio="Umidità terreno: " +String(umdtrr)+"% <br>Umidità ambiente: "+String(umidita)+"% <br> Temperatura ambiente: "+String(temperatura) +"°C"+"<br><br>Ora Aggiornamento: "+String(hour()+2)+":"+String(minute())+":"+String(second());
  Gsender *gsender = Gsender::Instance();    // Getting pointer to class instance
  String subject = oggetto;
  if(gsender->Subject(subject)->Send("iz4tow@gmail.com", messaggio)) {
    Serial.println("Message send.");
  } else {
    Serial.print("Error sending message: ");
    Serial.println(gsender->getError());
  }
}

void innaffia(int umdtrr,int umidita,int temperatura){
  digitalWrite(POMPA,LOW); // Attiva Relè 1
  oggetto="STO INNAFFIANDO";
  mail(umdtrr,umidita,temperatura,oggetto);
  delay(2000);
}






//FUNZIONI PER NTP TIME
unsigned long sendNTPpacket(IPAddress& address){
  Serial.println("sending NTP packet...");
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;

  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  udp.beginPacket(address, 123); //NTP requests are to port 123
  udp.write(packetBuffer, NTP_PACKET_SIZE);
  udp.endPacket();
}
void ricevintp(){
    int cb = 0;
  while (!cb){
    WiFi.hostByName(ntpServerName, timeServerIP); //RESOLVE HOSTNAME
    sendNTPpacket(timeServerIP); // send an NTP packet to a time server
    Serial.println("SINCRONIZZO IL TEMPO");
    cb = udp.parsePacket();
  }
  udp.read(packetBuffer, NTP_PACKET_SIZE); // read the packet into the buffer
    unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
    unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
    // combine the four bytes (two words) into a long integer
    // this is NTP time (seconds since Jan 1 1900):
    unsigned long secsSince1900 = highWord << 16 | lowWord;
    // Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
    const unsigned long seventyYears = 2208988800UL;
    // subtract seventy years:
    unsigned long epoch = secsSince1900 - seventyYears;
  setTime(epoch);
  Serial.print(hour());
  Serial.print(":");
  Serial.print(minute());
  Serial.print(":");
  Serial.print(second());
  Serial.println();
}

