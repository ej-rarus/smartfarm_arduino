#include <WebSocketClient.h>
#include <WiFiS3.h>

using namespace net;

WebSocketClient client;
String inputString = "";      // 시리얼 입력을 저장할 문자열
boolean stringComplete = false;  // 문자열 완성 여부

const char ssid[] = "Daesin2";
const char pass[] = "ds0123456";
int status = WL_IDLE_STATUS;

//핀 번호 설정


const FAN_PIN = 4;
const LIGHT_PIN = 5;
const PUMP_PIN = 6;
const MIST_PIN =7;


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

    // LED 제어
    if (strcmp(message, "light_on") == 0) {
      digitalWrite(LED_BUILTIN, HIGH);
      Serial.println("LED 켜짐");
    }
    else if (strcmp(message, "light_off") == 0) {
      digitalWrite(LED_BUILTIN, LOW);
      Serial.println("LED 꺼짐");
    }

    // FAN 제어
    if (strcmp(message, "fan_on") == 0) {
      digitalWrite(LED_BUILTIN, HIGH);
      Serial.println("FAN 켜짐");
    }
    else if (strcmp(message, "fan_off") == 0) {
      digitalWrite(LED_BUILTIN, LOW);
      Serial.println("FAN 꺼짐");
    }

    // WATER 제어
    if (strcmp(message, "water_on") == 0) {
      digitalWrite(LED_BUILTIN, HIGH);
      Serial.println("WATER 켜짐");
    }
    else if (strcmp(message, "water_off") == 0) {
      digitalWrite(LED_BUILTIN, LOW);
      Serial.println("WATER 꺼짐");
    }

    // MIST 제어
    if (strcmp(message, "mist_on") == 0) {
      digitalWrite(LED_BUILTIN, HIGH);
      Serial.println("MIST 켜짐");
    }
    else if (strcmp(message, "mist_off") == 0) {
      digitalWrite(LED_BUILTIN, LOW);
      Serial.println("MIST 꺼짐");
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
