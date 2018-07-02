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

#include "mqtt/MqttClient.h"

// Keep a global reference of the instance to use in FPT callback
AWSMqttClient* AWSMqttClient::instance = NULL;

MqttParams::~MqttParams() {}

AWSMqttClient::AWSMqttClient(AWSWebSocketClientAdapter& wsAdapter, MqttParams& p) :
  adapter(wsAdapter),
  ipstack(adapter),
  client(ipstack),
  params(p)
{
  AWSMqttClient::instance = this;
  for(int i = 0; i < AWS_IOT_MQTT_NUM_SUBSCRIBE_HANDLERS; ++i) {
    SubscriptionCallbacks[i].topic = 0;
    SubscriptionCallbacks[i].cb = NULL;
  }
}

AWSMqttClient::~AWSMqttClient()
{
}

int AWSMqttClient::connect()
{
  // make sure we're stopped
  adapter.stop();

  char* host = params.getHost();
  int port = params.getPort();
  int success = ipstack.connect(host, port);
  if (!success) {
    return -1;
  }

  MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
  data.MQTTVersion = params.getVersion();
  data.clientID.cstring = params.getClientId();

  return client.connect(data);
}

bool AWSMqttClient::isConnected()
{
  return adapter.connected() && client.isConnected();
}

void AWSMqttClient::yield()
{
  client.yield();
}

void AWSMqttClient::disconnect()
{
  client.disconnect();
}

int AWSMqttClient::publish(const char* topic, const char* payload, unsigned int qos, bool retained)
{
  // Need a non-const, null terminated string
  char pl[strlen(payload) + 1];
  snprintf(pl, strlen(payload) + 1, "%s", payload);
  MQTT::QoS qs = static_cast<MQTT::QoS>(qos); // Assuming default enum values
  return client.publish(topic, pl, strlen(payload) + 1, qs, retained);
}

int AWSMqttClient::subscribe(const char* topic, unsigned int qos, subscriptionCallback cb)
{
  MQTT::QoS qs = static_cast<MQTT::QoS>(qos); // Assuming default enum values
  addCallback(topic, cb);
  return client.subscribe(topic, qs,
    // Need to use a lamda w.o. capture list since MQTT::Client.messageHandler is PTF
    [](MQTT::MessageData& md) {
      // c strings from underlying implementation are not null terminated. Create new.
      char topic[md.topicName.lenstring.len + 1];
      snprintf(topic, md.topicName.lenstring.len + 1, "%s", md.topicName.lenstring.data);
      char msg[md.message.payloadlen + 1];
      snprintf(msg, md.message.payloadlen + 1, "%s", (char*)md.message.payload);
      instance->handleCallback(topic, msg);
    }
  );
}

void AWSMqttClient::unsubscribe(const char *topic)
{
  removeCallback(topic);
  client.unsubscribe(topic);
}

void AWSMqttClient::addCallback(const char* topic, subscriptionCallback cb)
{
  if (topic == NULL || cb == NULL) {
    return;
  }

  for(int i = 0; i < AWS_IOT_MQTT_NUM_SUBSCRIBE_HANDLERS; ++i) {
    if (SubscriptionCallbacks[i].topic == 0) {
      SubscriptionCallbacks[i].topic = topic;
      SubscriptionCallbacks[i].cb = cb;
      break;
    }
  }
}

void AWSMqttClient::removeCallback(const char* topic)
{
  if (topic == NULL) {
    return;
  }

  for(int i = 0; i < AWS_IOT_MQTT_NUM_SUBSCRIBE_HANDLERS; ++i) {
    if (strcmp(SubscriptionCallbacks[i].topic, topic) == 0) {
      SubscriptionCallbacks[i].topic = 0;
      SubscriptionCallbacks[i].cb = NULL;
      break;
    }
  }
}

subscriptionCallback AWSMqttClient::getCallback(const char* topic)
{
  if (topic == NULL) {
    return NULL;
  }

  for(int i = 0; i < AWS_IOT_MQTT_NUM_SUBSCRIBE_HANDLERS; ++i) {
    if (strcmp(SubscriptionCallbacks[i].topic, topic) == 0) {
      return SubscriptionCallbacks[i].cb;
    }
  }
  return NULL;
}

void AWSMqttClient::handleCallback(const char* topic, const char* msg)
{
  subscriptionCallback cb = getCallback(topic);
  if (topic != NULL && cb != NULL) {
    cb(topic, msg);
  }
}
