#include <Arduino.h>
#include <M5Unified.h>
#include <WiFi.h>
#include "Audio.h"
#include "CloudSpeechClient.h"
#include "network_param.h"
#include <ESP_Signer.h>

SignerConfig signer_config;
void tokenStatusCallback(TokenInfo info);

void setupWiFi() {
   WiFi.begin(WIFI_SSID, WIFI_PASS);
  // 前回接続時情報で接続する
  while (WiFi.status() != WL_CONNECTED) {
    M5.Display.print(".");
    Serial.print(".");
    delay(500);
    // 10秒以上接続できなかったら抜ける
    if ( 10000 < millis() ) {
      break;
    }
  }
  M5.Display.println("");
  Serial.println("");
  // 未接続の場合にはSmartConfig待受
  if ( WiFi.status() != WL_CONNECTED ) {
    WiFi.mode(WIFI_STA);
    WiFi.beginSmartConfig();
    M5.Display.println("Waiting for SmartConfig");
    Serial.println("Waiting for SmartConfig");
    while (!WiFi.smartConfigDone()) {
      delay(500);
      M5.Display.print("#");
      Serial.print("#");
      // 30秒以上接続できなかったら抜ける
      if ( 30000 < millis() ) {
        Serial.println("");
        Serial.println("Reset");
        ESP.restart();
      }
    }
    // Wi-fi接続
    M5.Display.println("");
    Serial.println("");
    M5.Display.println("Waiting for WiFi");
    Serial.println("Waiting for WiFi");
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      M5.Display.print(".");
      Serial.print(".");
      // 60秒以上接続できなかったら抜ける
      if ( 60000 < millis() ) {
        Serial.println("");
        Serial.println("Reset");
        ESP.restart();
      }
    }
  }
}


void setup() {
  auto cfg = M5.config();
  M5.begin(cfg);
  M5.Display.setTextSize(3);
  setupWiFi();
  M5.Display.clear();
  M5.Display.println("Press BtnA: Record");

  auto micConfig = M5.Mic.config();
  micConfig.stereo = false;
  micConfig.sample_rate = 16000;
  M5.Mic.config(micConfig);
    /* Assign the sevice account credentials and private key (required) */
  signer_config.service_account.data.client_email = CLIENT_EMAIL;
  signer_config.service_account.data.project_id = PROJECT_ID;
  signer_config.service_account.data.private_key = PRIVATE_KEY;

  /** Expired period in seconds (optional).
   * Default is 3600 sec.
   * This may not afftect the expiry time of generated access token.
   */
  signer_config.signer.expiredSeconds = 3600;

  /* Seconds to refresh the token before expiry time (optional). Default is 60 sec.*/
  signer_config.signer.preRefreshSeconds = 60;

  /** Assign the API scopes (required)
   * Use space or comma to separate the scope.
   */
  signer_config.signer.tokens.scope = "https://www.googleapis.com/auth/cloud-platform, https://www.googleapis.com/auth/userinfo.email";

  /** Assign the callback function for token ggeneration status (optional) */
  signer_config.token_status_callback = tokenStatusCallback;

  /** If Google Root certificate data provided */
  //signer_config.cert.data = rootCACert;

  /* Create token */
  Signer.begin(&signer_config);


}

void loop() {
  M5.update();
  if (M5.BtnA.wasClicked()) {
    M5.Speaker.begin();
    M5.Speaker.tone(2000, 100);
    delay(100);
    M5.Mic.begin();
    M5.Display.println("Record start!");
    Audio* audio = new Audio();
    audio->Record();
    CloudSpeechClient* cloudSpeechClient = new CloudSpeechClient(rootCACert, USE_ACCESSTOKEN, PROJECT_ID);
    //CloudSpeechClient* cloudSpeechClient = new CloudSpeechClient(rootCACert, USE_APIKEY, ApiKey);
    
    Signer.tokenReady();
    cloudSpeechClient->Transcribe(audio, Signer.accessToken());
    Serial.println("----------------------------------------");
    Serial.println(Signer.accessToken());
    Serial.println("----------------------------------------");
    delete cloudSpeechClient;
    delete audio;
  }
}

void tokenStatusCallback(TokenInfo info)
{
    if (info.status == esp_signer_token_status_error)
    {
        Signer.printf("Token info: type = %s, status = %s\n", Signer.getTokenType(info).c_str(), Signer.getTokenStatus(info).c_str());
        Signer.printf("Token error: %s\n", Signer.getTokenError(info).c_str());
    }
    else
    {
        Signer.printf("Token info: type = %s, status = %s\n", Signer.getTokenType(info).c_str(), Signer.getTokenStatus(info).c_str());
        if (info.status == esp_signer_token_status_ready)
            Signer.printf("Token: %s\n", Signer.accessToken().c_str());
    }
} 

