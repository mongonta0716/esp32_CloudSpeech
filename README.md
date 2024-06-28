# esp32_CloudSpeech

[esp32_CloudSpeech](https://github.com/MhageGH/esp32_CloudSpeech)をベースにM5Unified対応を行い、Google SpeechToTextサービスにOAuth2.0認証とAPIKey認証の2種類でアクセスするための実験プロジェクトです。


Let's try voice recognition by machine learning!<br>
Transcribe your voice by Google's Cloud Speech-to-Text API with esp32<br><br> 
In the case of esp32 + microphone<br>
 ![photo1](doc/photo1.jpg)<br><br>
In the case of M5Stack FIRE<br>
 ![M5StackFIRE](doc/M5StackFIRE.jpg)<br><br>
Serial monitor<br>
 ![Transcribe](doc/Transcribe.png)
 
## Prepare
 - M5Stack series with microphone<br>動作確認はCore2のみしかしていません。

## Development Environment
- VSCode + Platform IO

### Libraries
詳しくは[platformio.ini](./platformio.ini)を見てください。
- M5Unified
- ArduinoJSON
- ESP-Signer

## How to use
### Google Cloud Platform上での操作
Google Speech To Text APIを有効にします。

1. GoogleCloundPlatformでプロジェクトを作成し、「IAMと管理」ー＞サービスアカウントを作成します。
1. 「キー」タブー＞「鍵を追加」でJSONファイルを作成。
1. JSONファイルから下記の3つの値をnetwork_param.hに転記します。
  - project_id     -----> PROJECT_ID
  - private_key    -----> PRIVATE_KEY
  - client_email   -----> CLIENT_EMAIL
1. apiKey
  - APIKeyを設定します。

### WiFi設定
1. network_param.hにWIFI_SSIDとWIFI_PASSも記述してください。(2.4GHz帯のみ)

### M5Stackの操作

- BtnAを押すとOAuth認証で音声認識します。
- BtnBを押すとAPIKey認証で音声認識します。
- BtnCを押すと画面をクリアします。


## How to use with curl
YOUR_・・・の値を書き換えてください。アクセストークンはログに出力されるものをコピーしてください。一定時間で期限切れするので期限が切れたらアクセストークンを更新する必要があります。
```
curl -s -H "Content-Type: application/json" \
    -H "x-goog-user-project: YOUR_PROJECT_ID" \
    -H "Authorization: Bearer YOUR_ACCESSTOKEN" \
    https://speech.googleapis.com/v1/speech:recognize \
    -d @request.json
```

### request.json
おはようございますというWAVファイルをサンプルで登録しました。

[request.json](./request.json)


