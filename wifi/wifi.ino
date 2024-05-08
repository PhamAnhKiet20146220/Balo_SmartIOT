#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "HX711.h"
#include <TinyGPS++.h>
#include <HardwareSerial.h>
#include <WiFi.h>
#include <Wire.h>
#include <BlynkSimpleEsp32.h>


float latitude, longitude;
String lat_str, lng_str;

const char *ssid ="Redmi";
const char *pass ="88888888";

#define BLYNK_TEMPLATE_NAME "BALO Thông minh"
#define BLYNK_AUTH_TOKEN "kekWI-token-token-jSBcVGzB-jr"
char auth[] = "emjVIf_Mn0mFR2uQp7ql2mEKUME5l2Vs";

WidgetMap myMap(V0);
WiFiClient client;
TinyGPSPlus gps;
HardwareSerial SerialGPS(1);

// Định nghĩa chân DOUT và SCK của HX711
#define DOUT_PIN  4
#define SCK_PIN   5

//chân 18 là còi
#define coi 18

// Địa chỉ của màn hình LCD I2C
int lcdAddress = 0x27;
// Số cột và số dòng của màn hình LCD
int lcdColumns = 16;
int lcdRows = 2;

// Tạo đối tượng LCD
LiquidCrystal_I2C lcd(lcdAddress, lcdColumns, lcdRows);

// Khởi tạo đối tượng HX711
HX711 scale;

Adafruit_MPU6050 mpu;

//tạo timer cho còi:
unsigned long giatricu = 0;
boolean coiDangKeu = false;


void setup(void) {
  Serial.begin(115200);
  SerialGPS.begin(9600, SERIAL_8N1, 16, 17);
  
  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected");

  Blynk.begin(auth, ssid, pass, "blynk.tk", 8080); 
  Blynk.virtualWrite(V0, "clr");
  
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
  
  // Khởi tạo HX711 với chân DOUT và SCK
  scale.begin(DOUT_PIN, SCK_PIN);
  // Chờ đến khi cân sẵn sàng
  Serial.println("Waiting for the HX711 to settle...");
  scale.set_scale();
  delay(500);
  scale.tare(); // Thiết lập giá trị zero
  Serial.println("Ready!");

  pinMode(coi, OUTPUT);
  
  lcd.setCursor(3, 1);
  lcd.print("--READY--");
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

  float weight = scale.get_units(5); // Lấy giá trị trung bình từ 5 đọc

  /* in giá trị góc MPU6050 */
  Serial.print("Y: ");
  Serial.print(pitch,1);
  Serial.print(", X: ");
  Serial.println(roll,1);
  
  // In giá trị khối lượng ra Serial Monitor
  Serial.print("Weight: ");
  Serial.print(abs(weight/100000));
  Serial.println(" kg");

  
  lcd.clear();  // Xóa nội dung trước khi hiển thị giá trị mới
  
  //in giá trị góc của MPU6050 hàng 0
  lcd.setCursor(0, 0);
  lcd.print("X:");
  lcd.setCursor(8, 0);
  lcd.print("Y:");
  lcd.setCursor(2, 0);
  lcd.print(roll,1);
  lcd.setCursor(10, 0);
  lcd.print(pitch,1);
  
  //in giá trị khối lượng hàng 1
  lcd.setCursor(0, 1);
  lcd.print("khoi luong:");
  lcd.print(abs(weight/100000),1);
  lcd.setCursor(14, 1);
  lcd.print("kg");

  //GPS
 while (SerialGPS.available() > 0)
  {
    if (gps.encode(SerialGPS.read()))
    {
      if (gps.location.isValid())
      {
        latitude = gps.location.lat();
        lat_str = String(latitude, 6);
        longitude = gps.location.lng();
        lng_str = String(longitude, 6);

        Serial.print("Latitude = ");
        Serial.println(lat_str);
        Serial.print("Longitude = ");
        Serial.println(lng_str);

        Blynk.virtualWrite(V0, 1, latitude, longitude, "Location");
      }
    }
  }

  //data đến blynk
  Blynk.virtualWrite(V1, roll );
  Blynk.virtualWrite(V2, pitch );
  Blynk.virtualWrite(V3, abs(weight/100000) );
  
  Blynk.run();
  if ((abs(roll) >= 45) || (abs(pitch) >= 45) || abs(weight / 100000) >= 3) {
    if (!coiDangKeu) {
      analogWrite(coi, 100);
      giatricu = millis(); // Lưu giá trị thời gian hiện tại
      coiDangKeu = true;
    }
    else {
      // Kiểm tra nếu đã kêu 1 giây
      if (millis() - giatricu >= 1000) {
        analogWrite(coi, 0);
        coiDangKeu = false;
      }
    }
  } 
  else {
    analogWrite(coi, 0);
  }
  
  delay(1000); // Để giữ hiển thị trong khoảng thời gian ngắn
}
