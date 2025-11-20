#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include "RTClib.h"
#include <Adafruit_GFX.h>
//#include <Adafruit_SSD1306.h>
#include <ESP32Servo.h>
#include <LiquidCrystal_I2C.h>

RTC_DS3231 rtc;

#define MAX_SCHEDULE 10
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1

#define button1 4
#define button2 5
#define BUZZER_PIN 18   // bạn có thể đổi sang chân khác phù hợp ESP32
#define MODE_BUTTON 0
LiquidCrystal_I2C lcd(0x27, 16, 2);
const char* pushoverToken = "";
const char* pushoverUser = "";
Servo servo[2];
bool statusComp[2] = {false, false};
int k=0;
bool d[200];
int angle[2]={180,180};
int manualMode=0;
bool opened[2];
bool mopen[2];
volatile unsigned long lastInterruptTime1 = 0;
volatile unsigned long lastInterruptTime2 = 0;
const unsigned long debounceDelay = 200;  // 200ms chống dội
// =================== Ngắt nút bấm ===================
void IRAM_ATTR toggleMode() {
    manualMode++;
  if (manualMode > 2) manualMode = 0;   // quay vòng 0 → 1 → 2 → 0
}

void IRAM_ATTR tog1() {
  unsigned long now = millis();
  if (now - lastInterruptTime1 > debounceDelay) {
    if (manualMode==0) {
      closeservo(0);
    } else if (manualMode==1) {
      mopen[0] = !mopen[0];
    }
    lastInterruptTime1 = now;
  }
}

void IRAM_ATTR tog2() {
  unsigned long now = millis();
  if (now - lastInterruptTime2 > debounceDelay) {
    if (manualMode==0) {
      closeservo(1);
    } else if (manualMode==1) {
      mopen[1] = !mopen[1];
    }
    lastInterruptTime2 = now;
  }}

// =================== Class ngăn thuốc ===================
class Compartment {
  public:
    int id[MAX_SCHEDULE];
    int hour[MAX_SCHEDULE];
    int minute[MAX_SCHEDULE];
    int pills[MAX_SCHEDULE];
    int status[MAX_SCHEDULE];
    bool du[MAX_SCHEDULE];
    int count;

    Compartment() { count = 0; }

    void addSchedule(int _id, int h, int m, int p) {
      if (count < MAX_SCHEDULE) {
        id[count] = _id;
        hour[count] = h;
        minute[count] = m;
        pills[count] = p;
        count++;
      }
    }
    
    void clearSchedules() { count = 0; }

    void printData(int cid) {
      Serial.printf("Ngăn %d có %d lịch uống thuốc:\n", cid, count);

      for (int i = 0; i < count; i++) {
        du[i]=d[id[i]];
        Serial.printf("  [id=%d] %02d:%02d | Viên: %d | Status: %d | du: %d\n",
                      id[i], hour[i], minute[i], pills[i], status[i], du[i]);
      
                      
      }
    }
};

// =================== WiFi + Server ===================
const char* ssid = "";
const char* password = "";
const char* serverUrl = "";

// =================== Mảng 6 ngăn thuốc ===================
Compartment com[6];

// =================== Hàm load dữ liệu từ JSON ===================
void updateCompartments(String payload) {
  DynamicJsonDocument doc(8192);
  DeserializationError error = deserializeJson(doc, payload);

  if (error) {
    Serial.print("Lỗi parse JSON: ");
    Serial.println(error.c_str());
    return;
  }

  for (int i = 0; i < 6; i++) {
    com[i].clearSchedules();
  }

  for (JsonObject item : doc.as<JsonArray>()) {
    int _id = item["id"];
    int compartment = item["compartment"];
    int hour = item["hour"];
    int minute = item["minute"];
    int pills = item["pills"];

    if (compartment >= 1 && compartment <= 6) {
      com[compartment - 1].addSchedule(_id, hour, minute, pills);
    }
  }
}

// =================== Lấy thời gian hiện tại từ RTC ===================
void currentTime(int &h, int &m) {
  DateTime now = rtc.now();
  h = now.hour();
  m = now.minute();
}

