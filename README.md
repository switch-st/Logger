Logger
======

Linux环境下cpp日志库，支持日志分级别、自定义文件大小、自定义文件前后缀、打印日志到设备(终端)等功能，线程安全。

地址
	https://github.com/switch-st/Logger.git

说明
 * 支持自定义级别，默认定义五个级别(FATAL、ERROR、WARNING、INFO、DEBUG)
 * 支持分级别记录日志，定义基准级别，基准之下的不记录，基准之上的记录(默认基准级别:INFO)
 * 支持自定义单个日志文件大小，超过该大小自动分文件
 * 支持自定义日志文件路径、日志文件前缀后缀
 * 支持按日志级别拆分日志，不同级别的日志写入不同的文件，便于查看统计
 * 支持打印日志到设备(终端)，便于调试
 * 默认提供一个简单的单例类，便于调用
 * 线程安全
 *   
 * 依赖boost库，须链接boost_thread、boost_system
 * 效率有待提高
 * 欢迎补充

调用
	调用方法详见test目录下测试文件。test.cpp为功能测试文件，Logger.h、Logger.cpp是我们项目中使用该日志库时封装的，采用了七个日志级别，运行时定义基准级别，方便测试。

			by switch
			switch.st@gmail.com

