#include<Wire.h>
#include <Blynk.h>
#include <TinyGPS++.h>
#include <SoftwareSerial.h>
#define BLYNK_PRINT Serial
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

static const int RXPin = 4, TXPin = 5;   // GPIO 4=D2(conneect Tx of GPS) and GPIO 5=D1(Connect Rx of GPS
static const uint32_t GPSBaud = 9600; //if Baud rate 9600 didn't work in your case then use 4800

TinyGPSPlus gps; // The TinyGPS++ object
WidgetMap myMap(V0);  // V0 for virtual pin of Map Widget

SoftwareSerial ss(RXPin, TXPin);  // The serial connection to the GPS device

BlynkTimer timer;



char auth[] = "PRNBddsg6zo4zBnLSS1RDsemae0AB6EV";              //Your Project authentication key
char ssid[] = "karthiga";                                       // Name of your network (HotSpot or Router name)
char pass[] = "12345678";                                      // Corresponding Password

//initalise mpu accelo & gyro
const int MPU_addr=0x68;
int16_t AcX,AcY,AcZ;
float velocityFinnal=0,velocityInitial=0;
int didAccedentHappen=0;

//for gps
unsigned int move_index = 1;       // fixed location for now
float spd;       //Variable  to store the speed

void setup()
{
  Wire.begin();
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x6B);
  Wire.write(0);
  Wire.endTransmission(true);
  Serial.begin(115200);
  Serial.println();
  ss.begin(GPSBaud);
  Blynk.begin(auth, ssid, pass);
  timer.setInterval(5000L, checkGPS); // every 5s check if GPS is connected, only really needs to be done once
}

void checkGPS(){
  if (gps.charsProcessed() < 10)
  {
    Serial.println(F("No GPS detected: check wiring."));
      Blynk.virtualWrite(V4, "GPS ERROR");  // Value Display widget  on V4 if GPS not detected
  }
}

void loop()
{
  accelloGyro();                                            //initialize the new speed
  if(didAccedentHappen==0)                                  //check if accedent happen in past
  {
    float diffrenceInSpeed = velocityFinnal-velocityInitial;
    if(diffrenceInSpeed<0 && diffrenceInSpeed<-5)           //check with old speed (if sudden low speed means accdent)
    {
      Blynk.virtualWrite(V3, "Yes");
      didAccedentHappen=1;                                  //initalize that u had an accedent for past refference
    }
    else
    {
      Blynk.virtualWrite(V3, "No");
    }
  }
  else
  {
    Blynk.virtualWrite(V3, "Yes");
  }
  while (ss.available() > 0) 
  {
      // sketch displays information every time a new sentence is correctly encoded.
      if (gps.encode(ss.read()))
      {
        displayInfo();
      }
  }
  Blynk.run();
  timer.run();
  velocityInitial=velocityFinnal;
}

void accelloGyro()
{
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_addr,14,true);
  AcX=Wire.read()<<8|Wire.read();
  AcY=Wire.read()<<8|Wire.read();
  AcZ=Wire.read()<<8|Wire.read();
  velocityFinnal = sqrt((sq(AcX)+sq(AcY)+sq(AcZ)));
}

void displayInfo()
{ 
  if (gps.location.isValid() ) 
  {    
    float latitude = (gps.location.lat());     //Storing the Lat. and Lon. 
    float longitude = (gps.location.lng()); 
    
    Serial.print("LAT:  ");
    Serial.println(latitude, 6);  // float to x decimal places
    Serial.print("LONG: ");
    Serial.println(longitude, 6);
    Blynk.virtualWrite(V1, String(latitude, 6));   
    Blynk.virtualWrite(V2, String(longitude, 6));  
    myMap.location(move_index, latitude, longitude, "GPS_Location");   
    spd = gps.speed.kmph();               //get speed             
  }
  
 Serial.println();
}
