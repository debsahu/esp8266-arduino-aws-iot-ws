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

/* Modified version of AwsClient2
 * https://github.com/awslabs/aws-sdk-arduino/blob/master/src/common/AWSClient2.h
 *
 * Using simplified algorithm for mqtt/websockets
 * http://docs.aws.amazon.com/iot/latest/developerguide/protocols.html#mqtt-ws
 */

#ifndef AWSIOTSIGV4_H_
#define AWSIOTSIGV4_H_

#include <string.h>
#include <stdio.h>

#include "aws-sdk-arduino/DeviceIndependentInterfaces.h"

/* HTTP VERB */
static const char* METHOD = "GET";
/* URI scheme */
static const char* PROTOCOL = "wss";
/* URI path */
static const char* PATH = "/mqtt";
/* AWS service */
static const char* SERVICE = "iotdevicegateway";
/* Hash algorithm */
static const char* ALGORITHM = "AWS4-HMAC-SHA256";

/* GMT date in yyyyMMdd format (not including terminating null char) */
static const int DATE_LEN = 8;
/* GMT time in HHmmss format (not including terminating null char) */
static const int TIME_LEN = 6;
/* Size of sha hashes and signatures in hexidecimal */
static const int HASH_HEX_LEN = 64;
/* Size of algorithm (not including terminating null char) */
static const int ALG_LEN = strlen(ALGORITHM);
/* Size of access key id (not including terminating null char) */
static const int ACCESS_KEY_ID_LEN = 20;
/* Size of secret key (not including terminating null char) */
static const int SECRET_KEY_LEN = 40;
/* Size of query string (not including terminating null char) */
// TODO: Tweak size?
static const int QS_LEN = ALG_LEN + SECRET_KEY_LEN + DATE_LEN + TIME_LEN + 200;
/* Size of canonical request (not including terminating null char) */
// TODO: Tweak size?
static const int CR_LEN = 400;

/*
 * AwsIotSigv4
 *
 * A ws connection is initiated by sending a HTTP GET request using the URL:
 *  wss://<endpoint>.iot.<region>.amazonaws.com/mqtt
 */
class AwsIotSigv4
{
  public:
    AwsIotSigv4(IDateTimeProvider* dtp,
                char *region,
                char *endpoint,
                char *mqttHost,
                int mqttPort,
                char *iamKeyId,
                char *iamSecretKey);
    ~AwsIotSigv4();

    /*
     * Create a Sigv4 signed HTTP upgrade request (aka websocket handshake) to
     * the AWS IoT service.
     *
     * Example:
     *   wss://A2MBBEONHC9LUG.iot.us-east-1.amazonaws.com/mqtt?X-Amz-Algorithm=AWS4-HMAC-SHA256&X-Amz-Credential=AKIAIOSFODNN7EXAMPLE%2F20170508%2Feu-west-1%2Fiotdevicegateway%2Faws4_request&X-Amz-Date=20170508T123058Z&X-Amz-SignedHeaders=host&X-Amz-Signature=5d945eab451da3756ef5ad9bf1b1741dae26f5416d5e754d2ad260497df42d23
     * Caller must free memory
     */
    size_t createRequest(char** out);

    /*
     * Create the path part of a Sigv4 signed HTTP upgrade request (aka
     * websocket handshake) to the AWS IoT service.
     *
     * Example:
     *  /mqtt?X-Amz-Algorithm=AWS4-HMAC-SHA256&X-Amz-Credential=AKIAIOSFODNN7EXAMPLE%2F20170508%2Feu-west-1%2Fiotdevicegateway%2Faws4_request&X-Amz-Date=20170508T123058Z&X-Amz-SignedHeaders=host&X-Amz-Signature=5d945eab451da3756ef5ad9bf1b1741dae26f5416d5e754d2ad260497df42d23
     *
     * Caller must free memory
     */
    size_t createPath(char** out);

    /* Region, e.g. "us-east-1" in "A2MBBEONHC9LUG.iot.us-east-1.amazonaws.com" */
    char* awsRegion;

    /* Endpoint, e.g. "A2MBBEONHC9LUG" in "A2MBBEONHC9LUG.iot.us-east-1.amazonaws.com" */
    char* awsEndpoint;

    /* Host, e.g. "A2MBBEONHC9LUG.iot.us-east-1.amazonaws.com" */
    char* awsHost;

    /* Port, e.g. 443 */
    int awsPort;

    /* The user's AWS Access Key ID for accessing the AWS Resource. */
    char* awsKeyId;

    /* The user's AWS Secret Key for accessing the AWS Resource. */
    char* awsSecretKey;

  private:
    /* Used to keep track of time. */
    IDateTimeProvider* dateTimeProvider;

    /* First step of sigv4 signing */
    void createCanonicalRequest(char* out, char* qs, char* payloadHash);

    /* Second step of sigv4 signing */
    void createStringToSign(char *out, char* date, char* time, char* cs, char* canonicalRequestHash);

    /* Third step of sigv4 signing.
     * Given the string to sign, create the signature (a 64-char cstring) */
    void createSignature(char* out, char* sts, char* date);

    /* Fourth and last step of sigv4 signing, divided up in 2 methods
     * Add signature to request */
    void addSignatureToQueryString(char* out, char* signature);
    void addSignatureQSToRequest(char* out, char* qs);

    /* Calculate the size of the final request string based on queryString */
    int getRequestLength(char* queryString);

    /* Calculate the size of the credential scope string */
    int getCredentialScopeLength();

    /* Get the credential scope */
    void getCredentialScope(char* out, char* date);

    /* Calculate the size of the credential string */
    int getCredentialStringLength();

    /* Get the credential string (which is access key id + credential scope) */
    void getCredentialString(char* out, char* date);

    /* Get the canonical query string */
    void getQueryString(char* out, char* cs, char* date, char* time);

    /* Get the current date and time */
    void getDateTime(char* outDate, char* outTime);

    /* Create payloadHash from empty payload */
    void getPayloadHash(char* out);

    /* Create canonical request hash */
    void getCanonicalRequestHash(char* out, char* canonicalRequest);
};

#endif
