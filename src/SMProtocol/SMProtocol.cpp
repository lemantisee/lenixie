#include "SMProtocol.h"

#include "Interface.h"
// #include "Logger.h"

void SMProtocol::init(Interface *inter)
{
	mInterface = inter;
	mStringParametr.fill(0);
}

void SMProtocol::SendCommand(const char *str)
{
	mInterface->SendString(str);
}

void SMProtocol::SendCommand(const char *str, uint32_t parametr)
{
	char str_param[10];
	memset(str_param, 0, 10);
    sprintf(str_param, "%d", int(parametr));
	char * tmp_str = strcat((char *)str, ":");
	mInterface->SendString(strcat(tmp_str, str_param));
}

void SMProtocol::SendCommand(const char *str, const char *parametr)
{
	char *tmp_str = strcat((char *)str, ":");
	mInterface->SendString(strcat(tmp_str, (char *)parametr));
}

void SMProtocol::SendCommand(uint8_t cmd, uint32_t parametr)
{
	char str_param[10];
	char str_cmd[mInterface->getBufferSize()];
	memset(str_param, 0, 10);
	memset(str_cmd, 0, mInterface->getBufferSize());
	sprintf(str_param, "%d", int(parametr));
	sprintf(str_cmd, "%d", cmd);
	char * tmp_str = strcat(str_cmd, ":");
	mInterface->SendString(strcat(tmp_str, str_param));
}

void SMProtocol::SendCommand(uint8_t cmd, const char *parametr)
{
	char str_cmd[mInterface->getBufferSize()];
	memset(str_cmd, 0, mInterface->getBufferSize());
	sprintf(str_cmd, "%d", cmd);
	char * tmp_str = strcat(str_cmd, ":");
	mInterface->SendString(strcat(tmp_str, (char *)parametr));
}

bool SMProtocol::isCommandReady()
{
	return mInterface->hasIncomeData();
}

uint8_t SMProtocol::getCommand()
{
	mStringParametr.fill(0);
	char *tmpPtr = (char *)mInterface->getIncomeData();
    // Logger::instance().log("Income data:");
    // Logger::instance().log(tmpPtr);
    // Logger::instance().log("\n");
	uint8_t command = 0;
	if (tmpPtr != nullptr){
        char *cmd_pos = strchr(tmpPtr, ':');
		if (cmd_pos != nullptr) {
            const char *cmdStr = strsep(&tmpPtr, ":");
            // Logger::instance().log("Command:");
            // Logger::instance().log(cmdStr);
            // Logger::instance().log("\n");
            command = atoi(cmdStr);
			mStringParametrPtr = strsep(&tmpPtr, ":");
            strcpy(mStringParametr.data(), mStringParametrPtr);
            // Logger::instance().log("Parametr:");
            // Logger::instance().log(mStringParametrPtr);
            // Logger::instance().log("\n");
			mParametr = atoi(mStringParametrPtr);
		}
		else {
			command = atoi((char *)mInterface->getIncomeData());
		}
	}


	return command;
}

uint16_t SMProtocol::getParametr()
{
	return mParametr;
}

uint8_t * SMProtocol::getStringParametr() {
	return (uint8_t *)mStringParametr.data();
}
