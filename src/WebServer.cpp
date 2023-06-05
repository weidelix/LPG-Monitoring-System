#include <WiFi.h>
#include <WebServer.h>
#include "include/WebServer.h"
#include "include/Config.h"
#include "include/Scale.h"

char ssid[] = "Hakdog 2.4";
char pass[] = "SiopaoSiomai1234_";
unsigned long currentTime = millis();
unsigned long previousTime = 0;
const long timeoutTime = 2000;
String header;

WebServer server(80);
Config &conf = GetConfig();
Status status;

void OnIndex();
void OnGetConfig();
void OnPostConfig();
void OnGetStat();
void OnSetTare();

void ServerInit()
{
	Config &config = GetConfig();
	while (!Serial) {;}

	// WiFi.mode(WIFI_STA);
	// WiFi.begin("Hakdog 2.4", "SiopaoSiomai1234_");
	WiFi.softAP(config.wifiName, config.wifiPassword);
	
	// while (WiFi.status() != WL_CONNECTED)
	// {
	// 	Serial.print("Attempting to connect to SSID: ");
	// 	Serial.println(ssid);
	// 	delay(1000);
	// }

	server.on("/", HTTP_GET, OnIndex);
	server.on("/getconfig", HTTP_GET, OnGetConfig);
	server.on("/setconfig", HTTP_POST, OnPostConfig);
	server.on("/getstat", HTTP_GET, OnGetStat);
	server.on("/settare", HTTP_GET, OnSetTare);
	server.onNotFound(OnIndex);
	server.begin();

	PrintWifiStatus();
}

