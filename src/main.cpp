#include <Arduino.h>
#include "im920creceive_not_SoftwareSerial.h"
#include <DueTimer.h>

const int limit_pin[] = {34, 33, 35, 36}; //ステアリング左、右、矢押し出しギア、押し出し末端
const int tire_pin[2][2] = {{8, 7}, {6, 5}}; //タイヤモーターピン、[左][右]{PWM1, PWM2}
const int roller_pin[2][2] = {{41, 2}, {45, 4}}; //矢射出モーターピン、[上][下]{DIR, PWM}
const int steering_pin[] = {11, 12}; //ステアリングモーターピン、{PWM1, PWM2}
const int pushout_pin[] = {9, 10}; //矢押し出しモーターピン、{PWM1, PWM2}

const int disconnect_set_count = 100; //この時間以内にデータを受信できなかったら未接続とみなす

const int steering_speed = 80; //ステアリングの回転速度（PWM）
const int tire_max_speed = 100; //タイヤの最大以下略
const int tire_tyousei[2] = {90, 100}; //タイヤ各調整用（右、左）
const int roller_speed = 250; //矢射出ローラーの以下略
const int roller_tyousei[2] = {235, 255}; //矢射出ローラーの各調整用（上、下）
const int pushout_speed = 80; //矢押し出し機構の以下略

bool disconnect = 0; //通信状況
char receive_data[8]; //受信データ格納用
int pushout_stat = 0; //押し出し機構の状態
int steering_stat = 0; //ステアリングの状態

int tact_data[4]; //タクトスイッチ（上、下、左、右）
int js_data[4]; //ジョイスティック{左X, 左Y、右X、右Y}
int tgl_data; //トグルスイッチ
int limit_data[4]; //リミットスイッチ

int steering_operation = 0; //ステアリングのあるべき状態（-1=左、0=無、1=右）

int tire_pwm[2]; //タイヤのPWM格納用
int tire_dir[2]; //タイヤ回転向き格納用

void timer_com(){
  DUE_im920creceive(receive_data, &disconnect);
  receive_timecount++;
  //Serial.println("Timer_now");
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  //Serial.println("Hello");
  im920creceive_setup();
  timer_com();
  //Timer7.attachInterrupt(timer_com).start(1000);
  //Serial.println("Timer_start");

  for(int i=0; i<4; i++){
    pinMode(limit_pin[i], INPUT_PULLUP);
  }

  for(int i=0; i<2; i++){
    for(int j=0; j<2; j++){
      pinMode(tire_pin[i][j], OUTPUT);
      analogWrite(tire_pin[i][j], 0);
      pinMode(roller_pin[i][j], OUTPUT);
      analogWrite(roller_pin[i][j], 0);
    }
    pinMode(steering_pin[i], OUTPUT);
    analogWrite(steering_pin[i], 0);
    pinMode(pushout_pin[i], OUTPUT);
    analogWrite(pushout_pin[i], 0);
  }

  delay(100);
}

void data_store() {
  //Serial.println(receive_data);
  for(int i=0; i<4; i++) {
    tact_data[i] = ((int)receive_data[5] >> i) & 0b00000001; //タクト
  }

  tgl_data = receive_data[4]; //トグル

  for(int i=0; i<4; i++){
    js_data[i] = map(receive_data[i], 0, 126, -255, 255); //ジョイスティック
  }

  for(int i=0; i<4; i++){
    limit_data[i] = !digitalRead(limit_pin[i]); //リミットスイッチ
    //Serial.print(limit_data[i]);
  }
  //Serial.println();
}

