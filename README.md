# Instruction!
> Environment: ESP_RCTRL > Require "credentials.h"
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
> Environment: ESP_RCTRL > Required Data Files;
- MQTT_CERT.h ( CA Cert Embaded Into Program Memory )
- dio.json ( Json array of pin numbers ) eg. [5, 4, 14, 12]