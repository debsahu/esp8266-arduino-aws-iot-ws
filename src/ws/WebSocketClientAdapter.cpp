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

#include "ws/WebSocketClientAdapter.h"

WebSocketParams::~WebSocketParams() {}

AWSWebSocketClientAdapter::AWSWebSocketClientAdapter(WebSocketParams& p, size_t bufferSize) :
  ws(),
  params(p),
  isConnected(false)
{
  fifo.init(bufferSize);
  ws.onEvent([=] (WStype_t type, uint8_t * payload, size_t length) {
    webSocketEvent(type, payload, length);
  });
}

AWSWebSocketClientAdapter::~AWSWebSocketClientAdapter()
{
}

void AWSWebSocketClientAdapter::webSocketEvent(WStype_t type, uint8_t * payload, size_t length)
{
  switch(type) {
    case WStype_DISCONNECTED:
      isConnected = false;
      break;
    case WStype_CONNECTED:
      isConnected = true;
      break;
    case WStype_TEXT:
      fifo.push(payload, length);
      break;
    case WStype_BIN:
      fifo.push(payload, length);
      break;
  }
}

/*
 * Completely disregards arguments. Uses config values instead.
 *
 * Returns true if successful, false otherwise
 */
int AWSWebSocketClientAdapter::connect(IPAddress ip, uint16_t p)
{
  return connect(0, 0);
}

/*
 * Completely disregards arguments. Uses config values instead.
 *
 * Returns true if successful, false otherwise
 */
 int AWSWebSocketClientAdapter::connect(const char *h, uint16_t p)
{
  const char* host = params.getHost();
  const int port = params.getPort();
  const char* path = params.getPath();
  const char* fingerprint = params.getFingerprint();
  const char* protocol = params.getProtocol();
  const bool useSsl = params.useSsl();

  if (useSsl) {
    ws.beginSSL(host, port, path, fingerprint, protocol);
  } else {
    ws.begin(host, port, path, fingerprint);
  }

  // Wait until connected
  long connectionTimeout = 5000;
  long now = millis();
  while((millis() - now) < connectionTimeout) {
    ws.loop();
    if(connected()) return true;
    delay (10);
  }
  return false;
}

size_t AWSWebSocketClientAdapter::write(uint8_t b)
{
  if (!connected())
    return 0;

  return write (&b,1);;
}

size_t AWSWebSocketClientAdapter::write(const uint8_t *buf, size_t size)
{
  if (!connected())
    return 0;

  if (ws.sendBIN(buf,size))
  	  return size;

  return 0;
}

int AWSWebSocketClientAdapter::available()
{
  if (!connected())
    return false;

  ws.loop();

  return fifo.getSize();
}

int AWSWebSocketClientAdapter::read()
{
  if (!connected())
    return EXIT_FAILURE;

  return fifo.pop();
}

int AWSWebSocketClientAdapter::read(uint8_t *buf, size_t size)
{
  if (!connected())
    return EXIT_FAILURE;

  int s = (fifo.getSize() < size) ? fifo.getSize() : size;
  fifo.pop(buf, s);

  return s;
}

int AWSWebSocketClientAdapter::peek()
{
  if (!connected())
    return EXIT_FAILURE;

  return fifo.peek();
}

void AWSWebSocketClientAdapter::flush()
{
  // do nothing
}

void AWSWebSocketClientAdapter::stop()
{
  if(connected()) {
    isConnected = false;
    fifo.clear();
  }
  ws.disconnect();
}

uint8_t AWSWebSocketClientAdapter::connected()
{
  return isConnected;
}

AWSWebSocketClientAdapter::operator bool()
{
  return isConnected;
}
