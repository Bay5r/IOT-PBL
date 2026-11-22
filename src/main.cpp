// Libraries needed for the project
#include <Arduino.h>
#include <Wire.h> 
#include <SPI.h>
#include <SD.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h> 
#include <MFRC522.h>
// BLE Library if selected as +1
#include "BLE_UART.h"

// Pin definitions
#define RFID_SS 5
#define RFID_RST 27

#define GREEN_LED 16
#define RED_LED 17

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

//Global variables

String uid;
const int NUM_CARDS = 2; // Number of allowed cards
const String allowedCards[NUM_CARDS] = {"A7 53 2E 06", "53 4E 04 56"}; // Allowed card UIDs
unsigned long indicatorStart = 0; // Variable to store the start time of the indicator
unsigned long indicatorTime = 0; // Variable to store the duration of the indicator


bool indicatorOn = false; // Variable to track the state of the indicator
unsigned long lastScanTime = 0; // Variable to store the last scan time

float temperature; // Variable to store the temperature reading
unsigned long lastTempRead = 0; // Variable to store the last temperature reading time
const int TEMP_INTERVAL = 2000; // Interval for temperature readings in milliseconds
const int MAX_TEMP_READINGS = 10; // Maximum number of readings to store
float temperatureReadings[MAX_TEMP_READINGS] = {0}; // Array to store the temperature readings
int readingIndex = 0; // Index for storing temperature readings
int readingCount = 0; // Count of valid temperature readings

// Creating library objects
Adafruit_BMP280 bmp; // Object for BMP280 sensor
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
MFRC522 rfid(RFID_SS, RFID_RST); // Create MFRC522 instance

//BMP280 Functions
void storeTemperatureReading(float temp) {
  temperatureReadings[readingIndex] = temp;
  readingIndex = (readingIndex + 1) % MAX_TEMP_READINGS;
  if (readingCount < MAX_TEMP_READINGS) {
    readingCount++;
  }
}

void calculateAverageTemperature() {
  if (readingCount == 0) {
    Serial.println("No temperature readings available.");
    return;
  }
    float sum = 0;
    for (int i = 0; i < readingCount; i++) {
    sum += temperatureReadings[i];
  }
   float average = sum / readingCount;
    Serial.print("Average Temperature: ");
    Serial.print(average);
    Serial.println(" C");
}

//LED Functions
void startIndicatorTimer(unsigned long duration)
{
  indicatorStart = millis();
  indicatorTime = duration;
  indicatorOn = true;
}

void allIndicatorsOff()
{
  digitalWrite(GREEN_LED, LOW);
  digitalWrite(RED_LED, LOW);
}

void onAccessGranted()
{
  Serial.println("ACCESS GRANTED");
  allIndicatorsOff();
  digitalWrite(GREEN_LED, HIGH);
  startIndicatorTimer(2000);
}

void onAccessDenied()
{
  Serial.println("ACCESS DENIED");
  allIndicatorsOff();
  digitalWrite(RED_LED, HIGH);
  startIndicatorTimer(1000);
}

//Turns RFID card's UID bytes into text
String getUID()
{
  String uid = "";
  for (byte i = 0; i < rfid.uid.size; i++)
  {
    if (rfid.uid.uidByte[i] < 0x10)
    {
      uid = uid + "0";
    }
    uid = uid + String(rfid.uid.uidByte[i], HEX);
    if (i < rfid.uid.size - 1)
    {
      uid = uid + " ";
    }
  }
  uid.toUpperCase();
  return uid;
}
// Checks if card's UID is in list of allowed cards
bool isAuthorized(String uid) { 
  for (int i = 0; i < NUM_CARDS; i++) { 
    if (uid == allowedCards[i]) 
    { 
      return true; 
    } 
  } 
  return false; 
} 


void setup() {
  Serial.begin(115200);
  Serial.println("Program Started");

  //BMP280 Sensor Code
  Wire.begin(21,22);
  Serial.println();
  if (bmp.begin(0x76)) {
    Serial.println("BMP280 sensor initialized successfully at 0x76");
  } 
  else if (bmp.begin(0x77)) {
    Serial.println("BMP280 sensor initialized successfully at 0x77");
  }
  else {
    Serial.println("Could not find BMP280 sensor");
  }

  // RFID code
  SPI.begin();
  rfid.PCD_Init();
  delay(10);
  rfid.PCD_DumpVersionToSerial();
  Serial.println("Tap a card...");

  //LED Code
  pinMode(GREEN_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
}

void loop() {
  //RFID code
  if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
    String uid = getUID();
    Serial.print("Card UID: ");
    Serial.println(uid);

  if (isAuthorized(uid)) {
      onAccessGranted();
    } 
  else {
      onAccessDenied();
    }
      rfid.PICC_HaltA();
  }
  if (indicatorOn == true && millis() - indicatorStart >= indicatorTime) {
    allIndicatorsOff();
    indicatorOn = false;
  }
  //BMP280 Sensor Code
 if (millis() - lastTempRead >= TEMP_INTERVAL){
    temperature = bmp.readTemperature(); 
    calculateAverageTemperature();
    lastTempRead = millis(); 
    storeTemperatureReading(temperature);
 }  
}