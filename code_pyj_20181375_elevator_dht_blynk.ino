//엘리베이터 + 온습도센서 값을 이용한 화재경보기&자동제습기 + blynk 화면에 온습도 실시간 값 출력
#define BLYNK_TEMPLATE_ID "TMPLsHu_UpMF"                       
#define BLYNK_DEVICE_NAME "test11"
#define BLYNK_AUTH_TOKEN "D52VGatkHkWHoYH5pQvBFtSDJamshUje"             //blynk 사용을 위한 권한 인증
// Comment this out to disable prints and save space
#define BLYNK_PRINT Serial

#include <SPI.h>
#include <WiFiNINA.h>           //와이파이 라이브러리 
#include <BlynkSimpleWiFiNINA.h>
#include <Servo.h>             //서브모터 라이브러리 

#include <Arduino.h>        //OLED를 활용하기위한 라이브러리
#include <U8x8lib.h>

#include <DHT.h>   //온습도 센서를 활용하기위한 라이브러리
#define DHT_TYPE DHT22
#define DHT_PIN 2
DHT dht(DHT_PIN, DHT_TYPE);   //온습도 센서 핀 설정

int servoPin = 3;
Servo myServo;
Servo myservo2;
int BUZZER = 8;
BlynkTimer timer;

//int floor;
char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "iptime123";
char pass[] = "wkwk12!!";


const int trigPin = 10;    //Trig
const int echoPin = 11;    //Echo
unsigned long roundtrip_time_us;
float dist_cm;

U8X8_SSD1306_128X64_NONAME_SW_I2C u8x8(/* clock=*/ SCL, /* data=*/ SDA, /* reset=*/ U8X8_PIN_NONE);   // OLED 출력핀 설정  

unsigned long Tms = 2000;
unsigned long prev_time = 0;
float h;                //습도 변수
float t;                //온도 변수

BLYNK_WRITE(V0)
{
  // 가상 출력핀 v0 설정
  int servoAng = param.asInt(); //서브모터 각도를 blynk로 부터 받아옴
  myServo.write(servoAng);
}
BLYNK_CONNECTED()
{
  Blynk.syncVirtual(V0);  // will cause BLYNK_WRITE(V0) to be executed
}

void sendSensor()
{
  float h = dht.readHumidity();        //습도값 저장
  float t = dht.readTemperature();     //온도값 저장
 
  if (isnan(h) || isnan(t)) 
  {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
  // You can send any value at any time.
  // Please don't send more that 10 values per second.
  Blynk.virtualWrite(V13, t);           //가상핀 v13에 온도값 출력
  Blynk.virtualWrite(V14, h);          //가상핀 v14에 습도값 출력
}

void setup() 
{
  // Debug console
  Serial.begin(115200);
  pinMode(4, INPUT);
  pinMode(5, OUTPUT);
  pinMode(6, INPUT);
  pinMode(7, OUTPUT);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT_PULLUP);
  u8x8.begin();                             //OLED 출력 Setup
  u8x8.setPowerSave(0);             
  myServo.attach(servoPin);
  myservo2.attach(9);
  pinMode(BUZZER, OUTPUT);
  dht.begin();                         //온습도 센서 setup
  Blynk.begin(auth, ssid, pass);
  timer.setInterval(1000L, sendSensor);
}