void openbox(int n){
  if (angle[n]==180){
    while (angle[n]>120){
      angle[n]--;
      servo[n].write(angle[n]);
      delay(10);
    } 
  }
  opened[n]=1;
}
void closebox(int n){

  while (angle[n] < 180) {
    angle[n]++;
    servo[n].write(angle[n]);
    delay(10);
  }
  opened[n]=0;
}
// =================== Hàm đóng servo ===================
void closeservo(int n) {
  if (statusComp[n]) {
    closebox(n);
    Serial.print("Close servo ");
    Serial.println(n+1);
    statusComp[n] = false;
    
    if (n==0){
      for (int i=0; i<3; i++)
      {
        for (int j = 0; j < com[i].count; j++)
        {
          if (com[i].status[j]==1)
          {
            com[i].du[j]=1;
            d[com[i].id[j]]=1;
            com[i].status[j]=0;
          }
        }
      }
    }
        if (n==1){
      for (int i=3; i<6; i++)
      {
        for (int j = 0; j < com[i].count; j++)
        {
          if (com[i].status[j]==1)
          {
            com[i].du[j]=1;
            d[com[i].id[j]]=1;
            com[i].status[j]=0;
          }
        }
      }
    }
    // servo[n].write(0);
  }
}
void sendPushover(String message) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin("https://api.pushover.net/1/messages.json");
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");

    String postData = "token=" + String(pushoverToken) +
                      "&user=" + String(pushoverUser) +
                      "&message=" + message;

    int httpResponseCode = http.POST(postData);

    if (httpResponseCode > 0) {
      Serial.print("Pushover sent. HTTP ");
      Serial.println(httpResponseCode);
    } else {
      Serial.print("Error sending: ");
      Serial.println(httpResponseCode);
    }

    http.end();
  } else {
    Serial.println("WiFi disconnected");
  }
}
void printtime(int h, int m)
{
lcd.clear();
lcd.setCursor(0, 0);
lcd.print("Time:");
if (h<10){lcd.print("0");}
lcd.print(h);
lcd.print(":");
if (m<10){lcd.print("0");}
lcd.print(m);
}
// =================== Hàm check lịch uống thuốc ===================
void check() {
  int h, m;
  currentTime(h, m);
  Serial.print("Current time: ");
  Serial.print(h);
  Serial.print(":");
  Serial.println(m);
  if(opened[0]==0 && opened[1]==0){printtime(h,m);}
 if(h==0 && m==0){
  for (int i = 0; i < 200; i++) {
  d[i] = 0;   //reset toàn bộ mảng về 0
}
}
  for (int i = 0; i < 6; i++) { 
    for (int j = 0; j < com[i].count; j++) { 
      if (com[i].hour[j] == h && com[i].minute[j] == m && com[i].status[j] == 0 && com[i].du[j]==0) {
       // digitalWrite(BUZZER_PIN, HIGH);
       // delay(1000);
       // digitalWrite(BUZZER_PIN, LOW);
       lcd.clear();
        Serial.printf(">>> Open ngăn %d (id=%d)\n", i + 1, com[i].id[j]);
        if (i < 3) {
          Serial.println("Mở ngăn 1");
          openbox(0);
          statusComp[0] = true;
        } else {
          Serial.println("Mở ngăn 2");
          openbox(1);
          statusComp[1] = true;
        }
        com[i].status[j] = 1;  // đánh dấu đã mở
        lcd.setCursor(0,0);
          lcd.print("N");
          lcd.print(i+1);
          lcd.print(":");
          lcd.print(com[i].pills[j]);
          lcd.print("|");
          String message = String("Đã đến giờ uống thuốc: ") + "Ngăn " + String(i+1) + " - " + String(com[i].pills[j]) + " viên.";
          sendPushover(message);

      }
    }
  }
}
void updateLCD() {
  int h, m;
  currentTime(h, m);

  lcd.setCursor(0, 0);
  lcd.print("Mode: ");
  if (manualMode==1) {
    lcd.print("Manual ");
  } else {
    lcd.print("Auto   ");
  }

  lcd.setCursor(0, 1);
  lcd.print("Time ");
  if (h < 10) lcd.print("0");
  lcd.print(h);
  lcd.print(":");
  if (m < 10) lcd.print("0");
  lcd.print(m);
  lcd.print("     "); // xóa phần thừa nếu có
}

// =================== Setup ===================
void setup() {
  Serial.begin(115200);
  Wire.begin(6, 7);

  WiFi.begin(ssid, password);
  Serial.print("Đang kết nối WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected!");
  Serial.println(WiFi.localIP());

  if (!rtc.begin()) {
    Serial.println("Không tìm thấy RTC!");
    while (1);
  }

  if (rtc.lostPower()) {
    Serial.println("RTC chưa được set thời gian, set mặc định...");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
  servo[0].attach(2);   // servo ngăn 1 nối chân GPIO 12
  servo[1].attach(3);   // servo ngăn 2 nối chân GPIO 13
  servo[0].write(180);
  servo[1].write(180);
  pinMode(button1, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(button1), tog1, RISING);

  pinMode(button2, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(button2), tog2, RISING);

  pinMode(MODE_BUTTON, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(MODE_BUTTON), toggleMode, RISING);

  pinMode(BUZZER_PIN, OUTPUT);
digitalWrite(BUZZER_PIN, LOW);
opened[0]=0;
opened[1]=0;
mopen[0]=0;
mopen[1]=0;
lcd.init();           // khởi tạo LCD
lcd.backlight();      // bật đèn nền
lcd.clear();
lcd.setCursor(0, 0);
lcd.print("Smart Box");
lcd.setCursor(0, 1);
lcd.print("Dang khoi dong...");
sendPushover("ESP khởi động");
delay(1000);
lcd.clear();
}

// =================== Loop ===================
unsigned long lastCheck = 0;
unsigned long lastUpdate = 0;
const unsigned long checkInterval = 1000;    // 1 giây
const unsigned long updateInterval = 10000;  // 10 giây

void loop() {
  if (manualMode==0){
    unsigned long now = millis();

  // Load JSON từ server mỗi 10 giây
  if (now - lastUpdate >= updateInterval) {
    lastUpdate = now;

    if (WiFi.status() == WL_CONNECTED) {
      HTTPClient http;
      http.begin(serverUrl);
      int httpCode = http.GET();

      if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        updateCompartments(payload);

        for (int i = 0; i < 6; i++) {
          com[i].printData(i + 1);
        }
      } else {
        Serial.print("Lỗi HTTP: ");
        Serial.println(httpCode);
      }
      http.end();
    } else {
      Serial.println("WiFi không kết nối!");
    }
  }
  // Check lịch uống thuốc mỗi 1 giây
  if (now - lastCheck >= checkInterval) {
    lastCheck = now;
    check();
  }
  }
  else if (manualMode==1){
    lcd.setCursor(0, 0);
    lcd.print("Manual mode");
    for (int i = 0; i < 2; i++) {
      if (mopen[i] && !opened[i]) {
        openbox(i);
      } else if (!mopen[i] && opened[i]) {
        closebox(i);
      }
    }
  }
  else {
    Serial.println("Mode 3");
    delay(50);
  }
}
