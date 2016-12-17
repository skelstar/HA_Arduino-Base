#include <PubSubClient.h>
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include "wificonfig.h"

/*
 * https://home-assistant.io/blog/2015/10/11/measure-temperature-with-esp8266-and-report-to-mqtt/
 */



/* ------------------------------------------------------------ */

WiFiClient espClient;
PubSubClient client(espClient);

String strTopic;
String strPayload;
int ledPin = 0;
String switch1;

void callback(char* topic, byte* payload, unsigned int length) {
    payload[length] = '\0';
    strTopic = String((char*)topic);
    if (strTopic == "ha/switch1") {
        switch1 = String((char*)payload);
        if (switch1 == "ON") {
            Serial.println("ON");
            digitalWrite(ledPin, HIGH);
        }
        else {
            Serial.println("OFF");
            digitalWrite(ledPin, LOW);
        }
    } 
}
 
void reconnect() {
    // Loop until we're reconnected
    while (!client.connected()) {
        Serial.print("Attempting MQTT connection...");
        // Attempt to connect
        if (client.connect("arduinoClient")) {  //, mqtt_user, mqtt_password)) {
            Serial.println("connected");
            // Once connected, publish an announcement...
            client.subscribe("ha/#");
        } else {
            Serial.print("failed, rc=");
            Serial.print(client.state());
            Serial.println(" try again in 5 seconds");
            // Wait 5 seconds before retrying
            delay(5000);
        }
    }
}

void setup()
{
    Serial.begin(115200);
    Serial.println("Booting");

    pinMode(ledPin, OUTPUT);

    setupWifi();
    setupOTA("Firmata-Base");

    client.setServer(mqtt_server, 1883);
    client.setCallback(callback);
}

/*==============================================================================
 * LOOP()
 *============================================================================*/
void loop()
{
    if (!client.connected()) {
        reconnect();
    }
    client.loop();
}

void setupWifi() {

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    while (WiFi.waitForConnectResult() != WL_CONNECTED) {
        Serial.println("Connection Failed! Rebooting...");
        delay(5000);
        ESP.restart();
    }
}

void setupOTA(char* host) {
    
    ArduinoOTA.setHostname(host);
    ArduinoOTA.onStart([]() {
        Serial.println("Start");
    });
    ArduinoOTA.onEnd([]() {
        Serial.println("\nEnd");
    });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    });
    ArduinoOTA.onError([](ota_error_t error) {
        Serial.printf("Error[%u]: ", error);
        if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
        else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
        else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
        else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
        else if (error == OTA_END_ERROR) Serial.println("End Failed");
    });

    ArduinoOTA.begin();
}
