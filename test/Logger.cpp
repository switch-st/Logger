#include "Logger.h"

OPEN_NAMESPACE_SCANSDK

string LoggerSingleton::LogLevels[] = {SCANSDK_LOGGER_LEVEL_FATAL, \
										SCANSDK_LOGGER_LEVEL_ERROR, \
										SCANSDK_LOGGER_LEVEL_WARNING, \
										SCANSDK_LOGGER_LEVEL_INFO, \
										SCANSDK_LOGGER_LEVEL_DEBUG_1, \
										SCANSDK_LOGGER_LEVEL_DEBUG_2, \
										SCANSDK_LOGGER_LEVEL_DEBUG_3};

int LoggerSingleton::Init(int nBaseLevel, const string& sFilePath, const string& sLogPrefix, bool bIsDev)
{
	int ret;
    Switch::Tool::LogConfig_t logcfg;
    Switch::Tool::ErrorInfo err;

    logcfg.m_vLevels.clear();
    if (nBaseLevel < 0 || (size_t)nBaseLevel >= sizeof(LoggerSingleton::LogLevels) / sizeof(string*))
	{
		stringstream ss1, ss2;
		ss1 << sizeof(LoggerSingleton::LogLevels) / sizeof(string*) << endl;
		ss2 << nBaseLevel << endl;
		cerr << MODULE_SCANSDK"[ERROR]: log base level error, valid base level is [0 ~ " << ss1.str() << "]set base level is " << ss2.str() << endl;
		return -1;
	}
    for (size_t i = 0; i < sizeof(LoggerSingleton::LogLevels) / sizeof(string*); ++i)
	{
		logcfg.m_vLevels.push_back(LoggerSingleton::LogLevels[i]);
	}
	logcfg.m_sBaseLevel = LoggerSingleton::LogLevels[nBaseLevel];
	logcfg.m_sFilePrefix = sLogPrefix;
	if (bIsDev)
	{
		logcfg.m_sFilePath = "/dev/stderr";
		logcfg.m_bWrite2Device = bIsDev;
	}
	else
	{
		logcfg.m_sFilePath = sFilePath;
	}

	ret = Switch::Tool::Logger::Init(logcfg, err);
	if (ret != 0)
	{
		cerr << MODULE_SCANSDK"[ERROR]: log init failed. " << endl;
		SHOW_ERROR(err);
		return -1;
	}

	return 0;
}

void LoggerSingleton::SetChecker(void)
{
	Switch::Tool::Logger::SetChecker();
}

int LoggerSingleton::LogWrite(const std::string& level, const std::string& module, const std::string& format, ...)
{
	Switch::Tool::ErrorInfo err;

    va_list ap;
    va_start(ap, format);
    if (Switch::Tool::Logger::LogWrite(err, level, module, format, ap) != 0)
	{
		SHOW_ERROR(err);
		va_end(ap);
		return -1;
	}
    va_end(ap);
    return 0;
}

int LoggerSingleton::LogWrite(const std::string& level, const std::string& module, const std::string& format, va_list ap)
{
	Switch::Tool::ErrorInfo err;

    if (Switch::Tool::Logger::LogWrite(err, level, module, format, ap) != 0)
	{
		SHOW_ERROR(err);
		return -1;
	}
    return 0;
}


CLOSE_NAMESPACE_SCANSDK

