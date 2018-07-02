/*
 * Copyright (C) 2017 Tomas Nilsson (joekickass)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at

 *    http://www.apache.org/licenses/LICENSE-2.0

 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef MQTTCLIENT_H_
#define MQTTCLIENT_H_

#include <IPStack.h>
#include <Countdown.h>
#include <MQTTClient.h>

#include "ws/WebSocketClientAdapter.h"

// TODO Refactor away config here
#include "aws_iot_config.h"

// (const char* topic, const char* payload)
typedef void (*subscriptionCallback) (const char*, const char*);

struct {
  const char* topic;
  subscriptionCallback cb;
} SubscriptionCallbacks[AWS_IOT_MQTT_NUM_SUBSCRIBE_HANDLERS];

/*
 * MqttParams provides connection parameters for the MqttClient
 *
 * Using an abstract class instead of a plain data struct allows parameters that
 * are more dynamic in nature, such as signatures based on current date/time.
 * An example of this is AWS sigv4 signature authentication
 */
class MqttParams
{
public:
  virtual char* getHost()           =0;
  virtual unsigned int getPort()    =0;
  virtual unsigned int getVersion() =0;
  virtual char* getClientId()       =0;
  virtual ~MqttParams()             =0;
};

/**
 * MQTT client running MQTT over WebSockets.
 *
 * Assumes WebSockets handles security using TLS.
 *
 * TODO: Also assumes null terminated c strings as payload (i.e. json data)
 * Some information on the AWS IoT Websocket+MQTT protocols
 * http://docs.aws.amazon.com/iot/latest/developerguide/protocols.html
 */
class AWSMqttClient {

  public:

    AWSMqttClient(AWSWebSocketClientAdapter& a, MqttParams& p);
    ~AWSMqttClient();

    //Establish a Websocket connection and connect to the MQTT host.
    //Returns 0 if successful, or non-zero otherwise
    int connect();

    bool isConnected();

    void yield();

    void disconnect();

    // Publish to topic
    // Returns 0 if successful, or non-zero otherwise.
    // TODO: Remove retained?
    int publish(const char* topic, const char* payload, unsigned int qos, bool retained);

    // Subscribe to topic
    // Returns 0 if successful, or non-zero otherwise.
    int subscribe(const char* topic, unsigned int qos, subscriptionCallback cb);

    void unsubscribe(const char* topic);

  private:

    // Paho MQTT client callbacks are PTFs. Using a global variable to hold object reference
    // Note that this will break if more than one instance of AWSMqttClient is created
    static AWSMqttClient* instance;

    AWSWebSocketClientAdapter& adapter;
    IPStack ipstack;
    // TODO Remove config params
    MQTT::Client<IPStack, Countdown, AWS_IOT_MQTT_TX_BUF_LEN, AWS_IOT_MQTT_NUM_SUBSCRIBE_HANDLERS> client;
    MqttParams& params;

    void addCallback(const char* topic, subscriptionCallback cb);
    void removeCallback(const char* topic);

    subscriptionCallback getCallback(const char* topic);
    void handleCallback(const char* topic, const char* msg);
};

#endif
