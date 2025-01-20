// 서버에서 Web shocket을 활용한 LED, PUMP, FAN, MIST 제어 장치

// 웹소켓 설정
#include <WebSocketClient.h>

using namespace net;

WebSocketClient client;
String inputString = "";      // 시리얼 입력을 저장할 문자열
boolean stringComplete = false;  // 문자열 완성 여부

// Wi-Fi 설정
#include <WiFiS3.h>
char ssid[] = "Daesin2"  ;  // "Daesin2"   "KT_GiGA_8141"  "Farm_2.4g"  "sFarm_2.4g"
char pass[] = "ds0123456" ;   // "ds0123456"  "4ed30kb380"  "20240603"  "ds123456"
int status = WL_IDLE_STATUS;

WiFiClient wifiClient;

// LCD 설정
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 20, 4);

// 메트릭스 스위치 핀 설정
#define nROW 4
#define nCOL 4
byte ROW_PINs[4] = {6, 7, 8, 9};
byte COL_PINs[4] = {5, 4, 3, 2};
char keys[nROW][nCOL] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

// 장치별 제어핀
const int LED1_Pin = 10;
const int PUMP1_Pin = 11;
const int FAN1_Pin = 12;
const int MIST1_Pin = 13;

struct STATE_DATA {
  String LED1;
  String PUMP1;
  String FAN1;
  String MIST1;
};

STATE_DATA stateData;
void connectWebSocket();
void handleWiFiConnection();
void printWifiStatus();
void handleKeypadInput(); 
void handleKeyAction(String key); 
void updateDeviceState();
void timeSendData();
void defaultStateData();

// unsigned long lastLoopTime = 0;
// const unsigned long loopInterval = 1000; // 1분 간격
unsigned long wifiTimeout = 10000; // WiFi 연결 타임아웃 (10초)
unsigned long startAttemptTime;
const unsigned long debounceDelay = 10; // 디바운스 시간 디바운스 시간 (10ms로 설정)
char lastKey = '\0'; // 마지막 키 상태 저장
bool isWebSocketOpen = false; // WebSocket 상태 추적 변수

void setup() {
  Serial.begin(115200);

  lcd.init();
  lcd.backlight();
  lcd.print("Hello Smart Farm...");

  pinMode(LED1_Pin, OUTPUT);
  pinMode(PUMP1_Pin, OUTPUT);
  pinMode(FAN1_Pin, OUTPUT);
  pinMode(MIST1_Pin, OUTPUT);

  // 와이파이 초기화 접속
  connectWiFi() ;

  // Matrix SW Pin Setting 
  for (int i = 0; i < nROW; i++)
        pinMode(ROW_PINs[i], INPUT_PULLUP);

  for (int i = 0; i < nCOL; i++) {
        digitalWrite(COL_PINs[i], HIGH);
        pinMode(COL_PINs[i], OUTPUT);
  }

 // 제어장치 초기값 설정 
  defaultStateData();

  Serial.println("체크 포인트 1");

  // Web_shocket 시작
  inputString.reserve(200);  // 문자열을 위한 공간 예약
  
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

   // LED 제어    "LED_ON"  "LED_OFF"
    if (strcmp(message, "LED_ON") == 0) {
      digitalWrite(LED1_Pin, HIGH);
      stateData.LED1 = "LED_ON";
      Serial.println("LED 켜짐");
    }
    else if (strcmp(message, "LED_OFF") == 0) {
      digitalWrite(LED1_Pin, LOW);
      stateData.LED1 = "LED_OFF";      
      Serial.println("LED 꺼짐");
    }

    // WATER 제어  "PUMP_ON" "PUMP_OFF"
    if (strcmp(message, "PUMP_ON") == 0) {
      digitalWrite(PUMP1_Pin, HIGH);
      stateData.PUMP1 = "PUMP_ON";
      Serial.println("WATER 켜짐");
    }
    else if (strcmp(message, "PUMP_OFF") == 0) {
      digitalWrite(PUMP1_Pin, LOW);
      stateData.PUMP1 = "PUMP_ON";
      Serial.println("WATER 꺼짐");
    }

    // FAN 제어  "FAN_ON"  "FAN_OFF"
    if (strcmp(message, "FAN_ON") == 0) {
      digitalWrite(FAN1_Pin, HIGH);
      stateData.FAN1 = "FAN_ON";
      Serial.println("FAN 켜짐");
    }
    else if (strcmp(message, "FAN_OFF") == 0) {
      digitalWrite(FAN1_Pin, LOW);
      stateData.FAN1 = "FAN_OFF";
      Serial.println("FAN 꺼짐");
    }

    // MIST 제어  "MIST_ON" "MIST_OFF"
    if (strcmp(message, "MIST_ON") == 0) {
      digitalWrite(MIST1_Pin, HIGH);
      stateData.PUMP1 = "MIST_ON";
      Serial.println("MIST 켜짐");
    }
    else if (strcmp(message, "MIST_OFF") == 0) {
      digitalWrite(MIST1_Pin, LOW);
      stateData.PUMP1 = "MIST_OFF";      
      Serial.println("MIST 꺼짐");
    }
    else if (strcmp(message, "CHECK_STATUS") == 0) {
      timeSendData();    // 데이터 송신  
      Serial.println("CHECK_STATUS");
    }
  });

  // Web_shocket 
  client.onError([](net::WebSocketError error) {
    Serial.print("WebSocket 오류 발생: ");
    Serial.println(static_cast<int>(error));
  });
  
  if (!client.open("3.39.126.121", 3000, "/")) {  // path 파라미터 추가
    Serial.println("WebSocket 초기 연결 실패");
    isWebSocketOpen = false;
  }
}

