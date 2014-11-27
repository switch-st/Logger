#include <iostream>

#include "Logger.hpp"


USING_NAMESPACE_SWITCHTOOL


int main(void)
{
	ErrorInfo info;
	LogConfig_t conf;
	//conf.m_bSplitFileByLevel = true;
	//conf.m_bWrite2Device = true;
	//conf.m_sFilePath = "/dev/stderr";
    Logger logger;
    logger.Init(conf, info);

	for (int j = 0; j < 100; ++j)
	{
		std::cout << "count : " << j << std::endl;
		for (int i = 0; i < 10000; ++i)
		{
			logger.LogWrite(SWITCH_LOGGER_DEFAULT_LEVEL_INFO, "test", "lalala_%d", 24);
			logger.LogWrite(SWITCH_LOGGER_DEFAULT_LEVEL_WARNING, "test", "warning_%d", 2100);
			logger.LogWrite(SWITCH_LOGGER_DEFAULT_LEVEL_FATAL, "test", "fatal_%d", 550);
		}
	}


	/////
	/*
	LoggerSingleton* pLog = &LoggerSingleton::get_mutable_instance();
	conf.m_bSplitFileByLevel = true;
	pLog->Init(conf, info);

	pLog->LogWrite(SWITCH_LOGGER_DEFAULT_LEVEL_INFO, "test", "qqq%d", 24);
	*/

	//pause();









	return 0;
}

