#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include "OneWire.h"
#include "DallasTemperature.h"

const char *ssid = "your ssid";
const char *password = "your password";

const char *url = "your influx data cloud url";
const char *fingerPrint = "your influx data cloud ssl certificate fingerprint";
const char *headerName = "Authorization";
const char *headerValue = "Token <your token>";
const String data = "esp8266,mytag=1 roomDegree=";

int oneWirePin = 4;     // 18B20 sensor
int greenLedPin = 16;        // for blink ;)
int redLedPin = 5;      // for error blink ;(
bool lastGreenLedStatus;     // blink led status
bool lastRedLedStatus;     // blink led status

OneWire oneWire(oneWirePin);
DallasTemperature sensors(&oneWire);

void greenBlink() {
    if (lastGreenLedStatus) {
        digitalWrite(greenLedPin, HIGH);
        lastGreenLedStatus = false;
    } else {
        digitalWrite(greenLedPin, LOW);
        lastGreenLedStatus = true;
    }
}

void redBlink() {
    if (lastRedLedStatus) {
        digitalWrite(redLedPin, HIGH);
        lastRedLedStatus = false;
    } else {
        digitalWrite(redLedPin, LOW);
        lastRedLedStatus = true;
    }
}

void resetGreenStatus() {
    digitalWrite(greenLedPin, LOW);
    lastGreenLedStatus = true;
}

void resetRedStatus() {
    digitalWrite(redLedPin, LOW);
    lastRedLedStatus = true;
}

void initializeLedPinsAndStatus() {
    pinMode(greenLedPin, OUTPUT);
    pinMode(redLedPin, OUTPUT);

    resetGreenStatus();
    resetRedStatus();
}

void initializeClientAndSendData(float degree) {
    WiFiClientSecure wiFiClient;
    wiFiClient.setFingerprint(fingerPrint);

    HTTPClient client;
    client.begin(wiFiClient, url);
    client.addHeader(headerName, headerValue);

    String degreeStr = String(degree);

    int httpCode = client.POST(data + degreeStr);   // Send the request
    Serial.println(httpCode);   // Print HTTP return code

    client.end();  // Close connection
}

void setup() {
    Serial.begin(115200);
    delay(10);

    initializeLedPinsAndStatus();

    // connect to wifi
    Serial.println("");
    Serial.println("Connecting to: ");
    Serial.println(ssid);

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        redBlink();
        delay(500);
        Serial.print(".");
    }

    resetRedStatus();

    Serial.println("");
    Serial.println("WiFi connected.");

    sensors.begin();
    delay(10);
    Serial.println("Sensors started.");
}

void loop() {
    connected:
    if (WiFi.status() == WL_CONNECTED) {

        greenBlink();
        sensors.requestTemperatures();
        float degree = sensors.getTempCByIndex(0);

        Serial.print("Temperature: ");
        Serial.print(degree);
        Serial.println("");
        initializeClientAndSendData(degree);

    } else {

        resetGreenStatus();
        Serial.println("WiFi connection lost. Reconnect.");

        WiFi.disconnect(false);
        delay(10);
        WiFi.begin(ssid, password);

        while (WiFi.status() != WL_CONNECTED) {
            redBlink();
            delay(500);
            Serial.print(".");
        }

        resetRedStatus();
        Serial.println("");
        goto connected;
    }

    delay(10000); // loop every ten seconds
}