void loop() {
  unsigned long currentTime = millis();

  // WiFi 연결 확인 및 복구
  if (WiFi.status() != WL_CONNECTED) {
    connectWiFi();
  }

  if (!isWebSocketOpen) {
    Serial.println("WebSocket 재연결 시도 중...");
    connectWebSocket(); // WebSocket 재연결 시도
  }
  
  handleKeypadInput();
  client.listen(); 
  updateDeviceState(); // LCD 현재상태 표시
}

void connectWiFi() {
  WiFi.begin(ssid, pass);
  startAttemptTime = millis();

  while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < wifiTimeout) {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi connected");
    printWifiStatus();
  } else {
    Serial.println("\nWiFi connection failed");
    lcd.setCursor(0, 0);
    lcd.print("WiFi failed...");
  }
}

void handleWiFiConnection() {
  while (WiFi.status() != WL_CONNECTED) {
        lcd.setCursor(0, 0);
        lcd.print("To WiFi..........");
        status = WiFi.begin(ssid, pass);
        delay(5000);
  }
  printWifiStatus();
}

void printWifiStatus() {
    Serial.print("SSID: ");
    Serial.println(WiFi.SSID());
    IPAddress ip = WiFi.localIP();  // 아두이노의 IP 값
    String lcd_Ip = String(ip[0]) + '.' + ip[1] + '.' + ip[2] + '.' + ip[3] ; 
    // LCD에서 IP값 표시
    Serial.print("IP Addrr: ");
    Serial.println(ip);
    lcd.setCursor(0, 0);
    lcd.print("C_IP: " + lcd_Ip);
}

void connectWebSocket() {
  if (!isWebSocketOpen) {
    client.onOpen([](WebSocket &ws) {
      isWebSocketOpen = true;
      Serial.println("WebSocket 연결 성공");
    });

    client.onClose([](WebSocket &ws, const WebSocket::CloseCode code,
                     const char *reason, uint16_t length) {
      isWebSocketOpen = false;               
      Serial.println("WebSocket 연결 종료");
    });

    client.onError([](net::WebSocketError error) {
      Serial.print("WebSocket 오류: ");
      Serial.println(static_cast<int>(error));
      isWebSocketOpen = false;
    }); 

    if (!client.open("3.39.126.121", 3000, "/")) {  // path 파라미터 추가
      Serial.println("WebSocket 연결 실패");
      isWebSocketOpen = false;
    }
  }
}

