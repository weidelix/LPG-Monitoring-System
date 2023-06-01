#if defined(ESP8266) || defined(ESP32) || defined(AVR)
#include <EEPROM.h>
#endif

class Config
{
public:
	int warningLevel = 20;
	int tankType = 0;
	char recipientOne[20] = "";
	char recipientTwo[20] = "";
	char recipientThree[20] = "";
	bool notifyExternalRecipient = false;
	char externalRecipientNumber[20] = "";
	char externalRecipientMessage[160] = "External Recipient";
	char wifiName[20] = "LPGMonSystem"; // Max 14 chars
	char wifiPassword[20] = "1234567890";
	bool useCustomWeight = false;
	float customTare = 12.3;
	float customWeight = 11.0;
	bool warningSent = false;
	bool criticalSent = false;

	void SaveConfig();
	void ReadConfig();
	void PrintConfig();
	float GetTare();
	float GetWeight();
};

Config &GetConfig();