#if defined(ESP8266)
#include <LittleFS.h>
#define c_FS LittleFS
#elif defined(ESP32)
#include <SPIFFS.h>
#define c_FS SPIFFS
#endif

#include "JsonConfiguration.h"
#include "Logger.h"

/********************************************************/
/******************** Public Method *********************/
/********************************************************/

JsonConfiguration::JsonConfiguration()
{
}

JsonConfiguration::~JsonConfiguration()
{
}

void JsonConfiguration::setup(void)
{
  /* Initialize FS */
  if (!c_FS.begin())
  {
    Log.println("failed to initialize FS, try to format");
    c_FS.format();
    if (!c_FS.begin())
    {
      Log.println("definitely failed to initialize FS !");
      return;
    }
  }

  if (!readConfig())
  {
    Log.println("Invalid configuration values, restoring default values");
    restoreDefault();
  }

  print();
}

void JsonConfiguration::print(void)
{
  Log.println(String("Current configuration :"));
  Log.println(String("    hostname: ") + _hostname);
  Log.println(String("    mqttIpServer: ") + _mqttIpServer);
  Log.println(String("    mqttPortServer: ") + String(_mqttPortServer));
  Log.println(String("    mqttUsername: ") + String(_mqttUsername));
  Log.println(String("    mqttPassword: ") + String(_mqttPassword));
}

bool JsonConfiguration::readConfig()
{
  Log.println("Read Configuration file from FS...");

  // Open file
  File configFile = c_FS.open("/config.json", "r");
  if (!configFile)
  {
    Log.println("Failed to open config file");
    return false;
  }

  uint16_t size = configFile.size();

  // Allocate a buffer to store contents of the file.
  std::unique_ptr<char[]> buf(new char[size]);

  configFile.readBytes(buf.get(), size);
  decodeJsonFromFile(buf.get());

  configFile.close();

  return true;
}

bool JsonConfiguration::saveConfig()
{
  // StaticJsonDocument<512> doc;
  DynamicJsonDocument doc(512);

  Log.print("Try to save config... ");

  encodeToJson(doc);

  File configFile = c_FS.open("/config.json", "w");
  if (!configFile)
  {
    Log.println("Error: Failed to open config file for writing");
    return false;
  }

  // Serialize JSON to file
  if (serializeJson(doc, configFile) == 0)
  {
    Log.println(F("Error: Failed to write to file"));
    return false;
  }

  configFile.close();

  Log.println("Done !");

  return true;
}

void JsonConfiguration::restoreDefault()
{
  _hostname = DEFAULT_HOSTNAME;
  _mqttIpServer = DEFAULT_MQTTIPSERVER;
  _mqttPortServer = DEFAULT_MQTTPORTSERVER;
  _mqttUsername = "";
  _mqttPassword = "";

  saveConfig();
  Log.println("configuration restored.");
}

uint8_t JsonConfiguration::encodeToJson(JsonDocument &_json)
{
  _json.clear();
  _json["hostname"] = _hostname;
  _json["mqttIpServer"] = _mqttIpServer;
  _json["mqttPortServer"] = _mqttPortServer;
  _json["mqttUsername"] = _mqttUsername;
  _json["mqttPassword"] = _mqttPassword;

  return 0;
}

uint8_t JsonConfiguration::decodeJsonFromFile(const char *input)
{
  DynamicJsonDocument doc(512);

  doc.clear();
  // Deserialize the JSON document
  DeserializationError error = deserializeJson(doc, input);
  if (error)
  {
    Log.print("Failed to deserialize JSON, error: ");
    Log.println(error.c_str());
    restoreDefault();
    return -1;
  }

  if (!doc["hostname"].isNull())
    _hostname = doc["hostname"].as<String>();

  if (!doc["mqttIpServer"].isNull())
    _mqttIpServer = doc["mqttIpServer"].as<String>();

  if (!doc["mqttPortServer"].isNull())
    _mqttPortServer = doc["mqttPortServer"].as<uint16_t>();

  if (!doc["mqttUsername"].isNull())
    _mqttUsername = doc["mqttUsername"].as<String>();

  if (!doc["mqttPassword"].isNull())
    _mqttPassword = doc["mqttPassword"].as<String>();

  return 0;
}

/********************************************************/
/******************** Private Method ********************/
/********************************************************/

#if !defined(NO_GLOBAL_INSTANCES)
JsonConfiguration Configuration;
#endif
