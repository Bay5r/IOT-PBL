// Libraries needed for the project
  //Sensor Libraries
#include <Arduino.h>
#include <Wire.h> 
#include <SPI.h>
#include <SD.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <MFRC522.h>

  //WiFi Library 
#include <WiFi.h> 
#include <WebServer.h>
// BLE Library if selected as +1
#include "BLE_UART.h"

// Pin definitions
#define RFID_SS 5
#define RFID_RST 27

#define GREEN_LED 16
#define RED_LED 17

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_DC 25
#define OLED_CS 26
#define OLED_RST 33
#define OLED_CLK 14
#define OLED_MOSI 13




//Global variables
  // Variables for RFID card access control
String uid;
const int NUM_CARDS = 2; // Number of allowed cards
const String allowedCards[NUM_CARDS] = {"A7 53 2E 06", "53 4E 04 56"}; // Allowed card UIDs
unsigned long indicatorStart = 0; // Variable to store the start time of the indicator
unsigned long indicatorTime = 0; // Variable to store the duration of the indicator

  // Variables for indicator LED control
bool indicatorOn = false; // Variable to track the state of the indicator
unsigned long lastScanTime = 0; // Variable to store the last scan time
  
  // Variables for temperature reading
float temperature; // Variable to store the temperature reading
unsigned long lastTempRead = 0; // Variable to store the last temperature reading time
const int TEMP_INTERVAL = 2000; // Interval for temperature readings in milliseconds
const int MAX_TEMP_READINGS = 10; // Maximum number of readings to store
float temperatureReadings[MAX_TEMP_READINGS] = {0}; // Array to store the temperature readings
int readingIndex = 0; // Index for storing temperature readings
int readingCount = 0; // Count of valid temperature readings
  
  //Variable for WiFi and BLE
const char* ssid = "Bayer"; //WiFi SSID
const char* password = "12345678"; //WiFi password

// Creating library objects
Adafruit_BMP280 bmp; // Object for BMP280 sensor
MFRC522 rfid(RFID_SS, RFID_RST); // Create MFRC522 instance 
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT,&SPI, OLED_DC, OLED_RST, OLED_CS); // Create SSD1306 display object


//BMP280 Functions
void storeTemperatureReading(float temp) {
  temperatureReadings[readingIndex] = temp;
  readingIndex = (readingIndex + 1) % MAX_TEMP_READINGS;
  if (readingCount < MAX_TEMP_READINGS) {
    readingCount++;
  }
}

float calculateAverageTemperature() {
  if (readingCount == 0) {
    Serial.println("No temperature readings available.");
    return 0;
  }
    float sum = 0;
    for (int i = 0; i < readingCount; i++) {
    sum += temperatureReadings[i];
  }
   float average = sum / readingCount;
    Serial.print("Average Temperature: ");
    Serial.print(average);
    Serial.println(" C");
    return average;
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
  SPI.begin();
  display.begin(SSD1306_SWITCHCAPVCC);
  Wire.begin(21,22);
  Serial.println("Program Started");

  //OLED Display Code
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);

  //BMP280 Sensor Code
  
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
  rfid.PCD_Init();
  delay(10);
  rfid.PCD_DumpVersionToSerial();
  Serial.println("Tap a card...");

  //LED Code
  pinMode(GREEN_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  
  //WiFi Code
  WiFi.softAP(ssid, password);
  
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
    float temperature = bmp.readTemperature(); 
    float averageTemperature = calculateAverageTemperature();
    lastTempRead = millis(); 
    storeTemperatureReading(temperature);
  } 
 //OLED Display Code
 // Currnet Issus of screen not printing avarage temperature (come back later) 
  display.clearDisplay();
  display.setCursor(0, 20);
  display.print("Average Temperature: ");
  display.print(calculateAverageTemperature());
  display.println(" C");
  display.display();
}