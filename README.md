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
  uid: number;
  mac: string;
  services: { name: string; data: any }[];
};
/* ( CA Cert Embaded Into Program Memory in app_mqtt.h 'maybe change in future' ) */
const topicDevInfoSubscribe = "{SECRAT}/req/devinfo"; // No Data Required;
const topicDevInfoPublish = "{SECRAT}/req/devinfo/{id}"; // Data: DevInfo;
```

> ENV::rSONOFF > Required Data & Conf. Files Below;
```ts
/* Pin No. 13 Used By IR Reciver Sensor */
/* data/rsonoff.json */ [5, 4, 14, 12]
const topicSonoffSubscribe = "{SECRAT}/req/sonoff/{id}"; // Data eg. `index:state`;
const topicSonoffPublish = "{SECRAT}/res/sonoff/{id}"; // Data eg. Ordered `[states]`;
```
> ENV::rDoor > Required Data & Conf. Files Below;
```ts
/* Pin No. 13 Used By IR Reciver Sensor */
const topicDoorSubscribe = "{SECRAT}/req/door/{id}"; // Data eg. (time in sec.)|1|0|255;
const topicDoorPublish = "{SECRAT}/res/door/{id}"; // Data eg. 1|0|255;
```