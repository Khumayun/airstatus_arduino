/*
  Rui Santos
  Complete project details at Complete project details at https://RandomNerdTutorials.com/esp8266-nodemcu-http-get-post-arduino/

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files.

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
*/

#include <ESP8266WiFi.h>
#include <Arduino.h>
#include <DHT.h>
#include "MHZ19.h"

#define LENG 31   //0x42 + 31 bytes equal to 32 bytes
#define DHT11_PIN 2
#define DHTPIN D1
#define DHTTYPE DHT11

const char* ssid = "Unique_RDP";
const char* password = "Rkbc222FcBKVK";
const char* server = "104.248.198.113";
const int pwmpin = 14;

unsigned char buf[LENG];
String apiKey = "LKCEKPWB8JAWZFMU";
int PM01Value=0;          //define PM1.0 value of the air detector module
int PM2_5Value=0;         //define PM2.5 value of the air detector module
int PM10Value=0;         //define PM10 value of the air detector module

WiFiClient client;
DHT dht(DHTPIN, DHTTYPE);
MHZ19 *mhz19_pwm = new MHZ19(pwmpin);

void setup() {
  Serial.begin(9600);
  delay(10);

  dht.begin();
  
  Serial.println("Connecting");
  WiFi.begin(ssid, password);
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
 
  Serial.println("Timer set to 5 seconds (timerDelay variable), it will take 5 seconds before publishing the first reading.");
}

void loop() {
  // READ DATA FROM SENSORS
       
    if(Serial.find(0x42)){    //start to read when detect 0x42
    //read from plantower
    Serial.readBytes(buf,LENG);
    if(buf[0] == 0x4d){
      if(checkValue(buf,LENG)){
        PM01Value=transmitPM01(buf);
        PM2_5Value=transmitPM2_5(buf);
        PM10Value=transmitPM10(buf);
      }           
    }
  }
  //Send an HTTP POST request every 10 minutes
  static unsigned long lastTime = millis();
  if ((millis() - lastTime) >= 100) {
    lastTime=millis(); 
    if (client.connect(server,80)) {
      // Read the first line of the request
      float h = dht.readHumidity();
      // Read temperature as Celsius (the default)
      float t = dht.readTemperature();
      // read co2
      int co2ppm = mhz19_pwm->getPpmPwm();
      if (co2ppm < 0) co2ppm = 0;
      // Check if any reads failed and exit early (to try again).
      if (isnan(h) || isnan(t)) {
        Serial.println("Failed to read from DHT sensor!");
        return;
      }
      String postStr = "";
      postStr +="pm1=";
      postStr += String(PM01Value);
      postStr +="&pm25=";
      postStr += String(PM2_5Value);
      postStr +="&pm10=";
      postStr += String(PM10Value);
      postStr +="&temperature=";
      postStr += String(t);
      postStr +="&humidity=";
      postStr += String(h);
      postStr +="&co2=";
      postStr += String(co2ppm);
      
      Serial.print("Temperature: ");
      Serial.println(t);
      Serial.print("Humidity: ");
      Serial.println(h);
      Serial.print("co2: ");
      Serial.print(co2ppm);
      Serial.println("  ppm");
      Serial.print("PM1.0: ");
      Serial.print(PM01Value);
      Serial.println("  ug/m3");
    
      Serial.print("PM2.5: ");
      Serial.print(PM2_5Value);
      Serial.println("  ug/m3");
      
      Serial.print("PM10 : ");
      Serial.print(PM10Value);
      Serial.println("  ug/m3");
      
      Serial.println();
      client.print("POST /sendData/ HTTP/1.1\n");
      client.print("Host: 104.248.198.113\n");
      client.print("Content-Type: application/x-www-form-urlencoded\n");
      client.print("Content-Length: ");
      client.print(postStr.length());
      client.print("\n\n");
      client.print(postStr);
      }else Serial.print("did not connect to server");
      client.stop();
  }
}
char checkValue(unsigned char *thebuf, char leng)
{  
  char receiveflag=0;
  int receiveSum=0;
 
  for(int i=0; i<(leng-2); i++){
  receiveSum=receiveSum+thebuf[i];
  }
  receiveSum=receiveSum + 0x42;
 
  if(receiveSum == ((thebuf[leng-2]<<8)+thebuf[leng-1]))  //check the serial data 
  {
    receiveSum = 0;
    receiveflag = 1;
  }
  return receiveflag;
}
int transmitPM01(unsigned char *thebuf)
{
  int PM01Val;
  PM01Val=((thebuf[3]<<8) + thebuf[4]); //count PM1.0 value of the air detector module
  return PM01Val;
}
//transmit PM Value to PC
int transmitPM2_5(unsigned char *thebuf)
{
  int PM2_5Val;
  PM2_5Val=((thebuf[5]<<8) + thebuf[6]);//count PM2.5 value of the air detector module
  return PM2_5Val;
  }
//transmit PM Value to PC
int transmitPM10(unsigned char *thebuf)
{
  int PM10Val;
  PM10Val=((thebuf[7]<<8) + thebuf[8]); //count PM10 value of the air detector module  
  return PM10Val;
}
