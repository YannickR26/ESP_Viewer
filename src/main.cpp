#include <Arduino.h>
#include <WiFiManager.h>

#include "JsonConfiguration.h"
#include "Logger.h"
#include "Display.h"
#include "HttpServer.h"
#include "Mqtt.h"

/*************/
/*** SETUP ***/
/*************/

// Wifi setup
void wifiSetup()
{
  WiFiManager wifiManager;
  // wifiManager.setDebugOutput(false);
  // wifiManager.resetSettings();

  // WiFiManagerParameter
  WiFiManagerParameter custom_mqtt_hostname("hostname", "hostname", Configuration._hostname.c_str(), 60);
  WiFiManagerParameter custom_mqtt_server("mqttIpServer", "mqtt ip", Configuration._mqttIpServer.c_str(), 40);
  WiFiManagerParameter custom_mqtt_port("mqttPortServer", "mqtt port", String(Configuration._mqttPortServer).c_str(), 6);
  WiFiManagerParameter custom_mqtt_username("mqttUsername", "mqtt username", String(Configuration._mqttUsername).c_str(), 40);
  WiFiManagerParameter custom_mqtt_userpassword("mqttPassword", "mqtt password", String(Configuration._mqttPassword).c_str(), 40);

  // add all your parameters here
  wifiManager.addParameter(&custom_mqtt_hostname);
  wifiManager.addParameter(&custom_mqtt_server);
  wifiManager.addParameter(&custom_mqtt_port);
  wifiManager.addParameter(&custom_mqtt_username);
  wifiManager.addParameter(&custom_mqtt_userpassword);

  Log.println("Try to connect to WiFi...");
  wifiManager.setWiFiAPChannel(6);
  wifiManager.setConfigPortalTimeout(300); // Set Timeout for portal configuration to 300 seconds
  if (!wifiManager.autoConnect(Configuration._hostname.c_str()))
  {
    Log.println("failed to connect and hit timeout");
    delay(3000);
    //reset and try again, or maybe put it to deep sleep
    Log.println("Restart ESP now !");
    ESP.restart();
    delay(5000);
  }

  Log.println(String("Connected to ") + WiFi.SSID());
  Log.println(String("IP address: ") + WiFi.localIP().toString());

  WiFi.enableAP(false);
  WiFi.softAPdisconnect();

  /* Get configuration from WifiManager */
  Configuration._hostname = custom_mqtt_hostname.getValue();
  Configuration._mqttIpServer = custom_mqtt_server.getValue();
  Configuration._mqttPortServer = atoi(custom_mqtt_port.getValue());
  Configuration._mqttUsername = custom_mqtt_username.getValue();
  Configuration._mqttPassword = custom_mqtt_userpassword.getValue();
}

void setup()
{
  /* Initialize Logger */
  Log.setup();
  Log.println();
  Log.println("================================");
  Log.println(String(F("---------- ESP Viewer ----------")));
  Log.println(String(F("  Version: ")) + VERSION);
  Log.println(String(F("  Build: ")) + BUILD_DATE);
  Log.println("================================");
  Log.println();

  // Initialize Display
  Disp.setup();

  // Read configuration from SPIFFS
  Configuration.setup();

  wifiSetup();

  // Initialize HTTP Server
  HTTPServer.setup();

  // Initialize MQTT Client
  MqttClient.setup();

  Log.setupTelnet();
}

void loop()
{
  // call all handle
  Disp.handle();
  MqttClient.handle();
  Log.handle();
  HTTPServer.handle();
}