struct Status
{
public:
	float raw;
	float level;
	float weight;
	float tareWeight;
	float gasWeight;
	float avgUse;
	float daysBeforeEmpty;
};

void ServerInit();
void CheckConnections(Status);
void PrintWifiStatus();