void handleKeypadInput() { // Key input  module
    static int pushedCol = -1, pushedRow = -1;
    static int activeCol = 0;
    static unsigned long lastDebounceTime = 0;

  // 활성화된 열 핀 설정
  for (int i = 0; i < nCOL; i++) {
    digitalWrite(COL_PINs[i], i == activeCol ? LOW : HIGH);
  }

  // 행 핀 상태 읽기
  delay(5); // 안정적인 읽기를 위해 짧은 지연 추가

  for (int i = 0; i < nROW; i++) {
        int rowState = digitalRead(ROW_PINs[i]);
        if (rowState == LOW && (millis() - lastDebounceTime > debounceDelay)) {
            lastDebounceTime = millis(); // 디바운스 타이밍 갱신
            if (pushedRow != i || pushedCol != activeCol) {
                pushedRow = i;
                pushedCol = activeCol;
                char keyPressed = keys[pushedRow][pushedCol];
                Serial.print("Key Pressed: ");
                Serial.println(keyPressed);
                handleKeyAction(keyPressed);
            }
        }
    }

  activeCol = (activeCol + 1) % nCOL;  // 다음 열로 이동
}

// LED 제어     "LED_ON"  "LED_OFF"      FAN 제어   "FAN_ON"  "FAN_OFF"  
// WATER 제어   "PUMP_ON" "PUMP_OFF"     MIST 제어  "MIST_ON" "MIST_OFF"
void handleKeyAction(char key) {
    Serial.println("key Press : " + key);
  if (key != lastKey) { // 중복 키 입력 방지
       lastKey = key;

    switch (key) {
        case '1':
            digitalWrite(LED1_Pin, HIGH);
            stateData.LED1 = "LED_ON ";
            break;
        case '2':
            digitalWrite(LED1_Pin, LOW);
            stateData.LED1 = "LED_OFF";
            break;
        case '4':
            digitalWrite(PUMP1_Pin, HIGH);
            stateData.PUMP1 = "PUMP_ON ";
            break;
        case '5':
            digitalWrite(PUMP1_Pin, LOW);
            stateData.PUMP1 = "PUMP_OFF";
            break;
        case '7':
            digitalWrite(FAN1_Pin, HIGH);
            stateData.FAN1 = "FAN_ON ";
            break;
        case '8':
            digitalWrite(FAN1_Pin, LOW);
            stateData.FAN1 = "FAN_OFF";
            break;
        case '*':
            digitalWrite(MIST1_Pin, HIGH);
            stateData.MIST1 = "MIST_ON ";
            break;
        case '0':
            digitalWrite(MIST1_Pin, LOW);
            stateData.MIST1 = "MIST_OFF";
            break;
    }
  }
}

void updateDeviceState() {
    lcd.setCursor(0, 1);
    lcd.print("st: " + stateData.LED1 + 
                     " " + stateData.PUMP1);
    lcd.setCursor(0, 2);
    lcd.print("st: " + stateData.FAN1 + 
                     " " + stateData.MIST1);
    lcd.setCursor(0, 3);
    lcd.print("st: more control.... ");

}

void timeSendData() {  // 제어 데이터 전송   
// LED 제어  stateData.LED1   "LED_ON"  "LED_OFF" 
      inputString = stateData.LED1 ;
      client.send(WebSocket::DataType::TEXT, 
                inputString.c_str(), 
                inputString.length());
      Serial.println("전송됨: " + inputString);
      delay(1000);

// PUMP 제어 stateData.PUMP1 :  "PUMP_ON" "PUMP_OFF"     
      inputString = stateData.PUMP1 ;
      client.send(WebSocket::DataType::TEXT, 
                inputString.c_str(), 
                inputString.length());
      Serial.println("전송됨: " + inputString);
      delay(1000);

// FAN 제어 stateData.FAN1 :  FAN 제어   "FAN_ON"  "FAN_OFF"  
      inputString = stateData.FAN1 ;
      client.send(WebSocket::DataType::TEXT, 
                inputString.c_str(), 
                inputString.length());
      Serial.println("전송됨: " + inputString);
      delay(1000);

// MIST 제어  stateData.MIST1 : "MIST_ON" "MIST_OFF"
      inputString = stateData.MIST1 ;
      client.send(WebSocket::DataType::TEXT, 
                inputString.c_str(), 
                inputString.length());
      Serial.println("전송됨: " + inputString);
      delay(1000);
}

void defaultStateData() {
    stateData.LED1 = "LED_ON ";
    digitalWrite(LED1_Pin, HIGH);
    stateData.PUMP1 = "PUMP_ON ";
    digitalWrite(PUMP1_Pin, HIGH);
    stateData.FAN1 = "FAN_ON ";
    digitalWrite(FAN1_Pin, HIGH);
    stateData.MIST1 = "MIST_ON ";
    digitalWrite(MIST1_Pin, HIGH);
    updateDeviceState() ;
}