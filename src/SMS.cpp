#include <SoftwareSerial.h>
#include "include/SMS.h"

EspSoftwareSerial::UART sim(16, 17);

void SimInit()
{
	sim.begin(57600, SWSERIAL_8N1);
	sim.enableIntTx(false);
}

void Test()
{
	sim.println("AT");
	UpdateSerial();
	sim.println("AT+CSQ");
	UpdateSerial();
	sim.println("AT+CCID");
	UpdateSerial();
	sim.println("AT+CREG?");
	UpdateSerial();
}

void UpdateSerial()
{
	delay(1000);
	while (sim.available())
	{
		Serial.write(sim.read());
	}
	while (Serial.available())
	{
		sim.write(Serial.read());
	}
}

void SendSMS(char *recipient, float level)
{
	UpdateSerial();
	sim.println("AT+CMGF=1");
	delay(1000);
	sim.printf("AT+CMGS=\"+63%s\"\r", recipient);
	delay(1000);
	if (level > 5.0)
	{
		sim.printf("WARNING! Your tank has reached %.2f percent.", level);
	}
	else
	{
		sim.printf("CRITICAL! You're almost out of gas. %.2f percent remaining.", level);
	}
	delay(1000);
	sim.println((char)26);
	delay(1000);
}

void SendSMS(char *recipient, char *message)
{
	UpdateSerial();
	sim.println("AT+CMGF=1");
	delay(1000);
	sim.printf("AT+CMGS=\"+63%s\"\r", recipient);
	delay(1000);
	sim.println(message);
	delay(1000); 
	sim.println((char)26);
	delay(1000);
}