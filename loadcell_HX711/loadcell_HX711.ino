#include "HX711.h"

// Định nghĩa chân DOUT và SCK
#define DOUT_PIN  16
#define SCK_PIN   17

// Khởi tạo đối tượng HX711
HX711 scale;

void setup() {
  Serial.begin(115200);
  Serial.println("HX711 ESP32 Example");

  // Khởi tạo HX711 với chân DOUT và SCK
  scale.begin(DOUT_PIN, SCK_PIN);

  // Chờ đến khi cân sẵn sàng
  Serial.println("Waiting for the HX711 to settle...");
  scale.set_scale();
  delay(500);
  scale.tare(); // Thiết lập giá trị zero
  Serial.println("Ready!");
}

void loop() {
  // Đọc giá trị từ load cell
  float weight = scale.get_units(5); // Lấy giá trị trung bình từ 5 đọc

  // In giá trị ra Serial Monitor
  Serial.print("Weight: ");
  Serial.print(abs(weight/100000));
  Serial.println(" kg");

  delay(1000); // Đợi 1 giây trước khi đọc lại
}
