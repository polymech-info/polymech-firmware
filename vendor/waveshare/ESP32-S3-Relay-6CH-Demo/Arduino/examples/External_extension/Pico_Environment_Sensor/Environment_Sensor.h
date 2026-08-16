#ifndef _PICO_ENVIRONMENT_SENSOR_H_
#define _PICO_ENVIRONMENT_SENSOR_H_

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <Adafruit_TSL2591.h>
#include <Adafruit_LTR390.h>
#include <Adafruit_SGP40.h>

#define I2C_SDA       4
#define I2C_SCL       5
#define LTR390_MODE   LTR390_MODE_ALS               // LTR390_MODE_UVS(UV) or LTR390_MODE_ALS(Ambient light) modes measure UV or ambient light

/*********************************************** BME280 ***********************************************/
void BME280_Init();
void BME280_Loop();
/*********************************************** TSL2591 ***********************************************/
void TSL2591_Init();
void TSL2591_Loop();
/*********************************************** LTR390 ***********************************************/
void LTR390_Init();
void LTR390_Loop();
/*********************************************** SGP40 ***********************************************/
void SGP40_Init();
void SGP_Loop();
void SGP_Loop_Precision(float temperature,float humidity);
/*********************************************** MAIN ***********************************************/
void MAIN_Init();
void MAIN_Loop();         // 


#endif
