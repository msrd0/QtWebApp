set(PACKAGE_VERSION @qtwebapp_VERSION@)

if("@qtwebapp_VERSION@" MATCHES "^([0-9]+\\.[0-9]+)\\.") # only care about major.minor
	set(_package_version "${CMAKE_MATCH_1}")
else()
	set(_package_version "@qtwebapp_VERSION@")
endif()

if(PACKAGE_FIND_VERSION MATCHES "^([0-9]+\\.[0-9]+)\\.") # only care about major.minor
	set(_requested_version "${CMAKE_MATCH_1}")
else()
	set(_requested_version "${PACKAGE_FIND_VERSION}")
endif()

if(_requested_version STREQUAL _package_version)
	set(PACKAGE_VERSION_COMPATIBLE TRUE)
else()
	set(PACKAGE_VERSION_COMPATIBLE FALSE)
endif()

if (PACKAGE_FIND_VERSION STREQUAL PACKAGE_VERSION)
	set(PACKAGE_VERSION_EXACT TRUE)
endif()

# check bitness
if ("${CMAKE_SIZEOF_VOID_P}" STREQUAL "" OR "@CMAKE_SIZEOF_VOID_P@" STREQUAL "")
	return()
endif()

if (NOT CMAKE_SIZEOF_VOID_P STREQUAL "@CMAKE_SIZEOF_VOID_P@")
	math(EXPR installedBits "@CMAKE_SIZEOF_VOID_P@ * @CMAKE_SIZEOF_VOID_P@")
	set(PACKAGE_VERSION "@qtwebapp_VERSION@ (${installedBits}bit)")
	set(PACKAGE_VERSION_UNSUITABLE TRUE)
endif()
