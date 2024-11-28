#include <WebSocketClient.h>
#include <WiFiS3.h>

using namespace net;

WebSocketClient client;
String inputString = "";      // 시리얼 입력을 저장할 문자열
boolean stringComplete = false;  // 문자열 완성 여부

const char ssid[] = "Daesin1";
const char pass[] = "ds0123456";
int status = WL_IDLE_STATUS;

void setup() {
  Serial.begin(9600);
  pinMode(LED_BUILTIN, OUTPUT);
  inputString.reserve(200);  // 문자열을 위한 공간 예약
  
  String fv = WiFi.firmwareVersion();
  if (fv < WIFI_FIRMWARE_LATEST_VERSION)
    Serial.println("Please upgrade the firmware");

  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    status = WiFi.begin(ssid, pass);
    delay(4000);
  }

  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  client.onOpen([](WebSocket &ws) {
    Serial.println("연결됨 - 메시지를 입력하세요:");
  });
  
  client.onClose([](WebSocket &ws, const WebSocket::CloseCode code,
                   const char *reason, uint16_t length) {
    Serial.println("연결이 종료되었습니다.");
  });
  
  client.onMessage([](WebSocket &ws, const WebSocket::DataType dataType,
                     const char *message, uint16_t length) {
    Serial.print("서버 응답: ");
    Serial.println(message);
    if (strcmp(message, "on") == 0) {
      digitalWrite(LED_BUILTIN, HIGH);
      Serial.println("LED 켜짐");
    }
    // 메시지가 "off"이면 LED 끄기
    else if (strcmp(message, "off") == 0) {
      digitalWrite(LED_BUILTIN, LOW);
      Serial.println("LED 꺼짐");
    }
  });

  

  client.open("3.39.126.121", 3000);
}


void loop() {
  client.listen();

  while(Serial.available()) {
    inputString = Serial.readStringUntil('\n');
    client.send(WebSocket::DataType::TEXT, 
                inputString.c_str(), 
                inputString.length());
    Serial.println("전송됨: " + inputString);

    // while(Serial.read()) ;
    inputString = "";
  }

  
}
