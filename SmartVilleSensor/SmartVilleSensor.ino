
#include "SIM900.h"
#include <SoftwareSerial.h>
#include "inetGSM.h"
//#include <SPI.h>
//#include <SD.h>
#include <OneWire.h>
#include <DallasTemperature.h>

//#undef DEBUG_ON
#define DEBUG_ON

#define TEMP_ONE_WIRE_BUS 4
#define PIN_BOIA_1 8
#define PIN_BOIA_2 6
#define PIN_BOIA_3 5
#define TIME_ENVIO_DADOS 120000
#define SIM800_RESET_PIN 7
#define APN "timbrasil.com.br"
#define APN_USER "tim"
#define APN_PASS "tim"
#define PIN_RX_GSM 2
#define PIN_TX_GSM 3
#define PIN_CS_SD 10
#define LOG_FILE_NAME "log.txt"
#define PIN 1010

InetGSM inet;
char msg[70];
int numdata;
int i=0;
boolean gsmStarted=false;

long ultEnvioDados = 0; 
short valorBoia1 = LOW;
short valorBoia2 = LOW;
short valorBoia3 = LOW;
float temperatureValue = 0;

//File logFile;
OneWire oneWire(TEMP_ONE_WIRE_BUS);
DallasTemperature sensor(&oneWire);

int count = 0;

float readTemperatureCelsius(DallasTemperature sensor) {
  sensor.requestTemperatures();
  float tempValue = sensor.getTempCByIndex(0);
  return tempValue;
}

void resetGsm(){
  #ifdef DEBUG_ON
  Serial.println("Reseting gsm");
  #endif
  pinMode(SIM800_RESET_PIN, OUTPUT);
  digitalWrite(SIM800_RESET_PIN, HIGH);
  delay(10);
  digitalWrite(SIM800_RESET_PIN, LOW);
  delay(100);
  digitalWrite(SIM800_RESET_PIN, HIGH);
  delay(3000);
}

/*
void logToFile(int valorBoia1, int valorBoia2, int valorBoia3, float temperature){
  logFile = SD.open(LOG_FILE_NAME, FILE_WRITE);
  if (logFile) {
    logFile.write(valorBoia1);
    logFile.print("|");
    logFile.write(valorBoia2);
    logFile.print("|");
    logFile.write(valorBoia3);
    logFile.print("|");
    logFile.write(temperature);
    logFile.println();
    logFile.close();
  }else{
    #ifdef DEBUG_ON
    Serial.println("Failed to write to log.txt");
    #endif
  }
}
*/

void gsmInit(){
  if (inet.attachGPRS(APN, APN_USER, APN_PASS)){
    Serial.println("gsm=OK");
  }else{
    Serial.println("gsm=ERROR");
  }
  delay(1000);
  gsm.SimpleWriteln("AT+CIFSR");
  gsm.WhileSimpleRead();      
          
}

bool sendGsmData(int valorBoia1, int valorBoia2, int valorBoia3, float temperature){
  if(gsmStarted){
    //smartville.azurewebsites.net/api/sensors/CreateStatus/1?Boia1=1&Boia2=1&Boia3=1&Temperatura=30
    String data = "/api/sensors/CreateStatus/";
    data += "1";
    data += "?Boia1=";
    data += valorBoia1;
    data += "&Boia2=";
    data += valorBoia2;
    data += "&Boia3=";
    data += valorBoia3;
    data += "&Temperatura=";
    data += temperature;
    Serial.println(data);
    numdata=inet.httpGET("smartville.azurewebsites.net", 80, data.c_str(), msg, 70);
    
    Serial.println("Data: ");
    Serial.println(msg);
    gsm.WhileSimpleRead(); 
    if(numdata != 0){
      return true;
    }

  }else{
    #ifdef DEBUG_ON
    Serial.println("GSM not initialized");
    #endif
    return false;
  }

  return false;
  
}

void resetAndInitGSM(){
  Serial.println("Reseting GSM");
  delay(2000);
  resetGsm();
  delay(30000);
  
  
  if(gsm.begin(2400)){
    gsmStarted = true;
  }
  Serial.print("GSM started: ");
  Serial.println(gsmStarted);
  Serial.println("GSM Initing");
  gsmInit();
  delay(30000);
}

void setup() {
  delay(5000);
  Serial.begin(9600);
  while (!Serial) {
    ; 
  }

  pinMode(PIN_BOIA_1, INPUT_PULLUP);
  pinMode(PIN_BOIA_2, INPUT_PULLUP);
  pinMode(PIN_BOIA_3, INPUT_PULLUP);
  Serial.println("Init...");

  /*
  //SD Initialization
  if (!SD.begin(PIN_CS_SD)) {
    #ifdef DEBUG_ON
    Serial.println("SD initialization failed!");
    #endif
  }
  #ifdef DEBUG_ON
  Serial.println("SD initialization done.");
  #endif
  */

  resetAndInitGSM();
  
}

void loop() { 

  valorBoia1 = digitalRead(PIN_BOIA_1);
  valorBoia2 = digitalRead(PIN_BOIA_2);
  valorBoia3 = digitalRead(PIN_BOIA_3);
  temperatureValue = readTemperatureCelsius(sensor);

  
  
  long now = millis();
  if (now - ultEnvioDados > TIME_ENVIO_DADOS) {
    ultEnvioDados = now;

    Serial.print("Boias: ");
    Serial.println(valorBoia1);
    Serial.println(valorBoia2);
    Serial.println(valorBoia3);
    Serial.print("Temp: ");
    Serial.println(temperatureValue);
    
    
    //logToFile(valorBoia1, valorBoia2, valorBoia3, temperatureValue);
    sendGsmData(valorBoia1, valorBoia2, valorBoia3,temperatureValue);
    /*
    if(!sendGsmData(valorBoia1, valorBoia2, valorBoia3,temperatureValue)){
      resetAndInitGSM();
    }
    */
  }
  
  if (gsm.available()) {
    Serial.write(gsm.read());
  }
  if (Serial.available()) {
    gsm.SimpleWrite(Serial.read());
  }
}

