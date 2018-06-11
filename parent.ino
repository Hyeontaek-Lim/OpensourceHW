/*
 Name:    Implement.ino
 Created: 2018-05-23 오후 12:32:42
 Author:  Hyuntaek Lim
*/

#include <LiquidCrystal.h>  //LCD 라이브러리
#include <SoftwareSerial.h>  //시리얼통신 라이브러리
#include <SPI.h>
#include <MFRC522.h>
#include "Metro.h"
#define BT Serial1      // 메가에서 18,19핀을 Serial1로 시리얼통신이 가능하다.
#define RST_PIN   5                            // reset핀은 9번으로 설정
#define SS_PIN    53                           // SS핀은 10번으로 설정
Metro timer1(1000); // 1초마다 ACK 메시지를 보냄.
Metro timer2(50000); // 50초마다 미아상태임을 탐지
MFRC522 mfrc(SS_PIN, RST_PIN);                 // MFR522를 이용하기 위해 mfrc객체를 생성해 줍니다.

const int speaker = 10;   // 스피커 디지털 핀

LiquidCrystal lcd(2, 3, 4, 6, 7, 8);    //lcd 셋팅
byte BTShape_upper[8]{
  B000000,B001000,B001100, B001010, B001001, B101010, B011100, B001000
};
byte BTShape_under[8]{
  B001000, B011100, B101010, B001001, B001010, B001100, B001000, B000000 
};

const int RGB_Led[3] = { 13, 12, 11 };    //RGB 디지털 핀
int RGB[3] = {0, 0, 0};

bool state = false;

/* RGB 값을 설정해서 다양한 색상을 낼 수 있다. */
void SetColor(const int red, const int green, const int blue) {
  digitalWrite(RGB_Led[0], red);
  digitalWrite(RGB_Led[1], green);
  digitalWrite(RGB_Led[2], blue);
}
void RGBInit(){
  for(int i=0; i<3; i++)
    RGB[i] = 0;
}


// the setup function runs once when you press reset or power the board
void setup() {
  delay(3000);
  Serial1.begin(9600);    //블루투스와 아두이노의 통신 속도 설정
  Serial.begin(9600);     //시리얼 통신의 속도
  
  lcd.createChar(0, BTShape_upper);
  lcd.createChar(1, BTShape_under);
  lcd.begin(16, 2);   //lcd 초기화 작업
  lcd.clear();
  lcd.print("Hello world !");

  for (int i = 0; i < 3; i++)   //RGB LED
    pinMode(RGB_Led[i], OUTPUT);

  SPI.begin();                                // SPI 초기화
                        // (SPI : 하나의 마스터와 다수의 SLAVE(종속적인 역활)간의 통신 방식)
  pinMode(speaker, OUTPUT);
  mfrc.PCD_Init();

  RGB[1] = 255;

  Serial1.begin(9600);    //블루투스와 아두이노의 통신 속도 설정
  Serial.begin(9600);     //시리얼 통신의 속도
}
int cnt=0;
// the loop function runs over and over again until power down or reset
void loop() {
  SetColor(RGB[0],RGB[1],RGB[2]);
  delay(1000);
  state = 0;
  
  /*RFID 신호가 없다면..*/
  if (!mfrc.PICC_IsNewCardPresent() || !mfrc.PICC_ReadCardSerial()) {
    tone(speaker, 523, 1000);
    delay(500);                                // 0.5초 딜레이
    return;                                    // return
  }

  /* 해당 ID의 RFID ID가 들어온다면.. (추가적으로 서버에서 RFID 카드 정보 테이블을 만들면 여러카드 호환가능)*/
  if ( (mfrc.uid.uidByte[0] == 90 && mfrc.uid.uidByte[1] == 92
    && mfrc.uid.uidByte[2] == 74 && mfrc.uid.uidByte[3] == 115) 
    ) {    // 2번 태그 ID가 맞을경우
    //UID:37 102 215 45 (하얀색)
    

    Serial.println();
    Serial.println("RFID connection \n");    
    tone(speaker, 523, 1000);    //삐 소리
    delay(1000);
    char temp;
    BT.print("C"); // 시작한다는 사인
    while(!BT.available())
    {
      delay(500);
      lcd.home(); lcd.clear();
      lcd.print("WiFi Wait...");
    }
    BT.flush();
    timer2.reset();
    while (1) {
      RGBInit();
      RGB[1] = 255;
      SetColor(RGB[0],RGB[1],RGB[2]);
      
      /* 블루투스 모양 LCD에 찍기 */
       if (Serial1.available()) {
         lcd.clear();
         lcd.setCursor(0, 0);
         lcd.write(byte(0));
         lcd.setCursor(0, 1);
         lcd.write(byte(1));
         char c = BT.read();
         Serial.println(c); // 현재상태 시리얼 모니터에 출력
         /*만약 미아상태 메시지가 들어오면 
          BT 버퍼에 있는 것을 모두 비우고 미아상태를 만든 후, 상태 아두이노에게 응답메시지 m을 보낸다.*/
         if(c == 'M' || c=='m'){
            BT.flush();
            state = 1;
            BT.print("m"); 
         }
         if(c=='C'||c=='c')
            state = 0;
         cnt++;
       }
       
      if(timer1.check())
      {
        if(!state) // 미아상태가 아닐시 c를 보내준다.
          BT.print("c");
      }
      if(timer2.check())
      {
        if(cnt<10) // 데이터가 10번 이상 들어오지않으면 미아상태로 인식한다.
          state=1;
         cnt=0;
      }
      /* 블루투스 끊김. */
      if(state){
        RGBInit();
        RGB[0] = 255;
        SetColor(RGB[0],RGB[1],RGB[2]);
        delay(1000);
        lcd.clear();
        lcd.print("Don't connect!");
        break;
      }
    }
    
  }
}