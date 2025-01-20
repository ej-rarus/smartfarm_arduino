#include <ArduinoHttpClient.h>
#include <WiFiS3.h>

///////please enter your sensitive data in the Secret tab/arduino_secrets.h
/////// WiFi Settings ///////
const char ssid[] = "sfarm_2.4g";
const char pass[] = "ds123456";
int status = WL_IDLE_STATUS;

char serverAddress[] = "3.39.126.121";  // server address
int port = 3000;

WiFiClient wifi;
HttpClient client = HttpClient(wifi, serverAddress, port);

void setup() {
  Serial.begin(9600);
  while ( status != WL_CONNECTED) {
    Serial.print("Attempting to connect to Network named: ");
    Serial.println(ssid);                   // print the network name (SSID);

    // Connect to WPA/WPA2 network:
    status = WiFi.begin(ssid, pass);
  }

  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);
}

void loop() {
  // 랜덤 센서 데이터 생성
  int sensor_id = random(1, 4);  // 1~3 사이의 랜덤 센서 ID
  float temperature = random(2000, 3500) / 100.0;  // 20.00~35.00°C
  float humidity = random(4000, 8000) / 100.0;     // 40.00~80.00%
  float light_intensity = random(500, 1500);       // 500~1500 lux

  Serial.println("센서 데이터 POST 요청 전송");
  String contentType = "application/json";
  
  // JSON 형식으로 데이터 구성
  String postData = "{\"sensor_id\":" + String(sensor_id) + 
                    ",\"temperature\":" + String(temperature) + 
                    ",\"humidity\":" + String(humidity) + 
                    ",\"light_intensity\":" + String(light_intensity) + "}";

  client.post("/api/sensor-data", contentType, postData);

  // 응답 확인
  int statusCode = client.responseStatusCode();
  String response = client.responseBody();

  Serial.print("상태 코드: ");
  Serial.println(statusCode);
  Serial.print("응답: ");
  Serial.println(response);

  Serial.println("1분 대기");
  delay(60000);

}
