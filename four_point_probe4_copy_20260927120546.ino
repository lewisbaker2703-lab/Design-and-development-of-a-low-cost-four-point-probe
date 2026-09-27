#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#define MATERIALS 3
#define TESTS 3
#include <Adafruit_ADS1X15.h>
const int thermistor = A0;
const byte buttonpin = 2;
const byte resetpin = 3;
const float current = 0.0056;
int materials = 0;
int tests = 0;
LiquidCrystal_I2C lcd(0x27,16,2);
Adafruit_ADS1X15 ads;
byte currentgain = 5;
volatile bool resetFlag = false;
float length[2] = {0.010, 0.010}; // nail and pencil lead
float radius[2] = {0.002, 0.001}; // nail and penicl lead
float thickness[1] = {0.000874}; // aluminum can
adsGain_t gainplace[6] {
  GAIN_TWOTHIRDS,
  GAIN_ONE,
  GAIN_TWO,
  GAIN_FOUR,
  GAIN_EIGHT,
  GAIN_SIXTEEN,
};

 float conversionfactor[6] {
  0.0001875,
  0.000125,
  0.0000625,
  0.00003125,
  0.000015625,
  0.0000078125,
 };

 float resistanceValue[MATERIALS][TESTS];
 float voltageValue[MATERIALS][TESTS];
 void sendall();
 void resetISR();
 void performreset();
 void buttonWait();
 void adjustGain();
 int16_t readADC();
 float calculateVoltage(int16_t adc);
 float calculateResistance(float voltage);
 float calculateResistivity(float resistance, float voltage, float current, int materials);
 float calculateConductivity(float restivityValue);
 float temperature();
 void displayLCD();

void setup() {
  pinMode(buttonpin,INPUT_PULLUP);
  pinMode(resetpin, INPUT_PULLUP);
  Serial.begin(9600);
  Wire.begin();
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0,0);
  lcd.print("probe");
  lcd.setCursor(0,1);
  lcd.print("is ready");
  if (!ads.begin(0x48)) {
    Serial.println("ads1115 not found");
    lcd.clear();
    lcd.print("ads error");
    while(1);
  }  
  Serial.println("ads1115 found");
  ads.setGain(gainplace[currentgain]);
  delay(2000);
  attachInterrupt(digitalPinToInterrupt(3), resetISR, FALLING);
}
void resetISR() {
  resetFlag = true;
}
void performreset(){
  for (int m = 0; m < MATERIALS; m++) {
    for (int t = 0; t < TESTS; t++) {
      resistanceValue[m][t] = 0.0;
    }
  }
  materials = 0;
  tests = 0;
  currentgain = 5;
  ads.setGain(gainplace[currentgain]);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("reset!!!");
  delay(1500);
  Serial.println("reset!!!");
  lcd.setCursor(0,0);
  lcd.print("probe");
  lcd.setCursor(0,1);
  lcd.print("is ready");
}
bool buttonwait() {
  while(digitalRead(buttonpin) == HIGH) 
  {
    if (resetFlag) {
      return false;
    }
delay(200);
  }
  delay(500);
  return true;
}

float adjustGain(int16_t adcReading) {
  
  if (adcReading >= 28000 && currentgain > 0) {
  Serial.print("reducing gain");
  currentgain--;
  ads.setGain(gainplace[currentgain]);
  } else if (adcReading <= 8000 && currentgain < 5) {
    Serial.print("increasing gain");
    currentgain++;
    ads.setGain(gainplace[currentgain]);
  }
  return adcReading;
}

int16_t readADC() {
  int16_t adcValue;
adcValue = ads.readADC_Differential_0_1();
Serial.println("adc read complete");
return adcValue;
}

float calculateVoltage(int16_t adcValue){
  float voltage;
  voltage = adcValue * conversionfactor[currentgain];
  return fabs(voltage);
}

float calculateResistance(float voltage) {
float resistance;
resistance = fabs(voltage) / current;
return resistance;
}

float calculateResistivity(float voltage, float resistance, float current, int materials) {
  float resistivityValue;
  if (materials < 2) {
    float area = PI * radius[materials] * radius[materials];
    resistivityValue = (resistance * area) / length[materials];
  } else {
    float sheet_resistance = (PI / log(2) * (voltage / current));
    resistivityValue = sheet_resistance * thickness[materials - 2];
  }
  return resistivityValue;
}

float calculateConductivity(float resistivityValue) {
  float conductivityValue = 1.0 / resistivityValue;
  return conductivityValue;
  }

float temperature() {
  int reading = analogRead(thermistor);
  float temperature = 1.0 / (
  (1/298.15) + 
  (1/3950.0) * 
  log(reading / 10000.0)
  )- 273.15;
  return temperature;
} 
void displaylcd() {
lcd.clear();
lcd.setCursor(0,0);
lcd.print("press buttton");
lcd.setCursor(0,1);
lcd.print("to measure");
}

void readinglcd() {
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("testing");
}
void finishedlcd(){
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("finished");
}
void sendall() {
  for (int materials = 0; materials < MATERIALS; materials++) {
    for (int tests = 0; tests < TESTS; tests++) {
      float voltage = voltageValue[materials][tests];
      float resistance = resistanceValue[materials][tests];
      float resistivity = calculateResistivity(voltage, resistance, current, materials);
      float conductivity = calculateConductivity(resistivity);
      float temp = temperature();
      Serial.print(materials);
      Serial.print(",");
      Serial.print(tests);
      Serial.print(",");
      Serial.print(voltage, 12);
      Serial.print(",");
      Serial.print(resistance, 12);
      Serial.print(",");
      Serial.print(resistivity, 12);
      Serial.print(",");
      Serial.print(conductivity, 12);
      Serial.print(",");
      Serial.println(temp);
    }
  }
  Serial.println("DONE");
}
void loop() {
  if (resetFlag) {
    resetFlag = false;
    performreset();
    return;
  }
for (materials = 0; materials < MATERIALS; materials++) {
  for(tests = 0; tests < TESTS; tests++){

displaylcd();
if (!buttonwait()) {
      resetFlag = false;
      performreset();
      return;
    }
readinglcd();
delay(500);
  int16_t adcValue = readADC();
  adjustGain(adcValue);
  adcValue = readADC();
  float voltage = calculateVoltage(adcValue);
  voltageValue[materials][tests] = voltage;
  resistanceValue[materials][tests] = calculateResistance(voltage);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Test ");
  lcd.print(tests + 1);
  lcd.print(" complete");
  lcd.setCursor(0,1);
  lcd.print("Next test");
  delay(1000);
 }  
}
  finishedlcd();
  if (!buttonwait()) {

  resetFlag = false;
  performreset();

  return;
}
  delay(1000);

  sendall();
  while (!resetFlag) {
    delay(100);
  }
}

  

