void steering() {
  /*for(int i=0; i<4; i++){
    Serial.print(limit_data[i]);
  }
  Serial.println();
    for(int i=0; i<4; i++){
    Serial.print(tact_data[i]);
  }
  Serial.println();*/
  if((steering_operation == -1 || tact_data[2]) && !limit_data[0]) { //ステアリングが右方向にずれた・左タクトが押されている間（上限も考慮）
    Serial.println("a");
    analogWrite(steering_pin[0], steering_speed); //左に回転
    analogWrite(steering_pin[1], 0);
    steering_operation = -1;
  }else if((steering_operation == 1 || tact_data[3]) && !limit_data[1]){ //ステアリングが左方向にずれた・右タクトが押されている間（上限も考慮）
    analogWrite(steering_pin[0], 0);
    analogWrite(steering_pin[1], steering_speed);
    steering_operation = 1;
  }else{ //動かす必要がない場合（上限にかかっている時？）
    analogWrite(steering_pin[0], 0);
    analogWrite(steering_pin[1], 0);
  }
}

void tire(int motor) {
  switch(motor){
    case 0:
      tire_pwm[motor] = js_data[1] + js_data[2]; //左タイヤ
      Serial.println(js_data[1]);
      Serial.println(js_data[2]);
      break;
    case 1:
      tire_pwm[motor] = js_data[1] - js_data[2]; //右タイヤ
      break;
  }

  if(tire_pwm[motor] > tire_max_speed){ //上式でPWM上限超えた場合
    tire_pwm[motor] = tire_max_speed; //上限まで戻す
  }else if(tire_pwm[motor] < -1 * tire_max_speed){
    tire_pwm[motor] = tire_max_speed * (-1);
  }
  tire_pwm[motor] = map(tire_pwm[motor], -(tire_max_speed), tire_max_speed, -(tire_tyousei[motor]), tire_tyousei[motor]);
  //Serial.println(tire_pwm[motor]);
  if(tire_pwm[motor] >= 0){ //正転時
    analogWrite(tire_pin[motor][0], tire_pwm[motor]);
    analogWrite(tire_pin[motor][1], 0);
  }else if(tire_pwm[motor] < 0){ //逆転時
    analogWrite(tire_pin[motor][0], 0);
    analogWrite(tire_pin[motor][1], abs(tire_pwm[motor]));
  }
}

void arrow() {
  switch(tgl_data){
    case 1: //トグル上
    //Serial.println("a");
      analogWrite(pushout_pin[1], 0); //前進させるため、後進駆動用PWMピンはオフ
      if(!(!limit_data[2] && !limit_data[3])){ //押し出し機構が一番前まで出切っていなければ
        analogWrite(pushout_pin[0], pushout_speed); //押し出し機構前進
      }else{ //出切っていれば
        analogWrite(pushout_pin[0], 0); //何もしない
      }

      analogWrite(roller_pin[0][1], roller_tyousei[0]); //ローラーも回しておく
      analogWrite(roller_pin[1][1], roller_tyousei[1]);
      break;

    case 2: //トグル下
      analogWrite(pushout_pin[0], 0); //更新させるため、前進駆動用PWMピンはオフ
      if(!(!limit_data[2] && limit_data[3])){ //押し出し機構が一番後ろまで下がり切っていなければ
        analogWrite(pushout_pin[1], pushout_speed); //押し出し機構後退
      }else{ //下がり切っていれば
        analogWrite(pushout_pin[1], 0); //何もしない
      }
    
      analogWrite(roller_pin[0][1], 0); //ローラーは止めておく
      analogWrite(roller_pin[1][1], 0);
      break;
    
    case 0: //トグル中立
      for(int i=0; i<2; i++){ //何もしない
        analogWrite(pushout_pin[i], 0);
        analogWrite(roller_pin[i][1], 0);
      }
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  timer_com();
  data_store();
  for(int i=0; i<8; i++){
    Serial.print((int)receive_data[i]);
    Serial.print(" ");
  }
  Serial.println();
  
  if(receive_timecount > disconnect_set_count) { //未接続状態と見做したとき
    for(int i=0; i<2; i++){
      for(int j=0; j<2; j++){
        digitalWrite(tire_pin[i][j], LOW);
        digitalWrite(roller_pin[i][j], LOW);
      }
      digitalWrite(steering_pin[i], LOW);
      digitalWrite(pushout_pin[i], LOW);
    }

    disconnect = 1;
  }

  if(!disconnect) {
    steering();
    tire(0);
    tire(1);
    arrow();
  }
}
