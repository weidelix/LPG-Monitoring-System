#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include "include/WebServer.h"
#include "include/Config.h"
#include "include/Scale.h"

char ssid[] = "Hakdog 2.4";
char pass[] = "SiopaoSiomai1234_";
unsigned long currentTime = millis();
unsigned long previousTime = 0;
const long timeoutTime = 2000;
String header;

AsyncWebServer server(80);
Config &conf = GetConfig();
Status status;

void OnIndex(AsyncWebServerRequest *request);
void OnGetConfig(AsyncWebServerRequest *request);
void OnPostConfig(AsyncWebServerRequest *request);
void OnGetStat(AsyncWebServerRequest *request);
void OnSetTare(AsyncWebServerRequest *request);
void OnHelp(AsyncWebServerRequest *request);
void OnAbout(AsyncWebServerRequest *request);

void ServerInit()
{
	Config &config = GetConfig();
	while (!Serial) {;}

	WiFi.softAP(config.wifiName, config.wifiPassword);

	server.on("/", HTTP_GET, OnIndex);
	server.on("/help", HTTP_GET, OnHelp);
	server.on("/about", HTTP_GET, OnAbout);
	server.on("/getconfig", HTTP_GET, OnGetConfig);
	server.on("/setconfig", HTTP_POST, OnPostConfig);
	server.on("/getstat", HTTP_GET, OnGetStat);
	server.on("/settare", HTTP_GET, OnSetTare);
	server.on("/ui_1", HTTP_GET, [](AsyncWebServerRequest *request) {
		request->send(SPIFFS, "/ui_1.png", "image/png");
	});
	server.on("/ui_2", HTTP_GET, [](AsyncWebServerRequest *request) {
		request->send(SPIFFS, "/ui_2.png", "image/png");
	});
	server.on("/ui_3", HTTP_GET, [](AsyncWebServerRequest *request) {
		request->send(SPIFFS, "/ui_3.png", "image/png");
	});
	server.on("/parts", HTTP_GET, [](AsyncWebServerRequest *request) {
		request->send(SPIFFS, "/parts.png", "image/png");
	});
	server.on("/cvsu", HTTP_GET, [](AsyncWebServerRequest *request) {
		request->send(SPIFFS, "/cvsu.png", "image/png");
	});

	server.onNotFound(OnIndex);
	server.begin();

	PrintWifiStatus();
}

void OnIndex(AsyncWebServerRequest *request)
{
	request->send(SPIFFS, "/index.html", "text/html");
}

void OnHelp(AsyncWebServerRequest *request)
{
	request->send(SPIFFS, "/help.html", "text/html");
}

void OnAbout(AsyncWebServerRequest *request)
{
	request->send(SPIFFS, "/about.html", "text/html");
}

void OnGetConfig(AsyncWebServerRequest *request)
{
	String json = String("{") + 
	"\"warning_level\": " + String(conf.warningLevel) + ","+\
	"\"tank_type\": \"" + String(conf.tankType) + "\"," +\
	"\"recipients\": [\"" + String(conf.recipientOne) + "\",\"" + String(conf.recipientTwo) + "\",\"" + String(conf.recipientThree) + "\"]," +\
	"\"notify_ext\": " + String(conf.notifyExternalRecipient) + "," +\
	"\"ext_recipient_number\": " + String(conf.externalRecipientNumber) + "," +\
	"\"ext_recipient_message\": \"" + String(conf.externalRecipientMessage) + "\"," +\
	"\"wifi_name\": \"" + String(conf.wifiName) + "\"," +\
	"\"wifi_password\": \"" + String(conf.wifiPassword) + "\"," +\
	"\"use_custom_weight\": " + String(conf.useCustomWeight) + "," +\
	"\"custom_gas_weight\": " + String(conf.customWeight) + "," +\
	"\"custom_tare_weight\": " + String(conf.customTare) + "}";

	request->send(200, "application/json",  json);
}

