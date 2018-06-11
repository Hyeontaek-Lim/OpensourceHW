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

#define BT Serial1
#define RST_PIN   5                            // reset핀은 9번으로 설정
#define SS_PIN    53                           // SS핀은 10번으로 설정

MFRC522 mfrc(SS_PIN, RST_PIN);                 // MFR522를 이용하기 위해 mfrc객체를 생성해 줍니다.

int cnt = 0;      //비콘 메시지 카운트

const int speaker = 10;   // 스피커 디지털 핀

Metro timer1(500);
Metro timer2(10000);
Metro mia(3000);

LiquidCrystal lcd(2, 3, 4, 6, 7, 8);    //lcd 셋팅

byte BTShape_upper[8]{
	B000000,B001000,B001100, B001010, B001001, B101010, B011100, B001000
};
byte BTShape_under[8]{
	B001000, B011100, B101010, B001001, B001010, B001100, B001000, B000000
};

const int RGB_Led[3] = { 13, 12, 11 };    //RGB 디지털 핀
int RGB[3] = { 0, 0, 0 };

bool state = false;

/* RGB 값을 설정해서 다양한 색상을 낼 수 있다. */
void SetColor(const int red, const int green, const int blue) {
	digitalWrite(RGB_Led[0], red);
	digitalWrite(RGB_Led[1], green);
	digitalWrite(RGB_Led[2], blue);
}
void RGBInit() {
	for (int i = 0; i<3; i++)
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

// the loop function runs over and over again until power down or reset
void loop() {
	SetColor(RGB[0], RGB[1], RGB[2]);
	delay(1000);

	if (!mfrc.PICC_IsNewCardPresent() || !mfrc.PICC_ReadCardSerial()) {
		delay(500);                                // 0.5초 딜레이 
		return;                                    // return
	}

	if ((mfrc.uid.uidByte[0] == 90 && mfrc.uid.uidByte[1] == 92
		&& mfrc.uid.uidByte[2] == 74 && mfrc.uid.uidByte[3] == 115)
		) {    // 2번 태그 ID가 맞을경우
			   //UID:37 102 215 45 (하얀색)

		tone(speaker, 523, 1000);    //삐 소리
		delay(1000);

		Serial.println();
		Serial.println("RFID connection \n");

		while (1) {
			while (Serial.available())
				Serial1.write(Serial.read());

			/* 블루투스 모양 LCD에 찍기 */
			while (Serial1.available()) {
				char c = Serial1.read();
				if (c == 'c' || c == 'C') {
					cnt++;
					state = false;

					RGBInit();
					RGB[2] = 255;
					SetColor(RGB[0], RGB[1], RGB[2]);

					lcd.clear();
					lcd.setCursor(0, 0);
					lcd.write(byte(0));
					lcd.setCursor(0, 1);
					lcd.write(byte(1));
					Serial.println("Connect");
				}
				else if (c == 'w' || c == 'W') {
					lcd.clear();
					lcd.print("WIFI Wait....");
					while (!BT.available()) {
						RGBInit();
						RGB[1] = 255;
						SetColor(RGB[0], RGB[1], RGB[2]);
						delay(1000);

						lcd.clear();
						lcd.print("Don't connect!");
					}
				}
			}
			if (timer1.check())
				Serial1.write("C");

			if (timer2.check()) {
				if (cnt == 0)
					state = true;

				cnt = 0;
			}

			/* 블루투스 끊김. */
			if (state) {
				RGBInit();
				RGB[0] = 255;
				SetColor(RGB[0], RGB[1], RGB[2]);
				delay(1000);

				lcd.clear();
				lcd.print("Don't connect!");
				// 계속 미아상태임을 알려줌
				// rfid가 찍히면 미아상태해제 break; 

				tone(speaker, 523, 1000);    //삐 소리
				delay(3000);

				break;
			}
			/*RFID를 찍어도 연결되기 전까지 계속 울린다.*/
			tone(speaker, 523, 1000);    //삐 소리
		}

	}
}