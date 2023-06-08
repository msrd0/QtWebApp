**THIS PROJECT IS NO LONGER MAINTAINED**

# QtWebApp [![Build](https://github.com/msrd0/QtWebApp/workflows/Build/badge.svg)](https://github.com/msrd0/QtWebApp/actions?query=workflow%3ABuild) [![License](https://img.shields.io/badge/license-LGPL--3.0-blue.svg)](https://www.gnu.org/licenses/lgpl-3.0)

This library was forked from http://stefanfrings.de/qtwebapp/index-en.html

QtWebApp is a library to develop server-side web applications in C++. It depends on Qt5, minimum required version is 5.5.

The library comes with 3 components:

- HttpServer
- TemplateEngine
- Logging

## Usage

This short example demonstrates how to use the library:

```C++
class DefaultRequestHandler : public HttpRequestHandler
{
public:
	void service(HttpRequest &request, HttpResponse &response);
}

void DefaultRequestHandler::service(HttpRequest &request, HttpResponse &response)
{
	response.write("<html><body><h1>" + request.getPath() + "</h1></body></html>");
}

int main(int argc, char **argv)
{
	QCoreApplication app(argc, argv);
	
	QSettings *config = new QSettings;
	// ...
	
	new HttpListener(config, new DefaultRequestHandler);
	return app.exec();
}
```

You can the compile it using cmake, like this:

```CMake
# ...
find_package(QtWebApp REQUIRED COMPONENTS HttpServer)
add_executable(thinkofabettername main.cpp)
target_link_libraries(thinkofabettername ${QtWebApp_LIBRARIES})
# ...
```

## Build

QtWebApp uses CMake as the build system. To compile, simply run:

```bash
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j4
sudo make install
```
