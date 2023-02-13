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
/* ( CA Cert Embaded Into Program Memory in certs.h 'maybe change in future' ) */
/* TOPIC FORMAT: `{SECRAT}/${DEVICE | *}/{req|res}/{SERVICES}#` */
/* Sonoff:: Pin No. 13 Used By IR Reciver Sensor */

const topics = [ // Regex Inside [...];
  {topic: 'update', payload: `?{URL}`},
  {topic: 'sonoff', payload: `[index]:[state];...`}
]

// TODO: Future:: https://api.github.com/repos/jstbyte/Jiot/releases/latest;
```