#include "qtwebappglobal.h"

const char *qtwebapp::getQtWebAppLibVersion() {
	return QTWEBAPP_VERSION_STR;
}

int qtwebapp::parseNum(const QVariant &v, int base) {
	QString str = v.toString();
	int mul = 1;
	if (str.endsWith('K'))
		mul *= base;
	else if (str.endsWith('M'))
		mul *= base * base;
	else if (str.endsWith('G'))
		mul *= base * base * base;
	if (mul != 1)
		str = str.mid(str.length() - 1);
	return str.toInt() * mul;
}
