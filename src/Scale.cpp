#include <HX711_ADC.h>
#include "include/Scale.h"
#include "include/Config.h"

#if defined(ESP8266) || defined(ESP32) || defined(AVR)
#include <EEPROM.h>
#endif

void calibrate();
void changeSavedCalFactor();

// pins:
const int HX711_dout = 33; // mcu > HX711 dout pin
const int HX711_sck = 32;  // mcu > HX711 sck pin

// HX711 constructor:
HX711_ADC LoadCell(HX711_dout, HX711_sck);

const int calVal_eepromAdress = 0;
const int tareOffsetVal_eepromAdress = 4;
unsigned long t = 0;

void ScaleInit()
{
	LoadCell.begin();
	// LoadCell.setGain(64);
	float calibrationValue;	  // calibration value (see example file "Calibration.ino")

	EEPROM.get(calVal_eepromAdress, calibrationValue); // uncomment this if you want to fetch the calibration value from eeprom

	// restore the zero offset value from eeprom:
	long tare_offset = 0;
	EEPROM.get(tareOffsetVal_eepromAdress, tare_offset);
	LoadCell.setTareOffset(tare_offset);
	boolean _tare = false; // set this to false as the value has been resored from eeprom

	unsigned long stabilizingtime = 2000; // preciscion right after power-up can be improved by adding a few seconds of stabilizing time
	LoadCell.start(stabilizingtime, _tare);
	if (LoadCell.getTareTimeoutFlag())
	{
		Serial.println("Timeout, check MCU>HX711 wiring and pin designations");
		while (1)
			;
	}
	else
	{
		LoadCell.setCalFactor(calibrationValue); // set calibration value (float)
		Serial.println("Startup is complete");
	}

	while (!LoadCell.update())
		;
}

float ScaleRead()
{
	static boolean newDataReady = 0;
	const int serialPrintInterval = 250; // increase value to slow down serial print activity
	static float reading = 0;

	// check for new data/start next conversion:
	if (LoadCell.update())
		newDataReady = true;

	// get smoothed value from the dataset:
	if (newDataReady)
	{
		if (millis() > t + serialPrintInterval)
		{
			float i = LoadCell.getData();
			// Serial.print("Load_cell output val: ");
			// Serial.println(i);
			newDataReady = 0;
			reading = i;
			t = millis();
		}
	}

	// receive command from serial terminal, send 't' to initiate tare operation:
	if (Serial.available() > 0)
	{
		char inByte = Serial.read();
		if (inByte == 't')
			ResetTare();
		else if (inByte == 'c')
			calibrate();
	}

	return reading;
}

float ToGasLevel(float weight)
{
	auto &config = GetConfig();
	float ratio = 100 / config.GetWeight();
	float gasWeight = weight - config.GetTare();

	return gasWeight * ratio;
}

bool HasTank(float weight)
{
	auto &config = GetConfig();
	return weight > config.GetTare();
}

// zero offset value (tare), calculate and save to EEprom:
void ResetTare()
{
	long _offset = 0;
	Serial.println("Calculating tare offset value...");
	LoadCell.tare();								 // calculate the new tare / zero offset value (blocking)
	_offset = LoadCell.getTareOffset();				 // get the new tare / zero offset value
	EEPROM.put(tareOffsetVal_eepromAdress, _offset); // save the new tare / zero offset value to EEprom
#if defined(ESP8266) || defined(ESP32)
	EEPROM.commit();
#endif
	LoadCell.setTareOffset(_offset); // set value as library parameter (next restart it will be read from EEprom)
	Serial.print("New tare offset value:");
	Serial.print(_offset);
	Serial.print(", saved to EEprom adr:");
	Serial.println(tareOffsetVal_eepromAdress);
}

