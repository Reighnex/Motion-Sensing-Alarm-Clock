#include <DFRobot_DHT11.h>
#include <DS1302.h>
#include <LiquidCrystal.h>

DS1302 rtc(4, 3, 2);  // RST, DAT, CLK
LiquidCrystal lcd(44, 45, 46, 47, 48, 49); // LCD
DFRobot_DHT11 DHT;    // DHT11

String time_cur;
int mode = 0;
bool button_cur = false;
bool button_pre = false;
bool alarm_state = false;

/* struct for the information when the alarm will be working */
struct {
  int hour, min, sec;
  String timestr;

  void increase() {
    // increase min by 1
    if (++min == 60) {
      min = 0;
      if (++hour == 24) hour = 0;
    }
  }

  void decrease() {
    // decrease min by 1
    if (--min == -1) {
      min = 59;
      if (--hour == -1) hour = 23;
    }
  }

  inline void set_string() {
    char buffer[10] = {0, };
    snprintf(buffer, 9, "%02d:%02d:%02d", hour, min, sec);
    timestr = String(buffer);
  }

} configInfo = {0, 0, 0, "00:00:00"};

namespace display {
  void current_time_th() {
    // display current date, time, temp and humidity; used when mode is 0
    DHT.read(7);

    lcd.clear();
    lcd.setCursor(0, 0);
    String dt = rtc.getDateStr();
    time_cur = String(rtc.getTimeStr()) + " [" + dt[3]+dt[4] + "." + dt[0]+dt[1] + "]";
    lcd.print(time_cur);

    lcd.setCursor(0, 1);
    String dow = rtc.getDOWStr();
    lcd.print(String(dow[0]) + dow[1] + dow[2]);
    lcd.print(" / ");
    lcd.print(DHT.temperature);
    lcd.print((char)223);
    lcd.print("C");
    lcd.print(" / ");
    lcd.print(DHT.humidity);
    lcd.print("%");

    delay(760);
  }

  void config_time() {
    // display configurated time; used when mode is 1
    lcd.clear();
    lcd.setCursor(1, 0);
    lcd.print("Set Alarm at..");
    lcd.setCursor(2, 1);
    lcd.print("[ ");
    configInfo.set_string();
    lcd.print(configInfo.timestr);
    lcd.print(" ]");
    delay(1000/30);
  }
}

void check_alarm() {
  String current = rtc.getTimeStr();
  if(current == configInfo.timestr)
    alarm_state = !alarm_state;
  if(alarm_state){
    tone(A0, 261, 800);
    if(digitalRead(6))
      alarm_state = !alarm_state;
  }
}

void setup() {
  rtc.halt(false); 
  rtc.writeProtect(false); 
  // rtc.setDOW(MONDAY); //요일설정
  // rtc.setTime(13, 34, 00); // 24시간형식(시간,분,초)
  // rtc.setDate(26, 12, 2022); // (날짜,달,년도)

  lcd.begin(16, 2); // LCD 초기화

  for (int i = 9; i < 12; i++) pinMode(i, INPUT);  // each pin represents for mode switching, increasing time, decreasing time
  pinMode(6, INPUT);    // PIR sensor
  pinMode(7, INPUT);    // DHT sensor
  pinMode(A0, OUTPUT);  // buzzer
}

void loop() {
  button_cur = digitalRead(9);
  if (button_cur && !button_pre) mode = !mode;   // switching mode


  if (mode) {
    display::config_time();
    
    if (digitalRead(10)) configInfo.increase();     // increase time
    if (digitalRead(11)) configInfo.decrease();     // decrease time
  }
  else {
    display::current_time_th();

    check_alarm();
  }

  button_pre = button_cur;
}