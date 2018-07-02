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

#ifndef AWSCONNECTIONPARAMS_H_
#define AWSCONNECTIONPARAMS_H_

#include "ws/WebSocketClientAdapter.h"
#include "mqtt/MqttClient.h"
#include "aws/AwsIotSigv4.h"

class AWSConnectionParams : public WebSocketParams, public MqttParams
{
  public:
    AWSConnectionParams(AwsIotSigv4& sigv4);
    ~AWSConnectionParams();

    // TCP params
    char* getHost();
    unsigned int getPort();

    // WS params
    char* getPath();
    char* getFingerprint();
    char* getProtocol();
    bool useSsl();

    // MQTT params
    unsigned int getVersion();
    char* getClientId();

  private:

    AwsIotSigv4 sigv4;

    char* path;
};

#endif
