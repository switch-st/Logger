#ifndef SWITCH_LOGGER_HPP
#define SWITCH_LOGGER_HPP

/**
 * 日志库
 *
 * 欢迎补充！
 **/

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdarg.h>
#include <assert.h>
#include <time.h>
#include <string.h>
#include <vector>
#include <string>
#include <boost/thread.hpp>
#include <boost/interprocess/sync/file_lock.hpp>
#include <boost/interprocess/sync/sharable_lock.hpp>
#include <boost/serialization/singleton.hpp>
#include <boost/noncopyable.hpp>

#ifndef NAMESPACE_SWITCH_TOOL
    #define NAMESPACE_SWITCH_TOOL
    #define OPEN_NAMESPACE_SWITCHTOOL       namespace Switch { \
												namespace Tool {
    #define CLOSE_NAMESPACE_SWITCHTOOL      	}; \
											};
    #define USING_NAMESPACE_SWITCHTOOL      using namespace Switch::Tool;
#endif


OPEN_NAMESPACE_SWITCHTOOL

#ifndef SWITCH_ERROR_INFO
#define SWITCH_ERROR_INFO
	#ifndef SET_ERROR
	#define SET_ERROR(error,code,error_str) \
			do{\
				error.clear();\
				error.errno_ = code;\
				error.message += error_str;\
				error.filename += __FILE__;\
				error.funname += __func__;\
				error.line = __LINE__;\
			}while(0);
	#endif
	#ifndef SHOW_ERROR
	#define SHOW_ERROR(error)\
			do{\
				std::cerr << error.errno_;\
				std::cerr << ": " << error.message;\
				std::cerr << " : position : "  <<  error.filename;\
				std::cerr << " | " << error.funname;\
				std::cerr << " | " << error.line << std::endl;\
			}while(0);
	#endif
struct ErrorInfo
{
    int errno_;
    int line;
    std::string message;
    std::string filename;
    std::string funname;

    inline void clear()
    {
        message.clear();
        filename.clear();
        funname.clear();
        errno_ = 0;
        line = 0;
    }
};
#endif


/// 预定义级别
#define SWITCH_LOGGER_DEFAULT_LEVEL_FATAL					"FATAL"
#define SWITCH_LOGGER_DEFAULT_LEVEL_ERROR					"ERROR"
#define SWITCH_LOGGER_DEFAULT_LEVEL_WARNING					"WARNING"
#define SWITCH_LOGGER_DEFAULT_LEVEL_INFO					"INFO"
#define SWITCH_LOGGER_DEFAULT_LEVEL_DEBUG					"DEBUG"
#define SWITCH_LOGGER_BASE_LEVEL							SWITCH_LOGGER_DEFAULT_LEVEL_INFO

/// errno
#define SWITCH_LOGGER_ERRNO_PARAMENT                        1000
#define SWITCH_LOGGER_ERRNO_WRITE	                        1001
#define SWITCH_LOGGER_ERRNO_REINIT	                        1002
#define SWITCH_LOGGER_ERRNO_NOTINIT	                        1003


struct LogConfig_st
{
	std::vector< std::string >			m_vLevels;				// 级别从高到底
	std::string							m_sBaseLevel;			// 基准级别
	unsigned long 						m_nFileSize;			// 单个日志文件大小
	std::string							m_sFilePath;			// 日志路径
	std::string							m_sFilePrefix;			// 日志前缀
	std::string							m_sFileSuffix;			// 日志后缀
	bool								m_bSplitFileByLevel;	// 是否按级别拆分文件
	bool								m_bWrite2Device;		// 是否是设备文件

	LogConfig_st(void)
	{
		m_vLevels.push_back(SWITCH_LOGGER_DEFAULT_LEVEL_FATAL);
		m_vLevels.push_back(SWITCH_LOGGER_DEFAULT_LEVEL_ERROR);
		m_vLevels.push_back(SWITCH_LOGGER_DEFAULT_LEVEL_WARNING);
		m_vLevels.push_back(SWITCH_LOGGER_DEFAULT_LEVEL_INFO);
		m_vLevels.push_back(SWITCH_LOGGER_DEFAULT_LEVEL_DEBUG);
		m_sBaseLevel = SWITCH_LOGGER_BASE_LEVEL;
		m_nFileSize = 1024 * 1024 * 10;
		m_sFilePath = "./";
		m_sFilePrefix = "logger";
		m_sFileSuffix = "log";
		m_bSplitFileByLevel = false;
		m_bWrite2Device = false;
	}
};

