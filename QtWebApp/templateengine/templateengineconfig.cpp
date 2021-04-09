#include "templateengineconfig.h"

using namespace qtwebapp;

TemplateEngineConfig::TemplateEngineConfig() {}

TemplateEngineConfig::TemplateEngineConfig(const QSettings &settings) {
	parseSettings(settings);
}

TemplateEngineConfig::TemplateEngineConfig(QSettings *settings) {
	parseSettings(*settings);
}

void
TemplateEngineConfig::parseSettings(const QSettings &settings) {
#ifdef Q_OS_WIN
	if (settings.format() != QSettings::NativeFormat)
#endif
		fileName = settings.fileName();

	path = settings.value("path", path).toString();
	suffix = settings.value("suffix", suffix).toString();
	encoding = settings.value("encoding", encoding).toString();

	cacheSize = parseNum(settings.value("cacheSize", cacheSize), 1024);
	cacheTime = parseNum(settings.value("cacheTime", cacheTime));
}
