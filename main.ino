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
  });

  client.open("3.39.126.121", 3000);
}

void loop() {
  client.listen();
  
  // 시리얼 입력이 완성되면 전송
  if (stringComplete) {
    if (true) {
      client.send(WebSocket::DataType::TEXT, 
                 inputString.c_str(), 
                 inputString.length());
      Serial.println("전송됨: " + inputString);
    }
    
    // 문자열 초기화
    inputString = "";
    stringComplete = false;
  }
}

// 시리얼 이벤트 처리
void serialEvent() {
  while (Serial.available()) {
    char inChar = (char)Serial.read();
    
    // 개행문자가 입력되면 문자열 완성
    if (inChar == '\n') {
      stringComplete = true;
    } else {
      inputString += inChar;
    }
  }
}