void OnPostConfig(AsyncWebServerRequest *request)
{
	String json = request->arg((size_t)0);

	// Get warning level from json
	int warningLevelIndex = json.indexOf("\"warning_level\":");
	int warningLevelEndIndex = json.indexOf(",", warningLevelIndex);
	String warningLevelString = json.substring(warningLevelIndex + 16, warningLevelEndIndex);
	conf.warningLevel = warningLevelString.toInt();

	// Get tank type from json
	int tankTypeIndex = json.indexOf("\"tank_type\":");
	int tankTypeEndIndex = json.indexOf(",", tankTypeIndex);
	String tankTypeString = json.substring(tankTypeIndex + 13, tankTypeEndIndex - 1);
	conf.tankType = tankTypeString.toInt();

	// Get recipients from json
	int recipientsIndex = json.indexOf("\"recipients\":");
	int recipientsEndIndex = json.indexOf("]", recipientsIndex);
	String recipientsString = json.substring(recipientsIndex + 14, recipientsEndIndex);
	int recipientOneIndex = recipientsString.indexOf("\"");
	int recipientOneEndIndex = recipientsString.indexOf("\"", recipientOneIndex + 1);
	String recipientOneString = recipientsString.substring(recipientOneIndex + 2, recipientOneEndIndex);
	if (recipientOneString.length() > 1)
	recipientOneString.toCharArray(conf.recipientOne, 20);
	else
	conf.recipientOne[0] = '\0';

	int recipientTwoIndex = recipientsString.indexOf("\"", recipientOneEndIndex + 1);
	int recipientTwoEndIndex = recipientsString.indexOf("\"", recipientTwoIndex + 1);
	String recipientTwoString = recipientsString.substring(recipientTwoIndex + 2, recipientTwoEndIndex);
	if (recipientTwoString.length() > 1)
	recipientTwoString.toCharArray(conf.recipientTwo, 20);
	else
	conf.recipientTwo[0] = '\0';

	int recipientThreeIndex = recipientsString.indexOf("\"", recipientTwoEndIndex + 1);
	int recipientThreeEndIndex = recipientsString.indexOf("\"", recipientThreeIndex + 1);
	String recipientThreeString = recipientsString.substring(recipientThreeIndex + 2, recipientThreeEndIndex);
	if (recipientThreeString.length() > 1)
	recipientThreeString.toCharArray(conf.recipientThree, 20);
	else
	conf.recipientThree[0] = '\0';

	// Get notify external recipient from json
	int notifyExternalRecipientIndex = json.indexOf("\"notify_ext\":");
	int notifyExternalRecipientEndIndex = json.indexOf(",", notifyExternalRecipientIndex);
	String notifyExternalRecipientString = json.substring(notifyExternalRecipientIndex + 13, notifyExternalRecipientEndIndex);
	conf.notifyExternalRecipient = notifyExternalRecipientString == "true" ? true : false;

	// Get external recipient number from json
	int externalRecipientNumberIndex = json.indexOf("\"ext_recipient_number\":");
	int externalRecipientNumberEndIndex = json.indexOf(",", externalRecipientNumberIndex);
	String externalRecipientNumberString = json.substring(externalRecipientNumberIndex + 25, externalRecipientNumberEndIndex - 1);
	if (externalRecipientNumberString.length() > 1)
	externalRecipientNumberString.toCharArray(conf.externalRecipientNumber, 20);
	else
	conf.externalRecipientNumber[0] = '\0';
	
	// Get external recipient message from json
	int externalRecipientMessageIndex = json.indexOf("\"ext_recipient_message\":");
	int externalRecipientMessageEndIndex = json.indexOf(",", externalRecipientMessageIndex);
	String externalRecipientMessageString = json.substring(externalRecipientMessageIndex + 25, externalRecipientMessageEndIndex - 1);
	externalRecipientMessageString.toCharArray(conf.externalRecipientMessage, 160);

	// Get wifi name from json
	int wifiNameIndex = json.indexOf("\"wifi_name\":");
	int wifiNameEndIndex = json.indexOf(",", wifiNameIndex);
	String wifiNameString = json.substring(wifiNameIndex + 13, wifiNameEndIndex - 1);
	// Get wifi password from json
	int wifiPasswordIndex = json.indexOf("\"wifi_password\":");
	int wifiPasswordEndIndex = json.indexOf(",", wifiPasswordIndex);
	String wifiPasswordString = json.substring(wifiPasswordIndex + 17, wifiPasswordEndIndex - 1);
	if (wifiNameString.c_str() != conf.wifiName || wifiPasswordString.c_str() != conf.wifiPassword)
	{
	wifiNameString.toCharArray(conf.wifiName, 20);
	wifiPasswordString.toCharArray(conf.wifiPassword, 20);
	
	WiFi.softAP(conf.wifiName, conf.wifiPassword);
	}
	
	// Get use custom weight from json
	int useCustomWeightIndex = json.indexOf("\"use_custom_weight\":");
	int useCustomWeightEndIndex = json.indexOf(",", useCustomWeightIndex);
	String useCustomWeightString = json.substring(useCustomWeightIndex + 20, useCustomWeightEndIndex);
	conf.useCustomWeight = useCustomWeightString == "true" ? true : false;

	// Get custom weight from json
	int customWeightIndex = json.indexOf("\"custom_gas_weight\":");
	int customWeightEndIndex = json.indexOf(",", customWeightIndex);
	String customWeightString = json.substring(customWeightIndex + 20, customWeightEndIndex);
	conf.customWeight = customWeightString.toFloat();

	// Get custom tare from json
	int customTareIndex = json.indexOf("\"custom_tare_weight\":");
	int customTareEndIndex = json.indexOf("}", customTareIndex);
	String customTareString = json.substring(customTareIndex + 21, customTareEndIndex);
	conf.customTare = customTareString.toFloat();

	conf.PrintConfig();
	conf.SaveConfig();
	request->send(200, "application/json", "{\"result\": \"OK\"}");
}

void OnGetStat(AsyncWebServerRequest *request)
{
	String json =\
	String("{") +\
	"\"raw\": " + String(status.raw) + "," +\
	"\"level\": " + String(status.level) + "," +\
	"\"weight\": " + String(status.weight) + "," +\
	"\"tare_weight\": " + String(status.tareWeight) + "," +\
	"\"gas_weight\": " + String(status.gasWeight) + "," +\
	"\"avg_use\": " + String(status.avgUse) + "," +\
	"\"days_before_empty\": " + String(status.daysBeforeEmpty) +\
	"}";

	request->send(200, "application/json", json);	
}

void OnSetTare(AsyncWebServerRequest *request)
{
	ResetTare();
	request->send(200, "text/plain", "OK");
}

void CheckConnections(Status stat)
{
	status = stat;
}

void PrintWifiStatus()
{
	Serial.print("SSID: ");
	Serial.println(WiFi.SSID());

	IPAddress ip = WiFi.localIP();
	Serial.print("IP Address: ");
	Serial.println(ip);
}
