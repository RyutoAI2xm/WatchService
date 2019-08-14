#define TINY_GSM_MODEM_UBLOX
#define TINY_GSM_USE_GPRS true
#define TINY_GSM_USE_WIFI false
#define SerialMon Serial

#include <M5Stack.h>
#include <TinyGsmClient.h>
#include <ArduinoHttpClient.h>

// Your GPRS credentials
const char apn[]  = "soracom.io";
const char gprsUser[] = "sora";
const char gprsPass[] = "sora";

// Server details
const char server[] = "sendgrid.com";
String apiurl = "https://api.sendgrid.com/v3/mail/send";
const int  port = 443;

// Set serial for debug console (to the Serial Monitor, default speed 115200)
TinyGsm modem(Serial2); /* 3G board modem */

//TinyGsmClient ctx(modem);
TinyGsmClientSecure client(modem);

HttpClient http(client, server, port);

int humanCounter = 0;

void setup() {

    M5.begin(false, false, true);
    //M5.Lcd.clear(BLACK);
    //M5.Lcd.setTextColor(YELLOW);
    //M5.Lcd.setTextSize(2);
    
    // put your setup code here, to run once:
    Serial.begin(115200);
    //configure pin 2 as an input and enable the internal pull-up resistor
    pinMode(2, OUTPUT);
    digitalWrite(2,HIGH);
    
    pinMode(5, INPUT_PULLUP);
  
    dacWrite(25, 0); // Speaker OFF

}

void loop() {

  if (millis() > 43200000){ //12h (12h * 60min * 60sec * 1000ms)
  
    //監視結果発砲
    if (humanCounter >= 10){ //10秒以上センサーが反応した場合
      Serial.println("SURVIVE");  
    }else{
        // 3G通信用の初期設定
        Serial.println("ALERT");  
        Serial2.begin(115200, SERIAL_8N1, 16, 17);
        modem.restart();
        delay(200);
        while (!modem.waitForNetwork()) Serial.println(".");
        modem.gprsConnect("soracom.io", "sora", "sora");
        while (!modem.isNetworkConnected());
        delay(200);
  
        if (!modem.hasSSL()) {
            SerialMon.println(F("SSL is not supported by this modem"));
            while(true) { 
                delay(1000); 
            }
        }
        
        if (modem.isNetworkConnected()) {
          SerialMon.println("Network connected");
        }
        
        SerialMon.print(F("Performing HTTPS POST request... "));
        http.connectionKeepAlive(); // Currently, this is needed for HTTPS

        if (!http.connect(server, 443)) {    
          Serial.println("connection failed");
          return;
        }

        // ここの中身を編集して、送信元・送信先アドレス、送信時のタイトルやメッセージを決める
        String json =    "{\"personalizations\":[{\"to\":[{\"email\":\"soushinsaki@mail.com\"}],\"subject\":\"kenmei wo kaku\"}],\"from\":{\"email\":\"soushinmoto@mail.com\"},\"content\":[{\"type\":\"text/plain\",\"value\":\"honbun wo kaku\"}]}";  

        // Make a HTTPS POST request:
        http.print("POST " + apiurl + " HTTP/1.1\r\n");    
        http.print("Host: " + String(server) + ":443\r\n");
        http.print("Content-Type: application/json\r\n");
        // send gridの認証キーを入力する
        http.print("Authorization: Bearer SG.xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\r\n");
        http.print("Connection: Keep-Alive\r\n");
        http.print("application/json:" + json + "\r\n");
        http.print("Content-Length: "+ String(json.length()) + "\r\n");
        http.print("\r\n");
        http.print(json + "\r\n");
        http.println();

        unsigned long timeout = millis();
        while (http.available() == 0) {
            if (millis() - timeout > 60000) {
                Serial.println(">>> Client Timeout !");
               client.stop();
               return;
            }
         }
    
        while(http.available()) {
            String line = http.readStringUntil('\r');
            Serial.print(line);
        }
    }

    delay(100);
    //再起動    
    digitalWrite(2,LOW);
    delay(10);
    digitalWrite(2,HIGH);
    delay(10);
    digitalWrite(2,LOW);
    delay(10);
    
  }
  else{
    int jinkanVal = digitalRead(5);

    if (jinkanVal ==1 && humanCounter<=9 ){humanCounter++;}
    if (jinkanVal ==0 && humanCounter<=9 ){humanCounter=0;}

    Serial.println(humanCounter);  
    delay(1000);    
  }
}
