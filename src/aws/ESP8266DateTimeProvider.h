/*
 * Copyright 2010-2015 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * Modified 2017 joekickass
 *
 * Licensed under the Apache License, Version 2.0 (the "License").
 * You may not use this file except in compliance with the License.
 * A copy of the License is located at
 *
 *  http://aws.amazon.com/apache2.0
 *
 * or in the "license" file accompanying this file. This file is distributed
 * on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 */

 /* Modified version of ESP8266AWSImplementations.h
  * https://github.com/heskew/aws-sdk-arduino/blob/master/src/esp8266/ESP8266AWSImplementations.h
  */

#ifndef ESP8266DATETIMEPROVIDER_H_
#define ESP8266DATETIMEPROVIDER_H_

#include <ESP8266WiFi.h>

#include "aws-sdk-arduino/DeviceIndependentInterfaces.h"

class ESP8266DateTimeProvider : public IDateTimeProvider
{
  public:
    ESP8266DateTimeProvider();

    /* Retrieve the current GMT date and time in yyyyMMddHHmmss format. */
    virtual const char* getDateTime(void);

    /* Return true if the sync function requires the current time as in
     * argument. */
    virtual bool syncTakesArg(void);

    /* Called if AWS Service reports in accurate time. Sets the provider to
     * current time. If syncTakesArg() returns true, this argument takes the
     * current GMT date and time in yyyyMMddHHmmss format. Else, the nput value
     * is ignored and may be null. */
    virtual void sync(const char* dt);

  private:

    /* DateTime in yyyyMMddHHmmss format */
    char dateTime[15];

    WiFiClient timeClient;

    String getMonth(String sM);
};

#endif
