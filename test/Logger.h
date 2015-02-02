#ifndef SCANSDK_LOGGER_H
#define SCANSDK_LOGGER_H

#include <iostream>
#include <string>
#include <sstream>
#include <boost/serialization/singleton.hpp>
#include "Common.h"


OPEN_NAMESPACE_SCANSDK


using namespace std;


#define SCANSDK_LOGGER_LEVEL_FATAL					"FATAL"
#define SCANSDK_LOGGER_LEVEL_ERROR					"ERROR"
#define SCANSDK_LOGGER_LEVEL_WARNING				"WARNING"
#define SCANSDK_LOGGER_LEVEL_INFO					"INFO"
#define SCANSDK_LOGGER_LEVEL_DEBUG_1				"DEBUG_1"
#define SCANSDK_LOGGER_LEVEL_DEBUG_2				"DEBUG_2"
#define SCANSDK_LOGGER_LEVEL_DEBUG_3				"DEBUG_3"
#define FATAL										SCANSDK_LOGGER_LEVEL_FATAL
#define ERROR										SCANSDK_LOGGER_LEVEL_ERROR
#define WARNING										SCANSDK_LOGGER_LEVEL_WARNING
#define INFO										SCANSDK_LOGGER_LEVEL_INFO
#define DEBUG_1										SCANSDK_LOGGER_LEVEL_DEBUG_1
#define DEBUG_2										SCANSDK_LOGGER_LEVEL_DEBUG_2
#define DEBUG_3										SCANSDK_LOGGER_LEVEL_DEBUG_3

#define LOG_FUNC_STRING								"[%s(%s:%d)] "
#define LOG_FUNC_VALUE 								__PRETTY_FUNCTION__, __FILE__, __LINE__


class LoggerSingleton : private Switch::Tool::Logger, public boost::serialization::singleton< LoggerSingleton >
{
public:
	int Init(int nBaseLevel, const string& sFilePath, const string& sLogPrefix, bool bIsDev = false);
	void SetChecker(void);
	int LogWrite(const std::string& level, const std::string& module, const std::string& format, ...);
	int LogWrite(const std::string& level, const std::string& module, const std::string& format, va_list ap);

protected:
	LoggerSingleton(void) {}
	~LoggerSingleton(void) {}

private:
	static string LogLevels[];
};


CLOSE_NAMESPACE_SCANSDK


#endif // SCANSDK_LOGGER_H
