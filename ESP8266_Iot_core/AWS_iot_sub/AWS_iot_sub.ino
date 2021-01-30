/* ESP8266 AWS IoT
    Device 1
   Simplest possible example (that I could come up with) of using an ESP8266 with AWS IoT.
   No messing with openssl or spiffs just regular pubsub and certificates in string constants

   This is working as at 3rd Aug 2019 with the current ESP8266 Arduino core release:
   SDK:2.2.1(cfd48f3)/Core:2.5.2-56-g403001e3=20502056/lwIP:STABLE-2_1_2_RELEASE/glue:1.1-7-g82abda3/BearSSL:6b9587f

   Author: Anthony Elder
   License: Apache License v2

   Sketch Modified by Stephen Borsay for www.udemy.com/course/exploring-aws-iot/
   https://github.com/sborsay
   Add in EOF certificate delimiter
   Add in Char buffer utilizing sprintf to dispatch JSON data to AWS IoT Core
   First 9 chars of certs obfusicated, use your own, but you can share root CA / x.509 until revoked
*/
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

int Device1 = 12;
int Device2 = 13;
int Device3 = 14;

String msg = "";
extern "C" {
#include "libb64/cdecode.h"
}

const char* ssid = "emed@17vnpuri";
const char* password = "D17vnpuri";

// Find this awsEndpoint in the AWS Console: Manage - Things, choose your thing
// choose Interact, its the HTTPS Rest endpoint.  Endpoints ARE regional
const char* awsEndpoint = "a3efxowlbl3jv1-ats.iot.ap-south-1.amazonaws.com";

// For the two certificate strings below paste in the text of your AWS
// device certificate and private key, comment out the BEGIN and END
// lines, add a quote character at the start of each line and a quote
// and backslash at the end of each line:

// xxxxxxxxxx-certificate.pem.crt
const String certificatePemCrt = \
                                 //-----BEGIN CERTIFICATE-----
                                 R"EOF(

MIIDWjCCAkKgAwIBAgIVAJhPUi73Pi7TJAfhAE9rVUoTNM2kMA0GCSqGSIb3DQEB
CwUAME0xSzBJBgNVBAsMQkFtYXpvbiBXZWIgU2VydmljZXMgTz1BbWF6b24uY29t
IEluYy4gTD1TZWF0dGxlIFNUPVdhc2hpbmd0b24gQz1VUzAeFw0yMTAxMjYyMTEz
NDNaFw00OTEyMzEyMzU5NTlaMB4xHDAaBgNVBAMME0FXUyBJb1QgQ2VydGlmaWNh
dGUwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQCmLFj0PLIdi6Fez4mK
P0FJtpDcLI0GPk1GeLu85LSxQfC+68ZBn/P2nF1pDMhoP2leHy9Axz/EpK5erXKM
UZZMPhObaA/YG5a4nbxl8POGhF9hzY1rRZ6z1K6juaV1OdAwbkvOK47pRCL/pixj
VC/PGe4q5oZqKI9Px0USk2ylysdKJ7RELYWVDimbGPfZngd3MuktineTUEZlGqmy
LFol3NhyOpUsTdNnMXfzUFm4VK9d0fK8h6OuNwhzEoVbDZ6UB5UKrwj6rXQOSELC
HcBHxLk8mSXVq0FlyLg9C6C/xcQfMrSPhaqrfnBzqK8Oi+EOEtZrlpSiRS7nFSLx
NHsvAgMBAAGjYDBeMB8GA1UdIwQYMBaAFJRt9v5LXuDzCybpuraIZK+FbywYMB0G
A1UdDgQWBBSgw9qMVH14Cw86uBnRDjhtFQGZVDAMBgNVHRMBAf8EAjAAMA4GA1Ud
DwEB/wQEAwIHgDANBgkqhkiG9w0BAQsFAAOCAQEAanshexVO8dODpstZAKto5Z4s
sYjrWa0KSP9MioFLgWKkeF9AseqfpdMnaHE1dqQb46/1IxzOArwOPDjfVu5CZQKd
zAN8LZXfmpXid1Nkpv9Y4/M8R4BTdA+UE/LLJtwOt1iZjlcMJZHMHHJ9yoKT48lO
hrbOuRhH/CWt08TX5zKX0K5HHVbQIO73fHtY0Ki2jWRcSuLcQTe23ilJ2NZNF6cI
lGTUUwrUA8WkkWAbGFNg8uhjKjqf2gUbKsSDjfYzgsKjVB0jPni0YHp9u8nHBOrs
Nw6RxoYaumPCf4mWQJB5Zf4qYBSBN3AMXYENOgep6sfr+MzWCpTGDSIbUr8HDA==

)EOF"; 
//-----END CERTIFICATE-----

