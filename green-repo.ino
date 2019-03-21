/*
   Р СџРЎР‚Р С•Р ВµР С”РЎвЂљ "Р СћР ВµР С—Р В»Р С‘РЎвЂ Р В°" Р Р€Р СћР В­Р С™
   Р В Р В°Р В·РЎР‚Р В°Р В±Р С•РЎвЂљРЎвЂЎР С‘Р С” Р С—РЎР‚Р С•РЎв‚¬Р С‘Р Р†Р С”Р С‘ Р вЂєРЎвЂ№РЎРѓР С•Р Р† Р С’.Р вЂќ.
   Р СњР В°Р С—Р С‘РЎРѓР В°Р Р…Р С• Р С—Р С•Р Т‘ Р С�Р С‘Р С”РЎР‚Р С•Р С”Р С•Р Р…РЎвЂљРЎР‚Р С•Р В»Р В»Р ВµРЎР‚ STM32F103C(20k RAM, 64k Flash)
   Р вЂ™Р ВµРЎР‚РЎРѓР С‘РЎРЏ Р С”Р С•Р Т‘Р В° v0.1
*/

#define DEBUG true

#include <OneWire.h>

//Р РЋР С•Р В»Р Р…Р ВµРЎвЂЎР Р…Р В°РЎРЏ Р В±Р В°РЎвЂљР В°РЎР‚Р ВµРЎРЏ Р С‘ Р Р†Р ВµР Р…РЎвЂљР С‘Р В»РЎРЏРЎвЂљР С•РЎР‚
#define SOLAR_BATTERY_PIN PA1
#define FAN_PIN PB12

//Р В¤Р С•РЎвЂљР С•РЎР‚Р ВµР В·Р С‘РЎРѓРЎвЂљР С•РЎР‚ Р С‘ LED Р В»Р ВµР Р…РЎвЂљР В°
#define LIGHT_RES_PIN PA0
#define LED_PIN PB3

//1wire pin, Р Т‘Р Р†Р В° Р С”Р С•Р Р…РЎвЂ Р ВµР Р†Р С‘Р С”Р В° Р С‘ Р С�Р С•РЎвЂљР С•РЎР‚
OneWire  ds(PA3);
#define FINISH_UP_PIN PB7
#define FINISH_DOWN_PIN PB6
#define DRIVE_WINDOWS_PIN_R PB13
#define DRIVE_WINDOWS_PIN_L PB11

//Р вЂќР В°РЎвЂљРЎвЂЎР С‘Р С” Р Р†Р В»Р В°Р В¶Р Р…Р С•РЎРѓРЎвЂљР С‘ Р С—Р С•РЎвЂЎР Р†РЎвЂ№ Р С‘ Р Р…Р В°РЎРѓР С•РЎРѓ Р С—Р С•Р В»Р С‘Р Р†Р В°
#define HYDRO_PIN PB9
#define PUMP_WATERING_PIN PB15

//Р Р€РЎР‚Р С•Р Р†Р ВµР Р…РЎРЉ Р Р† Р ВµР С�Р С”Р С•РЎРѓРЎвЂљР С‘ Р С‘ Р Р…Р В°РЎРѓР С•РЎРѓ Р С•РЎвЂљР С”Р В°РЎвЂЎР С”Р С‘
#define LEVEL_PIN PB8
#define PUMP_RETURN_PIN PB14

//Р С™РЎР‚Р В°РЎРѓР Р…Р В°РЎРЏ Р С—Р С•Р Т‘РЎРѓР Р†Р ВµРЎвЂљР С”Р В° Р ВµР С�Р С”Р С•РЎРѓРЎвЂљР С‘
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

#if DEBUG
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB
  }
#endif
}

void toogleDigital(int pin, bool on) {
  if (on) {
    digitalWrite(pin, HIGH);
  } else {
    digitalWrite(pin, LOW);
  }
}

//Р вЂ™Р ВµР Р…РЎвЂљР С‘Р В»РЎРЏРЎвЂљР С•РЎР‚ - Р РЋР С•Р В»Р Р…Р ВµРЎвЂЎР Р…Р В°РЎРЏ Р С—Р В°Р Р…Р ВµР В»РЎРЉ
bool getFanState() {
  int procent = getSolarPanelStateProcents();

  return procent < 70;
}

void processSolarPanerLogic() {
  toogleDigital(FAN_PIN, getFanState());
}

//Р В¤Р С•РЎвЂљР С•РЎР‚Р ВµР В·Р С‘РЎРѓРЎвЂљР С•РЎР‚ - Р РЋР Р†Р ВµРЎвЂљР С•Р Т‘Р С‘Р С•Р Т‘Р Р…Р В°РЎРЏ Р В»Р ВµР Р…РЎвЂљР В°
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

