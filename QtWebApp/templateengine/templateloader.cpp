/**
  @file
  @author Stefan Frings
*/

#include "templateloader.h"
#include <QFile>
#include <QFileInfo>
#include <QStringList>
#include <QDir>
#include <QSet>

using namespace qtwebapp;

TemplateLoader::TemplateLoader(const TemplateEngineConfig &cfg, QObject* parent)
	: QObject(parent)
{
	templatePath=cfg.path;
	// Convert relative path to absolute, based on the directory of the config file.
	if (!cfg.fileName.isEmpty() && QDir::isRelativePath(templatePath))
	{
		QFileInfo configFile(cfg.fileName);
		templatePath=QFileInfo(configFile.absolutePath(),templatePath).absoluteFilePath();
	}
	fileNameSuffix=cfg.suffix;
	QString encoding=cfg.encoding;
	if (encoding.isEmpty())
	{
		textCodec=QTextCodec::codecForLocale();
	}
	else
	{
		textCodec=QTextCodec::codecForName(encoding.toLocal8Bit());
	}
#ifdef CMAKE_DEBUG
	qDebug("TemplateLoader: path=%s, codec=%s",qPrintable(templatePath),textCodec->name().data());
#endif
}

TemplateLoader::~TemplateLoader()
{}

QString TemplateLoader::tryFile(QString localizedName)
{
	QString fileName=templatePath+"/"+localizedName+fileNameSuffix;
#ifdef CMAKE_DEBUG
	qDebug("TemplateCache: trying file %s",qPrintable(fileName));
#endif
	QFile file(fileName);
	if (file.exists()) {
		file.open(QIODevice::ReadOnly);
		QString document=textCodec->toUnicode(file.readAll());
		file.close();
		if (file.error())
		{
			qCritical("TemplateLoader: cannot load file %s, %s",qPrintable(fileName),qPrintable(file.errorString()));
			return "";
		}
		else
		{
			return document;
		}
	}
	return "";
}

Template TemplateLoader::getTemplate(QString templateName, QString locales)
{
	QSet<QString> tried; // used to suppress duplicate attempts
	QStringList locs=locales.split(',',QString::SkipEmptyParts);
	
	// Search for exact match
	foreach (QString loc,locs)
	{
		loc.replace(QRegExp(";.*"),"");
		loc.replace('-','_');
		QString localizedName=templateName+"-"+loc.trimmed();
		if (!tried.contains(localizedName))
		{
			QString document=tryFile(localizedName);
			if (!document.isEmpty()) {
				return Template(document,localizedName);
			}
			tried.insert(localizedName);
		}
	}
	
	// Search for correct language but any country
	foreach (QString loc,locs)
	{
		loc.replace(QRegExp("[;_-].*"),"");
		QString localizedName=templateName+"-"+loc.trimmed();
		if (!tried.contains(localizedName))
		{
			QString document=tryFile(localizedName);
			if (!document.isEmpty())
			{
				return Template(document,localizedName);
			}
			tried.insert(localizedName);
		}
	}
	
	// Search for default file
	QString document=tryFile(templateName);
	if (!document.isEmpty())
	{
		return Template(document,templateName);
	}
	
	qCritical("TemplateCache: cannot find template %s",qPrintable(templateName));
	return Template("",templateName);
}
