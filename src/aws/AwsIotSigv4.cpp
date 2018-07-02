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
 */
#include "aws-sdk-arduino/sha256.h"
#include "aws-sdk-arduino/Utils.h"

#include "AwsIotSigv4.h"
#include "aws_iot_config.h"

AwsIotSigv4::AwsIotSigv4(IDateTimeProvider *dtp,
                         char *region,
                         char *endpoint,
                         char *mqttHost,
                         int mqttPort,
                         char *iamKeyId,
                         char *iamSecretKey) :
        awsRegion(region),
        awsEndpoint(endpoint),
        awsHost(mqttHost),
        awsPort(mqttPort),
        awsKeyId(iamKeyId),
        awsSecretKey(iamSecretKey),
        dateTimeProvider(dtp)
{
}

AwsIotSigv4::~AwsIotSigv4()
{
}

size_t AwsIotSigv4::createRequest(char** out)
{
  char* path;
  size_t pathLen = createPath(&path);
  size_t reqLen = strlen(awsHost) + pathLen + 7;
  *out = new char[reqLen];
  sprintf(*out, "wss://%s%s", awsHost, path);
  delete[] path;
  return reqLen;
}

size_t AwsIotSigv4::createPath(char** out)
{
  if (dateTimeProvider == 0) {
    return 0;
  }

  // step 1
  char date[DATE_LEN + 1];
  char time[TIME_LEN + 1];
  getDateTime(date, time);

  int csLen = getCredentialStringLength();
  char credentialString[csLen + 1];
  getCredentialString(credentialString, date);

  char queryString[QS_LEN + 1];
  getQueryString(queryString, credentialString, date, time);

  char payloadHash[HASH_HEX_LEN + 1];
  getPayloadHash(payloadHash);

  char canonicalRequest[CR_LEN];
  createCanonicalRequest(canonicalRequest, queryString, payloadHash);

  char canonicalRequestHash[HASH_HEX_LEN + 1];
  getCanonicalRequestHash(canonicalRequestHash, canonicalRequest);

  // step 2
  csLen = getCredentialScopeLength();
  char credentialScope[csLen + 1];
  getCredentialScope(credentialScope, date);
  char stringToSign[ALG_LEN + DATE_LEN + TIME_LEN + csLen + HASH_HEX_LEN + 6];
  createStringToSign(stringToSign, date, time, credentialScope, canonicalRequestHash);

  // step 3
  char signature[HASH_HEX_LEN];
  createSignature(signature, stringToSign, date);

  // step 4
  addSignatureToQueryString(queryString, signature);

  int requestLen = getRequestLength(queryString);
  (*out) = new char[requestLen]();
  addSignatureQSToRequest(*out, queryString);

  return requestLen;
}

/* CanonicalRequest =
 *   HTTPRequestMethod + '\n' +
 *   CanonicalURI + '\n' +
 *   CanonicalQueryString + '\n' +
 *   CanonicalHeaders + '\n' +
 *   SignedHeaders + '\n' +
 *   HexEncode(Hash(RequestPayload))
 */
void AwsIotSigv4::createCanonicalRequest(char* out, char* qs, char* payloadHash)
{
  // Method, URI
  sprintf(out, "%s\n", METHOD);
  sprintf(out, "%s%s\n", out, PATH);

  // Query String
  sprintf(out, "%s%s\n", out, qs);

  // Canonical headers. Only host is used.
  sprintf(out, "%shost:%s:%d\n\n", out, awsHost, awsPort);

  // Signed headers
  sprintf(out, "%shost\n", out);

  // Payload hash
  sprintf(out, "%s%s", out, payloadHash);
}

void AwsIotSigv4::createStringToSign(char *out, char* date, char* time, char* cs, char* canonicalRequestHash)
{
	sprintf(out, "%s\n%sT%sZ\n%s\n%s", ALGORITHM, date, time, cs, canonicalRequestHash);
}

