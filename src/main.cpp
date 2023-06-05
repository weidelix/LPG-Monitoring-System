#include <Arduino.h>
#include <deque>
#include "include/WebServer.h"
#include "include/Config.h"
#include "include/Scale.h"
#include "include/SMS.h"

#define ONEDAY 86400000UL

unsigned long startTime = 0;
std::deque<float> readings;

void setup()
{
#if defined(ESP8266) || defined(ESP32)
	EEPROM.begin(1024);
#endif

	Serial.begin(57600);
	SimInit();
	delay(10);
	Serial.println();
	Serial.println("Starting...");

	GetConfig().ReadConfig();
	ScaleInit();
	ServerInit();
	
	ResetTare();
	startTime = millis();

	// Read the last 30 readings from EEPROM from address 400
	for (int i = 0; i < 30; i++)
	{
		int reading = 0.0;
		EEPROM.get(400 + (5 * i), reading);
		readings.push_back(reading);
		Serial.println(reading);
	}
}

float prevWeight = 0;
float currWeight = 0;
void loop()
{
	Config &config = GetConfig();

	prevWeight = currWeight;
	currWeight = round(ScaleRead() * 100.0) / 100.0;

	float currLevel = ToGasLevel(currWeight);
	
	Status status 
	{
		currWeight,
		currLevel,
		currWeight - config.GetTare(),
		config.GetTare(),
		config.GetWeight(),
		0,
		0
	};

	if (HasTank(currWeight))
	{
		if (!config.warningSent && 
			currLevel <= config.warningLevel && 
			currWeight > 0.1 and currWeight < prevWeight)
		{
			config.warningSent = true;
			SendSMS(config.recipientOne, currLevel);
			delay(1000);
			SendSMS(config.recipientTwo, currLevel);
			delay(1000);
			SendSMS(config.recipientThree, currLevel);
			delay(1000);

			if (config.notifyExternalRecipient)
			{
				SendSMS(config.externalRecipientNumber, config.externalRecipientMessage);
			}
		}

		if (!config.criticalSent && currLevel < 6.0 && currWeight > 0.1 and currWeight < prevWeight)
		{
			config.criticalSent = true;
			SendSMS(config.recipientOne, currLevel);
			delay(1000);
			SendSMS(config.recipientTwo, currLevel);
			delay(1000);
			SendSMS(config.recipientThree, currLevel);
		}
	}
	else
	{
		config.warningSent = false;
		config.criticalSent = false;
	}

	if (millis() - startTime > 20000)
	{
		if (readings.size() > 30)
		{
			readings.pop_front();
			readings.push_back(currLevel);
		}
		else
		{
			readings.push_back(currLevel);
		}
		

		// Write the last 30 readings to EEPROM from address 400
		for (int i = 0; i < 30; i++)
		{
			EEPROM.put(400 + (i * 5), currLevel);
		}
		// Serial.println(currLevel);
		
		Serial.println(currWeight);
		// Truncate currLevel to 1 decimal place
		// currLevel = ;
		// Serial.println(currLevel);
		startTime = millis();
	}

	CheckConnections(status);
}
