project(newton)

include_directories(dAnimation)
include_directories(dContainers)
include_directories(dMath)
include_directories(dModel)
include_directories(dVehicle)
include_directories(dgMeshUtil)
include_directories(dCustomJoints)
include_directories(dVehicle/dPlayerController)
include_directories(dVehicle/dMultiBodyVehicle)
include_directories(dgCore)
include_directories(dgNewton)
include_directories(dgPhysics)

file(GLOB_RECURSE SOURCES
		# Common
		dAnimation/*.cpp
		dContainers/*.cpp
		dMath/*.cpp
		dModel/*.cpp
		dVehicle/*.cpp
		dgMeshUtil/*.cpp
		dCustomJoints/*.cpp
		dVehicle/dPlayerController/*.cpp
		dVehicle/dMultiBodyVehicle/*.cpp

		# Core
		dgCore/*.cpp
		dgNewton/*.cpp
		dgPhysics/*.cpp

		)

file(GLOB_RECURSE REMOVE_SOURCES
		"dCustomJoints/dCustomJointLibraryStdAfx.cpp"
		)
list(REMOVE_ITEM SOURCES ${REMOVE_SOURCES})

file(GLOB_RECURSE HEADERS
		# Common
		dAnimation/*.h
		dContainers/*.h
		dMath/*.h
		dModel/*.h
		dVehicle/*.h
		dgMeshUtil/*.h
		dCustomJoints/*.h
		dVehicle/dPlayerController/*.h
		dVehicle/dMultiBodyVehicle/*.h

		# Core
		dgCore/*.h
		dgNewton/*.h
		dgPhysics/*.h
		)

#file(GLOB_RECURSE REMOVE_HEADERS
#		"dCustomJoints/dCustomJointLibraryStdAfx.h"
#		)
#list(REMOVE_ITEM HEADERS ${REMOVE_HEADERS})

add_library(newton OBJECT ${HEADERS} ${SOURCES})

if (IOS)
	set_target_properties(
			newton
			PROPERTIES
			XCODE_ATTRIBUTE_IPHONEOS_DEPLOYMENT_TARGET 8.0
	)
endif(IOS)

if (PTR_SIZE EQUAL 8)
	set(BUILD_64 TRUE)
else ()
	set(BUILD_64 FALSE)
endif ()

#check for MAC
if(APPLE)
	add_definitions(-D_MACOSX_VER)
endif(APPLE)

#check for UNIX
if(UNIX)
	if (BUILD_64)
		add_definitions(-D_POSIX_VER_64)
	else (BUILD_64)
		add_definitions(-D_POSIX_VER)
	endif (BUILD_64)
endif(UNIX)

if(MSVC)
	if (BUILD_64)
		add_definitions(-D_WIN_64_VER)
	else (BUILD_64)
		add_definitions(-D_WIN_32_VER)
	endif (BUILD_64)
endif(MSVC)