typedef struct LogConfig_st LogConfig_t;


class Logger : public boost::noncopyable
{
public:
	Logger(void)
	{
        m_bInited = false;
	}

	~Logger(void)
	{
		Cleanup();
	}

	const std::vector< std::string >& GetLevels(void) { return m_vLevels;}
	const std::string& GetBaseLevel(void) { return m_sBaseLevel;}
	const unsigned long& GetFileSize(void) { return m_nFileSize;}
	const std::string& GetFilePath(void) { return m_sFilePath;}
	const std::string& GetFilePrefix(void) { return m_sFilePrefix;}
	const std::string& GetFileSuffix(void) { return m_sFileSuffix;}
	const std::vector< std::string >& GetFileNames(void) { return m_vFileNames;}
	const bool& GetSplitFileByLevel(void) { return m_bSplitFileByLevel;}
	const bool& GetWrite2Device(void) { return m_bWrite2Device;}

	int Init(const LogConfig_t& log_cfg, ErrorInfo& info)
	{
		int ret;

		if (m_bInited)
		{
			SET_ERROR(info, SWITCH_LOGGER_ERRNO_REINIT, std::string("logger has been initialized."));
			return SWITCH_LOGGER_ERRNO_REINIT;
		}

		this->m_vLevels = log_cfg.m_vLevels;
		this->m_sBaseLevel = log_cfg.m_sBaseLevel;
		this->m_nFileSize = log_cfg.m_nFileSize;
		this->m_sFilePath = log_cfg.m_sFilePath;
		this->m_sFilePrefix = log_cfg.m_sFilePrefix;
		this->m_sFileSuffix = log_cfg.m_sFileSuffix;
		this->m_bSplitFileByLevel = log_cfg.m_bSplitFileByLevel;
		this->m_bWrite2Device = log_cfg.m_bWrite2Device;

		ValidCheck();
		for (unsigned int i = 0; i < m_vLevels.size(); ++i)
		{
			m_mLevels.insert(std::make_pair(m_vLevels[i], i));
			if (m_sBaseLevel == m_vLevels[i])
			{
				m_nBaseLevel = i;
			}
		}
		if (m_bSplitFileByLevel && !m_bWrite2Device)
		{
			for(unsigned int i = 0; i < m_vLevels.size(); ++i)
			{
				std::string filename = m_sFilePath + m_sFilePrefix + std::string("_") + m_vLevels[i] + std::string(".") + m_sFileSuffix;
				m_vFileNames.push_back(filename);
                FILE* fp = fopen(m_vFileNames[i].c_str(), "a+");
                if (!fp)
                {
					SET_ERROR(info, errno, std::string("open file:") + m_vFileNames[i] + std::string(" failed:") + std::string(strerror(errno)));
					Cleanup();
					return errno;
                }
                setvbuf(fp, NULL, _IONBF, 0);
                m_vFilePointers.push_back(fp);
                struct stat f_stat;
                ret = stat(m_vFileNames[i].c_str(), &f_stat);
                if (ret)
                {
					f_stat.st_size = 0;
                }
                m_vCurrentSize.push_back(f_stat.st_size);
                m_vMutex.push_back(new boost::mutex);
            }
		}
		else
		{
			std::string filename = "";
			if (m_bWrite2Device)
			{
				filename = m_sFilePath;
			}
			else
			{
				filename = m_sFilePath + m_sFilePrefix + std::string(".") + m_sFileSuffix;
			}
			m_vFileNames.push_back(filename);
			FILE* fp = fopen(m_vFileNames[0].c_str(), "a+");
			if (!fp)
			{
                SET_ERROR(info, errno, std::string("open file:") + m_vFileNames[0] + std::string(" failed:") + std::string(strerror(errno)));
                return errno;
			}
			setvbuf(fp, NULL, _IONBF, 0);
			m_vFilePointers.push_back(fp);
			struct stat f_stat;
			ret = stat(m_vFileNames[0].c_str(), &f_stat);
			if (ret)
			{
                f_stat.st_size = 0;
			}
            m_vCurrentSize.push_back(f_stat.st_size);
            m_vMutex.push_back(new boost::mutex);
		}

		m_bInited = true;

		return 0;
	}