void OnIndex()
{
	server.send(200, "text/html", "<!doctype html><html lang=\"en\"><head> <meta charset=\"utf-8\"> <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\"> <title>Settings</title></head><style> body { background-color: white; font-family: 'Arial'; margin: 0px 0px; } .big-heading { line-height: 1.5; font-family: \"Arial\"; } #title { color: black; } .container-fluid { padding: 5%; max-width: 800px; } input[type=number] { width: 70px; height: 30px; } #level { display: flex; align-items: center; justify-content: center; width: 100%; height: 100%; background-color: red; transition: width 0.5s; } .item { border: solid; border-width: 1px; border-color: #999999EB; padding: 15px; background-color: #9999992B; border-radius: 8px; margin: 10px 0px; } .item-input { border: solid; border-width: 1px; border-color: #999999EB; padding: 15px; background-color: #8888882B; border-radius: 4px; margin: 5px 0px; }</style><body><div> <form name=\"config\" style=\"display: flex; flex-wrap: wrap; flex-direction: column; align-items: stretch;\"> <div class=\"container-fluid\"> <h1>Settings</h1> <div style=\"margin-bottom: 30px;\"> <div> <h3>Status</h3> </div> <hr width=\"100%\"> <div style=\"margin-bottom: 20px;\"> <div style=\"display: flex; justify-content: space-between; align-items: center; margin-bottom: 10px;\"> <label>Gas Level:</label> <div id=\"gas-info\" style=\"font-weight: bold; font-size: 14px\">0.0KG / 0.0KG</div> </div> <div style=\"width: 100%; height: 40px; background-color: lightgray;\"> <div id=\"level\" style=\"width: 100%;\"> 100% </div> </div> </div> <label style=\"font-size: 12px;\">If this is the first time the system is running, let the system run for a few more days uninterrupted for a more accurate data.</label> <div id=\"avg-use\" style=\"margin-top: 10px;\">Average use per day: 0%</div> <div id=\"days-bfr-empty\" style=\"margin-top: 10px;\">Days before empty (approx): 0 days</div> </div> <div style=\"margin-bottom: 50px;\"> <div> <h3>System</h3> </div> <hr width=\"100%\"> <div style=\"margin-bottom: 10px;\"> <div style=\"margin-bottom: 10px;\"> <label>WiFi Authentication</label> </div> <div style=\"display: flex; flex-wrap: wrap; flex-direction: column; font-size: 12px\"> <label>Name</label> <input class=\"item-input\" name=\"wifi_name\" type=\"text\" minlength=\"8\" placeholder=\"Wifi name\">&nbsp;&nbsp;&nbsp; <label>Password</label> <input class=\"item-input\" name=\"wifi_password\" type=\"password\" minlength=\"8\" placeholder=\"Wifi password\"> </div> </div> <div style=\"display: flex; justify-content: space-between; align-items: center; margin-bottom: 10px;\"> <div> <label>Warning level of gas weight: (6 - 50 %)</label> </div> <input name=\"warning_level\" type=\"number\" min=\"6\" max=\"50\" maxlength=\"2\" value=\"0\" style=\"width: 30px; height: 25px\"> </div> <div style=\"margin-bottom: 30px;\"> <div> <label>Tank type</label> <div style=\"width: 100%; max-height: 100px;\"> <div class=\"item\"> <input class=\"my-3\" type=\"radio\" name=\"tank_type\" value=\"0\"> <span class=\"mx-3\">Generic 11KG</span> </div> <div class=\"item\"> <input type=\"radio\" name=\"tank_type\" value=\"1\"> <span>EC Gas 11KG</span> </div> </div> </div> </div> <div style=\"display: flex; justify-content: space-between; align-items: center; margin-bottom: 10px;\"> <div> <label>Use custom weight</label> </div> <div> <input name=\"use_custom_weight\" type=\"checkbox\" style=\"transform: scale(1.5)\"> </div> </div> <div style=\"font-size: 14px;\"> <div>Tare weight: Weight of the tank minus the gas. Indicated as T.W on the side of the tank. </div> <div>Gas weight: Weight of the gas.</div> </div> <div style=\"display: flex; gap: 10px\"> <div> <label>Tare weight (kg):</label> <input class=\"item-input\" name=\"custom_tare_weight\" type=\"number\" step=\"0.01\" style=\"padding: 0px\"> </div> <div> <label>Gas weight (kg):</label> <input class=\"item-input\" name=\"custom_gas_weight\" type=\"number\" step=\"0.01\" style=\"padding: 0px\"> </div> </div> </div> <div class=\"row\"> <div class=\"col lg-6\"> <h3 class=\"fs-4 fw-bold\">SMS</h3> </div> <hr width=\"100%\"> </div> <div> <label>Recipients:</label> <div id=\"recipients\" style=\"display: flex; flex-direction: column; align-content: flex-start; margin-bottom: 10px;\"> <input class=\"item-input\" name=\"rec_one\" type=\"text\" placeholder=\"Contact number 1\"> <input class=\"item-input\" name=\"rec_two\" type=\"text\" placeholder=\"Contact number 2\"> <input class=\"item-input\" name=\"rec_three\" type=\"text\" placeholder=\"Contact number 3\"> </div> <div style=\"display: flex; justify-content: space-between; align-items: center; margin-bottom: 10px;\"> <div> <label>Notify external recipient</label> </div> <div> <input name=\"notify_ext\" type=\"checkbox\" style=\"transform: scale(1.5)\"> </div> </div> <div style=\"display: flex; flex-direction: column\"> <label>External recipient:</label> <input name=\"ext_recipient_number\" class=\"item-input\" type=\"text\" placeholder=\"Recipient number\"> <textarea name=\"ext_recipient_message\" class=\"item-input\" rows=\"4\" placeholder=\"Message (MAX 160 CHAR)\" maxlength=\"160\"></textarea> </div> </div> <div style=\"display: flex; gap: 5px\"> <button type=\"submit\" style=\"padding: 8px; border: 0px; border-radius: 4px; width: 80px; height: 50px; background-color: black; color: white; font-weight: bold;\"> SAVE </button> </div> </div> </form></div></body><script> let form = document.forms['config']; form.addEventListener('submit', saveConfig); document.getElementById('level').style.width = '0px'; let getStat = false; addEventListener(\"DOMContentLoaded\", (event) => { setInterval(() => { if (!getStat) { getStat = true; request('/getstat', 'GET', null).then(result => { console.table(result); let level = document.getElementById('level'); let gasInfo = document.getElementById('gas-info'); document.getElementById('avg-use').innerHTML = `Average use per day: ${result.avg_use.toFixed(2)}%`; document.getElementById('days-bfr-empty').innerHTML = `Days before empty (approx): ${result.days_before_empty} days`; let gasLevel = result.level; if (result.level < 0.0 && result.level > -1.0) { gasLevel = Math.ceil(result.level); } gasLevel = Math.round(gasLevel); currWeight = result.weight; currWeight = Math.min(result.gas_weight, Math.max(0.0, currWeight)); gasInfo.innerHTML = `${currWeight}KG / ${result.gas_weight}KG`; gasLevel = Math.min(100.0, Math.max(-1.0, gasLevel)); level.innerHTML = gasLevel >= 0 ? `${gasLevel}%` : 'No tank'; level.style.width = gasLevel >= 0 ? `${gasLevel}%` : '100%'; level.style.backgroundColor = `hsl(${100 * gasLevel / 100}, 100%, 50%)`; getStat = false; }).catch(err => { console.log(err); getStat = false; }); } }, 1000); request('/getconfig', 'GET', null).then(result => {console.log(result); let els = form.elements; els.warning_level.value = result.warning_level; els.tank_type.value = result.tank_type; if (result.recipients.length > 0 ) { els.rec_one.value = result.recipients[0] != '' ? '0' + result.recipients[0] : ''; els.rec_two.value = result.recipients[1] != '' ? '0' + result.recipients[1] : ''; els.rec_three.value = result.recipients[2] != '' ? '0' + result.recipients[2] : ''; } els.notify_ext.checked = result.notify_ext; els.ext_recipient_number.value = result.ext_recipient_number != '' ? '0' + result.ext_recipient_number : ''; els.ext_recipient_message.value = result.ext_recipient_message; els.wifi_name.value = result.wifi_name; els.wifi_password.value = result.wifi_password; els.use_custom_weight.checked = result.use_custom_weight; els.custom_tare_weight.value = result.custom_tare_weight; els.custom_gas_weight.value = result.custom_gas_weight; }); }); function saveConfig(e) { e.preventDefault(); let formData = { 'warning_level': parseInt(this.warning_level.value), 'tank_type': this.tank_type.value, 'recipients': [ this.rec_one.value ?? '', this.rec_two.value ?? '', this.rec_three.value ?? ''], 'notify_ext': this.notify_ext.checked, 'ext_recipient_number': this.ext_recipient_number.value ?? '', 'ext_recipient_message': this.ext_recipient_message.value ?? '', 'power_saving_mode': 'Balanced', 'wifi_name': this.wifi_name.value, 'wifi_password': this.wifi_password.value, 'use_custom_weight': this.use_custom_weight.checked, 'custom_gas_weight': parseFloat(this.custom_gas_weight.value), 'custom_tare_weight': parseFloat(this.custom_tare_weight.value) }; request('/setconfig', 'POST', JSON.stringify(formData)).then(res => { alert('Saved'); }); } function request(route, method, body) { return fetch(route, { method: method, headers: { 'Accept': 'application/json', 'Content-Type': 'application/json' }, body: body }).then(response => response.json()) }</script></html>");
}

void OnGetConfig()
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

	server.send(200, "application/json",  json);
}

void OnPostConfig()
{
	String json = server.arg(0);

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
	server.send(200, "application/json", "{\"result\": \"OK\"}");
}

void OnGetStat()
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

	server.send(200, "application/json", json);	
}

void OnSetTare()
{
	ResetTare();
	server.send(200, "text/plain", "OK");
}

void CheckConnections(Status stat)
{
	status = stat;
	server.handleClient();
}

void PrintWifiStatus()
{
	Serial.print("SSID: ");
	Serial.println(WiFi.SSID());

	IPAddress ip = WiFi.localIP();
	Serial.print("IP Address: ");
	Serial.println(ip);
}
