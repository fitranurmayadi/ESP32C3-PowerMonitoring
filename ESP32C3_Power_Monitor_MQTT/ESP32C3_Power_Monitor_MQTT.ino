#include <WiFi.h>
#include "ThingSpeak.h"

#include <Wire.h>

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <PZEM004Tv30.h>



#define SCREEN_WIDTH 128  // OLED display width, in pixels
#define SCREEN_HEIGHT 64  // OLED display height, in pixels

#define OLED_RESET -1        // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C  ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

PZEM004Tv30 pzem(Serial1, 20, 21);


//wifi configuration
const char* ssid = "your network ssid";             // your network SSID (name)
const char* password = "your network password";  // your network password

WiFiClient client;

//thingspeak configuration
unsigned long myChannelNumber = 4;
const char* myWriteAPIKey = "your API KEY";

// Timer variables
unsigned long lastTime = 0;
unsigned long timerDelay = 30000;


void setup() {
  Serial.begin(115200);
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ;  // Don't proceed, loop forever
  }

  WiFi.mode(WIFI_STA);
  ThingSpeak.begin(client);  // Initialize ThingSpeak

  display.clearDisplay();
  display.setTextSize(1);  // Draw 2X-scale text
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(10, 0);
  display.println(F("starting"));
  display.display();  // Show initial text
  delay(100);
  // Uncomment in order to reset the internal energy counter
  // pzem.resetEnergy();
}

void loop() {

  Serial.print("Custom Address:");
  Serial.println(pzem.readAddress(), HEX);

  // Read the data from the sensor
  float voltage = pzem.voltage();
  float current = pzem.current();
  float power = pzem.power();
  float energy = pzem.energy();
  float frequency = pzem.frequency();
  float pf = pzem.pf();

  // Check if the data is valid
  if (isnan(voltage)) {
    Serial.println("Error reading voltage");
  } else if (isnan(current)) {
    Serial.println("Error reading current");
  } else if (isnan(power)) {
    Serial.println("Error reading power");
  } else if (isnan(energy)) {
    Serial.println("Error reading energy");
  } else if (isnan(frequency)) {
    Serial.println("Error reading frequency");
  } else if (isnan(pf)) {
    Serial.println("Error reading power factor");
  } else {

    // Print the values to the Serial console
    Serial.print("Voltage: ");
    Serial.print(voltage);
    Serial.println("V");
    Serial.print("Current: ");
    Serial.print(current);
    Serial.println("A");
    Serial.print("Power: ");
    Serial.print(power);
    Serial.println("W");
    Serial.print("Energy: ");
    Serial.print(energy, 3);
    Serial.println("kWh");
    Serial.print("Frequency: ");
    Serial.print(frequency, 1);
    Serial.println("Hz");
    Serial.print("PF: ");
    Serial.println(pf);

    display.clearDisplay();
    display.setCursor(0, 0);
    display.print(F("Voltage : "));
    display.print(voltage);
    display.println(F("V"));
    display.setCursor(0, 10);
    display.print(F("Current : "));
    display.print(current);
    display.println(F("A"));
    display.setCursor(0, 20);
    display.print(F("Power : "));
    display.print(power);
    display.println(F("W"));
    display.setCursor(0, 30);
    display.print(F("Energy : "));
    display.print(energy);
    display.println(F("KWh"));
    display.setCursor(0, 40);
    display.print(F("Frequency : "));
    display.print(frequency);
    display.println(F("Hz"));
    display.setCursor(0, 50);
    display.print(F("P. Factor : "));
    display.println(pf);
    display.display();  // Show initial text
  }

  Serial.println();
  delay(2000);

  if ((millis() - lastTime) > timerDelay) {

    // Connect or reconnect to WiFi
    if (WiFi.status() != WL_CONNECTED) {
      Serial.print("Attempting to connect");
      display.clearDisplay();
      display.setCursor(0, 0);
      display.print(F("Connecting to WiFi"));
      display.display();
      while (WiFi.status() != WL_CONNECTED) {
        WiFi.begin(ssid, password);
        delay(500);
        Serial.print(".");
      }
      Serial.println("\nConnected.");
    }

    // set the fields with the values
    ThingSpeak.setField(1, voltage);
    ThingSpeak.setField(2, current);
    ThingSpeak.setField(3, power);
    ThingSpeak.setField(4, energy);
    ThingSpeak.setField(5, frequency);
    ThingSpeak.setField(6, pf);

    // Write to ThingSpeak. There are up to 8 fields in a channel, allowing you to store up to 8 different
    int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);

    if (x == 200) {
      Serial.println("Channel update successful.");
    } else {
      Serial.println("Problem updating channel. HTTP error code " + String(x));
    }
    lastTime = millis();
  }
}
