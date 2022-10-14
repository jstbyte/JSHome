# Instruction!
> Environment: MY_ROOM > Require "credentials.h"
```cpp 
#pragma once

#define IR_RECV_PIN 13

#define WIFI_SSID "Your SSID"
#define WIFI_PASS "Your Password"

/* Secure Web Socket Only */
#define MQTT_HOST "broker.emqx.io"
#define MQTT_PORT 8883

/* MQTT PARAMS  */
const char *mqtt_topic_dio = "secrat.domain.dev/req/switch";

```
> Environment: MY_ROOM > Required Data Files;
- certs.ar ( Archive file containing .der certificates for MQTT client )
- dio.json ( Json array of pin numbers ) eg. [5, 4, 14, 12]