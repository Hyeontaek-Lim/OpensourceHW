

#include <LiquidCrystal.h>  //LCD 라이브러리
#include "Metro.h"
#define esp8266 Serial2
#define BT Serial1
#define DEBUG true

const int speaker = 10;   // 스피커 디지털 핀
String IPaddress; // 현재 아두이노 IP
String phoneNumber; // 보호자 연락처
String ssid = "hongbin"; // 와이파이 ID
String pass = "tlghk123"; // 와이파이 pass
String destIP = "192.168.1.4"; // 웹의 IP주소
String destPort = "3000"; // 포트번호 Rails로 고정
String st = "";
bool state = false; // 미아상태 표시
bool connet = false;
int count = 0;
int exitcnt=0;
Metro sendtimer(1000);
Metro checktimer(50000); 
bool flag = false;
LiquidCrystal lcd(4, 5, 6, 7, 8, 9); //lcd 셋팅
byte BTShape_upper[8] {
  B000000, B001000, B001100, B001010, B001001, B101010, B011100, B001000
};
byte BTShape_under[8] {
  B001000, B011100, B101010, B001001, B001010, B001100, B001000, B000000
};

const int RGB_Led[3] = { 13, 12, 11 };    //RGB 디지털 핀
int RGB[3] = {0, 0, 0};


/* RGB 값을 설정해서 다양한 색상을 낼 수 있다. */
void SetColor(const int red, const int green, const int blue) {
  digitalWrite(RGB_Led[0], red);
  digitalWrite(RGB_Led[1], green);
  digitalWrite(RGB_Led[2], blue);
}
void RGBInit() {
  for (int i = 0; i < 3; i++)
    RGB[i] = 0;
}
String sendData(String command, const int timeout, boolean debug)
{
  String response = "";
  esp8266.print(command); // send the read character to the esp8266
  long int time = millis();
  while ( (time + timeout) > millis())
  {
    while (esp8266.available())
    {
      char c = esp8266.read(); // read the next character.
      response += c;
    }
  }
  if (debug)
    Serial.print(response);
  return response;
}
void sendQuery(String str)
{
  String t;
  sendData("AT+RST\r\n", 1000, DEBUG); // 초기화
  sendData("AT+CIPMUX=0\r\n", 2000, DEBUG);
  do
    t = sendData("AT+CIPSTART=\"TCP\",\"" + destIP + "\"," + destPort + "\r\n", 3000, DEBUG);
  while (t.indexOf("Linked") == -1); // 링크 연결될때까지
  sendData("AT+CIPSEND=" + String(str.length()) + "\r\n", 2000, DEBUG);
  sendData(str, 2000, DEBUG);
  sendData("AT+CIPCLOSE\r\n", 1000, DEBUG); // turn on server on port 80
}

String getInit() 
{
  /* WIFI모듈을 init한다.
   * 1. SSID와 password로 와이파이 접속
   * 2. CIFSR로 IP주소 반환
   * 3. CWMODE=3으로 client, AP 모드 사용
   * 4. CIFSR로 얻은 현재모듈의 IP주소 리턴
  */
  String temp = "";
  String res = "";

  sendData("AT+CWJAP=\"" + ssid + "\",\"" + pass + "\"\r\n", 7000, DEBUG); // 접속하기
  temp = sendData("AT+CIFSR\r\n", 1500, DEBUG);
  sendData("AT+CWMODE=3\r\n", 1000, DEBUG);
  int size = temp.length();
  int i, cnt = 0, k = 0;
  for (i = 0 ; i < size; i++)
  {
    if (temp[i] == '\n')
    {
      cnt++;
      if (cnt == 2)
        break;
    }
  }
  i++;
  for (k = i; k < size; k++)
    if (temp[k] == '\n')
      break;
  for (int j = i; j < k - 1; j++)
  {
    if (temp[j] == '.')
      res += '_';
    else
      res += temp[j];
  }
  return res;
}




