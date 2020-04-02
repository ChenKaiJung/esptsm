#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <SoftwareSerial.h>
#include <Wire.h> 
#include <Ultrasonic.h>
#include <SSD1306Wire.h>
#include "images.h"



/* Set these to your desired credentials. */
const char *ssid = "ESPTSMOLED";
const char *password = "12345678";

Ultrasonic ultrasonic(12, 14);
ESP8266WebServer server(80);
SoftwareSerial sSerial(2, 0);
SSD1306Wire display(0x3c, SDA, SCL);


float temperture[10] = {NAN,NAN,NAN,NAN,NAN,NAN,NAN,NAN,NAN,NAN}; 
unsigned int tp = 0;
float envTemperture[10] = {NAN,NAN,NAN,NAN,NAN,NAN,NAN,NAN,NAN,NAN}; 
unsigned int ep = 0;
unsigned char output[37];
unsigned char to[2];
unsigned char ta[2];
char buff[1024];
char dispbuf[128];
  
void handleRoot() {

  char temp[256];
  unsigned int pf=0;
   int ttp=tp;

  //Serial.println();
  //Serial.print("ttp:");
  //Serial.print(ttp);
  if(ttp!=0) {
    for( int i=ttp-1;i>=0;i--){   
     sprintf(temp, "<h1><font size=7>Temperture is: %.2f</font></h1><BR/>", temperture[i]);
     strncpy(&buff[pf],temp,strlen(temp)); 
     pf=pf+strlen(temp);  
     Serial.println();
     Serial.print("temperture:");
     Serial.print(temperture[i]);

     
              Serial.println();
       Serial.print("i:");
       Serial.print(i);
    }
  }

  
  //Serial.println();
  //Serial.print("ttp:");
  //Serial.print(ttp);
       
  if(ttp!=9) {
    for( int i=9;i>=ttp;i--){   
     sprintf(temp, "<h1><font size=7>Temperture is: %.2f</font></h1><BR/>", temperture[i]);
     strncpy(&buff[pf],temp,strlen(temp)); 
     pf=pf+strlen(temp);  
       //Serial.println();
       //Serial.print("temperture:");
       //Serial.print(temperture[i]);

       //       Serial.println();
       //Serial.print("i:");
       //Serial.print(i);
    }
  }
  server.send(200, "text/html", buff);
}

unsigned char checksum(unsigned char* buffer) {
  unsigned char *buf = (unsigned char *)buffer;
  unsigned int seed;
  seed = buffer[0] + buffer[1] + buffer[2] + buffer[3] + buffer[4] + buffer[5] + buffer[6] + buffer[7];
  unsigned char result = (unsigned char)seed & 0xff;
  return result;
}

void setup() {
  delay(1000);
  Serial.begin(115200);
  Serial.println();
  Serial.print("Configuring access point...");
  /* You can remove the password parameter if you want the AP to be open. */
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);

  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);

  server.on("/", handleRoot);
  server.begin();
  Serial.println("HTTP server started");

  sSerial.begin(115200);
  //sSerial.write(0xA5);
  //sSerial.write(0x45);  
  //sSerial.write(0xEA);

  display.init();
  //display.flipScreenVertically();
  display.setFont(ArialMT_Plain_10);

  //pinMode(16, INPUT); 
  pinMode(13,OUTPUT);
}
void loop() {
  server.handleClient();
  
  //int keyin=digitalRead(16);
  //Serial.println();
  //Serial.print(keyin);
  //if(keyin==1) {
  //  sSerial.write(0xA5);
  //  sSerial.write(0x15);  
  //  sSerial.write(0xBA);  
  //}
  int dist=ultrasonic.distanceRead();
        Serial.println("");
        Serial.print("Distance: ");
      Serial.print(dist);  //讀取距離
      Serial.println(" CM");

  if(dist <10 ) {
      tone(13,2525);
      delay(100);
      if(dist > 2 ) {
          noTone(13);
      }
      else {
        sSerial.write(0xA5);
        sSerial.write(0x15);  
        sSerial.write(0xBA); 
      }
   }
   else {
      noTone(13);
  }
        
  if (sSerial.available() == false) {
    delay(100);
    return;
  }
  noTone(13);

  Serial.println();
  Serial.print("Taking Readings:");
  for (int counter = 0; counter <= 36; counter++) {
    output[counter] = (unsigned char)sSerial.read();
    Serial.print(output[counter], HEX);;
  }

  for (int obj = 0; obj <= 36; obj++) {

    if (output[obj] != 0x5A
        || output[obj + 2] != 0x45
        || output[obj + 3] != 0x04
        || output[obj + 8] != checksum(&output[obj])) {
      delay(100);
      continue;
    }


    to[0] = output[obj + 4];
    to[1] = output[obj + 5];
    unsigned int total = to[0] << 8;
    total= total | to[1];
    temperture[tp] = (float) total / 100;

    //Serial.println();
    //Serial.print("Temperature is: ");
    //Serial.print(temperture[tp]);
    display.clear();
    display.drawXbm(15, 0, da_width, da_height, da_bits);    
    sprintf(dispbuf," %.2f °C",temperture[tp]);
    display.setFont(ArialMT_Plain_24);
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.drawString(0, 30, dispbuf);
    display.display();
    
    tp++;
     if(tp>9) {
      tp=0;
    } 
      
    ta[0] = output[obj + 6];
    ta[1] = output[obj + 7];

    total = ta[0] << 8;
    total= total | ta[1];
    envTemperture[ep] = (float)total / 100;
   // Serial.println();
   // Serial.print("Environment Temperature is: ");
    //Serial.print(envTemperture[ep]);  
    ep++;
    if(ep>9) {
      ep=0;
    }
  }
  
  delay(100);
}
