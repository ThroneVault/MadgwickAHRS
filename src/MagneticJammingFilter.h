#ifndef MagneticJammingFilter_h
#define MagneticJammingFilter_h
#include <Arduino.h>
#include <EEPROM.h>
#include <util/crc16.h>

#define NXP_MOTION_CAL_EEADDR  60
#define NXP_MOTION_CAL_SIZE    68

//--------------------------------------------------------------------------------------------
// Variable declaration
class MagneticJammingFilter{
private:
	float sampleFreq;

  float fieldIntensity = NAN;
  float fieldIntensityFiltered = 42.0f;
  float fieldIntensityFilteredMax = 62.0f;
  float fieldIntensityFilteredMin = 23.0f;
  
  float jammingThreshold = 5.0f;
  bool jammingActive = false;
  int jammingCounter = 0;

  float cal[16]; // 0-8=offsets, 9=field strength, 10-15=soft iron map
//-------------------------------------------------------------------------------------------
// Function declarations
public:
  MagneticJammingFilter(void){}
  
  void begin(float sampleFrequency) {    
    unsigned char buf[NXP_MOTION_CAL_SIZE];
    uint8_t i;
    uint16_t crc;
    
    sampleFreq = sampleFrequency; 
  
    for (i=0; i < NXP_MOTION_CAL_SIZE; i++) {
      buf[i] = EEPROM.read(NXP_MOTION_CAL_EEADDR + i);
    }
    crc = 0xFFFF;
    for (i=0; i < NXP_MOTION_CAL_SIZE; i++) {
      crc = _crc16_update(crc, buf[i]);
    }
    if (crc == 0 && buf[0] == 117 && buf[1] == 84) {
      //If a valid calibration exists, use it to tighten up the magnetic rejection
      memcpy(cal, buf+2, sizeof(cal));
      fieldIntensityFiltered = cal[9];
      fieldIntensityFilteredMax = cal[9] + 8;
      fieldIntensityFilteredMin = cal[9] - 8;
    }  
  }
  
	void update(float mx, float my, float mz) {
		fieldIntensity = sqrtf(mx*mx + my*my + mz*mz);

		if((fieldIntensity < (fieldIntensityFiltered + (jammingThreshold * 3))) && (fieldIntensity > (fieldIntensityFiltered - (jammingThreshold * 3)))) {
			fieldIntensityFiltered += ((fieldIntensity - fieldIntensityFiltered) * 0.0001);
		}
		if(fieldIntensityFiltered > fieldIntensityFilteredMax)
			fieldIntensityFiltered = fieldIntensityFilteredMax;
		if(fieldIntensityFiltered < fieldIntensityFilteredMin)
			fieldIntensityFiltered = fieldIntensityFilteredMin;
		
		// Use IMU algorithm if magnetic jamming is occurring
		if(jammingActive) {
			//Maintain jammingActive for 1 second
			if(jammingCounter++ < sampleFreq) {
				return;
			}
		}
		if((fieldIntensity > (fieldIntensityFiltered + jammingThreshold)) || (fieldIntensity < (fieldIntensityFiltered - jammingThreshold))) {
			jammingCounter = 0;
			jammingActive = true;
			return;
		}
		else {
			jammingCounter = 0;
			jammingActive = false;
		}
	}

  float getFieldIntensity() {
      return fieldIntensity;
  }
  
  float getFieldIntensityFiltered(){
    return fieldIntensityFiltered;
  }
  
  bool getJammingStatus(){
    return jammingActive;
	} 
  
  void setFieldIntensityWindow(float fieldIntensityWindow){
    if(fieldIntensityWindow < 1.0f)
      return;
    if(fieldIntensityWindow > 1000.0f)
      return;		
    jammingThreshold = fieldIntensityWindow/2;
  }    
};
#endif