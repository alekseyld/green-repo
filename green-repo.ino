/*
 * Проект "Теплица" УТЭК
 * Разработчик прошивки Лысов А.Д.
 * Написано под микроконтроллер STM32F103C(20k RAM, 64k Flash)
 * Версия кода v0.1
 */

#define DEBUG true

#include <OneWire.h>

//Солнечная батарея и вентилятор
#define SOLAR_BATTERY_PIN PA1
#define FAN_PIN PB12

//Фоторезистор и LED лента
#define LIGHT_RES_PIN PA0
#define LED_PIN PB3

//1wire pin, два концевика и мотор
OneWire  ds(PA3);
#define FINISH_UP_PIN PB7
#define FINISH_DOWN_PIN PB6
#define DRIVE_WINDOWS_PIN_R PB13
#define DRIVE_WINDOWS_PIN_L PB11

//Датчик влажности почвы и насос полива
#define HYDRO_PIN PB9
#define PUMP_WATERING_PIN PB15

//Уровень в емкости и насос откачки
#define LEVEL_PIN PB8
#define PUMP_RETURN_PIN PB14

//Красная подсветка емкости
#define RED_LIGHT_PIN PB4

void setup() {
  Serial.begin(115200);
  Serial1.begin(115200);

  pinMode(SOLAR_BATTERY_PIN, INPUT_ANALOG);
  pinMode(LIGHT_RES_PIN, INPUT_ANALOG);
  pinMode(HYDRO_PIN, INPUT_PULLDOWN);

  pinMode(FINISH_UP_PIN, INPUT_PULLDOWN);
  pinMode(FINISH_DOWN_PIN, INPUT_PULLDOWN);
  pinMode(LEVEL_PIN, INPUT_PULLDOWN);       

  pinMode(FAN_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT); 
  pinMode(DRIVE_WINDOWS_PIN_R, OUTPUT);
  pinMode(DRIVE_WINDOWS_PIN_L, OUTPUT);
  pinMode(PUMP_WATERING_PIN, OUTPUT);
  pinMode(PUMP_RETURN_PIN, OUTPUT);
  pinMode(RED_LIGHT_PIN, OUTPUT);      
}

void toogleDigital(int pin, bool on) {
  if (on) {
    digitalWrite(pin, HIGH);
  } else {
    digitalWrite(pin, LOW);
  }
}

//Вентилятор - Солнечная панель
bool getFanState() {
  int procent = getSolarPanelStateProcents();  
  
  return procent < 70;
}

void processSolarPanerLogic() {
  toogleDigital(FAN_PIN, getFanState());
}

//Фоторезистор - Светодиодная лента
bool getLightState() {
  int lght = analogRead(LIGHT_RES_PIN);
  Serial.print("LIGHT_RES_PIN analog = ");  
  Serial.println(lght);
  Serial.print("---LightState bool = ");  
  Serial.println(lght > 3000); 
  return lght < 3000;
}

void processLightLogic() {
  toogleDigital(LED_PIN, getLightState());
}

//Термометр - Открытие окон
bool getTermoState() {
  float term = getTempFromOneWire();
  Serial.print("TEMP cel = ");  
  Serial.println(term);
  Serial.print("TEMP STATE = ");  
  Serial.println(term > 33.0);
  return term > 33.0;
}

void processDriveLogic(int up, int down, bool termoState) {
    if ((termoState && down == HIGH)
      || (termoState && up == LOW)) {

    Serial.println("------Drive to RIGHT OPEN");     
    toogleDigital(DRIVE_WINDOWS_PIN_R, true);
    toogleDigital(DRIVE_WINDOWS_PIN_L, false);
    
  } else if ((!termoState && up == HIGH)
            ||(!termoState && down == LOW)) {

    Serial.println("------Drive to LEFT CLOSE");  
    toogleDigital(DRIVE_WINDOWS_PIN_L, true);
    toogleDigital(DRIVE_WINDOWS_PIN_R, false);
  } else {
    Serial.println("------Drive STOP");  
    toogleDigital(DRIVE_WINDOWS_PIN_R, false);
    toogleDigital(DRIVE_WINDOWS_PIN_L, false);
  }
}

void processTermoLogic() {
  int up = getFinishUpState();
  int down = getFinishDownState();
  bool termoState = getTermoState();

  Serial.print("FINISH_UP_PIN digital = ");  
  Serial.println(up);
  Serial.print("FINISH_DOWN_PIN digital = ");  
  Serial.println(down);
  Serial.print("---TermoState bool = ");  
  Serial.println(termoState);      

  processDriveLogic(up, down, termoState);
}

void processHydroLogic() {
  int needWattering = getHydroState();
  Serial.print("HYDRO_PIN analog = ");  
  Serial.println(needWattering); 
  Serial.print("---needWattering bool = ");  
  Serial.println(needWattering == LOW);
  
  toogleDigital(PUMP_WATERING_PIN, needWattering == LOW);
}

