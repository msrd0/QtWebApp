#pragma once

#include "qtwebappglobal.h"

#include <QSettings>

namespace qtwebapp {
	class TemplateLoader;

	/**
	 * This class stores all configuration information for the `TemplateCache`
	 * class. It can be either created as a standard object and filled from
	 * c++ code, or be constructed from a `QSettings` object. See `TemplateCache`
	 * for an example of such a configuration file.
	 */
	class QTWEBAPP_EXPORT TemplateEngineConfig {
		friend class TemplateLoader;

	  public:
		/** Creates a config with all standard values. */
		TemplateEngineConfig();
		/** Reads the configuration from the `QSettings` object. */
		TemplateEngineConfig(const QSettings &settings);
		/** Reads the configuration from the `QSettings` object. */
		TemplateEngineConfig(QSettings *settings);

		/// The path where the static files can be found. This can be either an
		/// absolute or relativ path or an qt resource path.
		QString path = ".";

		/// The default suffix of the template files.
		QString suffix = ".tpl";

		/// The encoding that is sent to the web browser in case of text files.
		QString encoding;

		/// The size of the server cache.
		int cacheSize = 1e6;
		/// The timeout of each file in the servers cache.
		int cacheTime = 6e4;

	  private:
		void parseSettings(const QSettings &settings);

		/// The filename of the settings if read from a file. It is used to resolve
		/// relative paths.
		QString fileName;
	};

}
