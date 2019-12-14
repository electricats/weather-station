

#include <Wire.h>
#define BLYNK_PRINT Serial
#include <SoftwareSerial.h>
#include <Adafruit_NeoPixel.h>
SoftwareSerial EspSerial(2, 3); // RX, TX
#include <dht.h>
dht DHT;
#include <ESP8266_Lib.h>
#include <BlynkSimpleShieldEsp8266.h>
int measurePin = 0; //Connect dust sensor to Arduino A0 pin
int ledPower = 7;   //Connect 3 led driver pins of dust sensor to Arduino D2
char auth[] = "auth-token";

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "wifiname";
char pass[] = "wifipass";  

// How many NeoPixels are attached to the Arduino?
Adafruit_NeoPixel  rgb_display_10(4);

ESP8266 wifi(&EspSerial);
int samplingTime = 280;
int deltaTime = 40;
int sleepTime = 9680;
int i=0;
  
float voMeasured = 0;
float voMeasuredReal = 0;
float voMeasuredLast = 123.0;
float calcVoltage = 0;
float dustDensity = 0;

void setup(){
  Serial.begin(9600);
  pinMode(ledPower,OUTPUT);
  pinMode(8,OUTPUT);
  digitalWrite(8,HIGH);
  delay(10);

  // Set ESP8266 baud rate
  EspSerial.begin(9600);
  delay(10);
   rgb_display_10.begin();
  rgb_display_10.setPin(10);
  Blynk.begin(auth, wifi, ssid, pass);
  //Serial.home();
}
BLYNK_WRITE(V5){
  int r = param[0].asInt(); 
  int g = param[1].asInt(); 
  int b = param[2].asInt(); 
   for(int i=0;i<NUMPIXELS;i++){

    // pixels.Color takes RGB values, from 0,0,0 up to 255,255,255
    pixels.setPixelColor(i, pixels.Color(r,g,b)); // Moderately bright green color.

    pixels.show(); // This sends the updated pixel color to the hardware.

  }
  
}
void loop(){
  Blynk.run();
 int chk = DHT.read11(5);
  float h = DHT.humidity;
  float t = DHT.temperature;
  calcVoltage = voMeasured/5.0*5.5;
  //
  dustDensity = 0.17 * calcVoltage - 0.1;
 
  digitalWrite(ledPower,LOW); // power on the LED
  delayMicroseconds(samplingTime);
  voMeasured=analogRead(A0); // read the dust value via pin 5 on the sensor
  voMeasuredReal=voMeasured;
  delayMicroseconds(deltaTime);
  digitalWrite(ledPower,HIGH); // turn the LED off
  
  //smooth noise
  voMeasured=voMeasuredLast+(voMeasured-voMeasuredLast)/10.0;
  
  if(voMeasured<150)
  dustDensity=0.15*(voMeasured-130.0);
  else if(voMeasured<160)
  dustDensity=3+1.7*(voMeasured-150.0);
  else if (voMeasured<220)
  dustDensity=5* (voMeasured-150.0);
  else
  //0.6~3.6 = 123~737 map to 0~500 ug/m3
  dustDensity=voMeasured-220.0;
  if (dustDensity<0) dustDensity=0.0;
  if (dustDensity>600) dustDensity=600.0;

  Serial.println("taoX2      ug/m3");

  Serial.print("PM2.5 ");
  Serial.println(dustDensity);
  Blynk.virtualWrite(V1,dustDensity);
  Blynk.virtualWrite(V2,h);
  Blynk.virtualWrite(V3,t);
  voMeasuredLast=voMeasured;
  delayMicroseconds(sleepTime);
  if (i<=30)delay(1000);
  else delay (5000);
  i++;
  if (i>1024) i=31;
}