void loop()
{
  Blynk.run();
  timer.run();
  int floor;
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);      

  //Trigh outputs 10us pulse Low -> High(10us) -> Low
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);              
  digitalWrite(trigPin, LOW);         


  roundtrip_time_us = pulseIn(echoPin, HIGH); //measure time until High->Low
  dist_cm = 0.017 * roundtrip_time_us;
  Serial.println((String)"Distance: " + dist_cm + " cm");
  delay(500);

    if (millis() - prev_time > Tms) {                   //실시간 온습도값 채킹
    prev_time = millis();
    h = dht.readHumidity();
    t = dht.readTemperature();
    if (isnan(h) || isnan(t)) {
      Serial.println("Failed to read from DHT sensor!");
      return;
    } else {
      Serial.print((String)"Humidity: " + h + " %\t");  
      Serial.println((String)"Temperature: " + t + "C");  
    }
  }

  if(t>50){                        //온도가 50도 초과인 경우 화재로 판단-> 버저 울림
    tone(BUZZER, 440);
  }
  else{                             //50도 미만인 경우 버저 끔
    noTone(BUZZER); 
  }
  if(h>70){                         //습도가 70도인 경우 제습기 on
    myServo.write(180);
  }
  
   myservo2.write(90);              //360도 서브모터 90-> 작동x
   
  if(digitalRead(4)==HIGH && 37 > dist_cm){        // 1층 호출
    down();     //oled에 down을 출력하는  함수 호출
    if(dist_cm <= 40) myservo2.write(-180);         
    delay(500);
    floor=1;
  }
  else if(digitalRead(5)==HIGH && 28 > dist_cm){   //2층 호출-> 엘리베이터 위치가 3,4층인 경우
    down();         //oled에 down을 출력하는  함수 호출
    myservo2.write(-180);
    delay(500);
    floor=2;
  }

  else if(digitalRead(5)==HIGH && dist_cm >= 38){  //2층 호출 -> 엘리베이터 위치가 1층인 경우
    up();   //oled에 up을 출력하는  함수 호출
    myservo2.write(180);
    delay(500);
    floor=2;
  }

  
  else if(digitalRead(6)==HIGH && dist_cm < 19){    //3층 호출-> 엘리베이터 위치가 4층인 경우
    down();     //oled에 down을 출력하는  함수 호출
    myservo2.write(-180);
    delay(500);
    floor=3;
  }


  else if(digitalRead(6)==HIGH && dist_cm >= 28 ){  //3층 호출-> 엘리베이터 위치가 1,2 층인 경우
    up();      //oled에 up을 출력하는  함수 호출
    myservo2.write(180);
    delay(500);
    floor=3;
  }
  
  else if(digitalRead(7)==HIGH && 11  < dist_cm){      //4층 호출
    up();       //oled에 up을 출력하는  함수 호출
    myservo2.write(180);
    delay(500);
    floor=4;
  }
  u8x8.clear();          //oled화면 클리어
  floor_print(floor);      //oled에 현재층수 출력
}


void down(){             //oled에 down을 출력
   u8x8.clear();                                                  //OLED 화면 clear
  u8x8.setFont(u8x8_font_chroma48medium8_r);                       //OLED로 출력되는 폰트 설정
  u8x8.drawString(0,0,"down");                                       //OLED에 (0,0)위치에 "down" 문구 출력
}


void up(){           //oled에 down을 출력
   u8x8.clear();                                                  //OLED 화면 clear
  u8x8.setFont(u8x8_font_chroma48medium8_r);                       //OLED로 출력되는 폰트 설정
  u8x8.drawString(0,0,"up");                                       //OLED에 (0,0)위치에 "up" 문구 출력
}

void floor_print(int floor){              //oled에 현재층수를 출력
  u8x8.clear(); 
  u8x8.setFont(u8x8_font_chroma48medium8_r);                       //OLED로 출력되는 폰트 설정
  if(floor==1){
  u8x8.drawString(0,0,"1F");                                       //OLED에 (0,0)위치에 "1F" 문구 출력
  }
  else  if(floor==2){
  u8x8.drawString(0,0,"2F");                                       //OLED에 (0,0)위치에 "2F" 문구 출력
  }
  else  if(floor==3){
  u8x8.drawString(0,0,"3F");                                       //OLED에 (0,0)위치에 "3F" 문구 출력
  }
  else{
  u8x8.drawString(0,0,"4F");                                       //OLED에 (0,0)위치에 "4F" 문구 출력
  }
}