	void SetChecker(void) 		// 注意：在用户进程fork之后调用SetChecker，因为fork时不会fork检测线程
	{
		if (m_bInited && !m_bWrite2Device)
		{
            boost::thread trd(boost::bind(&Logger::Checker, this));
            trd.detach();
		}
	}

	int LogWrite(ErrorInfo& info, const std::string& level, const std::string& module, const std::string& format, ...)
	{
		if (!m_bInited)
		{
			SET_ERROR(info, SWITCH_LOGGER_ERRNO_NOTINIT, std::string("logger not initialized."));
			return SWITCH_LOGGER_ERRNO_NOTINIT;
		}

		va_list ap;
		va_start(ap, format);
		int ret = LogWrite(info, level, module, format, ap);
		va_end(ap);
		return ret;
	}

	int LogWrite(const std::string& level, const std::string& module, const std::string& format, ...)
	{
		ErrorInfo info;

		if (!m_bInited)
		{
			SET_ERROR(info, SWITCH_LOGGER_ERRNO_NOTINIT, std::string("logger not initialized."));
			throw info;
			return SWITCH_LOGGER_ERRNO_NOTINIT;
		}

		va_list ap;
		va_start(ap, format);
		info.clear();
		int ret = LogWrite(info, level, module, format, ap);
		va_end(ap);
		if (ret) throw info;
		return ret;
	}

	int LogWrite(ErrorInfo& info, const std::string& level, const std::string& module, const std::string& format, va_list ap)
	{
		std::map< std::string, int >::iterator iter;
		int nLevel;
		time_t currTime;
		struct tm currTm;
		char currStr[64] = {'\0'};
		int len = 0;
		int ret1, ret2, ret3;

		iter = m_mLevels.find(level);
		if (iter == m_mLevels.end())
		{
			SET_ERROR(info, SWITCH_LOGGER_ERRNO_PARAMENT, std::string("parament level:") + level + std::string("invalid."));
			return SWITCH_LOGGER_ERRNO_PARAMENT;
		}
		nLevel = iter->second;
		if (nLevel > m_nBaseLevel)
		{
			return 0;
		}
		if (!m_bSplitFileByLevel || m_bWrite2Device)
		{
			nLevel = 0;
		}

		time(&currTime);
		localtime_r(&currTime, &currTm);
		len = strftime(currStr, 64, "[%Y-%m-%d_%H:%M:%S] ", &currTm);
		if (module == "")
		{
			snprintf(currStr + len, 64 - len, "%s[%s]: ", m_sFilePrefix.c_str(), level.c_str());
		}
		else
		{
			snprintf(currStr + len, 64 - len, "%s[%s]: ", module.c_str(), level.c_str());
		}
		currStr[63] = '\0';
		boost::lock_guard< boost::mutex > mGuard(*m_vMutex[nLevel]);
		if ((ret1 = fprintf(m_vFilePointers[nLevel], "%s", currStr)) >= 0 &&
			(ret2 = vfprintf(m_vFilePointers[nLevel], format.c_str(), ap)) >= 0 &&
			(ret3 = fprintf(m_vFilePointers[nLevel], "\n")) >= 0)
		{
            if (!m_bWrite2Device)
            {
                len = ret1 + ret2 + ret3;
                m_vCurrentSize[nLevel] += len;
                if (m_vCurrentSize[nLevel] >= m_nFileSize && MoveFile_Unlock(nLevel, info))
                {
                    return info.errno_;
                }
            }
		}
		else
		{
			SET_ERROR(info, SWITCH_LOGGER_ERRNO_WRITE, "write log failed.");
			return SWITCH_LOGGER_ERRNO_WRITE;
		}

		return 0;
	}

private:
	/// TODO
	int ValidCheck(void) { return 0;}

