#ifndef MagneticJammingFilter_h
#define MagneticJammingFilter_h

//--------------------------------------------------------------------------------------------
// Variable declaration
class MagneticJammingFilter{
private:
	float sampleFreq;

    bool firstRun = true;
    float magMagnitude = NAN;
    float magMagnitudeFiltered = 42.0f;
    float magMagnitudeFilteredMax = 62.0f;
    float magMagnitudeFilteredMin = 23.0f;
    
    float magJammingThreshold = 5.0f;
    bool magJammingActive = false;
    int magJammingCounter = 0;

//-------------------------------------------------------------------------------------------
// Function declarations
public:
    MagneticJammingFilter(void){}
    void begin(float sampleFrequency) { sampleFreq = sampleFrequency; }
	void update(float mx, float my, float mz) {
		magMagnitude = sqrtf(mx*mx + my*my + mz*mz);
	
		if(firstRun) {
			magMagnitudeFiltered = magMagnitude;
			firstRun = false;
		}
		if((magMagnitude < (magMagnitudeFiltered + (magJammingThreshold * 3))) && (magMagnitude > (magMagnitudeFiltered - (magJammingThreshold * 3)))) {
			magMagnitudeFiltered += ((magMagnitude - magMagnitudeFiltered) * 0.0001);
		}
		if(magMagnitudeFiltered > magMagnitudeFilteredMax)
			magMagnitudeFiltered = magMagnitudeFilteredMax;
		if(magMagnitudeFiltered < magMagnitudeFilteredMin)
			magMagnitudeFiltered = magMagnitudeFilteredMin;
		
		// Use IMU algorithm if magnetic jamming is occurring
		if(magJammingActive) {
			//Maintain magJammingActive for 1 second
			if(magJammingCounter++ < sampleFreq) {
				return;
			}
		}
		if((magMagnitude > (magMagnitudeFiltered + magJammingThreshold)) || (magMagnitude < (magMagnitudeFiltered - magJammingThreshold))) {
			magJammingCounter = 0;
			magJammingActive = true;
			return;
		}
		else {
			magJammingCounter = 0;
			magJammingActive = false;
		}
	}

    float getMagneticMagnitude() {
        return magMagnitude;
    }
    float getMagneticMagnitudeFiltered(){
    	return magMagnitudeFiltered;
    }
    bool getJammingStatus(){
    	return magJammingActive;
	} 
    void setMagneticJammingThreshold(float magneticJammingThreshold){
    	if(magneticJammingThreshold < 1.0f)
    		return;
    	if(magneticJammingThreshold > 1000.0f)
    		return;		
    	magJammingThreshold = magneticJammingThreshold;
    }    
};
#endif