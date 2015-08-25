#
# CMake file to build the Palgo tests. Note that Palgo itself is header
# only and doesn't require building.
#
# Mark Grimes
# 25/Aug/2015
#

PROJECT( PalgoTest )

CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

SET( ${PROJECT_NAME}_MAJOR_VERSION 0 )
SET( ${PROJECT_NAME}_MINOR_VERSION 0 )
SET( ${PROJECT_NAME}_PATCH_LEVEL 0 )


ADD_DEFINITIONS( "-std=c++11" )

INCLUDE_DIRECTORIES( "${CMAKE_SOURCE_DIR}/include" )
AUX_SOURCE_DIRECTORY( "${CMAKE_SOURCE_DIR}/test" unittests_sources )
ADD_EXECUTABLE( palgoTests ${unittests_sources} )
