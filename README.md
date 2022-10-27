# Instruction!
> Environment: ESP_RCTRL > Required Data & Conf. Files Below;
```js
/* ( CA Cert Embaded Into Program Memory in app_mqtt.h ) */

/* data/config/digio_pins.json */ [5, 4, 14, 12]
/* data/config/digio_stat.json */ [1, 1, 1, 1]   

/* data/config/espn_conf.json  */ {
  "channel": 6,
  "gateway": "FF:FF:FF:FF:FF:FF"
}

/* data/config/wlan_conf.json  */ {
  "identity": "secrat.domain.ext",
  "wlanSSID": "ssid@sta",
  "wlanPASS": "pass@sta",
  "hostNAME": "dev@name",
  "mqttHOST": "broker.emqx.io",
  "mqttPORT": 8883
}
```