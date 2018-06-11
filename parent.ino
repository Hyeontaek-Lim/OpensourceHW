/*
 Name:    Implement.ino
 Created: 2018-05-23 ���� 12:32:42
 Author:  Hyuntaek Lim
*/

#include <LiquidCrystal.h>  //LCD ���̺귯��
#include <SoftwareSerial.h>  //�ø������ ���̺귯��
#include <SPI.h>
#include <MFRC522.h>
#include "Metro.h"
#define BT Serial1      // �ް����� 18,19���� Serial1�� �ø�������� �����ϴ�.
#define RST_PIN   5                            // reset���� 9������ ����
#define SS_PIN    53                           // SS���� 10������ ����
Metro timer1(1000); // 1�ʸ��� ACK �޽����� ����.
Metro timer2(50000); // 50�ʸ��� �̾ƻ������� Ž��
MFRC522 mfrc(SS_PIN, RST_PIN);                 // MFR522�� �̿��ϱ� ���� mfrc��ü�� ������ �ݴϴ�.

const int speaker = 10;   // ����Ŀ ������ ��

LiquidCrystal lcd(2, 3, 4, 6, 7, 8);    //lcd ����
byte BTShape_upper[8]{
  B000000,B001000,B001100, B001010, B001001, B101010, B011100, B001000
};
byte BTShape_under[8]{
  B001000, B011100, B101010, B001001, B001010, B001100, B001000, B000000 
};

const int RGB_Led[3] = { 13, 12, 11 };    //RGB ������ ��
int RGB[3] = {0, 0, 0};

bool state = false;

/* RGB ���� �����ؼ� �پ��� ������ �� �� �ִ�. */
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
  Serial1.begin(9600);    //��������� �Ƶ��̳��� ��� �ӵ� ����
  Serial.begin(9600);     //�ø��� ����� �ӵ�
  
  lcd.createChar(0, BTShape_upper);
  lcd.createChar(1, BTShape_under);
  lcd.begin(16, 2);   //lcd �ʱ�ȭ �۾�
  lcd.clear();
  lcd.print("Hello world !");

  for (int i = 0; i < 3; i++)   //RGB LED
    pinMode(RGB_Led[i], OUTPUT);

  SPI.begin();                                // SPI �ʱ�ȭ
                        // (SPI : �ϳ��� �����Ϳ� �ټ��� SLAVE(�������� ��Ȱ)���� ��� ���)
  pinMode(speaker, OUTPUT);
  mfrc.PCD_Init();

  RGB[1] = 255;

  Serial1.begin(9600);    //��������� �Ƶ��̳��� ��� �ӵ� ����
  Serial.begin(9600);     //�ø��� ����� �ӵ�
}
int cnt=0;
// the loop function runs over and over again until power down or reset
void loop() {
  SetColor(RGB[0],RGB[1],RGB[2]);
  delay(1000);
  state = 0;
  
  /*RFID ��ȣ�� ���ٸ�..*/
  if (!mfrc.PICC_IsNewCardPresent() || !mfrc.PICC_ReadCardSerial()) {
    tone(speaker, 523, 1000);
    delay(500);                                // 0.5�� ������
    return;                                    // return
  }

  /* �ش� ID�� RFID ID�� ���´ٸ�.. (�߰������� �������� RFID ī�� ���� ���̺��� ����� ����ī�� ȣȯ����)*/
  if ( (mfrc.uid.uidByte[0] == 90 && mfrc.uid.uidByte[1] == 92
    && mfrc.uid.uidByte[2] == 74 && mfrc.uid.uidByte[3] == 115) 
    ) {    // 2�� �±� ID�� �������
    //UID:37 102 215 45 (�Ͼ��)
    

    Serial.println();
    Serial.println("RFID connection \n");    
    tone(speaker, 523, 1000);    //�� �Ҹ�
    delay(1000);
    char temp;
    BT.print("C"); // �����Ѵٴ� ����
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
      
      /* ������� ��� LCD�� ��� */
       if (Serial1.available()) {
         lcd.clear();
         lcd.setCursor(0, 0);
         lcd.write(byte(0));
         lcd.setCursor(0, 1);
         lcd.write(byte(1));
         char c = BT.read();
         Serial.println(c); // ������� �ø��� ����Ϳ� ���
         /*���� �̾ƻ��� �޽����� ������ 
          BT ���ۿ� �ִ� ���� ��� ���� �̾ƻ��¸� ���� ��, ���� �Ƶ��̳뿡�� ����޽��� m�� ������.*/
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
        if(!state) // �̾ƻ��°� �ƴҽ� c�� �����ش�.
          BT.print("c");
      }
      if(timer2.check())
      {
        if(cnt<10) // �����Ͱ� 10�� �̻� ������������ �̾ƻ��·� �ν��Ѵ�.
          state=1;
         cnt=0;
      }
      /* ������� ����. */
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