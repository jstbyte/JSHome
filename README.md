# Instruction!
> API: Environment: ESP_RCTRL > Required Data & Conf. Files Below;
```ts
type DevInfo = { id: string; mac: string; digioutCount: number };

/* ( CA Cert Embaded Into Program Memory in app_mqtt.h 'maybe change in future' ) */
const topicDevInfoSubscribe = "{SECRAT}/req/devinfo"; // No Data Required;
const topicDevInfoPublish = "{SECRAT}/req/devinfo/{id}"; // Data: DevInfo;
const topicDigiOutSubscribe = "{SECRAT}/req/digiout/{id}"; // Data eg. `index:state`;
const topicDigiOutPublish = "{SECRAT}/res/digiout/{id}"; // Data eg. Ordered `[states]`;

/* Pin No. 13 Used By IR Reciver Sensor */
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