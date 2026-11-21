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
#define RFID_RST 22

//Global variables
String uid;
const int SCREEN_WIDTH = 128; // OLED display width, in pixels
const int SCREEN_HEIGHT = 64; // OLED display height, in pixels
const int NUM_CARDS = 2; // Number of allowed cards
const String allowedCards[NUM_CARDS] = {"A7 53 2E 06", "53 4E 04 56"}; // Allowed card UIDs

// Create instances of the libraries
Adafruit_BMP280 bmp; // Object for BMP280 sensor
Adafruit_SSD1306 display(128, 64, &Wire, -1);
MFRC522 rfid(RFID_SS, RFID_RST); // Create MFRC522 instance

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

  // RFID code
  SPI.begin();
  rfid.PCD_Init();
  delay(10);
  rfid.PCD_DumpVersionToSerial();
  Serial.println("Tap a card...");
}

void loop() {
  if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
    String uid = getUID();
    Serial.print("Card UID: ");
    Serial.println(uid);

    if (isAuthorized(uid)) {
      Serial.println("Access granted");
    } else {
      Serial.println("Access denied");
    }

    rfid.PICC_HaltA();
  }
}