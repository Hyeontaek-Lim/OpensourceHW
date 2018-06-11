#include <SPI.h>
#include <MFRC522.h>

#define RST_PIN   5                            // reset핀은 9번으로 설정
#define SS_PIN    53                           // SS핀은 10번으로 설정
                                               // SS핀은 데이터를 주고받는 역할의 핀( SS = Slave Selector )
int speaker = 8;
MFRC522 mfrc(SS_PIN, RST_PIN);                 // MFR522를 이용하기 위해 mfrc객체를 생성해 줍니다.

void setup(){
  Serial.begin(9600);                         // 시리얼 통신, 속도는 9600
  SPI.begin();                                // SPI 초기화
                                              // (SPI : 하나의 마스터와 다수의 SLAVE(종속적인 역활)간의 통신 방식)
  pinMode(speaker, OUTPUT);
  mfrc.PCD_Init();                               
}

void loop(){
  if ( !mfrc.PICC_IsNewCardPresent() || !mfrc.PICC_ReadCardSerial() ) {   
    delay(500);                                // 0.5초 딜레이 
    return;                                    // return
  } 
    
  if(mfrc.uid.uidByte[0] == 90 && mfrc.uid.uidByte[1] == 92 
       && mfrc.uid.uidByte[2] == 74 && mfrc.uid.uidByte[3] == 115) {    // 2번 태그 ID가 맞을경우
        
    Serial.println("Hello, Eduino~");        // 시리얼 모니터에 "Hello, Eduino~" 출력
    tone(speaker,523,100);                 
    delay(500);
    
  }
}