// xxxxxxxxxx-private.pem.key
const String privatePemKey = \
//-----BEGIN RSA PRIVATE KEY-----
R"EOF(

MIIEowIBAAKCAQEApixY9DyyHYuhXs+Jij9BSbaQ3CyNBj5NRni7vOS0sUHwvuvG
QZ/z9pxdaQzIaD9pXh8vQMc/xKSuXq1yjFGWTD4Tm2gP2BuWuJ28ZfDzhoRfYc2N
a0Wes9Suo7mldTnQMG5LziuO6UQi/6YsY1QvzxnuKuaGaiiPT8dFEpNspcrHSie0
RC2FlQ4pmxj32Z4HdzLpLYp3k1BGZRqpsixaJdzYcjqVLE3TZzF381BZuFSvXdHy
vIejrjcIcxKFWw2elAeVCq8I+q10DkhCwh3AR8S5PJkl1atBZci4PQugv8XEHzK0
j4Wqq35wc6ivDovhDhLWa5aUokUu5xUi8TR7LwIDAQABAoIBAHkb0SkQI3BNF6sV
7tQHDfo/SIxvG8j1FzTJQWqF9PtLZ75979xcS13s0FeDrXvs20pxx+qaTOtwanUu
TFHgFH40r+2Phhym3s5mh6x0U8gyKaIKQYtxDh1aYH2LC//HxufFOxJjxKBVvHw+
DBPIg5+Y8m/p2wT70pb+UGUddQ3O8yTGjU6uRkmtmuYWkaFq7FpndGJFK3qLdXMw
HYrug7QtkPi6EFf1tsezpVW7fgzV+aKEfmLB5quMJLYCVwRzqcYcL5pF92gWVtTm
020eROho8Kr321MoeoOQhFpowWq7dwl7QMMdwcI1mkAXrR2QvSQhNMYZ/+Ogpt+k
zmywL8ECgYEA2Z4UXv3PBSI8zQ1Rl0800/ZQ7vISWY+HxEPLS4RaGTd0GssD0s9u
B/vc/1mkES9fR8LzSaIdTLmNKJnjW64zjRQ6QIlkRghmo/EKe/joIP0mIbX8LzQn
/nesD5HR0sSyE53YJYFCGJWpZDJ99s0blxH1osBR77HEL68/CbSkUUkCgYEAw3tx
gJiRSPGg6CofqdF/s86kXnh5vLowkm5JxLnAbIXz6jvEopRpM8aEcghbVPI+f2C8
6UB9MrdrYQxSGjluxueDHxaEyYRfTUGXsFcEhEOp12ns3+68c3eakau8AMWanYxh
5nhfSUeNVTOV+KakF4jpQDxfQrLM4G406TorYLcCgYB8KviQot1Gnpx2m4zcevM3
pinTzQevVngvAkezFRebn8p9pyzNUlID+C/G+0zkCVWiSpgqgXeaWZ0zd2sE2sbL
TqlWPY6bbxuxm01SI1m/yPHC9mWoQWg4h+wDDj7ctiKtkrjL3Y7rHCoE49J/ydRe
RH8VmAqoa5P3aVTcotfHEQKBgAqTsU598s55lHU7Yj80AK7f7XDXH+8gO+SQZxln
24uDq/DujvMlCJQul5f778Um3k7SEyajwKJv3jWADykwFlhYynVyKHkm2mk5tVVr
P+lQJ+5p4tEy/6tXu1tKaX+5MWZ45AsuVCmI35LOmby8d4B2ffhly9m2BvPVwrMj
3Jj9AoGBANK6kp0GKN7vFqir3AsREcz83ulbIUQg4Kh5aZYe+7KyDjCh0cHwYNDO
kv5DFHkvbxJxevpYcqk+ijktvb6nnYzYGCNfsWpHYySV+WkVHbC8TfEi9Go5PJ2L
Aa4erKgpGjmn8tEFrLPQ+FCVUB2jBo3/hXSmQnhO/ufZQy/6KX5K

)EOF";
//-----END RSA PRIVATE KEY-----

