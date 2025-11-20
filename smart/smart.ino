#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// Thông tin WiFi
const char* ssid = "";
const char* password = "";

// URL data.php trên server local
const char* serverUrl = ""; // thay IP/đường dẫn phù hợp

void setup() {
  Serial.begin(115200);
  delay(1000);

  // Kết nối WiFi
  WiFi.begin(ssid, password);
  Serial.print("Đang kết nối WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected!");
  Serial.println(WiFi.localIP());
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(serverUrl);
    int httpCode = http.GET();

    if (httpCode > 0) { // Kiểm tra HTTP response
      if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        //Serial.println("Dữ liệu nhận được:");
        //Serial.println(payload);

        // Parse JSON
        DynamicJsonDocument doc(2048);
        DeserializationError error = deserializeJson(doc, payload);

        if (!error) {
          for (JsonObject item : doc.as<JsonArray>()) {
            int id = item["id"];
            int compartment = item["compartment"];
            int hour = item["hour"];
            int minute = item["minute"];
            int pills = item["pills"];
            int status = item["status"];

            Serial.printf("ID: %d | Ngăn: %d | %02d:%02d | Viên: %d | Status: %d\n",
                          id, compartment, hour, minute, pills, status);
          }
        } else {
          Serial.print("Lỗi parse JSON: ");
          Serial.println(error.c_str());
        }
      }
    } else {
      Serial.print("Lỗi HTTP: ");
      Serial.println(httpCode);
    }

    http.end();
  } else {
    Serial.println("WiFi không kết nối!");
  }

  delay(10000); // Lấy dữ liệu mỗi 10 giây
}
