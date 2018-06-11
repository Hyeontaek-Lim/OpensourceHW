#include <LiquidCrystal.h>  //LCD ���̺귯��
#include "Metro.h"
#define esp8266 Serial2
#define BT Serial1
#define DEBUG true

const int speaker = 10;   // ����Ŀ ������ ��
String IPaddress; // ���� �Ƶ��̳� IP
String phoneNumber; // ��ȣ�� ����ó
String ssid = "hongbin"; // �������� ID
String pass = "tlghk123"; // �������� pass
String destIP = "192.168.1.4"; // ���� IP�ּ�
String destPort = "3000"; // ��Ʈ��ȣ Rails�� ����
String st = "";
bool state = false; // �̾ƻ��� ǥ��
bool connet = false;
int count = 0;
int exitcnt=0;
Metro sendtimer(1000);
Metro checktimer(50000); 
bool flag = false;
LiquidCrystal lcd(4, 5, 6, 7, 8, 9); //lcd ����
byte BTShape_upper[8] {
  B000000, B001000, B001100, B001010, B001001, B101010, B011100, B001000
};
byte BTShape_under[8] {
  B001000, B011100, B101010, B001001, B001010, B001100, B001000, B000000
};

const int RGB_Led[3] = { 13, 12, 11 };    //RGB ������ ��
int RGB[3] = {0, 0, 0};


/* RGB ���� �����ؼ� �پ��� ������ �� �� �ִ�. */
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
  sendData("AT+RST\r\n", 1000, DEBUG); // �ʱ�ȭ
  sendData("AT+CIPMUX=0\r\n", 2000, DEBUG);
  do
    t = sendData("AT+CIPSTART=\"TCP\",\"" + destIP + "\"," + destPort + "\r\n", 3000, DEBUG);
  while (t.indexOf("Linked") == -1); // ��ũ ����ɶ�����
  sendData("AT+CIPSEND=" + String(str.length()) + "\r\n", 2000, DEBUG);
  sendData(str, 2000, DEBUG);
  sendData("AT+CIPCLOSE\r\n", 1000, DEBUG); // turn on server on port 80
}

String getInit() 
{
  /* WIFI����� init�Ѵ�.
   * 1. SSID�� password�� �������� ����
   * 2. CIFSR�� IP�ּ� ��ȯ
   * 3. CWMODE=3���� client, AP ��� ���
   * 4. CIFSR�� ���� �������� IP�ּ� ����
  */
  String temp = "";
  String res = "";

  sendData("AT+CWJAP=\"" + ssid + "\",\"" + pass + "\"\r\n", 7000, DEBUG); // �����ϱ�
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
  Serial.begin(9600);    //��������� �Ƶ��̳��� ��� �ӵ� ����
  BT.begin(9600);     //�ø��� ����� �ӵ�
  esp8266.begin(9600);
  phoneNumber = "1038373355"; // ������ �ڵ�����ȣ ����

  IPaddress = getInit();
  /// LCD INIT//
  lcd.createChar(0, BTShape_upper);
  lcd.createChar(1, BTShape_under);
  lcd.begin(16, 2);   //lcd �ʱ�ȭ �۾�
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
    Serial.print('.'); // ��������� ������ �Ǿ����������� ���������̴�.]
  }
  Serial.println();Serial.println("Connect OK");
  lcd.clear(); lcd.home(); lcd.print("WiFi wait..");
  
  sendQuery("POST /home/create?number=" + phoneNumber + "&ip=" + IPaddress + " HTTP/1.1\r\n\r\n"); // WEB ���
  sendData("AT+CIPMUX=1\r\n", 1000, DEBUG);
  sendData("AT+CIPSERVER=1,80\r\n", 1000, DEBUG); // �����͸� ���� �� �ְ� ������ ������
  BT.print("o");
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//loop////loop////loop////loop////loop////loop////loop////loop////loop////loop////loop////loop////loop////loop////loop////loop////loop////loop////loop////loop////loop////loop/
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void loop() {


    /* ������� ��� LCD�� ��� */
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
  // �����ð� �Ŀ� cou
  if(checktimer.check())
  {
    if(count<10)
      state = true;
    count=0;
    exitcnt=0;
  }
 
  // ���� �ð����� ����Ǿ��ٴ� ���� C�� ������.
  if(sendtimer.check())  
  {
    if(!state)
      BT.print("C");
  }
  
  if(state!=connet) // ���°� �ٲ����� 1ȸ�����ؾ��ϴ� ��ɾ�
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


  /* �̾ƻ����� �׼�. */
  if (state) {
    RGBInit();
    tone(speaker,523,1000);
    SetColor(255,0,0); // RED ���
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

  while (esp8266.available()) // �������� REQUEST�� �� ��
  {
    char ch = esp8266.read();
    if (ch == '\r' || ch == '\n')
    {
      Serial.println(st);
      int temp = st.indexOf("state");
      if (temp != -1)
      {  
        state = st[temp + 6] - '0'; // ������Ʈ �� ����
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