void calibrate()
{
	Serial.println("***");
	Serial.println("Start calibration:");
	Serial.println("Place the load cell an a level stable surface.");
	Serial.println("Remove any load applied to the load cell.");
	Serial.println("Send 't' from serial monitor to set the tare offset.");

	boolean _resume = false;
	while (_resume == false)
	{
		LoadCell.update();
		if (Serial.available() > 0)
		{
			if (Serial.available() > 0)
			{
				char inByte = Serial.read();
				if (inByte == 't')
					LoadCell.tareNoDelay();
			}
		}
		if (LoadCell.getTareStatus() == true)
		{
			Serial.println("Tare complete");
			_resume = true;
		}
	}

	Serial.println("Now, place your known mass on the loadcell.");
	Serial.println("Then send the weight of this mass (i.e. 100.0) from serial monitor.");

	float known_mass = 0;
	_resume = false;
	while (_resume == false)
	{
		LoadCell.update();
		if (Serial.available() > 0)
		{
			known_mass = Serial.parseFloat();
			if (known_mass != 0)
			{
				Serial.print("Known mass is: ");
				Serial.println(known_mass);
				_resume = true;
			}
		}
	}

	LoadCell.refreshDataSet();
	float newCalibrationValue = LoadCell.getNewCalibration(known_mass);

	Serial.print("New calibration value has been set to: ");
	Serial.print(newCalibrationValue);
	Serial.println(", use this as calibration value (calFactor) in your project sketch.");
	Serial.print("Save this value to EEPROM adress ");
	Serial.print(calVal_eepromAdress);
	Serial.println("? y/n");

	_resume = false;
	while (_resume == false)
	{
		if (Serial.available() > 0)
		{
			char inByte = Serial.read();
			if (inByte == 'y')
			{
#if defined(ESP8266) || defined(ESP32)
				EEPROM.begin(512);
#endif
				EEPROM.put(calVal_eepromAdress, newCalibrationValue);
#if defined(ESP8266) || defined(ESP32)
				EEPROM.commit();
#endif
				EEPROM.get(calVal_eepromAdress, newCalibrationValue);
				Serial.print("Value ");
				Serial.print(newCalibrationValue);
				Serial.print(" saved to EEPROM address: ");
				Serial.println(calVal_eepromAdress);
				_resume = true;
			}
			else if (inByte == 'n')
			{
				Serial.println("Value not saved to EEPROM");
				_resume = true;
			}
		}
	}

	Serial.println("End calibration");
	Serial.println("***");
	Serial.println("To re-calibrate, send 'r' from serial monitor.");
	Serial.println("For manual edit of the calibration value, send 'c' from serial monitor.");
	Serial.println("***");
}

void changeSavedCalFactor()
{
	float oldCalibrationValue = LoadCell.getCalFactor();
	boolean _resume = false;
	Serial.println("***");
	Serial.print("Current value is: ");
	Serial.println(oldCalibrationValue);
	Serial.println("Now, send the new value from serial monitor, i.e. 696.0");
	float newCalibrationValue;
	while (_resume == false)
	{
		if (Serial.available() > 0)
		{
			newCalibrationValue = Serial.parseFloat();
			if (newCalibrationValue != 0)
			{
				Serial.print("New calibration value is: ");
				Serial.println(newCalibrationValue);
				LoadCell.setCalFactor(newCalibrationValue);
				_resume = true;
			}
		}
	}
	_resume = false;
	Serial.print("Save this value to EEPROM adress ");
	Serial.print(calVal_eepromAdress);
	Serial.println("? y/n");
	while (_resume == false)
	{
		if (Serial.available() > 0)
		{
			char inByte = Serial.read();
			if (inByte == 'y')
			{
#if defined(ESP8266) || defined(ESP32)
				EEPROM.begin(512);
#endif
				EEPROM.put(calVal_eepromAdress, newCalibrationValue);
#if defined(ESP8266) || defined(ESP32)
				EEPROM.commit();
#endif
				EEPROM.get(calVal_eepromAdress, newCalibrationValue);
				Serial.print("Value ");
				Serial.print(newCalibrationValue);
				Serial.print(" saved to EEPROM address: ");
				Serial.println(calVal_eepromAdress);
				_resume = true;
			}
			else if (inByte == 'n')
			{
				Serial.println("Value not saved to EEPROM");
				_resume = true;
			}
		}
	}
	Serial.println("End change calibration value");
	Serial.println("***");
}