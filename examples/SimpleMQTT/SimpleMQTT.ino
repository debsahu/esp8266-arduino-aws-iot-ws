#include <ESP8266WiFi.h>
#include <ArduinoJson.h>          //https://github.com/bblanchon/ArduinoJson (use v6.xx)
#include <ESP8266AWSIoTMQTTWS.h>  //https://github.com/debsahu/esp8266-arduino-aws-iot-ws
                                  //https://github.com/Links2004/arduinoWebSockets
                                  //https://projects.eclipse.org/projects/technology.paho/downloads (download Arduino version)

const char *ssid = "WIFI_SSID";
const char *password = "WIFI_PASSWORD";

// See `src/aws_iot_config.h` for formatting
char *region = (char *) "us-east-1";
char *endpoint = (char *) "xxxxxxxxxxxx";
char *mqttHost = (char *) "xxxxxxxxxxxx.iot.us-east-1.amazonaws.com";
int mqttPort = 443;
char *iamKeyId = (char *) "yyyyyyyyyyyyyyyyyy";
char *iamSecretKey = (char *) "YYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY";
const char* aws_topic  = "$aws/things/ZZZZZZZZZZZZ/shadow/update";

#if !(ARDUINOJSON_VERSION_MAJOR == 6 and ARDUINOJSON_VERSION_MINOR == 5)
  #error "Install ArduinoJson v6.5.0-beta"
#endif

ESP8266DateTimeProvider dtp;
AwsIotSigv4 sigv4(&dtp, region, endpoint, mqttHost, mqttPort, iamKeyId, iamSecretKey);
AWSConnectionParams cp(sigv4);
AWSWebSocketClientAdapter adapter(cp);
AWSMqttClient client(adapter, cp);

void setup() {

    Serial.begin(115200);
    while(!Serial) {
        yield();
    }

    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }

    int res = client.connect();
    Serial.printf("mqtt connect=%d\n", res);

    if (res == 0) {
      client.subscribe(aws_topic, 1,
        [](const char* topic, const char* msg)
        { Serial.printf("Got msg '%s' on topic %s\n", msg, topic); }
      );
    }
}

void loop() {
  if (client.isConnected()) {
    DynamicJsonDocument jsonBuffer;
    JsonObject root = jsonBuffer.to<JsonObject>();
    JsonObject state = root.createNestedObject("state");
    JsonObject state_reported = state.createNestedObject("reported");
    state_reported["value"] = random(100);
    serializeJson(root, Serial);
    Serial.println();
    char shadow[measureJson(root) + 1];
    serializeJson(root, shadow, sizeof(shadow));

    client.publish(aws_topic, shadow, 0, false);
    client.yield();

  } else {
    Serial.println("Not connected...");
    delay(2000);
  }

  delay(30000);
}