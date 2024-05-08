#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>


// Địa chỉ của màn hình LCD I2C
int lcdAddress = 0x27;
// Số cột và số dòng của màn hình LCD
int lcdColumns = 16;
int lcdRows = 2;

// Tạo đối tượng LCD
LiquidCrystal_I2C lcd(lcdAddress, lcdColumns, lcdRows);

Adafruit_MPU6050 mpu;

void setup(void) {
  Serial.begin(115200);
  while (!Serial)
    delay(10);

  Serial.println("Adafruit MPU6050 test!");

  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    while (1) {
      delay(10);
    }
  }
  Serial.println("MPU6050 Found!");

  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_94_HZ);

  Serial.println("");
  delay(100);

  
  // Khởi tạo màn hình LCD
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("X: ");
  lcd.setCursor(8, 0);
  lcd.print("Y: ");
}

void loop() {
  /* Get new sensor events with the readings */
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  /* Calculate pitch and roll */
  float Ax = a.acceleration.x / 9.81; // Convert to g
  float Ay = a.acceleration.y / 9.81; // Convert to g
  float Az = a.acceleration.z / 9.81; // Convert to g

  float pitch = atan2(Ax, sqrt(pow(Ay, 2) + pow(Az, 2))) * 360.0 / M_PI;
  float roll = atan2(Ay, sqrt(pow(Ax, 2) + pow(Az, 2))) * 360.0 / M_PI;


  /* Print out the values */
  Serial.print("Y: ");
  Serial.print(pitch,1);
  Serial.print(", X: ");
  Serial.println(roll,1);

  lcd.clear();  // Xóa nội dung trước khi hiển thị giá trị mới
  lcd.setCursor(0, 0);
  lcd.print("X:");
  lcd.setCursor(8, 0);
  lcd.print("Y:");
  lcd.setCursor(2, 0);
  lcd.print(roll,1);
  lcd.setCursor(10, 0);
  lcd.print(pitch,1);
  lcd.setCursor(0, 1);
  lcd.print("khoi luong:");
  delay(1000); // Để giữ hiển thị trong khoảng thời gian ngắn
}