// This is the AWS IoT CA Certificate from: 
// https://docs.aws.amazon.com/iot/latest/developerguide/managing-device-certs.html#server-authentication
// This one in here is the 'RSA 2048 bit key: Amazon Root CA 1' which is valid 
// until January 16, 2038 so unless it gets revoked you can leave this as is:
const String caPemCrt = \
//-----BEGIN CERTIFICATE-----
"MIIDQTCCAimgAwIBAgITBmyfz5m/jAo54vB4ikPmljZbyjANBgkqhkiG9w0BAQsF" \
"ADA5MQswCQYDVQQGEwJVUzEPMA0GA1UEChMGQW1hem9uMRkwFwYDVQQDExBBbWF6" \
"b24gUm9vdCBDQSAxMB4XDTE1MDUyNjAwMDAwMFoXDTM4MDExNzAwMDAwMFowOTEL" \
"MAkGA1UEBhMCVVMxDzANBgNVBAoTBkFtYXpvbjEZMBcGA1UEAxMQQW1hem9uIFJv" \
"b3QgQ0EgMTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALJ4gHHKeNXj" \
"ca9HgFB0fW7Y14h29Jlo91ghYPl0hAEvrAIthtOgQ3pOsqTQNroBvo3bSMgHFzZM" \
"9O6II8c+6zf1tRn4SWiw3te5djgdYZ6k/oI2peVKVuRF4fn9tBb6dNqcmzU5L/qw" \
"IFAGbHrQgLKm+a/sRxmPUDgH3KKHOVj4utWp+UhnMJbulHheb4mjUcAwhmahRWa6" \
"VOujw5H5SNz/0egwLX0tdHA114gk957EWW67c4cX8jJGKLhD+rcdqsq08p8kDi1L" \
"93FcXmn/6pUCyziKrlA4b9v7LWIbxcceVOF34GfID5yHI9Y/QCB/IIDEgEw+OyQm" \
"jgSubJrIqg0CAwEAAaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMC" \
"AYYwHQYDVR0OBBYEFIQYzIU07LwMlJQuCFmcx7IQTgoIMA0GCSqGSIb3DQEBCwUA" \
"A4IBAQCY8jdaQZChGsV2USggNiMOruYou6r4lK5IpDB/G/wkjUu0yKGX9rbxenDI" \
"U5PMCCjjmCXPI6T53iHTfIUJrU6adTrCC2qJeHZERxhlbI1Bjjt/msv0tadQ1wUs" \
"N+gDS63pYaACbvXy8MWy7Vu33PqUXHeeE6V/Uq2V8viTO96LXFvKWlJbYK8U90vv" \
"o/ufQJVtMVT8QtPHRh8jrdkPSHCa2XV4cdFyQzR1bldZwgJcJmApzyMZFo6IQ6XU" \
"5MsI+yMRQ+hDKXJioaldXgjUkK642M4UwtBV8ob2xJNDd2ZhwLnoQdeXeGADbkpy" \
"rqXRfboQnoZsG4q5WTP468SQvvG5";
//-----END CERTIFICATE-----

WiFiClientSecure wiFiClient;
String msgReceived(char* topic, byte* payload, unsigned int len);
PubSubClient pubSubClient(awsEndpoint, 8883, msgReceived, wiFiClient); 

void setup() {
   pinMode(Device1, OUTPUT);
   pinMode(Device2, OUTPUT);
   pinMode(Device3, OUTPUT);
   pinMode(LED_BUILTIN, OUTPUT);
   
   
    
  Serial.begin(115200); Serial.println();
  Serial.println("ESP8266 AWS IoT Example");

  Serial.print("Connecting to "); Serial.print(ssid);
  WiFi.begin(ssid, password);
  WiFi.waitForConnectResult();
  Serial.print(", WiFi connected, IP address: "); Serial.println(WiFi.localIP());

  // get current time, otherwise certificates are flagged as expired
  setCurrentTime();

  uint8_t binaryCert[certificatePemCrt.length() * 3 / 4];
  int len = b64decode(certificatePemCrt, binaryCert);
  wiFiClient.setCertificate(binaryCert, len);
  
  uint8_t binaryPrivate[privatePemKey.length() * 3 / 4];
  len = b64decode(privatePemKey, binaryPrivate);
  wiFiClient.setPrivateKey(binaryPrivate, len);

  uint8_t binaryCA[caPemCrt.length() * 3 / 4];
  len = b64decode(caPemCrt, binaryCA);
  wiFiClient.setCACert(binaryCA, len);

}
//unsigned long lastPublish;
//int msgCount;

