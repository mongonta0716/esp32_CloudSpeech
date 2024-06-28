#include "CloudSpeechClient.h"
//#include "network_param.h"
#include <base64.h>
#include <ArduinoJson.h>
#define STREAMUTILS_ENABLE_EEPROM 0
#include <StreamUtils.h>

String LANG_CODE = "ja-JP";

namespace {
constexpr char* API_HOST = "speech.googleapis.com";
constexpr int API_PORT = 443;
constexpr char* API_PATH = "/v1/speech:recognize";
}  // namespace

CloudSpeechClient::CloudSpeechClient(const char* root_ca, Authentication authentication, String project_id_or_apikey) {
  this->authentication = authentication;
  if (authentication == USE_APIKEY) {
    this->key = project_id_or_apikey;
  } else {
    this->project_id = project_id_or_apikey;
  }
  client.setCACert(root_ca);
  client.setTimeout( 10000 ); 
  if (!client.connect(API_HOST, API_PORT)) {
    Serial.println("Connection failed!");
  } else {
    Serial.println("Connected!");
  }

}
CloudSpeechClient::CloudSpeechClient(const char* root_ca, const char* api_key) : client(), key(api_key) {
//  this->authentication = authentication;
  client.setCACert(root_ca);
  client.setTimeout( 10000 ); 
  if (!client.connect(API_HOST, API_PORT)) {
    Serial.println("Connection failed!");
  }
}

CloudSpeechClient::~CloudSpeechClient() {
  client.stop();
}

void CloudSpeechClient::PrintHttpBody2(Audio* audio) {
  String enc = base64::encode(audio->paddedHeader, sizeof(audio->paddedHeader));
  //enc.replace("\n", "");  // delete last "\n"
  //client.print(enc);      // HttpBody2
  char* wavData = (char*)audio->wavData;
  for (int j = 0; j < audio->record_number; j++) {
    enc = base64::encode((byte*)&wavData[j*audio->record_length*2], audio->record_length*2);
    enc.replace("\n", "");// delete last "\n"
    client.print(enc);    //Serial.print(enc); // HttpBody2
    delay(10);
  } 
}

String CloudSpeechClient::Transcribe(Audio* audio, String access_token) {
  //String HttpBody1 = "{\"config\":{\"encoding\":\"FLAC\",\"sampleRateHertz\":16000,\"languageCode\":\""+LANG_CODE+"\",\"enableWordTimeOffsets\":\"false\"},\"audio\":{\"uri\":\"gs://cloud-samples-tests/speech/brooklyn.flac\"}}\r\n\r\n";
  String HttpBody1 = "{\"config\":{\"encoding\":\"LINEAR16\",\"sampleRateHertz\":\"16000\",\"languageCode\":\""+LANG_CODE+"\",\"enableWordTimeOffsets\":\"false\"},\"audio\":{\"content\":\"";
  String HttpBody3 = "\"}}\r\n\r\n";
  int httpBody2Length = (audio->wavDataSize + sizeof(audio->paddedHeader))*4/3;  // 4/3 is from base64 encoding
  String ContentLength = String(HttpBody1.length() + httpBody2Length + HttpBody3.length());// + access_token.length());

  String HttpHeaders;

  if (this->authentication == USE_APIKEY) {
    HttpHeaders = String("POST ") + String(API_PATH) + String("?key=") + key
      + String(" HTTP/1.1\r\nHost: ") + String(API_HOST) + String("\r\nContent-Type: application/json\r\nContent-Length: ") + ContentLength + String("\r\n\r\n");
  } else {
    HttpHeaders = String("POST /v1/speech:recognize HTTP/1.1\r\nx-goog-user-project: " + project_id + "\r\nHost: speech.googleapis.com\r\nContent-Type: application/json\r\nContent-Length: " + httpBody2Length + "\r\n\"Authorization: Bearer ")
      + access_token + String("\"\r\n\r\n\r\n");
  }
  Serial.println("client print\n" + HttpHeaders);
  Serial.print(HttpBody1);
  client.print(HttpHeaders);
  client.print(HttpBody1);
  PrintHttpBody2(audio);
  client.print(HttpBody3);
  client.flush();
  Serial.println(HttpBody3);
  Serial.println("End of Client print");
  Serial.println("wait client available");
  while (!client.available()) {
    Serial.printf("client wait:%d\n", client.connected());
    delay(1000);
  };
  // Skip HTTP headers
  Serial.println("Skip headers");
  char endOfHeaders[] = "\r\n\r\n";
  if (!client.find(endOfHeaders)) {
    Serial.println(F("Invalid response"));
    return String("");
  } else {
    Serial.println("Find EOH");
  }
  if(client.available())client.read();
  if(client.available())client.read();
  if(client.available())client.read();

  // Parse JSON object
  StaticJsonDocument<2048> jsonBuffer;
  Serial.println(client.readString());
  ReadLoggingStream loggingStream(client, Serial);
  DeserializationError error = deserializeJson(jsonBuffer,loggingStream);
//root.prettyPrintTo(Serial); //Serial.println("");
  String result = "";
  if (error) {
    Serial.println("Parsing failed!");
    Serial.printf("error: %s\n", error.c_str());
    return result;
  } else {
    String  json_string;
    serializeJsonPretty(jsonBuffer, json_string);
    Serial.println("====================");
    Serial.println(json_string);
    Serial.println("====================");
//  root.prettyPrintTo(Serial);
    const char* text = jsonBuffer["results"][0]["alternatives"][0]["transcript"];
    Serial.print("\n認識結果：");
    if(text) {
      result = String (text);
      Serial.println((char *)text);
    }
    else {
      Serial.println("NG");
    }
  }
  return result;
}

