#pragma once

#if defined(ESP8266)
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>
#define c_webServer ESP8266WebServer
#define c_httpUpdateServer ESP8266HTTPUpdateServer
#elif defined(ESP32)
#include <WebServer.h>
#include "ESP32HTTPUpdateServer.h"
#define c_webServer WebServer
#define c_httpUpdateServer ESP32HTTPUpdateServer
#endif

#include <ArduinoJson.h>

#include "JsonConfiguration.h"

class HttpServer
{
public:
  HttpServer();
  virtual ~HttpServer();

  void setup(void);
  void handle(void);

  c_webServer &webServer();

  String getContentType(String filename);
  bool handleFileRead(String path);
  void handleNotFound();
  void handleSet();
  void getStatus();
  void getConfig();
  void setConfig();
  void sendJson(const uint16_t code, JsonDocument &doc);

private:
  c_webServer _webServer;
  c_httpUpdateServer _httpUpdater;
};

#if !defined(NO_GLOBAL_INSTANCES)
extern HttpServer HTTPServer;
#endif
