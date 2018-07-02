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

#include "config/AWSConnectionParams.h"
#include "aws_iot_config.h"

AWSConnectionParams::AWSConnectionParams(AwsIotSigv4& sigv4) :
  sigv4(sigv4)
{
}

AWSConnectionParams::~AWSConnectionParams()
{
  if (path != 0) {
    delete[] path;
  }
}

char* AWSConnectionParams::getHost()
{
  return sigv4.awsHost;
}

unsigned int AWSConnectionParams::getPort()
{
  return sigv4.awsPort;
}

char* AWSConnectionParams::getPath()
{
  if (path == 0) {
    sigv4.createPath(&path);
  }
  return path;
}

char* AWSConnectionParams::getFingerprint()
{
  return "";
}

char* AWSConnectionParams::getProtocol()
{
  return "mqtt";
}

bool AWSConnectionParams::useSsl()
{
  return true;
}

unsigned int AWSConnectionParams::getVersion()
{
  return 4;
}
char* AWSConnectionParams::getClientId()
{
  return AWS_IOT_MQTT_CLIENT_ID;
}
