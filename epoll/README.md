gcc epoll_restapi_mqtt.c -o server

```bash

$mosquitto_pub -h test.mosquitto.org -t "test/topic" -m "Hello MQTT"
$curl -v http://localhost:8080/
```