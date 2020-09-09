#pragma once

#include <ArduinoJson.h>
#include "settings.h"

class JsonConfiguration 
{
  public:
  	JsonConfiguration();
  	virtual ~JsonConfiguration();
  
  	void setup();
	  
	void print(void);

    bool readConfig();
  	bool saveConfig();
  
  	void restoreDefault();
    
	uint8_t encodeToJson(JsonDocument &_json);
	uint8_t decodeJsonFromFile(const char* input);
  
	/* Members */
  	String _hostname;
	String _mqttIpServer;
	uint16_t _mqttPortServer;
	String _mqttUsername;
	String _mqttPassword;
     	
  private:
  
};

#if !defined(NO_GLOBAL_INSTANCES)
extern JsonConfiguration Configuration;
#endif
