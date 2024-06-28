#include <Arduino.h>
#include <M5Unified.h>
#include <WiFi.h>
#include "Audio.h"
#include "CloudSpeechClient.h"
#include "network_param.h"
#include <ESP_Signer.h>

CloudSpeechClient* cloud_speech_client;

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

void initScreen() {
  M5.Display.clear();
  M5.Display.setCursor(0, 0);
  M5.Display.println("BtnA: OAuth  STT, BtnB: APIKey STT, BtnC: ClearLCD");
}

void setup() {
  auto cfg = M5.config();
  M5.begin(cfg);
  M5.Log.setLogLevel(m5::log_target_display, ESP_LOG_INFO);
  M5.Display.setTextFont(&fonts::efontJA_12);
  M5.setLogDisplayIndex(0);
  M5.Log.setSuffix(m5::log_target_display, "");
  M5.Display.setTextWrap(true, true);
  setupWiFi();
  initScreen();

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
  signer_config.signer.tokens.scope = "https://www.googleapis.com/auth/cloud-platform, https://www.googleapis.com/auth/userinfo.email, https://www.googleapis.com/auth/appengine.admin";

  /** Assign the callback function for token ggeneration status (optional) */
  signer_config.token_status_callback = tokenStatusCallback;

  /** If Google Root certificate data provided */
  signer_config.cert.data = rootCACert;

  /* Create token */
  Signer.begin(&signer_config);


}

void loop() {
  M5.update();
  // OAuth認証を使用する。
  if (M5.BtnA.wasClicked()) {
    M5.Speaker.begin();
    M5.Speaker.tone(2000, 200);
    delay(300);
    M5.Mic.begin();
    uint32_t temp_millis;
    uint32_t start_millis = millis();
    M5_LOGI("OAuth Mode Record start: %d\n", start_millis);
    Audio* audio = new Audio();
    audio->Record();
    M5_LOGI("Record End: %d\n", millis() - start_millis);
    
    cloud_speech_client = new CloudSpeechClient(rootCACert, USE_ACCESSTOKEN, PROJECT_ID);
    temp_millis = millis();
    M5_LOGI("Transcribe start: %d\n", temp_millis);
    Signer.tokenReady();
    cloud_speech_client->Transcribe(audio, Signer.accessToken());
    uint32_t response_millis = millis() - temp_millis;
    M5_LOGI("CloudSpeechResponse: %d\n", response_millis);
    M5_LOGI("Total Response: %d\n", millis() - start_millis);
    delete cloud_speech_client;
    delete audio;
  }
  // API認証を使用する。
  if (M5.BtnB.wasClicked()) {
    M5.Speaker.begin();
    M5.Speaker.tone(1000, 200);
    delay(300);
    M5.Mic.begin();
    uint32_t temp_millis;
    uint32_t start_millis = millis();
    M5_LOGI("APIKey STT Record start: %d\n", start_millis);
    Audio* audio = new Audio();
    audio->Record();
    M5_LOGI("Record End: %d\n", millis() - start_millis);
    
    cloud_speech_client = new CloudSpeechClient(rootCACert, USE_APIKEY, ApiKey);
    temp_millis = millis();
    M5_LOGI("Transcribe start: %d\n", temp_millis);
    cloud_speech_client->Transcribe(audio);
    uint32_t response_millis = millis() - temp_millis;
    M5_LOGI("CloudSpeechResponse: %d\n", response_millis);
    M5_LOGI("Total Response: %d\n", millis() - start_millis);
    delete cloud_speech_client;
    delete audio;
  }
  if (M5.BtnC.wasClicked()) {
    initScreen();
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

