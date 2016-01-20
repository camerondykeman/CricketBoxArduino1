#include "DHT.h"

//Wire for master/slave arduino connection
#include <Wire.h>

//SD
#include <SPI.h>
#include <SD.h>

//DHT sensor.
#define DHTTYPE DHT11   // DHT 11
#define DHTPIN 2        // DHT = Pin2
DHT dht(DHTPIN, DHTTYPE);

const int FAN_POWER = 3;

//Heat Index
const int HEATINDEX_BAD = 7;
const int HEATINDEX_POWER = 4;  //This is box#4 (lower right)
const int HEATINDEX_MIN = 84;
const int HEATINDEX_MAX = 88;

//Humidity
const int HUMIDITY_BAD = 8;
const int HUMIDITY_POWER = 5;
const int HUMIDITY_MIN = 30;
const int HUMIDITY_MAX = 40;


void setup() {
  Serial.begin(9600);
  // Start the I2C Bus as Master
  Wire.begin(); 
  //Start DHT Sensor
  dht.begin();

  // Initialize Temperature LED Outputs
  pinMode(HEATINDEX_BAD, OUTPUT);
  pinMode(HEATINDEX_POWER, OUTPUT);
  // Initialize Humidity LED Outputs
  pinMode(HUMIDITY_BAD, OUTPUT);
  pinMode(HUMIDITY_POWER, OUTPUT);
  // Initialize Fan Output
  pinMode(FAN_POWER, OUTPUT);

  // see if the card is present and can be initialized:
  if (!SD.begin()) {
    Serial.println("SDCard failed, or not present");
    // don't do anything more:
    return;
  }
  Serial.println("SDCard initialized.");
  
}


void loop() {
  
  float t = checkTemperature();
  float h = checkHumidity();
  float i = checkHeatIndex(t, h);
  
  outputDHT(t,h,i);
  bool adjustHeatIndexBool = adjustHeatIndex(i);
  bool adjustHumidityBool = adjustHumidity(h);

  if(adjustHeatIndexBool || adjustHumidityBool){
    Serial.println("FAN ON");
    digitalWrite(FAN_POWER, LOW);
  }
  else{
    Serial.println("FAN OFF");
    digitalWrite(FAN_POWER, HIGH);
  }

  //Display using Slave arduino
  //Wire.beginTransmission(1); // open transmission line to device #1
  //byte myData[] = {i, h};
  //Wire.write(2);              // que i and h in line
  //Wire.endTransmission();    // send qued bytes to slave device

  // Wait a few seconds between measurements.
  delay(2000);
}

//Returns TEMP
float checkTemperature(){
  float t = dht.readTemperature(true); //Fahrenheit
  //float t = dht.readTemperature(); 
  return t;
}

//Returns HUMIDITY
float checkHumidity(){
  float h = dht.readHumidity(); // Read temperature as Celsius (the default)
  return h;
}
//Checks if the current HUMIDITY is high
bool isHumidityHigh(float max_, float value) {
  return value > max_;
}
//Checks if the current HUMIDITY is low
bool isHumidityLow(float min_, float value) {
  return value < min_;
}
//Act on current HUMIDITY reading
bool adjustHumidity(float h){
  //BlueLED if HUMIDITY is out of range
  if (isHumidityHigh(HUMIDITY_MAX, h) || isHumidityLow(HUMIDITY_MIN, h)) {
    digitalWrite(HUMIDITY_BAD, HIGH);
  } 
  else{
    Serial.println("HUMIDITY GOOD");
    digitalWrite(HUMIDITY_BAD, LOW);
  }
  //Run Humidifier if low
  bool adjustHumidityBool = 0;
  if(isHumidityLow(HUMIDITY_MIN, h)){
    Serial.println("HUMIDITY LOW");
    digitalWrite(HUMIDITY_POWER, LOW);
  }
  else{
    digitalWrite(HUMIDITY_POWER, HIGH);
  }
  //Run fan if high
  if(isHumidityHigh(HUMIDITY_MAX, h)){
    adjustHumidityBool = 1;
    Serial.println("HUMIDITY HIGH");
  }
  return adjustHumidityBool;
}

//Returns HEATINDEX
float checkHeatIndex(float t, float h){
  float i = dht.computeHeatIndex(t, h, true);   //Fahrenheit
  //float i = dht.computeHeatIndex(t, h, false);
  return i;
}
//Checks if the current HEATINDEX is too high
bool isHeatIndexHigh(float max_, float value) {
  return value > max_;
}
//Checks if the current HEATINDEX is too low
bool isHeatIndexLow(float min_, float value) {
  return value < min_;
}
//Act on current HEATINDEX reading
bool adjustHeatIndex(float i){
  //RedLED if HEATINDEX is out of range
  if (isHeatIndexHigh(HEATINDEX_MAX, i) || isHeatIndexLow(HEATINDEX_MIN, i)) {
    digitalWrite(HEATINDEX_BAD, HIGH);
  } 
  else{
    Serial.println("HEAT GOOD");
    digitalWrite(HEATINDEX_BAD, LOW);
  }
  //Run Heater if low
  bool adjustHeatIndex = 0;
  if(isHeatIndexLow(HEATINDEX_MIN, i)){
    Serial.println("HEAT LOW");
    digitalWrite(HEATINDEX_POWER, LOW);
  }
  else{
    digitalWrite(HEATINDEX_POWER, HIGH);
  }
  //Run fan if high
  if(isHeatIndexHigh(HEATINDEX_MAX, i)){
    Serial.println("HEAT HIGH");
    adjustHeatIndex = 1;
  }
  return adjustHeatIndex;
}

//Output results from DHT
void outputDHT(float t, float h, float i){
  if (isnan(t) || isnan(h) || isnan(i)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  } 

  Serial.print("Humidity: ");
  Serial.print(h);
  Serial.print(" %\t");
  Serial.print("Temperature: ");
  //Serial.print(t);
  //Serial.print(" *C\t");
  Serial.print(t);
  Serial.print(" *F\t");
  Serial.print("Heat index: ");
  Serial.print(i);
  Serial.println(" *C ");

  //Record to SDCard
  //String dataString = "";
  //File dataFile = SD.open("datalog.txt", FILE_WRITE);
  //if (dataFile) {
  //  dataFile.println(dataString);
  //  dataFile.close();
  //  // print to the serial port too:
  //  Serial.println(dataString);
  //}
}