//Р СћР ВµРЎР‚Р С�Р С•Р С�Р ВµРЎвЂљРЎР‚ - Р С›РЎвЂљР С”РЎР‚РЎвЂ№РЎвЂљР С‘Р Вµ Р С•Р С”Р С•Р Р…
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
             || (!termoState && down == LOW)) {

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

//Р вЂќР В°РЎвЂљРЎвЂЎР С‘Р С” РЎС“РЎР‚Р С•Р Р†Р Р…РЎРЏ Р Р† Р ВµР С�Р С”Р С•РЎРѓРЎвЂљР С‘ - Р СњР В°РЎРѓР С•РЎРѓ Р С•РЎвЂљР С”Р В°РЎвЂЎР С”Р С‘
void processLevelLogic() {
  int needReturn = getLevelState();
  Serial.print("LEVEL_PIN digital = ");
  Serial.println(needReturn);
  Serial.print("---needReturn bool = ");
  Serial.println(needReturn == HIGH);

  toogleDigital(PUMP_RETURN_PIN, needReturn == HIGH);
}

//Р С™РЎР‚Р В°РЎРѓР Р…РЎвЂ№Р в„– РЎРѓР Р†Р ВµРЎвЂљР С•Р Т‘Р С‘Р С•Р Т‘
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

//Р СџР С•Р В»РЎС“РЎвЂЎР ВµР Р…Р С‘Р Вµ Р В·Р Р…Р В°РЎвЂЎР ВµР Р…Р С‘РЎРЏ Р Р…Р С‘Р В¶Р Р…Р ВµР С–Р С• Р С”Р С•Р Р…Р ВµРЎвЂЎР Р…Р С•Р С–Р С• Р Р†РЎвЂ№Р С”Р В»РЎР‹РЎвЂЎР В°РЎвЂљР ВµР В»РЎРЏ
int getFinishDownState() {
  return digitalRead(FINISH_DOWN_PIN);
}

//Р СџР С•Р В»РЎС“РЎвЂЎР ВµР Р…Р С‘Р Вµ Р В·Р Р…Р В°РЎвЂЎР ВµР Р…Р С‘РЎРЏ Р Р†Р ВµРЎР‚РЎвЂ¦Р Р…Р ВµР С–Р С• Р С”Р С•Р Р…Р ВµРЎвЂЎР Р…Р С•Р С–Р С• Р Р†РЎвЂ№Р С”Р В»РЎР‹РЎвЂЎР В°РЎвЂљР ВµР В»РЎРЏ
int getFinishUpState() {
  return digitalRead(FINISH_UP_PIN);
}

//Р СџР С•Р В»РЎС“РЎвЂЎР ВµР Р…Р С‘Р Вµ Р В·Р Р…Р В°РЎвЂЎР ВµР Р…Р С‘РЎРЏ РЎвЂљР ВµР С�Р С—Р ВµРЎР‚Р В°РЎвЂљРЎС“РЎР‚РЎвЂ№ Р С‘Р В· ds18b20 Р Р† РЎвЂ Р ВµР В»РЎРЉРЎРѓР С‘РЎРЏРЎвЂ¦
float getTempFromOneWire() {
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

//Р СџР С•Р В»РЎС“РЎвЂЎР ВµР Р…Р С‘Р Вµ Р Т‘Р С‘РЎРѓРЎР‚Р ВµРЎвЂљР Р…Р С•Р С–Р С• Р В·Р Р…Р В°РЎвЂЎР ВµР Р…Р С‘РЎРЏ Р Р†Р В»Р В°Р В¶Р Р…Р С•РЎРѓРЎвЂљР С‘
//1 - РЎРѓРЎС“РЎвЂ¦Р С•
int getHydroState() {
  return digitalRead(HYDRO_PIN);
}

//Р СџР С•Р В»РЎС“РЎвЂЎР ВµР Р…Р С‘Р Вµ Р Т‘Р С‘РЎРѓРЎР‚Р ВµРЎвЂљР Р…Р С•Р С–Р С• Р В·Р Р…Р В°РЎвЂЎР ВµР Р…Р С‘РЎРЏ РЎС“РЎР‚Р С•Р Р†Р Р…РЎРЏ
//1 - Р Р€РЎР‚Р С•Р Р†Р ВµР Р…РЎРЉ Р Т‘Р С•РЎРѓРЎвЂљР С‘Р С–Р Р…РЎС“РЎвЂљ
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
    buffer[i * 2 + 0] = nib1  < 0xA ? '0' + nib1  : 'A' + nib1  - 0xA;
    buffer[i * 2 + 1] = nib2  < 0xA ? '0' + nib2  : 'A' + nib2  - 0xA;
  }
  buffer[len * 2] = '\0';
}

String incomingByte;
char buff[256] = "";

void loop() {
  Serial.println(millis());
  Serial.println();

  Serial.print("avalible on serial1: ");
  Serial.println(Serial1.available());

  if (Serial1.available()) {
    
    Serial1.readBytes(buff, Serial1.available());

    Serial.print("I received: ");
    Serial.println(buff);
    
    Serial1.println("STATUS"); 
    Serial1.println("STATUS"); 
    Serial1.println("STATUS");  
  }

  //  processSolarPanerLogic();
  //  processLightLogic();
  //  processHydroLogic();
  //  processLevelLogic();
  //  processRedLightLogic();
  //  processTermoLogic();
  //
  Serial.println("-----------------------");
  delay(500);
  //delay(5000);
}


