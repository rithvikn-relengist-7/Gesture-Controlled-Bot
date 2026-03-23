#include <Arduino.h>
#include<WiFi.h>
#include<WiFiUdp.h>

#define AIN1 26
#define AIN2 25
#define PWMA 33  

#define BIN1 14
#define BIN2 12
#define PWMB 13  

#define STDBY 27

#define PWM_CHANNEL_A 0
#define PWM_CHANNEL_B 1

void runMotor(int motor, int spd, int dir);
void left(int spd);
void right(int spd);
void forward(int spd);
void back(int spd);
void stop();

const char* ssid = "NAME_OF_AP";
const char* password = "PASSWORD_AP";

IPAddress local_IP(192, 168, 4, 1);
IPAddress gateway(192, 168, 4, 1);
IPAddress subnet(255, 255, 255, 0);

WiFiUDP udp;
const int udpPort = 4210;
char incomingPacket[255];

unsigned long previousMillis = 0;
const long interval = 500; 
int packetSize = 0;


void setup() {
  Serial.begin(9600);
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(local_IP, gateway, subnet);
  WiFi.softAP(ssid, password);
  Serial.println(WiFi.softAPIP());  

  udp.begin(udpPort);
  Serial.println("UDP server started");

  pinMode(AIN1, OUTPUT);
  pinMode(AIN2, OUTPUT);
  pinMode(PWMA, OUTPUT);
  pinMode(STDBY, OUTPUT);
  pinMode(BIN1, OUTPUT);
  pinMode(BIN2, OUTPUT);
  pinMode(PWMB, OUTPUT);

  ledcSetup(PWM_CHANNEL_A, 1000, 8);
  ledcAttachPin(PWMA, PWM_CHANNEL_A);

  ledcSetup(PWM_CHANNEL_B, 1000, 8); 
  ledcAttachPin(PWMB, PWM_CHANNEL_B);
}

void loop() {
  int packetSize = udp.parsePacket();
  if (packetSize) {
      int len = udp.read(incomingPacket, 255);
      if (len > 0) {
          incomingPacket[len] = '\0';
      }
      int fingers_extended = atoi(incomingPacket);
      Serial.print("Received fingers: ");
      Serial.println(fingers_extended);
      if (fingers_extended == 0) {
        stop();
        Serial.println("Stop");
      } 
      else if (fingers_extended == 1) {
        forward(255);
        Serial.println("Forward");
      } 
      else if (fingers_extended == 2) {
        back(255);
        Serial.println("Back");
      } 
      else if (fingers_extended == 3) {
        left(255);
        Serial.println("Left");
      } 
      else if (fingers_extended == 4) {
        right(255);
        Serial.println("Right");
      } 
      else {
        stop();
        Serial.println("Stop");
      } 
      delay(100);
  }
  else{
    stop();
    Serial.println("Data not recieved");
    delay(100);
  }
}

void runMotor(int motor, int spd, int dir) {
  digitalWrite(STDBY, HIGH); 

  boolean dirPin1 = LOW;
  boolean dirPin2 = HIGH;

  if(dir == 1) {
    dirPin1 = HIGH;
    dirPin2 = LOW;
  }

  if(motor == 0) {  // Motor A
    digitalWrite(AIN1, dirPin1);
    digitalWrite(AIN2, dirPin2);
    ledcWrite(PWM_CHANNEL_A, spd);
  } 
  else {          // Motor B
    digitalWrite(BIN1, dirPin1);
    digitalWrite(BIN2, dirPin2);
    ledcWrite(PWM_CHANNEL_B, spd);
  }
}

void left(int spd) {
  runMotor(0, spd, 0);
  runMotor(1, spd, 1);
}

void right(int spd) {
  runMotor(0, spd, 1);
  runMotor(1, spd, 0);
}

void forward(int spd) {
  runMotor(0, spd, 0);
  runMotor(1, spd, 0);
}

void back(int spd) {
  runMotor(0, spd, 1);
  runMotor(1, spd, 1);
}

void stop() {
  digitalWrite(STDBY, LOW);
}