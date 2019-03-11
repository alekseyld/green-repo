/*
 * Проект "Теплица" УТЭК
 * Разработчик прошивки Лысов А.Д.
 * Написано под микроконтроллер STM32F103C(20k RAM, 64k Flash)
 * Версия кода v0.1
 */

//Солнечная батарея и вентилятор
#define SOLAR_BATTERY_PIN PA1
#define FAN_PIN PB12

//Фоторезистор и LED лента
#define LIGHT_RES_PIN PA0
#define LED_PIN PB3

//Термистор, два концевика и мотор
#define TERMO_RES_PIN PA3
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

#define THERMISTORNOMINAL 10000
#define TEMPERATURENOMINAL 25
#define BCOEFFICIENT 3950
#define SERIESRESISTOR 10000

void setup() {
  Serial.begin(115200);

  pinMode(SOLAR_BATTERY_PIN, INPUT_ANALOG);
  pinMode(LIGHT_RES_PIN, INPUT_ANALOG);
  pinMode(TERMO_RES_PIN, INPUT_ANALOG);

  pinMode(FINISH_UP_PIN, INPUT);
  pinMode(FINISH_DOWN_PIN, INPUT);
  pinMode(HYDRO_PIN, INPUT);
  pinMode(LEVEL_PIN, INPUT);       

  pinMode(FAN_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT); 
  pinMode(DRIVE_WINDOWS_PIN_R, OUTPUT);
  pinMode(DRIVE_WINDOWS_PIN_L, OUTPUT);
  pinMode(PUMP_WATERING_PIN, OUTPUT);
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
  int slr = analogRead(SOLAR_BATTERY_PIN);
  Serial.print("SOLAR_BATTERY_PIN analog = ");  
  Serial.println(slr);
    Serial.print("---FanState bool = ");  
  Serial.println(slr > 3000);  
  
  return slr > 3000;
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

double Getterm(int RawADC) {
  RawADC = map(RawADC, 0, 4095, 0, 1024);
  float average = RawADC; 
  float steinhart;
  average = 1023 / average - 1;
  average = SERIESRESISTOR / average;
  steinhart = average / THERMISTORNOMINAL; // (R/Ro)
  steinhart = log(steinhart); // ln(R/Ro)
  steinhart /= BCOEFFICIENT; // 1/B * ln(R/Ro)
  steinhart += 1.0 / (TEMPERATURENOMINAL + 273.15); // + (1/To)
  steinhart = 1.0 / steinhart; // инвертируем
  steinhart -= 273.15; // конвертируем в градусы по Цельсию
  return steinhart;
}

//Терморезистор - Открытие окон
bool getTermoState() {
  int term = analogRead(TERMO_RES_PIN);
  Serial.print("TERMO_RES_PIN analog = ");  
  Serial.println(term);
  Serial.print("-temp cel = ");
  double grad = Getterm(term)  
  Serial.println();    
  //todo перевод в градусы
  return grad > 33.0;
}

void processTermoLogic() {
  int up = digitalRead(FINISH_UP_PIN);
  int down = digitalRead(FINISH_DOWN_PIN);
  bool termoState = getTermoState();

  Serial.print("FINISH_UP_PIN digital = ");  
  Serial.println(up);
  Serial.print("FINISH_DOWN_PIN digital = ");  
  Serial.println(down);
  Serial.print("---TermoState bool = ");  
  Serial.println(termoState);      

  if ((termoState && down == HIGH)
      || (termoState && up == LOW)) {

    Serial.println("------Drive to RIGHT");     
    toogleDigital(DRIVE_WINDOWS_PIN_R, true);
    toogleDigital(DRIVE_WINDOWS_PIN_L, false);
    
  } else if ((!termoState && up == HIGH)
            ||(!termoState && down == LOW)) {

    Serial.println("------Drive to LEFT");  
    toogleDigital(DRIVE_WINDOWS_PIN_L, true);
    toogleDigital(DRIVE_WINDOWS_PIN_R, false);
  } else {
    Serial.println("------Drive STOP");  
    toogleDigital(DRIVE_WINDOWS_PIN_R, false);
    toogleDigital(DRIVE_WINDOWS_PIN_L, false);
  }
}

//Датчик влажности - Насос полив
void processHydroLogic() {
  int needWattering = digitalRead(HYDRO_PIN);
  Serial.print("HYDRO_PIN analog = ");  
  Serial.println(needWattering); 
  Serial.print("---needWattering bool = ");  
  Serial.println(needWattering == HIGH);
  
  toogleDigital(PUMP_WATERING_PIN, needWattering == HIGH);
}

//Датчик уровня в емкости - Насос откачки
void processLevelLogic() {
  int needReturn = digitalRead(LEVEL_PIN);
  Serial.print("LEVEL_PIN digital = ");  
  Serial.println(needReturn);
  Serial.print("---needReturn bool = ");  
  Serial.println(needReturn == LOW);  
  
  toogleDigital(PUMP_RETURN_PIN, needReturn == LOW);
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

void loop() {
  Serial.println(millis()); 
  Serial.println(); 
  
  processSolarPanerLogic();
  processLightLogic();
  processTermoLogic();
  processHydroLogic();
  processLevelLogic();
  processRedLightLogic();

  Serial.println("-----------------------");  
  //delay(500);
  delay(5000);
}
