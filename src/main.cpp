/*
 __    __   __   _______ .______      ______   ___   ___  __   _______    ______   
|  |  |  | |  | |       \ |   _  \   /  __  \  \  \ /  / |  | |       \  /  __  \  
|  |__|  | |  | |  .--.  ||  |_)  | |  |  |  |  \  V  /  |  | |  .--.  ||  |  |  | 
|   __   | |  | |  |  |  ||      /  |  |  |  |   >   <   |  | |  |  |  ||  |  |  | 
|  |  |  | |  | |  '--'  ||  |\  \ .|  `--'  |  /  .  \  |  | |  '--'  ||  `--'  | 
|__|  |__| |__| |_______/ | _| `._|  \______/  /__/ \__\ |__| |_______/  \______/  
                                                                    Alexandre Souto 
   Bluetooth Low Energy
 
   Compilador: Visual Studio 2022

   Autor: Alexandre Souto
   Data:  Setember 2022 
*/


#include <Arduino.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <WiFiClient.h>

const char* ssid     = "4le3x_AP";
const char* password = "1020304050";



// TCP server at port 80 will respond to HTTP requests
WiFiServer server(80);
String lastPackage = "";

String chave = "$POK";

String Gerador_de_Checksum(String package);
String Sent_Package();


void setup()
{   
    Serial.begin(9600);
    Serial.println("\n[*] Creating AP");
    WiFi.mode(WIFI_AP);
    WiFi.softAP(ssid, password);
    Serial.print("[+] AP Created with IP Gateway ");
    Serial.println(WiFi.softAPIP());


    // Set up mDNS responder:
    // - first argument is the domain name, in this example
    //   the fully-qualified domain name is "esp32.local"
    // - second argument is the IP address to advertise
    //   we send our IP address on the WiFi network
    if (!MDNS.begin("esp32")) {
        Serial.println("Error setting up MDNS responder!");
        while(1) {
            delay(1000);
        }
    }
    Serial.println("mDNS responder started");

    // Start TCP (HTTP) server
    server.begin();
    Serial.println("TCP server started");

    // Add service to MDNS-SD
    MDNS.addService("http", "tcp", 80);
}


void loop(void)
{


    // Check if a client has connected
    WiFiClient client = server.available();
    if (!client) {
        return;
    }

    Serial.println("");
    Serial.println("New client");

    // Wait for data from client to become available
    while(client.connected() && !client.available()){
        delay(1);
    }

    // Read the first line of HTTP request
    String req = client.readStringUntil('\r');

    // First line of HTTP request looks like "GET /path HTTP/1.1"
    // Retrieve the "/path" part by finding the spaces
    int addr_start = req.indexOf(' ');
    int addr_end = req.indexOf(' ', addr_start + 1);
    if (addr_start == -1 || addr_end == -1) {
        Serial.print("Invalid request: ");
        Serial.println(req);
        return;
    }

    req = req.substring(addr_start + 1, addr_end);
    Serial.print("Request: ");
    Serial.println(req);

    String s;
    if (req == "/")
    {
        IPAddress ip = WiFi.localIP();
        String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);

        //recebe o package
        //String coisa = "$ALX,600,0,0,0,0,0,5B";
        String Package;
        Package = Sent_Package();

        s = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML>\r\n" + Package + "\r\n";
        //s += ipStr;
        s += "</html>\r\n\r\n";
        //Serial.println("Sending 200");
    }

    else
    {
        s = "HTTP/1.1 404 Not Found\r\n\r\n";
        Serial.println("Sending 404");
    }
    client.print(s);

    client.stop();
    Serial.println("Done with client");

}


struct obc_frame 
{
  unsigned int rpm ;
  unsigned int digital1;
  unsigned int digital2;
  unsigned int digital3;
  unsigned int digital4;
  unsigned int pulse1;
}frame = {1,0,0,0,0,0};


String Sent_Package()
{

  String package;

  frame.digital1 =  random(2);
  frame.digital2 =  random(2);
  frame.digital3 =  random(2);
  frame.digital4 =  random(2);
  frame.rpm      =  random(1000);
  frame.pulse1   =  random(100);
  
 
  package = "$ALX,";
  //$ALX,600,0,0,0,0,0,checksum\r\n
  //Serial.printf("Digital 1: %d | Digital 2: %d | Digital 3: %d  | Digital 4: %d | RPM: %d | Pulse: %d \n", frame.digital1,frame.digital2,frame.digital3,frame.digital4,frame.rpm,frame.pulse1);

  package = (package + frame.rpm + "," + frame.digital1 + "," + frame.digital2 + "," + frame.digital3 + "," + frame.digital4 + "," + frame.pulse1 + ",");
  
  String temp_checksum = Gerador_de_Checksum(package);
  
  package = (package + temp_checksum + "\r\n");
  Serial.print(package);

  if (package != lastPackage)
  {
      //pacote -> setValue(package.c_str());
      //pacote -> notify(); //notifica que houve alterações no pacote
      lastPackage = package;
  }

  return package;
}



String Gerador_de_Checksum(String package)
{
  String checksum = "";
  int DV = 0;
  int tamanho = package.length();            
  for (int i = 0; i < tamanho; i++)
  {
    DV ^= package[i];   // bitwise XOR
  }

  String hexValue = String(DV, HEX) + "\0"; // conver DV para hexadecimal
  
  if (hexValue.length() == 1)
    checksum = "0" + hexValue;
  if (hexValue.length() == 2)
    checksum = hexValue;  
  if (hexValue.length() == 3)
    checksum = "0" + hexValue[2];
  if (hexValue.length() == 4)
    checksum = (hexValue[2] + hexValue[4]);
  if (hexValue == "0")
    checksum = "00"; 
                         
  return checksum;  
}


