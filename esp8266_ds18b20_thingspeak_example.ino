/*************
  Steve David
**************/

// Including the ESP8266 WiFi library
#include <ESP8266WiFi.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "ThingSpeak.h"

#define SECRET_SSID "MySSID"                                // replace MySSID with your WiFi network name
#define SECRET_PASS "MyPassword"                            // replace MyPassword with your WiFi password

#define SECRET_CH_ID 123456789                              // replace with your channel number
#define SECRET_WRITE_APIKEY "ABCDEFGHIJKLM"                 // replace ABCDEFGHIJKLM with your channel write API Key

#define COLLECTION_INTERVAL_MS 15000

unsigned long myChannelNumber = SECRET_CH_ID;
const char * myWriteAPIKey = SECRET_WRITE_APIKEY;

// Replace with your network details
char ssid[] = SECRET_SSID;   // your network SSID (name)
char pass[] = SECRET_PASS;   // your network password

// Variables to hold temperature
char temperatureCString[7];
char temperatureFString[7];

// Data wire is plugged into pin D1 on the ESP8266 12-E - GPIO 5
#define ONE_WIRE_BUS 5

#define LED_RED 4
#define LED_BLUE 14
#define LED_WHITE 12

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Setup a WiFiClient instance
WiFiClient  client;

// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature DS18B20(&oneWire);

// only runs once on boot
void setup() {
  // Initializing serial port for debugging purposes
  Serial.begin(9600);
  delay(10);

  pinMode(LED_RED, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  pinMode(LED_WHITE, OUTPUT);
  
  DS18B20.begin(); // IC Default 9 bit. If you have troubles consider upping it 12. Ups the delay giving the IC more time to process the temperature measurement
  
  // Connecting to WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  digitalWrite(LED_BLUE, HIGH);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  digitalWrite(LED_BLUE, LOW);
  digitalWrite(LED_WHITE, HIGH);
  
  WiFi.mode(WIFI_STA);

  ThingSpeak.begin(client);
  
  // Printing the ESP IP address
  Serial.println(WiFi.localIP());
}

void getTemperature() {
  float tempC;
  float tempF;
  do {
    DS18B20.requestTemperatures(); 
    tempC = DS18B20.getTempCByIndex(0);
    dtostrf(tempC, 2, 2, temperatureCString);
    tempF = DS18B20.getTempFByIndex(0);
    dtostrf(tempF, 3, 2, temperatureFString);
    delay(100);
  } while (tempC == 85.0 || tempC == (-127.0));
}

// runs over and over again
void loop() {

    // Connect or reconnect to WiFi
  if (WiFi.status() != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    digitalWrite(LED_BLUE, HIGH);

    Serial.println(SECRET_SSID);
    while (WiFi.status() != WL_CONNECTED) {
      WiFi.begin(ssid, pass); // Connect to WPA/WPA2 network. Change this line if using open or WEP network
      Serial.print(".");
      delay(5000);
    }
    Serial.println("\nConnected.");
    digitalWrite(LED_BLUE, LOW);

  }
  
  // Read sensor and populate temperature variables.
  getTemperature();
  Serial.println(temperatureFString);

  digitalWrite(LED_BLUE, HIGH);

  // Write value to Field 1 of a ThingSpeak Channel
  int httpCode = ThingSpeak.writeField(myChannelNumber, 1, temperatureFString, myWriteAPIKey);

  digitalWrite(LED_BLUE, LOW);

  if (httpCode == 200) {
    Serial.println("Channel write successful.");
      digitalWrite(LED_RED, LOW);
  }
  else {
    Serial.println("Problem writing to channel. HTTP error code " + String(httpCode));
    digitalWrite(LED_RED, HIGH);
  }
  
  delay(COLLECTION_INTERVAL_MS);
}   
