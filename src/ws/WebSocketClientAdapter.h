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

#ifndef WEBSOCKETCLIENTADAPTER_H_
#define WEBSOCKETCLIENTADAPTER_H_

#include <Client.h>
#include <Hash.h>
#include <WebSocketsClient.h>

#include "ws/CircularByteBuffer.h"

/*
 * WebSocketParams provides connection parameters for the AWSWebSocketClientAdapter
 *
 * Using an abstract class instead of a plain data struct allows parameters that
 * are more dynamic in nature, such as signatures based on current date/time.
 * An example of this is AWS sigv4 signature authentication
 */
class WebSocketParams
{
public:
  virtual char* getHost()        =0;
  virtual unsigned int getPort() =0;
  virtual char* getPath()        =0;
  virtual char* getFingerprint() =0;
  virtual char* getProtocol()    =0;
  virtual bool useSsl()          =0;
  virtual ~WebSocketParams()     =0;
};

/**
 * Implements the Arduino Client interface used by mqtt client and IpStack,
 * see https://github.com/esp8266/Arduino/blob/master/cores/esp8266/Client.h
 *
 * Delegates to WebSockets for Arduino.
 *
 * Note that adapting the Client.h interface to a websocket implementation can
 * not really be done in a good way. Client.h is a TCP/IP layer interface and
 * uses only hosts and ports for addressing. Websockets, on the other hand, is a
 * higher layer protocol. It's an independent TCP-based protocol with a
 * handshake that mimics a HTTP upgrade request, after which it establishes an
 * open, two-way connection. Websockets thus uses port 80 (or 443 for TLS) and
 * URL addressing (most notably path and protocol arguments), which is not
 * really possible to squeeze into a TCP/IP interface. This is instead passed
 * to the adapter at creation time.
 */
class AWSWebSocketClientAdapter : public Client
{
public:

  AWSWebSocketClientAdapter(WebSocketParams& p, size_t bufferSize = 1000);
  ~AWSWebSocketClientAdapter();

  // Arduino Client.h interface
  virtual int connect(IPAddress ip, uint16_t port);
  virtual int connect(const char *host, uint16_t port);
  virtual size_t write(uint8_t b);
  virtual size_t write(const uint8_t *buf, size_t size);
  virtual int available();
  virtual int read();
  virtual int read(uint8_t *buf, size_t size);
  virtual int peek();
  virtual void flush();
  virtual void stop();
  virtual uint8_t connected();
  virtual operator bool();

private:

  // Websocket implementation
  WebSocketsClient ws;

  // Used for buffering data when reading/writing
  CircularByteBuffer fifo;

  // Connection params used instead of connect() arguments
  WebSocketParams& params;

  // Tracks connection state
  bool isConnected;

  // Callback handling websocket events
  void webSocketEvent(WStype_t type, uint8_t * payload, size_t length);
};

#endif
