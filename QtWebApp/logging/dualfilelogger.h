/**
  @file
  @author Stefan Frings
*/

#pragma once

#include "qtwebappglobal.h"
#include "filelogger.h"
#include "logger.h"

#include <QSettings>

namespace qtwebapp {

/**
  Logs messages into two log files simultaneously.
  May be used to create two logfiles with different configuration settings.
  @see FileLogger for a description of the two underlying loggers.
*/

class QTWEBAPP_EXPORT DualFileLogger : public Logger {
	Q_OBJECT
	Q_DISABLE_COPY(DualFileLogger)
public:
	
	/**
	  Constructor.
	  @param firstSettings Configuration settings for the first log file, usually stored in an INI file.
	  Must not be 0.
	  Settings are read from the current group, so the caller must have called settings->beginGroup().
	  Because the group must not change during runtime, it is recommended to provide a
	  separate QSettings instance that is not used by other parts of the program.
	  The FileLogger does not take over ownership of the QSettings instance, so the caller
	  should destroy it during shutdown.
	  @param secondSettings Same as firstSettings, but for the second log file.
	  @param refreshInterval Interval of checking for changed config settings in msec, or 0=disabled
	  @param parent Parent object.
	*/
	DualFileLogger(QSettings* firstSettings, QSettings* secondSettings,
	               const int refreshInterval=10000, QObject *parent = nullptr);
	
	/**
	  Decorate and log the message, if type>=minLevel.
	  This method is thread safe.
	  @param type Message type (level)
	  @param message Message text
	  @param file Name of the source file where the message was generated (usually filled with the macro __FILE__)
	  @param function Name of the function where the message was generated (usually filled with the macro __LINE__)
	  @param line Line Number of the source file, where the message was generated (usually filles with the macro __func__ or __FUNCTION__)
	  @see LogMessage for a description of the message decoration.
	*/
	virtual void log(const QtMsgType type, const QString& message, const QString &file=QString(),
	                 const QString &function=QString(), const int line=0);
	
	/**
	  Clear the thread-local data of the current thread.
	  This method is thread safe.
	  @param buffer Whether to clear the backtrace buffer
	  @param variables Whether to clear the log variables
	*/
	virtual void clear(const bool buffer=true, const bool variables=true);
	
private:
	
	/** First logger */
	FileLogger* firstLogger;
	
	/** Second logger */
	FileLogger* secondLogger;
	
};

} // end of namespace