	int MoveFile_Unlock(int nLevel, ErrorInfo& info)
	{
		time_t currTime;
		struct tm currTm;
		char currStr[20] = {'\0'};
		std::string bkFile;
		FILE* fp;
        struct stat f_stat;
		int ret;

		fclose(m_vFilePointers[nLevel]);
		m_vFilePointers[nLevel] = NULL;
		time(&currTime);
		localtime_r(&currTime, &currTm);
		strftime(currStr, 20, "%Y%m%d%H%M%S", &currTm);
		bkFile = m_vFileNames[nLevel] + std::string("_") + std::string(currStr);
		ret = rename(m_vFileNames[nLevel].c_str(), bkFile.c_str());
		if (ret)
		{
			unlink(m_vFileNames[nLevel].c_str());
		}
        fp = fopen(m_vFileNames[nLevel].c_str(), "a+");
        if (!fp)
        {
			SET_ERROR(info, errno, std::string("open file:") + m_vFileNames[nLevel] + std::string(" failed:") + std::string(strerror(errno)));
			return errno;
        }
        setvbuf(fp, NULL, _IONBF, 0);
        ret = stat(m_vFileNames[nLevel].c_str(), &f_stat);
        if (ret)
        {
        	f_stat.st_size = 0;
        }
        m_vFilePointers[nLevel] = fp;
        m_vCurrentSize[nLevel] = f_stat.st_size;

		return 0;
	}

	void Checker(void)
	{
		FILE* fp;
        struct stat f_stat;
		int ret;

        if (!m_bInited || m_vMutex.size() != m_vFilePointers.size())
		{
			return;
		}
		while (1)
		{
			for (size_t i = 0; i < m_vMutex.size(); ++i)
			{
				boost::lock_guard< boost::mutex > mGuard(*m_vMutex[i]);
				if (access(m_vFileNames[i].c_str(), F_OK))
				{
					fclose(m_vFilePointers[i]);
					fp = fopen(m_vFileNames[i].c_str(), "a+");
					if (!fp)
					{
						continue;
					}
					setvbuf(fp, NULL, _IONBF, 0);
					ret = stat(m_vFileNames[i].c_str(), &f_stat);
					if (ret)
					{
						f_stat.st_size = 0;
					}
					m_vFilePointers[i] = fp;
					m_vCurrentSize[i] = f_stat.st_size;
				}
			}
            boost::this_thread::sleep(boost::posix_time::minutes(1));
		}

	}

	void Cleanup(void)
	{
		for (unsigned int i = 0; i < m_vFilePointers.size(); ++i)
		{
            fclose(m_vFilePointers[i]);
		}
		for (unsigned int i = 0; i < m_vMutex.size(); ++i)
		{
            delete m_vMutex[i];
		}
	}

private:
	std::vector< std::string >			m_vLevels;				// 级别从高到底
	std::map< std::string, int >		m_mLevels;
	std::string							m_sBaseLevel;
	int									m_nBaseLevel;
	unsigned long 						m_nFileSize;
	std::string							m_sFilePath;
	std::string							m_sFilePrefix;
	std::string							m_sFileSuffix;
	bool								m_bSplitFileByLevel;
	bool								m_bWrite2Device;
	bool								m_bInited;

	std::vector< FILE* >				m_vFilePointers;
	std::vector< std::string >			m_vFileNames;
	std::vector< unsigned long >		m_vCurrentSize;
	std::vector< boost::mutex* >		m_vMutex;
};

/// 单例
class LoggerSingleton : public Logger, public boost::serialization::singleton< LoggerSingleton >
{
protected:
	LoggerSingleton(void) {}
	~LoggerSingleton(void) {}
};


CLOSE_NAMESPACE_SWITCHTOOL


#endif // SWITCH_LOGGER_HPP

