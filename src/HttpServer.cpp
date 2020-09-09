#if defined(ESP8266)
#include <LittleFS.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPUpdateServer.h>
#define c_FS LittleFS
#elif defined(ESP32)
#include <SPIFFS.h>
#include <ESPmDNS.h>
#define c_FS SPIFFS
#endif

#include <WiFiManager.h>

#include "HttpServer.h"
#include "Logger.h"
#include "Mqtt.h"

// You can update by 'curl -F "image=@firmware.bin" ESP_Viewer.local/update'

/********************************************************/
/******************** Public Method *********************/
/********************************************************/

HttpServer::HttpServer()
    : _webServer(80), _httpUpdater(true)
{
}

HttpServer::~HttpServer()
{
}

void HttpServer::setup(void)
{
  MDNS.begin(Configuration._hostname.c_str());
  MDNS.addService("http", "tcp", 80);

  _webServer.on("/restart", [&]() {
    _webServer.sendHeader("Access-Control-Allow-Origin", "*");
    _webServer.send(200, "text/plain", "ESP restart now !");
    delay(1000);
    ESP.restart();
  });

  _webServer.on("/reset", [&]() {
    _webServer.sendHeader("Access-Control-Allow-Origin", "*");
    _webServer.send(200, "text/plain", "Reset WifiManager configuration, restart now in AP mode...");
    delay(200);
    WiFiManager wifiManager;
    wifiManager.resetSettings();
    delay(200);
    ESP.restart();
  });

  _webServer.on("/", [&]() {
    _webServer.sendHeader("Access-Control-Allow-Origin", "*");
    _webServer.client().println(String(F("<h1>ESP Viewer</h1>")));
    _webServer.client().println(String("<h3>" + Configuration._hostname + "</h3>"));
    _webServer.client().println(String(F("<p>Version: ")) + F(VERSION) + F("</p>"));
    _webServer.client().println(String(F("<p>Build: ")) + F(__DATE__) + " " + F(__TIME__) + F("</p>"));
    _webServer.client().println(String(F("<p><a href=\"update\">Update ESP</a></p>")));
    _webServer.client().println(String(F("<p><a href=\"restart\">Restart</a></p>")));
    _webServer.client().println(String(F("</br>")));
    _webServer.client().println(String(F("<p><a href=\"reset\">Reset WiFi configuration</a></p>")));
  });

  _webServer.onNotFound([&]() { handleNotFound(); });

  _httpUpdater.setup(&_webServer, String("/update"));
  _webServer.begin();
}

void HttpServer::handle(void)
{
  _webServer.handleClient();

#if defined(ESP8266)
  MDNS.update();
#endif
}

String HttpServer::getContentType(String filename)
{
  if (_webServer.hasArg("download"))
    return "application/octet-stream";
  else if (filename.endsWith(".htm"))
    return "text/html";
  else if (filename.endsWith(".html"))
    return "text/html";
  else if (filename.endsWith(".css"))
    return "text/css";
  else if (filename.endsWith(".js"))
    return "application/javascript";
  else if (filename.endsWith(".png"))
    return "image/png";
  else if (filename.endsWith(".gif"))
    return "image/gif";
  else if (filename.endsWith(".jpg"))
    return "image/jpeg";
  else if (filename.endsWith(".ico"))
    return "image/x-icon";
  else if (filename.endsWith(".xml"))
    return "text/xml";
  else if (filename.endsWith(".pdf"))
    return "application/x-pdf";
  else if (filename.endsWith(".zip"))
    return "application/x-zip";
  else if (filename.endsWith(".gz"))
    return "application/x-gzip";
  return "text/plain";
}

// send the right file to the client (if it exists)
bool HttpServer::handleFileRead(String path)
{
  Log.println("handleFileRead: " + path);
  if (path.endsWith(F("/")))
    path += "index.html"; // If a folder is requested, send the index file

  String contentType = getContentType(path); // Get the MIME type

  String pathWithGz = path + F(".gz");
  if (c_FS.exists(pathWithGz)) // If there's a compressed version available
    path += ".gz";                 // Use the compressed verion

  if (c_FS.exists(path))
  {
    File file = c_FS.open(path, "r"); // Open the file
    _webServer.sendHeader("Access-Control-Allow-Origin", "*");
    _webServer.streamFile(file, contentType); // Send it to the client
    file.close();                             // Close the file again
    Log.println(String("\tSent file: ") + path);
    return true;
  }
  Log.println(String("\tFile Not Found: ") + path); // If the file doesn't exist, return false
  return false;
}

void HttpServer::handleNotFound()
{
  if (!handleFileRead(_webServer.uri()))
  {
    String message = "File Not Found\n\n";
    message += "URI: ";
    message += HTTPServer.webServer().uri();
    message += "\nMethod: ";
    message += (HTTPServer.webServer().method() == HTTP_GET) ? "GET" : "POST";
    message += "\nArguments: ";
    message += HTTPServer.webServer().args();
    message += "\n";
    for (uint8_t i = 0; i < HTTPServer.webServer().args(); i++)
    {
      message += " " + HTTPServer.webServer().argName(i) + ": " + HTTPServer.webServer().arg(i) + "\n";
    }
    HTTPServer.webServer().send(404, "text/plain", message);
  }
}

c_webServer &HttpServer::webServer()
{
  return _webServer;
}

/********************************************************/
/******************** Private Method ********************/
/********************************************************/

void HttpServer::sendJson(const uint16_t code, JsonDocument &doc)
{
  WiFiClient client = _webServer.client();

  // Write Header
  client.print(F("HTTP/1.0 "));
  client.print(code);
  client.println(F(" OK"));
  client.println(F("Content-Type: application/json"));
  client.println(F("Access-Control-Allow-Origin: *"));
  client.print(F("Content-Length: "));
  client.println(measureJson(doc));
  client.println(F("Connection: close"));
  client.println();

  // Write JSON document
  serializeJson(doc, client);
}

#if !defined(NO_GLOBAL_INSTANCES)
HttpServer HTTPServer;
#endif
