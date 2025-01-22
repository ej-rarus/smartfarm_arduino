#include <WebSocketClient.h>
#include <WiFiS3.h>

using namespace net;

WebSocketClient client;
String inputString = "";         // 시리얼 입력을 저장할 문자열
boolean stringComplete = false;  // 문자열 완성 여부

const char ssid[] = "sfarm_2.4g";
const char pass[] = "ds123456";
int status = WL_IDLE_STATUS;

//핀 번호 설정
int FAN_PIN = 4;
int LED_PIN = 5;
int PUMP_PIN = 6;
int MIST_PIN = 7;

// LED와 FAN 타이머를 위한 전역 변수
unsigned long ledStartTime = 0;
int ledTimerDuration = 0;
bool isLedTimerActive = false;

unsigned long fanStartTime = 0;
int fanTimerDuration = 0;
bool isFanTimerActive = false;

void setup() {
  Serial.begin(9600);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(FAN_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(PUMP_PIN, OUTPUT);
  pinMode(MIST_PIN, OUTPUT);
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
    if (strcmp(message, "LED_ON") == 0) {
      digitalWrite(LED_PIN, HIGH);
      Serial.println("LED 켜짐");
    } else if (strcmp(message, "LED_OFF") == 0) {
      digitalWrite(LED_PIN, LOW);
      Serial.println("LED 꺼짐");
    }

    // LED 타이머 제어
    if (strncmp(message, "LED_TIMER_", 10) == 0) {
        const char* timerValue = message + 10;  // "LED_TIMER_" 이후의 문자열
        int hours = atoi(timerValue);  // 문자열을 정수로 변환
        
        if (hours > 0) {
            Serial.printf("LED 타이머 %d시간 설정\n", hours);
            // 타이머 시작 로직
            digitalWrite(LED_PIN, HIGH);  // LED 켜기
            
            // 타이머 시작
            ledStartTime = millis();
            ledTimerDuration = hours;
            isLedTimerActive = true;
        }
    }

    // FAN 제어
    if (strcmp(message, "FAN_ON") == 0) {
      digitalWrite(FAN_PIN, HIGH);
      Serial.println("FAN 켜짐");
    } else if (strcmp(message, "FAN_OFF") == 0) {
      digitalWrite(FAN_PIN, LOW);
      Serial.println("FAN 꺼짐");
    }

    // FAN 타이머 제어
    if (strncmp(message, "FAN_TIMER_", 10) == 0) {
        const char* timerValue = message + 10;
        int hours = atoi(timerValue);
        
        if (hours > 0) {
            Serial.printf("FAN 타이머 %d시간 설정\n", hours);
            pinMode(FAN_PIN, OUTPUT);     
            digitalWrite(FAN_PIN, HIGH);  
            
            fanStartTime = millis();
            fanTimerDuration = hours;
            isFanTimerActive = true;
        }
    }

    // PUMP 제어
    if (strcmp(message, "PUMP_ON") == 0) {
      digitalWrite(PUMP_PIN, HIGH);
      Serial.println("WATER 켜짐");
    } else if (strcmp(message, "PUMP_OFF") == 0) {
      digitalWrite(PUMP_PIN, LOW);
      Serial.println("WATER 꺼짐");
    }

    // MIST 제어
    if (strcmp(message, "MIST_ON") == 0) {
      digitalWrite(MIST_PIN, HIGH);
      Serial.println("MIST 켜짐");
    } else if (strcmp(message, "MIST_OFF") == 0) {
      digitalWrite(MIST_PIN, LOW);
      Serial.println("MIST 꺼짐");
    }

  });

  client.open("3.39.126.121", 3000);
}

void loop() {
  client.listen();

  while (Serial.available()) {
    inputString = Serial.readStringUntil('\n');
    client.send(WebSocket::DataType::TEXT,
                inputString.c_str(),
                inputString.length());
    Serial.println("전송됨: " + inputString);

    // while(Serial.read()) ;
    inputString = "";
  }

  // LED 타이머 체크
  if (isLedTimerActive) {
    unsigned long currentTime = millis();
    // 경과 시간을 시간 단위로 계산 (1시간 = 3600000 밀리초)
    unsigned long elapsedHours = (currentTime - ledStartTime) / (1000UL * 60 * 60);
    
    if (elapsedHours >= ledTimerDuration) {
      digitalWrite(LED_PIN, LOW);  // LED 끄기
      isLedTimerActive = false;
      Serial.println("LED 타이머 종료");
    }
  }

  // FAN 타이머 체크
  if (isFanTimerActive) {
    unsigned long currentTime = millis();
    unsigned long elapsedHours = (currentTime - fanStartTime) / (1000UL * 60 * 60);
    
    if (elapsedHours >= fanTimerDuration) {
      digitalWrite(FAN_PIN, LOW);
      isFanTimerActive = false;
      Serial.println("FAN 타이머 종료");
    }
  }
}
