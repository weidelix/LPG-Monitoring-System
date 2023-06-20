#include <Arduino.h>
#include <deque>
#include <SPIFFS.h>
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
	EEPROM.begin(512);
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

	if (!SPIFFS.begin())
	{
		Serial.println("An error has occured while mounting the File System.");
		return;
	}

	// Read readings from SPIFFS
	File file = SPIFFS.open("/readings.csv", FILE_READ);
	if (!file)
	{
		Serial.println("Failed to open file for reading");
		return;
	}

	String line = file.readStringUntil('\n');
	file.close();
	
	int start = 0;
	int end = 0;
	while (end != -1)
	{
		end = line.indexOf(',', start);
		if (end != -1)
		{
			String reading = line.substring(start, end);
			readings.push_back(reading.toFloat());
			start = end + 1;
		}
		else
		{
			String reading = line.substring(start);
			readings.push_back(reading.toFloat());
		}
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
			float gasWeight = config.GetWeight();
			SendSMS(config.recipientOne, currLevel, currWeight, gasWeight);
			delay(1000);
			SendSMS(config.recipientTwo, currLevel, currWeight, gasWeight);
			delay(1000);
			SendSMS(config.recipientThree, currLevel, currWeight, gasWeight);
			delay(1000);

			if (config.notifyExternalRecipient)
			{
				SendSMS(config.externalRecipientNumber, config.externalRecipientMessage);
			}
		}

		if (!config.criticalSent && currLevel < 6.0 && currWeight > 0.1 and currWeight < prevWeight)
		{
			config.criticalSent = true;
			float gasWeight = config.GetWeight();
			SendSMS(config.recipientOne, currLevel, currWeight, gasWeight);
			delay(1000);
			SendSMS(config.recipientTwo, currLevel, currWeight, gasWeight);
			delay(1000);
			SendSMS(config.recipientThree, currLevel, currWeight, gasWeight);
		}
	}
	else
	{
		config.warningSent = false;
		config.criticalSent = false;
	}

	if (millis() - startTime > ONEDAY)
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
		
		// Save readings in SPIFFS in csv format
		File file = SPIFFS.open("/readings.csv", FILE_WRITE);
		if (!file)
		{
			Serial.println("Failed to open file for writing");
			return;
		}

		for (int i = 0; i < readings.size(); i++)
		{
			file.print(readings[i]);
			file.print(",");
		}

		file.close();

		startTime = millis();
	}

	CheckConnections(status);
}
