#include "Mqtt.h"

#include "JsonConfiguration.h"

#include <WiFi.h>
#include <WiFiManager.h>
#include "Logger.h"

WiFiClient espClient;

/********************************************************/
/******************** Public Method *********************/
/********************************************************/

Mqtt::Mqtt()
{
}

Mqtt::~Mqtt()
{
}

void Mqtt::setup()
{
  clientMqtt.setClient(espClient);
  clientMqtt.setServer(Configuration._mqttIpServer.c_str(), Configuration._mqttPortServer);
  clientMqtt.setCallback([this](char *topic, uint8_t *payload, unsigned int length) { this->callback(topic, payload, length); });
  startedAt = String(Log.getDateTimeString());
}

void Mqtt::handle()
{
  if (!clientMqtt.connected())
  {
    reconnect();
  }
  clientMqtt.loop();
}

void Mqtt::publish(String topic, String body)
{
  clientMqtt.publish(String(Configuration._hostname + '/' + topic).c_str(), String(body).c_str());
  delay(5);
}

void Mqtt::log(String level, String str)
{
  publish("log/" + level, str);
}

/********************************************************/
/******************** Private Method ********************/
/********************************************************/

void Mqtt::reconnect()
{
  static unsigned long tick = 0;

  if (!clientMqtt.connected())
  {
    if ((millis() - tick) >= 5000)
    {
      Log.print("Attempting MQTT connection... ");
      // Create a random clientMqtt ID
      String clientId = Configuration._hostname + String(random(0xffff), HEX);
      // Attempt to connect
      if (clientMqtt.connect(clientId.c_str(), 0, 1, 0, 0))
      {
        Log.println("connected !");
        // Once connected, publish an announcement...
        char *time = Log.getDateTimeString();
        publish(String("connectedFrom"), String(time));
        publish(String("version"), String(VERSION));
        publish(String("build"), String(String(__DATE__) + " " + String(__TIME__)));
        publish(String("ip"), WiFi.localIP().toString());
        publish(String("startedAt"), String(startedAt));
        // ... and resubscribe
        clientMqtt.subscribe(String(Configuration._hostname + "/set/#").c_str(), 1);
      }
      else
      {
        Log.print("failed, rc=");
        Log.print(String(clientMqtt.state()));
        Log.println(" try again in 5 seconds");
        tick = millis();
      }
    }
  }
}

void Mqtt::callback(char *topic, uint8_t *payload, unsigned int length)
{
  String data;

  Log.print("Message arrived [");
  Log.print(topic);
  Log.print("] ");
  for (unsigned int i = 0; i < length; i++)
  {
    data += (char)payload[i];
  }
  Log.println(data);

  String topicStr(topic);
  topicStr.remove(0, topicStr.lastIndexOf('/') + 1);
}

#if !defined(NO_GLOBAL_INSTANCES)
Mqtt MqttClient;
#endif
