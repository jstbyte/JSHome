# Instructions!

> Config For All
```ts

/* data/wconfig.json  */ {
  "identity": "secrat.domain.ext",
  "wlanSSID": "ssid@sta",
  "wlanPASS": "pass@sta",
  "hostNAME": "dev@name",
  "mqttHOST": "broker.emqx.io",
  "mqttPORT": 8883
}

type DevInfo = {
  mac: string;
  name: string;
  services: { name: string; data: any }[];
};

/* ( CA Cert Embaded Into Program Memory in app_mqtt.h 'maybe change in future' ) */
const topicDevInfoSubscribe = "{SECRAT}/req/devinfo"; // No Data Required;
const topicDevInfoPublish = "{SECRAT}/req/devinfo/{name}"; // Data: DevInfo;
```

> ENV::rSONOFF > Required Data & Conf. Files Below;
```ts
/* Pin No. 13 Used By IR Reciver Sensor */
/* data/rsonoff.json */ [5, 4, 14, 12]
const topicSonoffSubscribe = "{SECRAT}/req/sonoff/{name}"; // Data eg. `index:state`;
const topicSonoffPublish = "{SECRAT}/res/sonoff/{name}"; // Data eg. Ordered `[states]`;
```
> ENV::rDoor > Required Data & Conf. Files Below;
```ts
/* Pin No. 13 Used By IR Reciver Sensor */
const topicDoorSubscribe = "{SECRAT}/req/door/{name}"; // Data eg. (time in sec.)|1|0|255;
const topicDoorPublish = "{SECRAT}/res/door/{name}"; // Data eg. 1|0|255;
```