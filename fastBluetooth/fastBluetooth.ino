#include <SoftwareSerial.h>
#include <PWMServo.h>
#include <string.h>  // strtok kullanmak için gerekli

// Pin tanımlamaları
const int in1 = 8;
const int in2 = 6;
const int in3 = 7;
const int in4 = 12;
const int enA = 3;
const int enB = 5;
const int laser = 2;
const int firlaticilar = 11;
const int atesleyici = 4;

PWMServo myServo;
PWMServo myServo2;

// Hedef açı değişkenleri
int targetHorizontalAngle = 90;
int targetVerticalAngle = 90;

// Son güncelleme zamanı
unsigned long lastServoUpdate = 0;
const unsigned long servoUpdateInterval = 170;  // Servo güncelleme aralığı
unsigned long lastMotorUpdate = 0;
const unsigned long motorUpdateInterval = 150;  // Motor güncelleme aralığı

char inputBuffer[26];             // Gelen Bluetooth verisini tutmak için buffer
SoftwareSerial bluetooth(13, 1);  // RX = pin 10, TX = pin 11

void setup() {
  bluetooth.begin(9600);
  myServo.attach(10);
  myServo2.attach(9);
  myServo.write(targetHorizontalAngle);
  myServo2.write(targetVerticalAngle);

  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  pinMode(in3, OUTPUT);
  pinMode(in4, OUTPUT);
  pinMode(laser, OUTPUT);
  pinMode(enA, OUTPUT);
  pinMode(enB, OUTPUT);
  pinMode(firlaticilar, OUTPUT);
  pinMode(atesleyici, OUTPUT);

  digitalWrite(firlaticilar, LOW);
  resetMotorPins();
}

void loop() {
  // Bluetooth verisi kontrol et
  if (bluetooth.available()) {
    // Gelen veriyi oku
    int bytesRead = bluetooth.readBytesUntil('\n', inputBuffer, sizeof(inputBuffer) - 1);
    inputBuffer[bytesRead] = '\0'; // Null terminator ekle

    // Veriyi strtok ile ayır
    char* motorCommand = strtok(inputBuffer, ";");  // Motor komutunu al
    char* turretCommand = strtok(NULL, ";");        // Turret komutunu al
    char* deviceCommand = strtok(NULL, ";");        // Cihaz komutunu al

    if (motorCommand && turretCommand && deviceCommand) {
      // Motor komutunu al ve kontrol et
      controlTankMotors(motorCommand);  // Motorları kontrol et
      updateTurretTargets(turretCommand);  // Kule hedef açılarını güncelle
      updateServos();                      // Servo motorlarını güncelle

      controlDevice(deviceCommand, 'L', laser);         // Lazer kontrolü
      controlDevice(deviceCommand, 'F', firlaticilar);  // Ateşleme kontrolü
      controlDevice(deviceCommand, 'T', atesleyici);    // Ateşleme kontrolü
    }
  }
}

// Motor pinlerini sıfırlamak için fonksiyon
void resetMotorPins() {
  digitalWrite(in1, LOW);
  digitalWrite(in2, LOW);
  digitalWrite(in3, LOW);
  digitalWrite(in4, LOW);
}

// Tank motorlarını kontrol eden fonksiyon
void controlTankMotors(char* command) {
  char* xValueStr = strtok(command, ",");  // X ekseni değeri
  char* yValueStr = strtok(NULL, ",");     // Y ekseni değeri

  if (xValueStr && yValueStr) {
    int xValue = atoi(xValueStr);  // X ekseni değerini string'den integer'a çevir
    int yValue = atoi(yValueStr);  // Y ekseni değerini string'den integer'a çevir
    
    // Değerleri -9 ile 9 arasında sınırla
    xValue = constrain(xValue, -9, 9);
    yValue = constrain(yValue, -9, 9);
    
    // Joystick değerlerini motor hızına dönüştür
    int leftSpeed = map(xValue, -9, 9, -255, 255);
    int rightSpeed = map(yValue, -9, 9, -255, 255);

    moveMotors(leftSpeed, rightSpeed);    // Motorları hareket ettir
  }
}

// Motorları normal hareket ettir
void moveMotors(int leftSpeed, int rightSpeed) {
  setMotorSpeed(enB, leftSpeed, in3, in4);
  setMotorSpeed(enA, -rightSpeed, in1, in2);
}

// Motor hızını ayarlamak için fonksiyon
void setMotorSpeed(int enPin, int speed, int inPin1, int inPin2) {
  if (speed >= 0) {
    analogWrite(enPin, speed);  // Motor ileri
    digitalWrite(inPin1, LOW);
    digitalWrite(inPin2, HIGH);
  } else {
    analogWrite(enPin, -speed);  // Motor geri
    digitalWrite(inPin1, HIGH);
    digitalWrite(inPin2, LOW);
  }
}

// Hedef açıları güncelle
void updateTurretTargets(char* command) {
  char* horizontalAngleStr = strtok(command, ",");
  char* verticalAngleStr = strtok(NULL, ",");

  if (horizontalAngleStr && verticalAngleStr) {
    targetHorizontalAngle = map(atoi(horizontalAngleStr), -9, 9, 10, 180);  // 30-166 derece arası
    targetVerticalAngle = map(atoi(verticalAngleStr), -9, 9, 52, 108);      // 45-100 derece arası
  }
}

// Servo motorlarını güncelle
void updateServos() {
  myServo.write(targetHorizontalAngle);  // Hedef açıyı ayarla
  myServo2.write(targetVerticalAngle);   // Hedef açıyı ayarla
}

// Lazer ve ateşleme cihazlarını kontrol etmek için fonksiyon
void controlDevice(char* command, char deviceChar, int pin) {
  if (strchr(command, deviceChar) != NULL) {
    digitalWrite(pin, HIGH);  // Cihazı aç
  } else {
    digitalWrite(pin, LOW);  // Cihazı kapat
  }
}