//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//setup////setup////setup////setup////setup////setup////setup////setup////setup////setup////setup////setup////setup////setup////setup////setup////setup////setup////setup///
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void setup() {
  delay(3000);
  Serial.begin(9600);    //블루투스와 아두이노의 통신 속도 설정
  BT.begin(9600);     //시리얼 통신의 속도
  esp8266.begin(9600);
  phoneNumber = "1038373355"; // 임의의 핸드폰번호 저장

  IPaddress = getInit();
  /// LCD INIT//
  lcd.createChar(0, BTShape_upper);
  lcd.createChar(1, BTShape_under);
  lcd.begin(16, 2);   //lcd 초기화 작업
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Hello world !");

  // LED INIT ///
  for (int i = 0; i < 3; i++)   //RGB LED
    pinMode(RGB_Led[i], OUTPUT);

  pinMode(speaker, OUTPUT);
  RGB[1] = 255;

  Serial.print("BT Wait");
  while (!BT.available()) {
    delay(1000);
    Serial.print('.'); // 블루투스가 연결이 되어있지않으면 정지상태이다.]
  }
  Serial.println();Serial.println("Connect OK");
  lcd.clear(); lcd.home(); lcd.print("WiFi wait..");
  
  sendQuery("POST /home/create?number=" + phoneNumber + "&ip=" + IPaddress + " HTTP/1.1\r\n\r\n"); // WEB 등록
  sendData("AT+CIPMUX=1\r\n", 1000, DEBUG);
  sendData("AT+CIPSERVER=1,80\r\n", 1000, DEBUG); // 데이터를 받을 수 있게 서버를 열어줌
  BT.print("o");
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//loop////loop////loop////loop////loop////loop////loop////loop////loop////loop////loop////loop////loop////loop////loop////loop////loop////loop////loop////loop////loop////loop/
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void loop() {


    /* 블루투스 모양 LCD에 찍기 */
  if (BT.available()) {
    char c = BT.read();
    Serial.println(c);
    if(c == 'm' || c=='M')
    {
      state = 1;
    }
    else if(c=='C' || c=='c'){
      state = 0;
      count++;
    }
    else{
      exitcnt++;
      if(exitcnt>=100)
      {
        exitcnt=0;
        state=1;
      }
    }

  }
  // 일정시간 후에 cou
  if(checktimer.check())
  {
    if(count<10)
      state = true;
    count=0;
    exitcnt=0;
  }
 
  // 일정 시간마다 연결되었다는 문자 C를 보내줌.
  if(sendtimer.check())  
  {
    if(!state)
      BT.print("C");
  }
  
  if(state!=connet) // 상태가 바꼈을때 1회실행해야하는 명령어
  {
    connet = state;
    if(!flag)
    {
      sendQuery("POST /home/setstate?number="+phoneNumber+"&state="+state+" HTTP/1.1\r\n\r\n");  
      sendData("AT+CIPMUX=1\r\n", 1000, DEBUG);
      sendData("AT+CIPSERVER=1,80\r\n", 1000, DEBUG);
      if(state)
        BT.print("m");
      if(!state)
        BT.print("c");
    }
    flag = false;
  }


  /* 미아상태의 액션. */
  if (state) {
    RGBInit();
    tone(speaker,523,1000);
    SetColor(255,0,0); // RED 출력
    delay(500);
    noTone(speaker);
    lcd.clear();
    lcd.print("help me!");
    lcd.setCursor(0,1);
    lcd.print("0"+phoneNumber);
  }
  else{
    SetColor(0,255,0);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.write(byte(0));
    lcd.setCursor(0, 1);
    lcd.write(byte(1));
  }

  while (esp8266.available()) // 서버에서 REQUEST를 줄 때
  {
    char ch = esp8266.read();
    if (ch == '\r' || ch == '\n')
    {
      Serial.println(st);
      int temp = st.indexOf("state");
      if (temp != -1)
      {  
        state = st[temp + 6] - '0'; // 스테이트 값 변경
        connet = state;
        if(state) 
          BT.write('M');
        else
        {BT.write('C');
          BT.flush();
        }
        delay(100);
        flag =true;
      }
      st = "";
    }
    else
      st += ch;
  }

}