/*
#include <Arduino.h>
#include <WiFi.h>
#include "analogRead.hpp"

#define READ_INTERVAL     2000
#define LEDPIN               2
#define TIMER_INTERVAL 1000000 // 1 SEGUNDO

#define SERVICE_UUID "0716bf69-27fa-44bd-b636-4ab49725c6b0"
#define PACOTE_UUID  "0716bf69-27fa-44bd-b636-4ab49725c6b1"
#define RX_UUID      "4ac8a682-9736-4e5d-932b-e9b31405049c"

#define PIN_DIGITAL_1  5
#define PIN_DIGITAL_2 18
#define PIN_DIGITAL_3 19
#define PIN_DIGITAL_4 21
#define PIN_RPM       34
#define PIN_PULSE_1   35



String lastPackage = "";
String password = "$POK";

int devicesConnected = 0; //Contador de usuários conectados

unsigned int blinkMillis = 0;
unsigned int readMillis = 0;

int flag_retorno = 0;

volatile int interruptCounter;
int totalInterruptCounter;

struct obc_frame 
{
  unsigned int rpm ;
  unsigned int digital1;
  unsigned int digital2;
  unsigned int digital3;
  unsigned int digital4;
  unsigned int pulse1;
}frame = {1,0,0,0,0,0};


analog_read pins(PIN_DIGITAL_1,PIN_DIGITAL_2,PIN_DIGITAL_3,PIN_DIGITAL_4,PIN_RPM,PIN_PULSE_1);


hw_timer_t * timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;



void printteste(std::string teste)
{
  Serial.println(teste.c_str());

}

void testeReset()
{
  Serial.println("Restarting ...");
  delay(2000);
  ESP.restart();
}


void IRAM_ATTR onTimer();
void sense();
String Gerador_de_Checksum(String package);

void setup() 
{

  Serial.begin(9600);
  // Rotina de interrupção
  timer = timerBegin(0, 80, true);
  timerAttachInterrupt(timer, &onTimer, true);
  timerAlarmWrite(timer,TIMER_INTERVAL, true);
  timerAlarmEnable(timer);


  Serial.println("Starting ...");
  pinMode(LEDPIN,OUTPUT); //Define pino como saida (led azul da devkit)

}

void IRAM_ATTR onTimer() 
{
  portENTER_CRITICAL_ISR(&timerMux);
  interruptCounter++;
  portEXIT_CRITICAL_ISR(&timerMux);

}

String Gerador_de_Checksum(String package)
{
  String checksum = "";
  int DV = 0;
  int tamanho = package.length();            
  for (int i = 0; i < tamanho; i++)
  {
    DV ^= package[i];   // bitwise XOR
  }

  String hexValue = String(DV, HEX) + "\0"; // conver DV para hexadecimal
  
  if (hexValue.length() == 1)
    checksum = "0" + hexValue;
  if (hexValue.length() == 2)
    checksum = hexValue;  
  if (hexValue.length() == 3)
    checksum = "0" + hexValue[2];
  if (hexValue.length() == 4)
    checksum = (hexValue[2] + hexValue[4]);
  if (hexValue == "0")
    checksum = "00"; 
                         
  return checksum;  
}

void Sent_Package()
{
  String package;

   frame.digital1 =  pins.analog_digital(PIN_DIGITAL_1);
   frame.digital2 =  pins.analog_digital(PIN_DIGITAL_2);
   frame.digital3 =  pins.analog_digital(PIN_DIGITAL_3);
   frame.digital4 =  pins.analog_digital(PIN_DIGITAL_4);
   frame.rpm      = random(1000);// pins.analog_rpm(PIN_RPM);
   frame.pulse1   =  pins.analog_pulse(PIN_PULSE_1);

    
  // frame.digital1 =  random(2);
  // frame.digital2 =  random(2);
  // frame.digital3 =  random(2);
  // frame.digital4 =  random(2);
  // frame.rpm      =  random(1000);
  //  frame.pulse1   =  pins.analog_pulse(PIN_PULSE_1);
  
 
  package = "$ALX,";
   //$ALX,600,0,0,0,0,0,checksum\r\n

  //Serial.printf("Digital 1: %d | Digital 2: %d | Digital 3: %d  | Digital 4: %d | RPM: %d | Pulse: %d \n", frame.digital1,frame.digital2,frame.digital3,frame.digital4,frame.rpm,frame.pulse1);

  package = (package + frame.rpm + "," + frame.digital1 + "," + frame.digital2 + "," + frame.digital3 + "," + frame.digital4 + "," + frame.pulse1 + ",");
  
  String temp_checksum = Gerador_de_Checksum(package);
  
  package = (package + temp_checksum + "\r\n");
  Serial.print(package);

  if (package != lastPackage)
  {
      //pacote -> setValue(package.c_str());
      //pacote -> notify(); //notifica que houve alterações no pacote
      lastPackage = package;
  }

}

void loop() 
{

  if (interruptCounter > 0) 
  {
 
    portENTER_CRITICAL(&timerMux);
    interruptCounter--;
    portEXIT_CRITICAL(&timerMux);

    if (flag_retorno == 1) //flag_retorno == 1
    {
      Sent_Package();
    }

   totalInterruptCounter++;

  }

  if (readMillis == 0 || (millis() - readMillis) >= READ_INTERVAL)
  {
    readMillis = millis();
  }

  if (!devicesConnected)
  {
    if(blinkMillis == 0 || (millis() - blinkMillis) >= 1000)
    {
      digitalWrite(LEDPIN, !digitalRead(LEDPIN));
      blinkMillis = millis();
    }
  }
  else
  {
    digitalWrite(LEDPIN,HIGH);
  }


}

*/




