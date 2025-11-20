#include <WiFi.h>
#include <HTTPClient.h>
#include "DHT.h"
#include <LiquidCrystal_I2C.h>
// WiFi credentials
const char* ssid = "Viet Tho";
const char* password = "987654321";
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Pushover credentials
const char* pushoverToken = "amejqgya25b3ynjd8p2thwphh7vjnz";
const char* pushoverUser = "uf9382era3gv64ueuoptsk656g6wo9";

// DHT11 setup
#define DHTPIN 4      // GPIO3
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// Buzzer
#define BUZZER 5

// Ngưỡng cảnh báo
const float TEMP_THRESHOLD = 35.0;  // °C
const float HUM_THRESHOLD = 80.0;   // %

void setup() {
  Wire.begin(6,7);
    lcd.init();              // Khởi tạo LCD
  lcd.backlight();         // Bật đèn nền
  lcd.setCursor(0, 0);  
  pinMode(BUZZER, OUTPUT);
  digitalWrite(BUZZER, LOW); // Tắt buzzer lúc khởi động
  Serial.begin(115200);

  // Khởi tạo DHT11
  dht.begin();

  // Kết nối WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected!");

  // Gửi thông báo test
  sendPushover("Thông báo từ ESP32", "Hệ thống đã khởi động thành công.");
}

void loop() {
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("Không đọc được dữ liệu từ DHT11!");
    lcd.print("Không đọc được dữ liệu từ DHT11!");
  } else {

    Serial.print("Nhiệt độ: ");
    Serial.print(temperature);
    lcd.clear();
    lcd.print("Temp: ");
    lcd.print(temperature);
    lcd.print(" C      ");
    Serial.print("°C  |  Độ ẩm: ");
    Serial.print(humidity);
    Serial.println("%");
    lcd.setCursor(0,1);
    lcd.print("Humid: ");
    lcd.print(humidity);
    lcd.println(" %       ");
    // Cảnh báo nhiệt độ
    if (temperature > TEMP_THRESHOLD) {
      buzzerBeep();
      sendPushover("Cảnh báo nhiệt độ!", "Nhiệt độ quá cao: " + String(temperature) + "°C");
    }

    // Cảnh báo độ ẩm
    if (humidity > HUM_THRESHOLD) {
      buzzerBeep();
      sendPushover("Cảnh báo độ ẩm!", "Độ ẩm quá cao: " + String(humidity) + "%");
    }
  }

  delay(3000); // đọc mỗi 5 giây
}

// Hàm beep buzzer 1 giây
void buzzerBeep() {
  digitalWrite(BUZZER, HIGH);
  delay(1000);
  digitalWrite(BUZZER, LOW);
}

// Hàm encode URL
String urlEncode(const String &str) {
  String encoded = "";
  char c;
  char code0;
  char code1;
  for (int i =0; i < str.length(); i++){
    c=str.charAt(i);
    if (isalnum(c)) {
      encoded += c;
    } else {
      code1=(c & 0xf)+'0';
      if ((c & 0xf) >9) code1=(c & 0xf) -10+'A';
      c=(c>>4)&0xf;
      code0=c+'0';
      if (c>9) code0=c-10+'A';
      encoded += '%';
      encoded += code0;
      encoded += code1;
    }
  }
  return encoded;
}

// Gửi Pushover
void sendPushover(const String& title, const String& message) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin("https://api.pushover.net/1/messages.json");
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");

    String postData = "token=" + String(pushoverToken) +
                      "&user=" + String(pushoverUser) +
                      "&title=" + urlEncode(title) +
                      "&message=" + urlEncode(message);

    int httpResponseCode = http.POST(postData);

    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.println("Message sent successfully:");
      Serial.println(response);
    } else {
      Serial.print("Error sending message: ");
      Serial.println(httpResponseCode);
    }

    http.end();
  } else {
    Serial.println("WiFi disconnected!");
  }
}
