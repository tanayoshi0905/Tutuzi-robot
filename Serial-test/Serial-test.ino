#include "im920creceive_not_SoftwareSerial.h"
#include <DueTimer.h>

const int disconnect_set_count = 100; //この時間以内にデータを受信できなかったら未接続とみなす
bool disconnect = 0;
char receive_date[8] = {0,0,0,0,0,0,0,0}; //受信データ格納用

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  im920creceive_setup();
  //Timer7.attachInterrupt(timer_com).start(1000);
  for(int i=30; i<50; i++){
    pinMode(i, INPUT_PULLUP);
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  //timer_com();
  //print_receive_date();
  for(int i=30; i<50; i++){
    Serial.print(!digitalRead(i));
  }
  Serial.println();
}

void timer_com() {
  DUE_im920creceive(receive_date, &disconnect);
  receive_timecount++;
  disconnect = 0;
  //Serial.println("timer");
  //for(int i=0; i<8; i++){
    //Serial.print(/*receive_data[i]*/"a");
  //}
  //Serial.println();
}

void print_receive_date() {
  for (int i = 0; i < 6 ; i++) {
    Serial.print((int)receive_date[i]);
    Serial.print("  ");
  }
  Serial.println();
}
