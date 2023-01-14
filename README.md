# Instructions!

```ts

/* data/config.json  */ {
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

/* ( CA Cert Embaded Into Program Memory in certs.h 'maybe change in future' ) */
/* TOPIC FORMAT: `{SECRAT}/req|res/${service}/?{hostname}` */
/* Sonoff:: Pin No. 13 Used By IR Reciver Sensor */

const topics = [
  {topic: 'devinfo', payload: `?sync`},
  {topic: 'update', payload: `?{URL}`},
  {topic: 'sonoff' , payload: `[index]:[state];...`}
]

// TODO: Future:: https://api.github.com/repos/jstbyte/Jiot/releases/latest;

```