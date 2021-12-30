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
			XCODE_ATTRIBUTE_IPHONEOS_DEPLOYMENT_TARGET 9.0
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

	set(CMAKE_C_FLAGS_DEBUG "/MTd")
	set(CMAKE_C_FLAGS_RELWITHDEBINFO "/MT")
	set(CMAKE_C_FLAGS_RELEASE "/MT")
	set(CMAKE_CXX_FLAGS_DEBUG "/MTd")
	set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "/MT")
	set(CMAKE_CXX_FLAGS_RELEASE "/MT")

	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /Zi /W4 /Od")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Zi /GS- /W4 /Ox /Oi /Ot /Ob2 /fp:fast")

	#	set_target_properties(newton
#			PROPERTIES
#			COMPILE_FLAGS "/YudAnimationStdAfx.h"
#			COMPILE_FLAGS "/YudContainersStdAfx.h"
#			COMPILE_FLAGS "/YudStdAfxMath.h"
#			COMPILE_FLAGS "/YudModelStdAfx.h"
#			COMPILE_FLAGS "/YudStdafxVehicle.h"
#			COMPILE_FLAGS "/YudCustomJointLibraryStdAfx.h"
#			COMPILE_FLAGS "/YudgStdAfx.h"
#			COMPILE_FLAGS "/YuNewtonStdAfx.h"
#			COMPILE_FLAGS "/YudgPhysicsStdafx.h"
#			)
	#set_source_files_properties(dAnimation/dAnimationStdAfx.cpp PROPERTIES COMPILE_FLAGS "/YcdAnimationStdAfx.h")

	#set_target_properties(newton PROPERTIES COMPILE_FLAGS "/YudContainersStdAfx.h")
	#set_source_files_properties(dContainers/dContainersStdAfx.cpp PROPERTIES COMPILE_FLAGS "/YcdContainersStdAfx.h")

	#set_target_properties(newton PROPERTIES COMPILE_FLAGS "/YudStdAfxMath.h")
	#set_source_files_properties(dMath/dStdAfxMath.cpp PROPERTIES COMPILE_FLAGS "/YcdStdAfxMath.h")

	#set_target_properties(newton PROPERTIES COMPILE_FLAGS "/YudModelStdAfx.h")
	#set_source_files_properties(dModel/dModelStdAfx.cpp PROPERTIES COMPILE_FLAGS "/YcdModelStdAfx.h")

	#set_target_properties(newton PROPERTIES COMPILE_FLAGS "/YudStdafxVehicle.h")
	#set_source_files_properties(dVehicle/dStdafxVehicle.cpp PROPERTIES COMPILE_FLAGS "/YcdStdafxVehicle.h")

	#set_target_properties(newton PROPERTIES COMPILE_FLAGS "/YudCustomJointLibraryStdAfx.h")
	#set_source_files_properties(dCustomJoints/dCustomJointLibraryStdAfx.cpp PROPERTIES COMPILE_FLAGS "/YcdCustomJointLibraryStdAfx.h")

	#set_target_properties(newton PROPERTIES COMPILE_FLAGS "/YudgStdAfx.h")
	#set_source_files_properties(dgCore/dgTypes.cpp PROPERTIES COMPILE_FLAGS "/YcdgStdAfx.h")

	#set_target_properties(newton PROPERTIES COMPILE_FLAGS "/YuNewtonStdAfx.h")
	#set_source_files_properties(dNewton/NewtonClass.cpp PROPERTIES COMPILE_FLAGS "/YcNewtonStdAfx.h")

	#set_target_properties(newton PROPERTIES COMPILE_FLAGS "/YudgPhysicsStdafx.h")
	#set_source_files_properties(dgPhysics/dgWorld.cpp PROPERTIES COMPILE_FLAGS "/YcdgPhysicsStdafx.h")

endif(MSVC)

