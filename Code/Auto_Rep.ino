#include <WiFi.h>
#include <ESP32Servo.h>

//========== WiFi ==========
const char* ssid = "Auto Rep";
const char* password = "12345678";

WiFiServer server(80);

//========== موتورها ==========
// جلو چپ
#define RPWM1 27
#define LPWM1 14

// عقب چپ
#define RPWM2 32
#define LPWM2 33

// عقب راست
#define RPWM3 18
#define LPWM3 19

// جلو راست
#define RPWM4 4
#define LPWM4 2

#define SPEED 255


Servo jackServo;

#define SERVO_PIN 13  
//===========================

void stopMotors() {
  analogWrite(RPWM1, 0);
  analogWrite(LPWM1, 0);

  analogWrite(RPWM2, 0);
  analogWrite(LPWM2, 0);

  analogWrite(RPWM3, 0);
  analogWrite(LPWM3, 0);

  analogWrite(RPWM4, 0);
  analogWrite(LPWM4, 0);
}

void forward() {
  analogWrite(RPWM1, SPEED);
  analogWrite(LPWM1, 0);  // جلو چپ

  analogWrite(RPWM2, SPEED);
  analogWrite(LPWM2, 0);  // عقب چپ

  analogWrite(RPWM3, SPEED);
  analogWrite(LPWM3, 0);  // عقب راست

  analogWrite(RPWM4, SPEED);
  analogWrite(LPWM4, 0);  // جلو راست
}

// عقب

void backward() {

  analogWrite(RPWM1, 0);
  analogWrite(LPWM1, SPEED);

  analogWrite(RPWM2, 0);
  analogWrite(LPWM2, SPEED);

  analogWrite(RPWM3, 0);
  analogWrite(LPWM3, SPEED);

  analogWrite(RPWM4, 0);
  analogWrite(LPWM4, SPEED);
}

// حرکت به چپ (Strafe Left)

void left() {

  // جلو چپ = عقب

  analogWrite(RPWM1, 0);
  analogWrite(LPWM1, SPEED);

  // عقب چپ = جلو

  analogWrite(RPWM2, SPEED);
  analogWrite(LPWM2, 0);

  // عقب راست = عقب

  analogWrite(RPWM3, 0);
  analogWrite(LPWM3, SPEED);

  // جلو راست = جلو

  analogWrite(RPWM4, SPEED);
  analogWrite(LPWM4, 0);
}

// حرکت به راست (Strafe Right)

void right() {

  // جلو چپ = جلو

  analogWrite(RPWM1, SPEED);
  analogWrite(LPWM1, 0);

  // عقب چپ = عقب

  analogWrite(RPWM2, 0);
  analogWrite(LPWM2, SPEED);

  // عقب راست = جلو

  analogWrite(RPWM3, SPEED);
  analogWrite(LPWM3, 0);

  // جلو راست = عقب

  analogWrite(RPWM4, 0);
  analogWrite(LPWM4, SPEED);
}

// چرخش ساعتگرد

void turnRight() {

  analogWrite(RPWM1, SPEED);
  analogWrite(LPWM1, 0);

  analogWrite(RPWM2, SPEED);
  analogWrite(LPWM2, 0);

  analogWrite(RPWM3, 0);
  analogWrite(LPWM3, SPEED);

  analogWrite(RPWM4, 0);
  analogWrite(LPWM4, SPEED);
}

// چرخش پادساعتگرد

void turnLeft() {

  analogWrite(RPWM1, 0);
  analogWrite(LPWM1, SPEED);

  analogWrite(RPWM2, 0);
  analogWrite(LPWM2, SPEED);

  analogWrite(RPWM3, SPEED);
  analogWrite(LPWM3, 0);

  analogWrite(RPWM4, SPEED);
  analogWrite(LPWM4, 0);
}

// مورب جلو چپ

void upLeft() {

  stopMotors();

  analogWrite(RPWM2, SPEED);
  analogWrite(LPWM2, 0);  // عقب چپ

  analogWrite(RPWM4, SPEED);
  analogWrite(LPWM4, 0);  // جلو راست
}

// مورب جلو راست

void upRight() {

  stopMotors();

  analogWrite(RPWM1, SPEED);
  analogWrite(LPWM1, 0);  // جلو چپ

  analogWrite(RPWM3, SPEED);
  analogWrite(LPWM3, 0);  // عقب راست
}

// مورب عقب چپ

void downLeft() {

  stopMotors();

  analogWrite(RPWM1, 0);
  analogWrite(LPWM1, SPEED);  // جلو چپ

  analogWrite(RPWM3, 0);
  analogWrite(LPWM3, SPEED);  // عقب راست
}

// مورب عقب راست

void downRight() {

  stopMotors();

  analogWrite(RPWM2, 0);
  analogWrite(LPWM2, SPEED);  // عقب چپ

  analogWrite(RPWM4, 0);
  analogWrite(LPWM4, SPEED);  // جلو راست
}

void setup() {

  Serial.begin(115200);

  pinMode(RPWM1, OUTPUT);
  pinMode(LPWM1, OUTPUT);
  pinMode(RPWM2, OUTPUT);
  pinMode(LPWM2, OUTPUT);
  pinMode(RPWM3, OUTPUT);
  pinMode(LPWM3, OUTPUT);
  pinMode(RPWM4, OUTPUT);
  pinMode(LPWM4, OUTPUT);

  jackServo.attach(SERVO_PIN);
  jackServo.write(0);  

  stopMotors();

  WiFi.softAP(ssid, password);

  Serial.print("IP : ");
  Serial.println(WiFi.softAPIP());

  server.begin();
}

void loop() {

  WiFiClient client = server.available();

  if (!client) return;

  String req = client.readStringUntil('\r');
  Serial.println(req);

  if (req.indexOf("/F") >= 0) forward();
  else if (req.indexOf("/B") >= 0) backward();
  else if (req.indexOf("/L") >= 0) left();
  else if (req.indexOf("/R") >= 0) right();
  else if (req.indexOf("/TR") >= 0) turnRight();
  else if (req.indexOf("/TL") >= 0) turnLeft();
  else if (req.indexOf("/UL") >= 0) upLeft();
  else if (req.indexOf("/UR") >= 0) upRight();
  else if (req.indexOf("/DL") >= 0) downLeft();
  else if (req.indexOf("/DR") >= 0) downRight();
  else if (req.indexOf("/S") >= 0) stopMotors();
  else if (req.indexOf("/U") >= 0) jackServo.write(140);
  else if (req.indexOf("/D") >= 0) jackServo.write(0);
  else {
    stopMotors();
  }


  client.stop();
}