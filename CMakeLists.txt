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

SET( LIBS "" )
ADD_DEFINITIONS( "-std=c++11" )
INCLUDE_DIRECTORIES( "${CMAKE_SOURCE_DIR}/include" )

OPTION( BUILD_SYCL_TESTS "Set to ON to build SYCL tests. Requires ComputeCpp on your system (https://www.codeplay.com/products/computecpp)." OFF )
MESSAGE( STATUS "BUILD_SYCL_TESTS: ${BUILD_SYCL_TESTS}" )
IF( BUILD_SYCL_TESTS )
	# ToDo - Find or write a proper FindComputeCpp.cmake file
	IF( ComputeCpp_DIR )
		SET( ComputeCpp_INCLUDE_DIRS "${ComputeCpp_DIR}/include" )
		SET( ComputeCpp_LIBRARY_DIRS "${ComputeCpp_DIR}/lib" )
		SET( ComputeCpp_LIBRARIES "-lSYCL" )
		SET( ComputeCpp_FOUND TRUE )
	ENDIF()

	IF( NOT ComputeCpp_FOUND )
		MESSAGE( SEND_ERROR "ComputeCpp not found. Please set BUILD_SYCL_TESTS to OFF or ComputeCpp_DIR=/path/to/ComputeCpp" )
	ELSE()
		INCLUDE_DIRECTORIES( "${ComputeCpp_INCLUDE_DIRS}" )
		LINK_DIRECTORIES( "${ComputeCpp_LIBRARY_DIRS}" )
		SET( LIBS ${LIBS} ${ComputeCpp_LIBRARIES} )
		SET( DEVICE_COMPILER ${ComputeCpp_DIR}/bin/compute++ )
		SET( DEVICE_COMPILER_FLAGS -O2 -sycl -intelspirmetadata -emit-llvm -D__DEVICE_SPIR32__ -DBUILD_PLATFORM_SPIR -DPALGO_MAKE_SYCL_TESTS -I${ComputeCpp_INCLUDE_DIRS} )
		SET( SYCL_INPUT_FILES "sycl_test.cpp" )
		FOREACH( src ${SYCL_INPUT_FILES} ) # List of files with sycl source code in them
			SET( INPUT_FILE "${CMAKE_SOURCE_DIR}/test/${src}" ) # temp variable for convenience
			SET( OUTPUT_SYCL_FILE "${src}.sycl" ) # temp variable for convenience
			ADD_CUSTOM_COMMAND( OUTPUT ${OUTPUT_SYCL_FILE} COMMAND ${DEVICE_COMPILER} ${CMAKE_CXX_FLAGS} ${DEVICE_COMPILER_FLAGS} -I${CMAKE_SOURCE_DIR}/include -c ${INPUT_FILE} -o ${src}.bc DEPENDS ${INPUT_FILE} )
			SET_PROPERTY( SOURCE ${INPUT_FILE} PROPERTY COMPILE_FLAGS "-DPALGO_MAKE_SYCL_TESTS --include ${OUTPUT_SYCL_FILE}" )
			SET( sycl_dependencies ${sycl_dependencies} ${OUTPUT_SYCL_FILE} )
		ENDFOREACH()
	ENDIF()
ENDIF()


AUX_SOURCE_DIRECTORY( "${CMAKE_SOURCE_DIR}/test" unittests_sources )
ADD_EXECUTABLE( palgoTests ${unittests_sources} ${sycl_dependencies} )

IF( ComputeCpp_FOUND )
	#FOREACH( src ${SYCL_INPUT_FILES} )
	#	BUILD_SPIR( palgoTests "${CMAKE_SOURCE_DIR}/test/${src}" "." )
	#	ADD_DEPENDENCIES( palgoTests "${src}.sycl" )
	#ENDFOREACH()
	
	TARGET_LINK_LIBRARIES( palgoTests "${ComputeCpp_LIBRARIES}" )
ENDIF()