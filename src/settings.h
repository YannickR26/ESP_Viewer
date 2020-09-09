#pragma once

// Version
#define VERSION "V1.0.0"
#define BUILD_DATE String((__DATE__) + String(" ") + (__TIME__))

#define DEFAULT_HOSTNAME "ESP_Viewer"
#define DEFAULT_MQTTIPSERVER "192.168.1.201"
#define DEFAULT_MQTTPORTSERVER 1883

// Timezone
#define UTC_OFFSET +1

// change for different ntp (time servers)
#define NTP_SERVERS "0.fr.pool.ntp.org", "time.nist.gov", "pool.ntp.org"
#define EPOCH_1_1_2019 1546300800
