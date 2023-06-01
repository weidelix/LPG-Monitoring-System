#include <Arduino.h>
#include "include/WebServer.h"
#include "include/Config.h"
#include "include/Scale.h"
#include "include/SMS.h"

#define ONEDAY 86400000UL

unsigned long startTime = 0;
float readings[30] = {0};

void saveConfigToEEPROM();
void readConfigInEEPROM();

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
		EEPROM.get(400 + (i * sizeof(float)), readings[i]);
	}

	// Print the last 30 readings
	for (int i = 0; i < 30; i++)
	{
		Serial.print("Reading ");
		Serial.print(i);
		Serial.print(" = ");
		Serial.println(readings[i]);
	}
}

float prevWeight = 0;
float currWeight = 0;
void loop()
{
	Config &config = GetConfig();

	prevWeight = currWeight;
	currWeight = ScaleRead();

	float currLevel = ToGasLevel(currWeight);
	Serial.println(currWeight);
	
// 	Status status 
// 	{
// 		currWeight,
// 		currLevel,
// 		currWeight - config.GetTare(),
// 		config.GetTare(),
// 		config.GetWeight(),
// 		0,
// 		0
// 	};

// 	if (HasTank(currWeight))
// 	{
// 		if (!config.warningSent && currLevel <= config.warningLevel && currWeight > 0.1 and currWeight < prevWeight)
// 		{
// 			config.warningSent = true;
// 			SendSMS(config.recipientOne, currLevel);
// 			delay(1000);
// 			SendSMS(config.recipientTwo, currLevel);
// 			delay(1000);
// 			SendSMS(config.recipientThree, currLevel);
// 			delay(1000);

// 			if (config.notifyExternalRecipient)
// 			{
// 				SendSMS(config.externalRecipientNumber, config.externalRecipientMessage);
// 			}
// 		}

// 		if (!config.criticalSent && currLevel < 6.0 && currWeight > 0.1 and currWeight < prevWeight)
// 		{
// 			config.criticalSent = true;
// 			SendSMS(config.recipientOne, currLevel);
// 			delay(1000);
// 			SendSMS(config.recipientTwo, currLevel);
// 			delay(1000);
// 			SendSMS(config.recipientThree, currLevel);
// 		}
// 	}
// 	else
// 	{
// 		config.warningSent = false;
// 		config.criticalSent = false;
// 	}

// 	if (millis() - startTime > 1000)
// 	{
// 		// Save readings to EEPROM starting from address 400
// 		for (int i = 0; i < 30; i++)
// 		{
// 			EEPROM.put(400 + i * sizeof(float), currLevel);
// 		}

// 		startTime = millis();
// 		Serial.println("Saved readings to EEPROM");
// 	}

// 	CheckConnections(status);
}
