#include "Environment_Sensor.h"

#define SEALEVELPRESSURE_HPA (1013.25)              // Standard atmospheric pressure is about 1013.25 hPa (black Pascals)

TwoWire I2C = TwoWire(1); 
Adafruit_BME280 bme;
Adafruit_TSL2591 tsl = Adafruit_TSL2591(2591);
Adafruit_LTR390 ltr390;
Adafruit_SGP40 sgp;  

/*********************************************** BME280 ***********************************************/
void BME280_Init() {
  if(!bme.begin(0x76,&I2C)) {
    printf("Could not find a valid BME280 sensor, check wiring!\r\n");
    delay(10);
  }
}
void BME280_Loop() {
  float temperature = bme.readTemperature();
  float humidity = bme.readHumidity();
  float pressure = bme.readPressure() / 100.0F;

  printf("BME280 ——  Temperature: %f °C \r\n",temperature);
  printf("BME280 ——  Humidity: %f % \r\n",humidity);
  printf("BME280 ——  Pressure: %f hPa \r\n",pressure);
  printf("\r\n");
}
/*********************************************** TSL2591 ***********************************************/
void TSL2591_Init() {
  if(!tsl.begin(&I2C,0x29)) {
    printf("Could not find a valid TSL2591 sensor, check wiring!\r\n");
    delay(10);
  }
  tsl.setGain(TSL2591_GAIN_MED);                
  tsl.setTiming(TSL2591_INTEGRATIONTIME_100MS);      
}
void TSL2591_Loop() {
  uint32_t TSL2591_Data = tsl.getFullLuminosity();   
  uint16_t full         = TSL2591_Data >> 16;        
  uint16_t ir           = TSL2591_Data & 0xFFFF;     

  printf("TSL2591 —— Full spectrum: %d \r\n",full);
  printf("TSL2591 —— Infrared: %d \r\n",ir);
  printf("\r\n");
}
/*********************************************** LTR390 ***********************************************/
void LTR390_Init() {
  if(!ltr390.begin(&I2C)) {
    printf("Could not find a valid LTR390 sensor, check wiring!\r\n");
  }
  ltr390.setMode(LTR390_MODE);                     
  ltr390.setGain(LTR390_GAIN_3);                    
  ltr390.setResolution(LTR390_RESOLUTION_16BIT);    

  ltr390.setThresholds(100, 1000);                  
  ltr390.configInterrupt(true, LTR390_MODE);        
}

void LTR390_Loop() {
  if(ltr390.newDataAvailable()) {                     
    if(LTR390_MODE == LTR390_MODE_UVS){
      float UVS = ltr390.readUVS();
      printf("LTR390 ——  UV data:  %f \r\n",UVS);
    }         
    else{
      float ALS = ltr390.readALS();       
      printf("LTR390 ——  ALS data:  %f \r\n",ALS);
    }
    printf("\r\n");
  }
}
/*********************************************** SGP40 ***********************************************/
void SGP40_Init() {         
  if(!sgp.begin(&I2C)) {                         
    printf("Could not find a valid SGP40 sensor, check wiring!\r\n");
  }   
}

void SGP_Loop() {
  uint16_t raw;
  int32_t voc_index;
  raw = sgp.measureRaw();                                            
  printf("SGP40 ——— Measurement: %d \r\n",raw);
  voc_index = sgp.measureVocIndex();                         
  printf("SGP40 ——— Voc Index: %d \r\n",voc_index);
  printf("\r\n");
}
void SGP_Loop_Precision(float temperature,float humidity) {
  uint16_t sraw;
  sraw = sgp.measureRaw(temperature, humidity);                
  printf("SGP40 —— Raw measurement: %d \r\n",sraw);
  int32_t voc_index;
  voc_index = sgp.measureVocIndex(temperature, humidity);      
  printf("SGP40 —— Voc Index: %d \r\n",voc_index);
  printf("\r\n");
}

/*****************************************************************************************************
MAIN
*****************************************************************************************************/
void MAIN_Init() {
  I2C.begin(I2C_SDA, I2C_SCL); // SDA-Pin, SCL-Pin
  BME280_Init();
  TSL2591_Init();
  LTR390_Init();
  SGP40_Init();
}

void MAIN_Loop() {
  printf("\r\n");
  printf("|********************************************* NEW Data *********************************************|\r\n");
  BME280_Loop();
  TSL2591_Loop();
  LTR390_Loop();
  SGP_Loop();
  printf("|******************************************** Data Over *********************************************|\r\n");
}