void loop() {
  
  pubSubCheckConnect();
  delay(100);
  //sprintf(pubData,  "{\"uptime\":%lu,\"id\":1,\"Switch\":%d}", millis() / 1000, Data);
  //pubSubClient.publish("outTopic", pubData);
  //delay(100);
  //Serial.print("Published: "); Serial.println(pubData);

   //delay(5000);
  
}


String msgReceived(char* topic, byte* payload, unsigned int length) {
  
  //char* data = "";
  //String msg = "";

  Serial.print("Message received on "); Serial.print(topic); Serial.print(": ");
  for (int i = 0; i < length; i++) {
    //Serial.print((char)payload[i]);
    //data = payload[i];
    msg += (char)payload[i];
    
  }
  Serial.print("DATA :");
    Serial.println(msg);
  Serial.println();
  parseDevice();
  return msg;
}

void pubSubCheckConnect() {

  if ( ! pubSubClient.connected()) {
    Serial.print("PubSubClient connecting to: "); Serial.print(awsEndpoint);
    while ( ! pubSubClient.connected()) {
      
      Serial.print(".");
      pubSubClient.connect("ESPthing");
      yield();
      //delay(5000);
    }
    Serial.println(" connected");
     digitalWrite(LED_BUILTIN, LOW);
    //int Data = 1;
    //char pubData[10];
    //sprintf(pubData,  "{\"uptime\":%lu,\"id\":1,\"Switch\":%d}", millis() / 1000, Data);
    //pubSubClient.publish("outTopic", pubData);
    //delay(100);
    //Serial.print("Published: "); Serial.println(pubData);

    //delay(100);
    Serial.println(pubSubClient.subscribe("outTopic"));
    
  // Serial.println(pubSubClient.subscribe("Device2"));
   //Serial.println(pubSubClient.subscribe("Device3"));
    //yield();

  }
  pubSubClient.loop();
}

int b64decode(String b64Text, uint8_t* output) {
  base64_decodestate s;
  base64_init_decodestate(&s);
  int cnt = base64_decode_block(b64Text.c_str(), b64Text.length(), (char*)output, &s);
  return cnt;
}

void setCurrentTime() {
  configTime(3 * 3600, 0,"pool.ntp.org", "time.nist.gov");

  Serial.print("Waiting for NTP time sync: ");
  time_t now = time(nullptr);
  while (now < 8 * 3600 * 2) {
    delay(500);
    Serial.print(".");
    now = time(nullptr);
  }
  Serial.println("");
  struct tm timeinfo;
  gmtime_r(&now, &timeinfo);
  Serial.print("Current time: "); Serial.print(asctime(&timeinfo));
}

int parseDevice(){
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(msg);
  delay(10);

  
  int Sw = root["Switch"];
 Sw = root["Switch"].as<int>();

  Serial.println(Sw);

 int id = root["id"];
  id = root["id"].as<int>();

   Serial.println(id);
if(id==1){
  Serial.print("Device 1 Status :");
  
  if(Sw == 1){
    Serial.println("ON");
     digitalWrite(Device1, HIGH);
  }else{
    
    Serial.println("OFF");
    digitalWrite(Device1, LOW);
 }
}
if(id==2){
  Serial.print("Device 2 Status :");
  
  if(Sw == 1){
    Serial.println("ON");
    digitalWrite(Device2, HIGH);
    
  }else{
    Serial.println("OFF");
    digitalWrite(Device2, LOW);
 }
}

if(id==3){
  Serial.print("Device 3 Status :");
  
  if(Sw == 1){
    Serial.println("ON");
    digitalWrite(Device3, HIGH);
    
  }else{
    Serial.println("OFF");
    digitalWrite(Device3, LOW);
 }
}

   msg = "";

}
