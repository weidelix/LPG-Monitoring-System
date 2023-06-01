#include "include/Config.h"

Config config = Config();
const float tankTypes[2][2] = {
	{12.3, 11.0}, {5.1, 10.0}
};

void Config::SaveConfig()
{
	EEPROM.put(10, warningLevel);
	EEPROM.put(20, tankType);
	EEPROM.put(60, notifyExternalRecipient);

	for (int i = 0; i < 10; i++)
	{
		EEPROM.write(70 + i, externalRecipientNumber[i]);
	}

	for (int i = 0; i < 20; i++)
	{
		EEPROM.write(80 + i, wifiName[i]);
	}

	for (int i = 0; i < 20; i++)
	{
		EEPROM.write(100 + i, wifiPassword[i]);
	}

	for (int i = 0; i < 160; i++)
	{
		EEPROM.write(120 + i, externalRecipientMessage[i]);
	}

	EEPROM.write(280, useCustomWeight);
	EEPROM.put(285, customWeight);
	EEPROM.put(290, customTare);

	for (int i = 0; i < 10; i++)
	{
		EEPROM.write(300 + i, recipientOne[i]);
	}

	for (int i = 0; i < 10; i++)
	{
		EEPROM.write(320 + i, recipientTwo[i]);
	}

	for (int i = 0; i < 10; i++)
	{
		EEPROM.write(340 + i, recipientThree[i]);
	}

	EEPROM.commit();
}

void Config::ReadConfig()
{
	Serial.println("Reading configuration...");
	EEPROM.get(10, warningLevel);
	EEPROM.get(20, tankType);

	for (int i = 0; i < 10; i++)
	{
		EEPROM.get(300 + i, recipientOne[i]);
	}

	for (int i = 0; i < 10; i++)
	{
		EEPROM.get(320 + i, recipientTwo[i]);
	}

	for (int i = 0; i < 10; i++)
	{
		EEPROM.get(340 + i, recipientThree[i]);
	}

	EEPROM.get(60, notifyExternalRecipient);

	for (int i = 0; i < 10; i++)
	{
		EEPROM.get(70 + i, externalRecipientNumber[i]);
	}

	for (int i = 0; i < 20; i++)
	{
		EEPROM.get(80 + i, wifiName[i]);
	}
	for (int i = 0; i < 20; i++)
	{
		EEPROM.get(100 + i, wifiPassword[i]);
	}
	for (int i = 0; i < 160; i++)
	{
		EEPROM.get(120 + i, externalRecipientMessage[i]);
	}

	EEPROM.get(280, useCustomWeight);
	EEPROM.get(285, customWeight);
	EEPROM.get(290, customTare);
}

void Config::PrintConfig()
{
	Serial.println();
	Serial.print("Warning Level: ");
	Serial.println(warningLevel);
	Serial.print("Tank Type: ");
	Serial.println(tankType);
	Serial.print("Recipient One: ");
	Serial.println(recipientOne);
	Serial.print("Recipient Two: ");
	Serial.println(recipientTwo);
	Serial.print("Recipient Three: ");
	Serial.println(recipientThree);
	Serial.print("Notify External Recipient: ");
	Serial.println(notifyExternalRecipient);
	Serial.print("External Recipient Number: ");
	Serial.println(externalRecipientNumber);
	Serial.print("External Recipient Message: ");
	Serial.println(externalRecipientMessage);
	Serial.print("Wifi Name: ");
	Serial.println(wifiName);
	Serial.print("Wifi Password: ");
	Serial.println(wifiPassword);
	Serial.print("Use Custom Weight: ");
	Serial.println(useCustomWeight);
	Serial.print("Custom Weight: ");
	Serial.println(customWeight);
	Serial.print("Custom Tare: ");
	Serial.println(customTare);
	Serial.println();
}

float Config::GetTare()
{
	if (useCustomWeight)
	{
		return customTare;
	}
	else
	{
		return tankTypes[tankType][0];
	}
}

float Config::GetWeight()
{
	if (useCustomWeight)
	{
		return customWeight;
	}
	else
	{
		return tankTypes[tankType][1];
	}
}

Config &GetConfig()
{
	return config;
}