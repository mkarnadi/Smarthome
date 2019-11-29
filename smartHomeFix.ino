#include <WiFi.h>
#include <IOXhop_FirebaseESP32.h>
#include <ESP32Servo.h>

#define FIREBASE_HOST "https://tesfirebase-e187b.firebaseio.com/"
#define FIREBASE_AUTH "4vGwwulG8EsoZLHCeNHyMaDcrsPwFKakUUdDAXkC"
#define WIFI_SSID "Fir"
#define WIFI_PASSWORD "ghevan3186"

Servo myservo;

const int pinBuzzer = 23;
const int pinFlame = 4;
const int pinKipas1 = 27;
const int pinKipas2 = 26;
const int pinKipasEn = 14;
const int pinLDR = 2;
const int pinLed = 22;
const int pinServo = 18;
const int pinSuhu = 36;

const int freq = 30000;
const int pwmChannel = 0;
const int resolution = 8;

String kipasControl;
String ledControl;
String servoControl;
String buzzerStatus = "OFF";
String kipasStatus;
String ledStatus = "OFF";
String servoStatus;
String lastServoStatus;

int buzzer=0,led,ldr,flame,servo,dutyCycle;
float suhu;

void setup() {
  Serial.begin(115200);
  setupPin();
  setupServo();
  setupKipas();
  connectWifi();
  setupFirebase();
}

void loop() {
  flame = digitalRead(pinFlame);
  ldr = digitalRead(pinLDR);
  suhu = analogRead(pinSuhu)/9.31;
  ledControl = Firebase.getString("control_led");
  servoControl = Firebase.getString("control_servo");
  kipasControl = Firebase.getString("control_kipas");

  String strFlame = String(flame);
  String strLDR = String(ldr);
  String strSuhu = String(suhu);
  
  Firebase.setString("status_flame",strFlame);
  Firebase.setString("status_ldr",strLDR);
  Firebase.setString("status_suhu",strSuhu);

  if(flame == 0){
    servoStatus = Firebase.getString("status_servo");
    
    nyalakanBuzzer();
    
    if(servoStatus == "tutuup"){
      do{
        bukaPintu();
        lastServoStatus = "buka";
      }
      while(servoStatus == lastServoStatus);
      servoStatus = "buka";
      Firebase.setString("status_servo",servoStatus);
    }
    Firebase.setString("status_buzzer","ON");
  }
  else{
    matikanBuzzer();
    Firebase.setString("status_buzzer","OFF");
  }

  
  if(ledControl == "manual"){
    ledStatus = Firebase.getString("status_led");
    if(ledStatus == "ON"){
      nyalakanLed();
    }
    else if(ledStatus == "OFF"){
      matikanLed();
    }
  }
  else if(ledControl == "auto"){
    if(ldr == 1){
      nyalakanLed();
      ledStatus = "ON";
      Firebase.setString("status_led",ledStatus);
    }
    else if(ldr == 0){
      matikanLed();
      ledStatus = "OFF";
      Firebase.setString("status_led",ledStatus);
    }
  }
  
  if(servoControl == "manual"){
    servoStatus = Firebase.getString("status_servo");
    if(servoStatus == "buka"){
      if(servoStatus != lastServoStatus){
        bukaPintu();
      }
      lastServoStatus = servoStatus;
    }
    else if(servoStatus == "tutup"){
      if(servoStatus != lastServoStatus){
        tutupPintu();
      }
      lastServoStatus = servoStatus;
    }
  }

  if(kipasControl == "manual"){
    kipasStatus = Firebase.getString("status_kipas");
    if(kipasStatus == "LOW"){
      dutyCycle = 200;
      digitalWrite(pinKipas1, HIGH);
      digitalWrite(pinKipas2, LOW);
      ledcWrite(pwmChannel,dutyCycle);
    }
    else if(kipasStatus == "HIGH"){
      dutyCycle = 250;
      digitalWrite(pinKipas1, HIGH);
      digitalWrite(pinKipas2, LOW);
      ledcWrite(pwmChannel,dutyCycle);
    }
    else if(kipasStatus == "OFF"){
      digitalWrite(pinKipas1, LOW);
      digitalWrite(pinKipas2, LOW);
    }
  }
  
  else if(kipasControl == "auto"){
    if(suhu >= 16 && suhu <= 20){
      dutyCycle = 200;
      digitalWrite(pinKipas1, HIGH);
      digitalWrite(pinKipas2, LOW);
      ledcWrite(pwmChannel,dutyCycle);
      kipasStatus = "LOW";
      Firebase.setString("status_kipas",kipasStatus);
    }
    else if(suhu > 20 && suhu <= 25){
      dutyCycle= 250;
      digitalWrite(pinKipas1, HIGH);
      digitalWrite(pinKipas2, LOW);
      ledcWrite(pwmChannel,dutyCycle);
      kipasStatus = "HIGH";
      Firebase.setString("status_kipas",kipasStatus);
    }
    else{
      digitalWrite(pinKipas1, LOW);
      digitalWrite(pinKipas2, LOW);
      kipasStatus = "OFF";
      Firebase.setString("status_kipas",kipasStatus);
    }
  }
  
  lihatData();
  //delay(1000);
}

