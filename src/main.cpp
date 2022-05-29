#include <Arduino.h>
#include <GyverOLED.h>
#include "GyverFilters.h"

#define buz 6                 // пин пищалки
#define buz_delay 3           // время писка
#define buz_power 10          // громкость

#define LED 10                // пин светодиода
#define LED_delay 10          // время мигания
#define LED_power 100         // яркость

#define screen_refr 1000      // период обновления экрана
#define sens 13.33            // чувствительность датчика мкР/ч/CPS

#define bat A3            // чувствительность датчика мкР/ч/CPS

volatile int count = 0;       // число частиц
volatile bool LED_flag = 0;   // флаг для светодиода
volatile bool buz_flag = 0;   // флаг для пищалки
volatile bool NEW = 1;        // есть новая частица
uint32_t timer = 0;           // таймер экрана и усреднения
uint32_t LED_timer = 0;       // таймер для импульса световой индикации
uint32_t buz_timer = 0;       // таймер для импульса звуковой сигнализации
unsigned long last_delay = 0;      // время между последними импульсами
unsigned long last_time = 0;       // время последнего импульса
float result;
float k = 0.1;
GyverOLED<SSD1306_128x64, OLED_NO_BUFFER>  oled;
// GFilterRA analog0;  

float expRunningAverageAdaptive(float newVal) {
   static float filVal = 0;
  filVal += (newVal - filVal) * k;
  return filVal;
}

void countPulse() {
  count++;
  LED_flag = 1;
  buz_flag = 1;
  NEW = 1;
  LED_timer = millis();
  buz_timer = millis();
  last_delay = micros()-last_time;
  last_time = micros();
  
}



void setup() {
  // analog0.setCoef(0.01);
  // analog0.setStep(10);
  oled.init();        // инициализация
  oled.clear();       // очистка
  oled.setScale(4);   // масштаб текста (1..4)
  oled.home();        // курсор в 0,0
  // oled.print("Привет!");
  
  pinMode(2 ,INPUT_PULLUP); // кнопка на пине 2
  pinMode(LED ,OUTPUT);
  attachInterrupt(0,countPulse,FALLING); 
  last_time = micros();
  // oled.dot(127, 63, 1);
  oled.line(0, 55, 124, 55, 1);
  oled.line(0, 63, 124, 63, 1);
  oled.line(0, 55, 0, 63, 1);
  oled.line(124, 55, 124, 63, 1);
  oled.rect(125, 57, 127, 61, 1);
  
  // oled.rect(1, 56, 124, 57, 0);
}
void loop() {
  if (millis() - timer >= screen_refr) {
    analogWrite(buz, LOW);
    buz_flag = 0;
    // oled.clear();       // очистка
    oled.home();        // курсор в 0,0
    // oled.print(sens*float(count)*1000/float(screen_refr));
    oled.setScale(4);
    oled.print(result,2);
    oled.setCursor(64,4);
    oled.setScale(2);
    oled.print("uR/hr");
    oled.setCursor(0,4);
    oled.print((5.0*analogRead(bat)/1024.0));
    oled.rect(2, 57, round(1.2*(68.96*((5.0*analogRead(bat)/1024.0)-2.75))-2), 61, 1);
    count = 0;
    do {
      timer += screen_refr;
      if (timer < screen_refr) break;  // переполнение uint32_t
    } while (timer < millis() - screen_refr); // защита от пропуска шага
  }

  if (LED_flag) {
    analogWrite(LED, LED_power);
  } 
  if (LED_flag && millis() - LED_timer >= LED_delay) {
    digitalWrite(LED, LOW);
    LED_flag = 0;
  } 

if (buz_flag) {
    analogWrite(buz, buz_power);
  }

if (buz_flag && millis() - buz_timer >= buz_delay) {
    analogWrite(buz, LOW);
    buz_flag = 0;
  }

if (NEW) {
  result = sens*1e6/expRunningAverageAdaptive(float(last_delay));
  NEW = 0;
}
  // result = analog0.filteredTime(sens/(float(last_delay)*1e3));

}

