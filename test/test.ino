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

const char *ssid = "Redmi";
const char *pass = "88888888";

char auth[] = "emjVIf_Mn0mFR2uQp7ql2mEKUME5l2Vs";

WidgetLED LED_ON_APP(V4);
int button;

WidgetMap myMap(V0);
WiFiClient client;
TinyGPSPlus gps;
HardwareSerial SerialGPS(1);

#define DOUT_PIN 4
#define SCK_PIN 5
#define coi 18

int lcdAddress = 0x27;
int lcdColumns = 16;
int lcdRows = 2;
LiquidCrystal_I2C lcd(lcdAddress, lcdColumns, lcdRows);

HX711 scale;

Adafruit_MPU6050 mpu;

BlynkTimer timer;

unsigned long giatricu = 0;
boolean coiDangKeu = false;

void coiOn() {
  analogWrite(coi, 100);
  LED_ON_APP.on();
}

void coiOff() {
  analogWrite(coi, 0);
  LED_ON_APP.off();
}

void readGPS() {
  while (SerialGPS.available() > 0) {
    if (gps.encode(SerialGPS.read())) {
      if (gps.location.isValid()) {
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
}

void readSensorData() {
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  float Ax = a.acceleration.x / 9.81;
  float Ay = a.acceleration.y / 9.81;
  float Az = a.acceleration.z / 9.81;

  float pitch = atan2(Ax, sqrt(pow(Ay, 2) + pow(Az, 2))) * 360.0 / M_PI;
  float roll = atan2(Ay, sqrt(pow(Ax, 2) + pow(Az, 2))) * 360.0 / M_PI;

  float weight = scale.get_units(5);

  Serial.print("Y: ");
  Serial.print(pitch, 1);
  Serial.print(", X: ");
  Serial.println(roll, 1);

  Serial.print("Weight: ");
  Serial.print(abs(weight / 100000));
  Serial.println(" kg");

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("X:");
  lcd.setCursor(8, 0);
  lcd.print("Y:");
  lcd.setCursor(2, 0);
  lcd.print(roll, 1);
  lcd.setCursor(10, 0);
  lcd.print(pitch, 1);

  lcd.setCursor(0, 1);
  lcd.print("khoi luong:");
  lcd.print(abs(weight / 100000), 1);
  lcd.setCursor(14, 1);
  lcd.print("kg");

  Blynk.virtualWrite(V1, roll);
  Blynk.virtualWrite(V2, pitch);
  Blynk.virtualWrite(V3, abs(weight / 100000));

  if ((abs(roll) >= 45) || (abs(pitch) >= 45) || abs(weight / 100000) >= 3 || button == 1) {
    if (!coiDangKeu) {
      coiOn();
      giatricu = millis();
      coiDangKeu = true;
    } else {
      if (millis() - giatricu >= 1000) {
        coiOff();
        coiDangKeu = false;
      }
    }
  } else {
    coiOff();
  }
}

void setup(void) {
  Serial.begin(115200);
  SerialGPS.begin(9600, SERIAL_8N1, 16, 17);

  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
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

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("X: ");
  lcd.setCursor(8, 0);
  lcd.print("Y: ");

  scale.begin(DOUT_PIN, SCK_PIN);
  Serial.println("Waiting for the HX711 to settle...");
  scale.set_scale();
  delay(500);
  scale.tare();
  Serial.println("Ready!");

  pinMode(coi, OUTPUT);

  lcd.setCursor(3, 1);
  lcd.print("--READY--");

  // Đặt các hàm vào Blynk Timer
  timer.setInterval(1000L, readGPS);
  timer.setInterval(1000L, readSensorData);
}
BLYNK_WRITE(V5) {
  button = param.asInt();
  if (button == 1) {
    analogWrite(coi, 100);
    LED_ON_APP.on();
  } else {
    analogWrite(coi, 0);
    LED_ON_APP.off();
  }
}
void loop() {
  Blynk.run();
  timer.run();
}
