# esp32_CloudSpeech
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


## How to use
Get your account in https://cloud.google.com/speech-to-text/ <br>
1. GoogleCloundPlatformでプロジェクトを作成し、「IAMと管理」ー＞サービスアカウントを作成します。
1. 「キー」タブー＞「鍵を追加」でJSONファイルを作成。
1. JSONファイルから下記の3つの値をnetwork_param.hに転記します。
  - project_id     -----> PROJECT_ID
  - private_key    -----> PRIVATE_KEY
  - client_email   -----> CLIENT_EMAIL
1. network_param.hにWIFI_SSIDとWIFI_PASSも記述してください。(2.4GHz帯のみ)

Set network parameter and your account information in network_param.h.<br>
Say to the microphone and see serial monitor.


