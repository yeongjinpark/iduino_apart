//휴대폰 push알람 + 거리 센서를 이용한 자동 조명& 자동 창문
#include <SPI.h>
#include <WiFiNINA.h>
#include "utility/wifi_drv.h"
#include <Servo.h>
int servoPin = 3;
Servo myServo; 

#define DEVICEID      "vBD9DDF2FDA28FD6"   //휴대폰 PUSH 알람에 필요한 푸시불렛 DEVICE 코드

 

char ssid[] = "iptime123";                //SSID
char pass[] = "wkwk12!!";                 //PASSWARD

const char* server = "api.pushingbox.com";   //휴대폰 PUSH알람에 필요한 푸시불렛 링크

const int trigPin = 4;    //Trig 
const int echoPin = 5;    //Echo
unsigned long roundtrip_time_us;  //초음파 거리센서값 반환을 위한 time값
float dist_cm;                   //초음파 거리센서값을 저장하는 변수 선언
const int LED=6;                //led핀 번호지정
int status = WL_IDLE_STATUS;     //wifi연결상태 체크를 위한 변수 선언


 

void setup() {

  Serial.begin(9600);                   
  myServo.attach(servoPin);               //서브모터 핀 지정
  pinMode(trigPin, OUTPUT);               //초음파거리 센서 tiug 출력핀 지정
  pinMode(echoPin, INPUT_PULLUP);          //초음파거리 센서 echo 출력핀 지정
  pinMode(LED, OUTPUT);                 //led 출력핀 지정

// 와이파이 연결시도
  int status = WL_IDLE_STATUS;     //
  WiFiClient client;
  while (status != WL_CONNECTED) {        // 와이파이 미연결시
    Serial.print("Attempting to connect to WPA SSID: ");  //시리얼 문구 출력
    Serial.println(ssid);                     
    // Connect to WPA/WPA2 network:
    status = WiFi.begin(ssid, pass);                //재연결
    // wait 10 seconds for connection:
    delay(10000);
  }
  // 와이파이 연결 성공 시 출력 문구
  Serial.println("You're connected to the network");
}

 

void loop()

{
  int val=analogRead(A0);    //조도 센서값을 받아오는 변수 선언
  
  digitalWrite(trigPin, LOW);   //
  delayMicroseconds(2);      

  //Trigh outputs 10us pulse Low -> High(10us) -> Low
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);              
  digitalWrite(trigPin, LOW);

  //measure time (us(=microsecond)) from Echo pin
  roundtrip_time_us = pulseIn(echoPin, HIGH); // echopin이 high가 된 시간을 저장하는 변수 
  dist_cm = 0.017 * roundtrip_time_us; //0.017 * trig핀에서 echo핀으로 초음바가 돌아오는 시간= 물체와의 거리

  Serial.println((String)"Distance: " + dist_cm + " cm");   //거리 출력
  delay(100); 
  Serial.println((String)"sensor out: " + val);   //조도 센서값 출력
  delay(500);

  if(dist_cm<8){                    //거리가 8cm 미만인 경우
    digitalWrite(LED,LOW);         //LED조명 ON
    if(val<100){                   //조도센서값 100 이하인 경우
      wifiMode();                  //PUSH알람 함수 호출
    }
  }  
  else{
    digitalWrite(LED,HIGH);       //LED OFF
  }
   
   if(dist_cm <5){              //거리가 5CM 미만인 경우
    myServo.write(0);           //서브모터 0도-> 창문 닫음
    delay(100);
  }
  else{
    myServo.write(180);          //서브모터180도-> 창문을 연다
  } 
  
}

 

 

void wifiMode(){

  WiFiClient client;  //client 설정
  if (client.connect(server, 80)) {  //client 서버 연결
    //HTTP PUT 요청을 보냄
    client.print(F("GET /pushingbox?devid="));
    client.print(DEVICEID);    
    client.print(F(" HTTP/1.1\r\n"));
    client.print(F("Host: api.pushingbox.com\r\n"));
    client.print(F("User-Agent: Arduino\r\n"));
    client.print(F("\r\n\r\n"));
  }

  else {
    // 서버 연결 실패 시 문구 출력
    Serial.println("Connection failed");

  }

}
