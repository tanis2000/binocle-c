project(chipmunk)

include_directories(include)

file(GLOB SOURCES
        src/chipmunk.c
        src/cpArbiter.c
        src/cpArray.c
        src/cpBBTree.c
        src/cpBody.c
        src/cpCollision.c
        src/cpConstraint.c
        src/cpDampedRotarySpring.c
        src/cpDampedSpring.c
        src/cpGearJoint.c
        src/cpGrooveJoint.c
        src/cpHashSet.c
        src/cpMarch.c
        src/cpPinJoint.c
        src/cpPivotJoint.c
        src/cpPolyline.c
        src/cpPolyShape.c
        src/cpRatchetJoint.c
        src/cpRobust.c
        src/cpRotaryLimitJoint.c
        src/cpShape.c
        src/cpSimpleMotor.c
        src/cpSlideJoint.c
        src/cpSpace.c
        src/cpSpaceComponent.c
        src/cpSpaceDebug.c
        src/cpSpaceHash.c
        src/cpSpaceQuery.c
        src/cpSpaceStep.c
        src/cpSpatialIndex.c
        src/cpSweep1D.c
        )

set(SOURCES
        ${SOURCES}
        )


file(GLOB_RECURSE HEADERS
        include/*.h
        )

# Define source files
set_source_files_properties(${SOURCES} PROPERTIES LANGUAGE C)

add_library(chipmunk OBJECT ${SOURCES})

if (IOS)
    set_target_properties(
            chipmunk
            PROPERTIES
            XCODE_ATTRIBUTE_IPHONEOS_DEPLOYMENT_TARGET 13.1
    )
endif (IOS)

if (NOT ANDROID)
    file(GLOB_RECURSE CP_64_BIT_SOURCES
            src/cpHastySpace.c
            )
    list(APPEND SOURCES ${CP_64_BIT_SOURCES})
endif(NOT ANDROID)