void AwsIotSigv4::createSignature(char* out, char* sts, char* date)
{
  int keyLen = SECRET_KEY_LEN + 4;
  char* key = new char[keyLen + 1];
  sprintf(key, "AWS4%s", awsSecretKey);

  char* k1 = hmacSha256(key, keyLen, date, DATE_LEN);
  delete[] key;

  char* k2 = hmacSha256(k1, SHA256_DEC_HASH_LEN, awsRegion, strlen(awsRegion));
  delete[] k1;

  char* k3 = hmacSha256(k2, SHA256_DEC_HASH_LEN, SERVICE, strlen(SERVICE));
  delete[] k2;

  char* k4 = hmacSha256(k3, SHA256_DEC_HASH_LEN, "aws4_request", 12);
  delete[] k3;

  char* k5 = hmacSha256(k4, SHA256_DEC_HASH_LEN, sts, strlen(sts));
  delete[] k4;

  /* Convert the chars in hash to hex for signature. */
  for (int i = 0; i < SHA256_DEC_HASH_LEN; ++i) {
      sprintf(out + 2 * i, "%02lx", 0xff & (unsigned long) k5[i]);
  }
  delete[] k5;
}

void AwsIotSigv4::addSignatureToQueryString(char* out, char* signature)
{
  sprintf(out, "%s&X-Amz-Signature=%s", out, signature);
}

void AwsIotSigv4::addSignatureQSToRequest(char* out, char* qs)
{
  sprintf(out, "%s%s?%s", out, PATH, qs);
}

int AwsIotSigv4::getRequestLength(char* queryString) {
  return strlen(PATH) + strlen(queryString)+2;
}

// Size of credential scope (not including terminating null char) */
int AwsIotSigv4::getCredentialScopeLength() {
  return DATE_LEN + strlen(awsRegion) + strlen(SERVICE) + 16;
}

// Create credential scope, e.g. "/<date>/<region>/<service>/aws4_request"
void AwsIotSigv4::getCredentialScope(char *out, char* date)
{
  sprintf(out, "%s/%s/%s/aws4_request", date, awsRegion, SERVICE);
}

// Size of credential scope (not including terminating null char) */
int AwsIotSigv4::getCredentialStringLength() {
  return ACCESS_KEY_ID_LEN + DATE_LEN + strlen(awsRegion) + strlen(SERVICE) + 24;
}

// Create credential string, e.g. "<access key id>/<date>/<region>/<service>/aws4_request"
// Must be URI encoded (i.e. '/' -> %2F and ' ' -> %20)
void AwsIotSigv4::getCredentialString(char *out, char* date)
{
  sprintf(out, "%s%%2F%s%%2F%s%%2F%s%%2Faws4_request", awsKeyId, date, awsRegion, SERVICE);
}

void AwsIotSigv4::getQueryString(char* out, char* cs, char* date, char* time)
{
	sprintf(out, "X-Amz-Algorithm=%s", ALGORITHM);
	sprintf(out, "%s&X-Amz-Credential=%s", out, cs);
	sprintf(out, "%s&X-Amz-Date=%sT%sZ", out, date, time);
  sprintf(out, "%s&X-Amz-Expires=86400", out);
	sprintf(out, "%s&X-Amz-SignedHeaders=host", out);
}

// Payload is always empty. Generate hash anyway.
void AwsIotSigv4::getPayloadHash(char* out)
{
  SHA256* sha256 = new SHA256();
  char* payloadHash = (*sha256)("", 0);
  snprintf(out, HASH_HEX_LEN + 1, "%s", payloadHash);
  delete[] payloadHash;
  delete sha256;
}

void AwsIotSigv4::getCanonicalRequestHash(char* out, char* canonicalRequest)
{
  SHA256* sha256 = new SHA256();
  char* canonicalRequestHash = (*sha256)(canonicalRequest, strlen (canonicalRequest));
  snprintf(out, HASH_HEX_LEN + 1, "%s", canonicalRequestHash);
  delete[] canonicalRequestHash;
	delete sha256;
}

void AwsIotSigv4::getDateTime(char* date, char* time)
{
  const char* dateTime = dateTimeProvider->getDateTime();
  snprintf(date, DATE_LEN + 1, "%s", dateTime);
  snprintf(time, TIME_LEN + 1, "%s", dateTime + DATE_LEN);
}
