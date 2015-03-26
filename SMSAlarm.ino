#include "Timer.h"                     //http://github.com/JChristensen/Timer
Timer t, t1,t2;
int once;
String trustedNo = "+40731491417";
String password = "1234567890";
int stare = 0;
 int sensorValue;
//a-gsm settings============================
#include <SoftwareSerial.h>
#define powerPIN     7//Arduino Digital pin used to power up / power down the modem
#define resetPIN     6//Arduino Digital pin used to reset the modem 
#define statusPIN    5//Arduino Digital pin used to monitor if modem is powered 
int state = 0, i = 0, powerState = 0, ready4SMS = 0, ready4Voice = 0;
#define BUFFDSIZE 240
SoftwareSerial SSerial(2, 3);  //RX==>2 ,TX soft==>3
char ch;
char buffd[BUFFDSIZE];
char o_buffd[BUFFDSIZE];

int noSMS = 0, totSMS = 0;

char readBuffer[200];
///========================================

int effect[] = {13, 12, 11, 10, 9, 8};
int count, n = 0;
void setup() {
  t.every(250, alarmSMS);
  t1.every(50, lightEffect);
  t2.every(1, sensor);
  
  for (int i = 0; i < 6; i++)
  {
    pinMode(effect[i], OUTPUT);
  }
  // a-gsm setup=====================================
  SSerial.begin(9600);
  Serial.begin(57600);//1ms

  clearSSerial();
  clearSerial();
  delay(10);
  pinMode(resetPIN, OUTPUT);
  digitalWrite(resetPIN, LOW);
  pinMode(powerPIN, OUTPUT);
  pinMode(statusPIN, INPUT);
  digitalWrite(powerPIN, LOW);
  delay(100);
  Serial.flush();
  SSerial.flush();
  if (!digitalRead(statusPIN)) restartMODEM();
  clearBUFFD();
  ready4SMS = 0;
  ready4Voice = 0;
  Serial.println("SMSAlarm ready");
  //===================================================
}

void loop() {
  t2.update();
  t.update();
  t1.update();
}

void alarmSMS() {

  listSMS();
  int cnt;
  cnt = noSMS;
  while (cnt > 0) {
    readSMS(cnt);

    if ((trustedNo == getValue(buffd, ',', 1).substring(1, 13)) && (password == getValue(buffd, ',', 4))) {
      if (stare == 0) {
        stare = 1;
        Serial.println("ARMED");
      }
      else if (stare == 1) {
        stare = 0;
        Serial.println("DISARMED");
      }
      else if (stare == 2) {
        stare = 1;
        Serial.println("ALARM CANCEL BUT STILL ARMED");
      }

    }
    deleteSMS(cnt);
    clearBUFFD();
    clearSSerial();
    cnt--;
  }
}

void lightEffect() {
  if (stare == 0) {
    for (int i = 0; i < 6; i++) {digitalWrite(effect[i], HIGH);
    delay(50);
  digitalWrite(effect[i], LOW);}
    for (int i = 6; i >=0; i--) {digitalWrite(effect[i], HIGH);
    delay(50);
    digitalWrite(effect[i], LOW);  
}
  }
  else if (stare == 1)
  {once=0;
    for (int i = 0; i < 6; i++) {digitalWrite(effect[i], HIGH);
 }

  }
  
    else if (stare == 2)
  {
    for (int i = 0; i < 6; i++) digitalWrite(effect[i], HIGH);
    delay(50);
    for (int i = 0; i < 6; i++) digitalWrite(effect[i], LOW);

  }
  


}

void sensor()
{
sensorValue=analogRead(A0);
if((stare==1)&&(sensorValue<=950))
{stare=2;
t.stop(0);
if ((sendSMS("+40731491417","ALARM")==1)&(once==0)){ once=1;
  t.every(250, alarmSMS);

}

//send SMS once
}
}
