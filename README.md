# Instruction!
> Environment: ESP_RCTRL > Require "credentials.h"
```cpp

#pragma once
#define HOST_NAME "Device Name"
#define WIFI_SSID "SSID Name"
#define WIFI_PASS "Paskey"

#define SERIAL_DEBUG_LOG /* ENABLE DEBUGING */

#define WIFI_RETRY_TIMEOUT 0 /* WiFi Only: 1 */

#define IR_RECV_PIN 13

#define MQTT_HOST "broker.emqx.io"
#define MQTT_PORT 8883

#define MQTT_TOPIC_DIGI_OUT "secrat.domian.dev/req/digiout/+"
#define REGX_TOPIC_DIGI_OUT "^secrat.domian.dev\\/req\\/digiout\\/([1-8])$"

/* ESPNOW PARAMS */
#define ESPNOW_CHANNEL 6
#define ESPNOW_GATEWAY_MAC_ADDR            \
    {                                      \
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff \
    }

```
> Environment: ESP_RCTRL > Required Data Files;
- ( CA Cert Embaded Into Program Memory in app_mqtt.h )
- dio.json ( Json array of pin numbers ) eg. [5, 4, 14, 12] !DON'T USE LED_BUILDIN PIN!