//Датчик уровня в емкости - Насос откачки
void processLevelLogic() {
  int needReturn = getLevelState();
  Serial.print("LEVEL_PIN digital = ");  
  Serial.println(needReturn);
  Serial.print("---needReturn bool = ");  
  Serial.println(needReturn == HIGH);  
  
  toogleDigital(PUMP_RETURN_PIN, needReturn == HIGH);
}

//Красный светодиод
long lastMilli = 0;
bool redLightOn = false;
void processRedLightLogic() {
  long milli = millis();
  if (milli - lastMilli >= 60 * 1000) {
     lastMilli = milli;
     redLightOn = !redLightOn;
     toogleDigital(RED_LIGHT_PIN, redLightOn);
  }
}

//Получение значения нижнего конечного выключателя
int getFinishDownState() {
  return digitalRead(FINISH_DOWN_PIN);
}

//Получение значения верхнего конечного выключателя
int getFinishUpState() {
  return digitalRead(FINISH_UP_PIN);
}

//Получение значения температуры из ds18b20 в цельсиях
float getTempFromOneWire(){
  byte i;
  byte present = 0;
  byte type_s;
  byte data[12];
  byte addr[8];
  float celsius, fahrenheit;
  
  while (!ds.search(addr)) {
    ds.reset_search();
    delay(250);
  }

  ds.reset();
  ds.select(addr);
  ds.write(0x44, 1);        // start conversion, with parasite power on at the end
  
  delay(1000);     // maybe 750ms is enough, maybe not
  // we might do a ds.depower() here, but the reset will take care of it.
  
  present = ds.reset();
  ds.select(addr);    
  ds.write(0xBE);         // Read Scratchpad
  for ( i = 0; i < 9; i++) {           // we need 9 bytes
    data[i] = ds.read();
  }

  // Convert the data to actual temperature
  // because the result is a 16 bit signed integer, it should
  // be stored to an "int16_t" type, which is always 16 bits
  // even when compiled on a 32 bit processor.
  int16_t raw = (data[1] << 8) | data[0];
  if (type_s) {
    raw = raw << 3; // 9 bit resolution default
    if (data[7] == 0x10) {
      // "count remain" gives full 12 bit resolution
      raw = (raw & 0xFFF0) + 12 - data[6];
    }
  } else {
    byte cfg = (data[4] & 0x60);
    // at lower res, the low bits are undefined, so let's zero them
    if (cfg == 0x00) raw = raw & ~7;  // 9 bit resolution, 93.75 ms
    else if (cfg == 0x20) raw = raw & ~3; // 10 bit res, 187.5 ms
    else if (cfg == 0x40) raw = raw & ~1; // 11 bit res, 375 ms
    //// default is 12 bit resolution, 750 ms conversion time
  }
  celsius = (float)raw / 16.0;
  return celsius;
}

//Получение дисретного значения влажности
//1 - сухо
int getHydroState() {
  return digitalRead(HYDRO_PIN);
}

//Получение дисретного значения уровня
//1 - Уровень достигнут
int getLevelState() {
  return digitalRead(LEVEL_PIN);
}

int getSolarPanelState() {  
  return analogRead(SOLAR_BATTERY_PIN);
}

int getSolarPanelStateProcents() {
  #if (DEBUG)
    int procent = getSolarPanelState() * 100 / 4095;
    Serial.print("SOLAR_STATE = ");  
    Serial.print(procent); 
    Serial.println("%"); 
    return procent;
  #endif
  
  return getSolarPanelState() * 100 / 4095;
}

void array_to_string(byte array[], unsigned int len, char buffer[])
{
    for (unsigned int i = 0; i < len; i++)
    {
        byte nib1 = (array[i] >> 4) & 0x0F;
        byte nib2 = (array[i] >> 0) & 0x0F;
        buffer[i*2+0] = nib1  < 0xA ? '0' + nib1  : 'A' + nib1  - 0xA;
        buffer[i*2+1] = nib2  < 0xA ? '0' + nib2  : 'A' + nib2  - 0xA;
    }
    buffer[len*2] = '\0';
}

void loop() {
  Serial.println(millis()); 
  Serial.println(); 

  if (Serial1.available()) {
    Serial.print("bytes from serial1 = "); 
    while(Serial1.available()){
      Serial.write(Serial1.read());
      
//      byte myByteArray[8];
//      
//      //Serial.print(Serial1.readString(1));// Serial1.read() - '0'
//      Serial1.readBytes(myByteArray, Serial.available());
//
//      char str[32] = "";
//      array_to_string(myByteArray, 4, str);
//
//      Serial.print(str);
    }
    Serial.println(" ; end"); 
  }
  
//  processSolarPanerLogic();
//  processLightLogic();
//  processHydroLogic();
//  processLevelLogic();
//  processRedLightLogic();
//  processTermoLogic();
//
//  Serial.println("-----------------------");  
  delay(500);
  //delay(5000);
}
