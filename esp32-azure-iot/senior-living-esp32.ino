
#include <LiquidCrystal_I2C.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include "MAX30105.h"
#include "heartRate.h"
#include "secrets.h"



MAX30105 particleSensor;

const byte RATE_SIZE = 4; //Increase this for more averaging. 4 is good.
byte rates[RATE_SIZE]; //Array of heart rates
byte rateSpot = 0;
long lastBeat = 0; //Time at which the last beat occurred

float beatsPerMinute;
int beatAvg;


#define SDA 21                    //Define SDA pins
#define SCL 22                    //Define SCL pins

//Provide WiFi credentials are in the secret.h
//String for storing server response
String response = "";
//JSON document
DynamicJsonDocument doc(2048);

LiquidCrystal_I2C lcd(0x27,16,2); 


void initWifi(){
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.println("");
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println(WiFi.status());
    Serial.print("*");
  }
  Serial.print("WiFi connected with IP: ");
  Serial.println(WiFi.localIP());
}

int initSuccess = 0;
unsigned long lastSignalTime = millis();


void setup()
{
  Serial.begin(115200);
  Serial.println("Initializing...");

  initWifi();
  
  Wire.begin(SDA, SCL);           // attach the IIC pin
  lcd.init();                     // LCD driver initialization
  lcd.backlight();                // Open the backlight
  lcd.setCursor(0,0);             // Move the cursor to row 0, column 0
  lcd.print(WiFi.localIP());     // The print content is displayed on the LCD
  lcd.setCursor(0,1);             // Move the cursor to row 0, column 0
  lcd.print("Init MAX30105...");     // The print content is displayed on the LCD
  

  // Initialize sensor
  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) //Use default I2C port, 400kHz speed
  {
    Serial.println("MAX30105 was not found. Please check wiring/power. HALT");
      return;
  }
  Serial.println("Place your index finger on the sensor with steady pressure.");
  initSuccess = 1;
  particleSensor.setup(); //Configure sensor with default settings
  particleSensor.setPulseAmplitudeRed(0x0A); //Turn Red LED to low to indicate sensor is running
  particleSensor.setPulseAmplitudeGreen(0); //Turn off Green LED
}


bool skipNextBeat = false;

void loop()
{
  if (initSuccess == 0){
    return;
  }
  long irValue = particleSensor.getIR();

  if (checkForBeat(irValue) == true)
  {
    //We sensed a beat!
    long delta = millis() - lastBeat;
    lastBeat = millis();
    if(skipNextBeat == true){
      // Due to telemetry upload lastBeat is not accuare. We skip this lastBeat value
      skipNextBeat = false;
      Serial.println("Heart Beat Skipped");
      return;
    }
    else {
      Serial.println("Heart Beat! ***********************");
    }
    beatsPerMinute = 60 / (delta / 1000.0);

    if (beatsPerMinute < 255 && beatsPerMinute > 20)
    {
      rates[rateSpot++] = (byte)beatsPerMinute; //Store this reading in the array
      rateSpot %= RATE_SIZE; //Wrap variable

      //Take average of readings
      beatAvg = 0;
      for (byte x = 0 ; x < RATE_SIZE ; x++)
        beatAvg += rates[x];
      beatAvg /= RATE_SIZE;
    }
  }



  int sensorAttached = 1;
  if (irValue < 50000){
    Serial.print(" No finger?");
    sensorAttached = 0;
    for (byte x = 0 ; x < RATE_SIZE ; x++){
      rates[x] = 0;
    }
    beatAvg = 0;
    beatsPerMinute = 0;
  }
  Serial.print("IR=");
  Serial.print(irValue);
  Serial.print(", BPM=");
  Serial.print(beatsPerMinute);
  Serial.print(", Avg BPM=");
  Serial.print(beatAvg);

  // Serial.println();
  // return;
  const int telemetryUploadFrequencyMS = 2000;
  
  String wifiPulseIndicator = "";
  int wifiPulseFlag = 0;
  unsigned int currentTime = millis();
  if (currentTime - lastSignalTime > telemetryUploadFrequencyMS ){
    unsigned long currentTime = millis();
    lastSignalTime = currentTime;
    wifiPulseIndicator = " *";
    wifiPulseFlag = 1;
  }

  Serial.println();
  String csv = String(irValue) + "," + String(beatsPerMinute) + "," + String(beatAvg) + "," + String(sensorAttached) ;
  String displaycsv = csv + wifiPulseIndicator + "               ";
  
  String jsonPayload = "{ \"IR\": " + String(irValue) + " , \"BPM\": " + String((int)beatsPerMinute)  + " , \"AVG_BPM\": " + String(beatAvg) +  " , \"Attached\": " + String(sensorAttached) + " }";  
 
  lcd.setCursor(0,1);               // Move the cursor to row 1, column 0
  lcd.print(displaycsv);                   // The count is displayed every second
  if (wifiPulseFlag>0){
      HTTPClient http;

      String request = "https://" + String(IOT_HUB_NAME) + ".azure-devices.net/devices/" + String(DEVICE_ID) + "/messages/events?api-version=2018-04-01";
      Serial.print("Sending WIFI: ");
      Serial.print(request);
      Serial.print(jsonPayload);
      Serial.print(" -> Result: ");
      http.begin(request);
      http.addHeader("Content-Type", "application/json");
      http.addHeader("Authorization", SAS_TOKEN);
      
      int httpStatus = http.POST(jsonPayload);
      response = http.getString();
      Serial.println(httpStatus);
      http.end();
      skipNextBeat = true;
  }
}
