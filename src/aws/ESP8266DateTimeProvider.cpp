/*
 * Copyright Sander van de Graaf (svdgraaf)
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

#include <aws/ESP8266DateTimeProvider.h>

ESP8266DateTimeProvider::ESP8266DateTimeProvider() : timeClient()
{
  strcpy(dateTime,"20120101000000");
}

const char* ESP8266DateTimeProvider::getDateTime()
{
  sync(0);
  return dateTime;
}

bool ESP8266DateTimeProvider::syncTakesArg(void)
{
    return false;
}

void ESP8266DateTimeProvider::sync(const char* dt)
{
  if (!timeClient.connect("aws.amazon.com", 80)) {
    Serial.println("Could not connect to timeserver. Using old timestamp");
    return;
  }

  // send a bad header on purpose, so we get a 400 with a DATE: timestamp
  timeClient.println("GET example.com/ HTTP/1.1");
  timeClient.println("Connection: close");
  timeClient.println();

  int timeout_busy = 0;
  while( ( !timeClient.available() ) && ( timeout_busy++ < 5000 ) ) {
    // Wait until the client sends some data
    delay(1);
  }

  // kill client if timeout
  if(timeout_busy >= 5000) {
    timeClient.flush();
    timeClient.stop();
    Serial.println("Timeout receiving timeserver data. Using old timestamp");
    return;
  }

  // read the http GET Response
  String req2 = timeClient.readString();

  // close connection
  delay(1);
  timeClient.flush();
  timeClient.stop();

  int ipos = req2.indexOf("Date:");
  if(ipos > 0) {
    String gmtDate = req2.substring(ipos, ipos + 35);
    String utctime = gmtDate.substring(18,22) + getMonth(gmtDate.substring(14,17)) + gmtDate.substring(11,13) + gmtDate.substring(23,25) + gmtDate.substring(26,28) + gmtDate.substring(29,31);
    utctime.substring(0, 14).toCharArray(dateTime, 15);
  }
}

String ESP8266DateTimeProvider::getMonth(String sM) {
  if(sM=="Jan") return "01";
  if(sM=="Feb") return "02";
  if(sM=="Mar") return "03";
  if(sM=="Apr") return "04";
  if(sM=="May") return "05";
  if(sM=="Jun") return "06";
  if(sM=="Jul") return "07";
  if(sM=="Aug") return "08";
  if(sM=="Sep") return "09";
  if(sM=="Oct") return "10";
  if(sM=="Nov") return "11";
  if(sM=="Dec") return "12";
  return "01";
}
