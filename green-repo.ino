/*
  Проект "Теплица" УТЭК
  Разработчик прошивки Лысов А.Д.
  Написано под микроконтроллер STM32F103C(20k RAM, 64k Flash)
  Версия кода v0.2
*/

#define DEBUG true

#define ESP_COM Serial //Serial1

#include <OneWire.h>

//Солнечная батарея и вентилятор
#define SOLAR_BATTERY_PIN PA1
#define FAN_PIN PB12

//LED лента
#define LED_PIN PB3

//1wire pin, два концевика и мотор створок
OneWire ds(PA3);
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

bool manualMode = false;

void setup() {
  Serial.begin(115200);
  Serial1.begin(115200);

  pinMode(SOLAR_BATTERY_PIN, INPUT_ANALOG);
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

/*
 * Геттеры состояний
 */

int getLedState(){
  return digitalRead(LED_PIN);
}

int getPumpWateringState(){
  return digitalRead(PUMP_WATERING_PIN);
}

int getPumpReturnState(){
  return digitalRead(PUMP_RETURN_PIN);
}

//Получение дисретного значения уровня
//1 - Уровень достигнут
int getLevelState(){
  return digitalRead(LEVEL_PIN);
}

//Кэшированное значение температуры. Задается в начале цикла и исползуется на его протяжении.
float _tempCacheValue;
float getTempState(){
  return _tempCacheValue;
}

//Получение дисретного значения влажности
//1 - сухо
int getHydroState(){
  return digitalRead(HYDRO_PIN);
}

int getFanState(){
  return digitalRead(FAN_PIN);
}

//Получение состояния солнечной панели в процентах %
int getSolarState(){
  return analogRead(SOLAR_BATTERY_PIN) * 100 / 4095;
}

//Получение значения нижнего конечного выключателя
int getFinishDownState(){
  return digitalRead(FINISH_DOWN_PIN);
}

//Получение значения верхнего конечного выключателя
int getFinishUpState(){
  return digitalRead(FINISH_UP_PIN);
}

String WIN_STATES[3] = { "stop", "left", "right" };
int _winDriveStateIndex = 0;
String getWinDriveState(){
  return WIN_STATES[_winDriveStateIndex];
}

int getRedLedState(){
  return digitalRead(RED_LIGHT_PIN);
}

/*
 * Специальные геттеры
 */

//Вентилятор - Солнечная панель
bool isSolarLow() {
  int procent = getSolarState();
  return procent < 70;
}

bool getTermoStatus() {
  float term = getTempState();
  Serial.print("TEMP cel = ");
  Serial.println(term);
  Serial.print("TEMP STATE = ");
  Serial.println(term > 33.0);
  return term > 33.0;
}

/*
 * Сеттеры состояний
 */
 
void setLedState(int value){
  digitalWrite(LED_PIN, value);
}

void setPumpWateringState(int value){
  digitalWrite(PUMP_WATERING_PIN, value);
}

bool setPumpReturnState(int value){
  if (value && getLevelState()) {
    return false;
  }
  
  digitalWrite(PUMP_RETURN_PIN, value);
  return true;
}

void setFanState(int value){
  digitalWrite(FAN_PIN, value);
}

//1 - ошибка left
int setWinDriveState(String state){
  bool stateR = false;
  bool stateL = false;
   
  if (state.equals("left")) {
    //LEFT
    if (manualMode && getFinishDownState()) {
      return 1;
    }
    
    _winDriveStateIndex = 1;
    stateR = false;
    stateL = true;
  } else if (state.equals("right")){
    //RIGHT
    if (manualMode && getFinishUpState()) {
      return 2;
    }

    _winDriveStateIndex = 2;
    stateR = true;
    stateL = false;
  } else {
    //STOP
    _winDriveStateIndex = 0;
    stateR = false;
    stateL = false;
  }
  
  digitalWrite(DRIVE_WINDOWS_PIN_R, stateR);
  digitalWrite(DRIVE_WINDOWS_PIN_R, stateL);

  return 0;
}

void setRedLedState(int value){
  digitalWrite(RED_LIGHT_PIN, value);
}

/*
 * Обработка логики
 */

void setManualMode(bool mode) {
  manualMode = mode;

  if (mode) {
      setPumpWateringState(0);
      setPumpReturnState(0);
  }
}
 
//Получение значения температуры из ds18b20 в цельсиях
void updateTempCacheFromOneWire() { 
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
  ds.write(0x44, 1); // start conversion, with parasite power on at the end

  delay(1000); // maybe 750ms is enough, maybe not
  // we might do a ds.depower() here, but the reset will take care of it.

  present = ds.reset();
  ds.select(addr);
  ds.write(0xBE); // Read Scratchpad
  for ( i = 0; i < 9; i++) { // we need 9 bytes
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
    if (cfg == 0x00) raw = raw & ~7; // 9 bit resolution, 93.75 ms
    else if (cfg == 0x20) raw = raw & ~3; // 10 bit res, 187.5 ms
    else if (cfg == 0x40) raw = raw & ~1; // 11 bit res, 375 ms
    //// default is 12 bit resolution, 750 ms conversion time
  }
  celsius = (float)raw / 16.0;
  _tempCacheValue = celsius;
}

//Красный светодиод
long lastMilli = 0;
bool redLightOn = false;
void processRedLightLogic() {
  long milli = millis();
  if (milli - lastMilli >= 60 * 1000) {
    lastMilli = milli;
    redLightOn = !redLightOn;
    setRedLedState(redLightOn);
  }
}

//Датчик уровня в емкости - Насос откачки
void processLevelLogic() {
  int levelState = getLevelState();
  
  Serial.print("LEVEL_PIN digital = ");
  Serial.println(levelState);
  Serial.print("---levelState bool = ");
  Serial.println(levelState == 0);

  setPumpReturnState(levelState == 0);
}

void processSolarPanerLogic() {
  bool solar = isSolarLow();
  
  setFanState(!solar);
  setLedState(solar);
}

//STATE{ "stop", "left", "right" };
void processDriveLogic() {
  int up = getFinishUpState();
  int down = getFinishDownState();

  Serial.print("FINISH_UP_PIN digital = ");
  Serial.println(up);
  Serial.print("FINISH_DOWN_PIN digital = ");
  Serial.println(down);

  if (_winDriveStateIndex == 2 && up) {
    
    _winDriveStateIndex = 0;
    
  } else if (_winDriveStateIndex == 1 && down) {

    _winDriveStateIndex = 0;
    
  }
   
  setWinDriveState(WIN_STATES[_winDriveStateIndex]);
}

void processTermoLogic() {
  bool termoStatus = getTermoStatus();
  int up = getFinishUpState();
  int down = getFinishDownState();

  Serial.print("---TermoState bool = ");
  Serial.println(termoStatus);

  if (termoStatus && down) {
    Serial.println("------Drive to RIGHT OPEN");
    _winDriveStateIndex = 2;
  } else if (!termoStatus && up) {
    Serial.println("------Drive to LEFT CLOSE");
    _winDriveStateIndex = 1;
  } else {
    Serial.println("------Drive STOP"); 
    _winDriveStateIndex = 0;
  }
}

void processHydroLogic() {
  int needWattering = getHydroState();
  
  Serial.print("HYDRO_PIN dig = ");
  Serial.println(needWattering);

  setPumpWateringState(needWattering);
}

void processAutoMode() {
  processRedLightLogic();
  processLevelLogic();
  processSolarPanerLogic();
  processTermoLogic();
  processHydroLogic();
}

/*
 * Формирование ответов для ESP
 */

String getValueJson(String title, String value, bool isEnd){
  String json =  "\"" + title + "\":\"" + value + "\"";

  if (!isEnd) {
    json += ",";
  }

  return json;
}

String getValueJson(String title, int value, bool isEnd){
  return getValueJson(title, String(value, DEC), isEnd);
}

String getResponseJson(String title, String value){
  return "{\"" + title + "\":\"" + value + "\"}";
}

String getResponseJson(String title, int value){
  return getResponseJson(title, String(value, DEC));
}

String getErrorResponse(String error){
  return "{\"error\":\"" + error + "\"}";
}


/*
 * Обработка запросов от ESP
 */

String parseValue(String param, String node) {
  param.replace(node, "");
  param.trim();
  return param;
}

String parseParams(String request, String command) {
  request.replace(command, "");
  request.trim();
  return request;
}

String processSetMode(String param) {
  if (param.equalsIgnoreCase("manual")) {

    setManualMode(true);
    return getResponseJson("manualMode", "manual");
    
  } else {

    setManualMode(false);
    return getResponseJson("manualMode", "auto");
  }
}

String processGetState(String param) {
   if (param.equalsIgnoreCase("led")) {
      
      return getResponseJson("led", getLedState());
      
   } else if (param.equalsIgnoreCase("pump_watering")) {
    
      return getResponseJson("pump_watering", getPumpWateringState());
      
   } else if (param.equalsIgnoreCase("pump_return")) {
    
      return getResponseJson("pump_return", getPumpReturnState());
      
   } else if (param.equalsIgnoreCase("level")) {
    
      return getResponseJson("level", getLevelState());
      
   } else if (param.equalsIgnoreCase("temp")) {
    
      return getResponseJson("temp", getTempState());
      
   } else if (param.equalsIgnoreCase("hydro")) {
    
      return getResponseJson("hydro", getHydroState());
      
   } else if (param.equalsIgnoreCase("fan")) {
    
      return getResponseJson("fan", getFanState());
      
   } else if (param.equalsIgnoreCase("solar")) {
    
      return getResponseJson("solar", getSolarState());
      
   } else if (param.equalsIgnoreCase("finish_down")) {
    
      return getResponseJson("finish_down", getFinishDownState());
      
   } else if (param.equalsIgnoreCase("finish_up")) {
    
      return getResponseJson("finish_up", getFinishUpState());
      
   } else if (param.equalsIgnoreCase("win_drive")) {
    
      return getResponseJson("win_drive", getWinDriveState());
      
   } else if (param.equalsIgnoreCase("red_led")) {
    
      return getResponseJson("red_led", getRedLedState());
      
   }  else if (param.equalsIgnoreCase("all")) {

      String response = "{";

      response += getValueJson("led", getLedState(), false);
      response += getValueJson("pump_watering", getPumpWateringState(), false);
      response += getValueJson("level", getLevelState(), false);
      response += getValueJson("temp", getTempState(), false);
      response += getValueJson("hydro", getHydroState(), false);
      response += getValueJson("fan", getFanState(), false);
      response += getValueJson("solar", getSolarState(), false);
      response += getValueJson("finish_down", getFinishDownState(), false);
      response += getValueJson("finish_up", getFinishUpState(), false);
      response += getValueJson("win_drive", getWinDriveState(), false);
      response += getValueJson("red_led", getRedLedState(), true);

      return response + "}";
   } else {
    
      return getErrorResponse("Incorrect param for GetState");
   } 
}

String processSetState(String param) {
     if (param.equalsIgnoreCase("led")) {

      String value =  parseValue(param, "led");
      setLedState(value.toInt());
      
      return getResponseJson("led", value);
      
   } else if (param.equalsIgnoreCase("pump_watering")) {
    
      String value =  parseValue(param, "pump_watering");
      setPumpWateringState(value.toInt());
      
      return getResponseJson("pump_watering", value);
      
   } else if (param.equalsIgnoreCase("pump_return")) {

      String value =  parseValue(param, "pump_return");
      bool resp = setPumpReturnState(value.toInt());

      if (!resp) {
        return getErrorResponse("pump_return cannot set HIGH, because level is HIGH");
      }
      
      return getResponseJson("pump_return", value);
      
   } else if (param.equalsIgnoreCase("fan")) {

      String value =  parseValue(param, "fan");
      setFanState(value.toInt());
      
      return getResponseJson("fan", value);
      
   } else if (param.equalsIgnoreCase("win_drive")) {

      String value =  parseValue(param, "win_drive");
      int resp = setWinDriveState(value);

      if (resp == 1) {
        return getErrorResponse("win_drive cannot set LEFT, because finish_down is HIGH");
      } else if (resp == 2) {
        return getErrorResponse("win_drive cannot set HIGH, because finish_up is HIGH");
      }
      
      return getResponseJson("win_drive", value);
      
   } else if (param.equalsIgnoreCase("red_led")) {

      String value =  parseValue(param, "red_led");
      setRedLedState(value.toInt());
      
      return getResponseJson("red_led", value);
      
   } else {
    
      return getErrorResponse("Incorrect param for SetState");
   } 
}

String routeRequest(String request) {
  String response;

  if (request.indexOf("setmode") != -1) {
    
    response = processSetMode(parseParams(request, "SetMode"));
    
  } else if (request.indexOf("getstate") != -1) {
    
    response = processGetState(parseParams(request, "GetState"));
    
  }  else if (request.indexOf("setstate") != -1) {
    
    response = processSetState(parseParams(request, "SetState"));
    
  } else {
    
    response = getErrorResponse("Incorrect request");
  }
  
  return response;
}

char buff[256] = "";

void loop() {
  Serial.println(millis());
  Serial.println();

  updateTempCacheFromOneWire();

  Serial.print("avalible on serial1: ");
  Serial.println(ESP_COM.available());

  if (ESP_COM.available()) {

    ESP_COM.readBytes(buff, ESP_COM.available());

    String response = routeRequest(String(buff));

    ESP_COM.println(response);
  }

  if (!manualMode) {
    processAutoMode();
  }

  processDriveLogic();

  Serial.println("-----------------------");
  delay(500);
}