void setupPin(){
  pinMode(pinFlame,INPUT);
  pinMode(pinLDR,INPUT);
  pinMode(pinSuhu,INPUT);
  pinMode(pinBuzzer,OUTPUT);
  pinMode(pinKipas1,OUTPUT);
  pinMode(pinKipas2,OUTPUT);
  pinMode(pinKipasEn,OUTPUT);
  pinMode(pinLed,OUTPUT);
  pinMode(pinServo,OUTPUT);
  digitalWrite(pinBuzzer,buzzer);
}

void setupServo(){
  myservo.setPeriodHertz(50);    
  myservo.attach(pinServo, 1000, 2000);
}

void setupKipas(){
  ledcSetup(pwmChannel, freq, resolution);
  ledcAttachPin(pinKipasEn, pwmChannel);
}

void connectWifi(){
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to ");
  Serial.print(WIFI_SSID);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.println();
  Serial.print("Connected to ");
  Serial.println(WIFI_SSID);
  Serial.print("IP Address is : ");
  Serial.println(WiFi.localIP());
}

void setupFirebase(){
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.setString("status_buzzer",buzzerStatus);
  Firebase.setString("status_led",ledStatus);
  Firebase.setString("status_servo",servoStatus);
  Firebase.setString("status_kipas",kipasStatus);
}

void bukaPintu(){
  for (servo = 0; servo <= 180; servo += 1) {
    myservo.write(servo);
    delay(15);
  }
}

void tutupPintu(){
  for (servo = 180; servo >= 0; servo -= 1) {
    myservo.write(servo);
    delay(15);
  }
}

void nyalakanBuzzer(){
  buzzer = 1;
  digitalWrite(pinBuzzer,buzzer);
}

void matikanBuzzer(){
  buzzer = 0;
  digitalWrite(pinBuzzer,buzzer);
}

void nyalakanLed(){
  led = 1;
  digitalWrite(pinLed,led);
}

void matikanLed(){
  led = 0;
  digitalWrite(pinLed,led);
}

void lihatData(){
  Serial.print("Suhu: ");
  Serial.print(suhu);
  Serial.print(" Kipas: ");
  Serial.print(kipasStatus);
  Serial.print(" LED: ");
  Serial.print(led);
  Serial.print(" LDR: ");
  Serial.print(ldr);
  Serial.print(" Flame: ");
  Serial.print(flame);
  Serial.print(" Buzzer: ");
  Serial.print(buzzer);
  Serial.print(" Pintu: ");
  Serial.println(servoStatus);
}
