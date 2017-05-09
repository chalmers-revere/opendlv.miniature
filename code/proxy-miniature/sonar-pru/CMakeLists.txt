# Copyright (C) 2016 Chalmers Revere
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

CMAKE_MINIMUM_REQUIRED (VERSION 2.8)

PROJECT (opendlv-proxy-miniature-sonar-pru)

###########################################################################
# Set the search path for .cmake files.
SET (CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../../cmake.Modules" ${CMAKE_MODULE_PATH})

# Add a local CMake module search path dependent on the desired installation destination.
# Thus, artifacts from the complete source build can be given precendence over any installed versions.
IF(UNIX)
    SET (CMAKE_MODULE_PATH "${CMAKE_INSTALL_PREFIX}/share/cmake-${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}/Modules" ${CMAKE_MODULE_PATH})
ENDIF()
IF(WIN32)
    SET (CMAKE_MODULE_PATH "${CMAKE_INSTALL_PREFIX}/CMake-${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}/Modules" ${CMAKE_MODULE_PATH})
ENDIF()

###########################################################################
# Include flags for compiling.
INCLUDE (CompileFlags)

###########################################################################
# Find and configure CxxTest.
INCLUDE (CheckCxxTestEnvironment)

###########################################################################
# Find OpenDaVINCI.
FIND_PACKAGE (OpenDaVINCI REQUIRED)

###########################################################################
# Find Pruss.
find_package(Pruss REQUIRED)

###########################################################################
# Find ODVDVehicle.
set(CMAKE_MODULE_PATH "${ODVDVEHICLE_DIR}/share/cmake-${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}/Modules" ${CMAKE_MODULE_PATH})
find_package(ODVDVehicle REQUIRED)

###########################################################################
# Find ODVDOpenDLVData.
set(CMAKE_MODULE_PATH "${ODVDOPENDLVDATA_DIR}/share/cmake-${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}/Modules" ${CMAKE_MODULE_PATH})
find_package(ODVDOpenDLVData REQUIRED)

###########################################################################
# Find ODVDMiniature.
find_package(ODVDMiniature REQUIRED)

###########################################################################
# Find AutomotiveData.
set(AUTOMOTIVEDATA_DIR "${OPENDAVINCI_DIR}")
find_package(AutomotiveData REQUIRED)

###############################################################################
# Set header files from ODVDMiniature.
INCLUDE_DIRECTORIES (SYSTEM ${ODVDMINIATURE_INCLUDE_DIRS})
# Set header files from AutomotiveData.
INCLUDE_DIRECTORIES (SYSTEM ${AUTOMOTIVEDATA_INCLUDE_DIRS})
# Set header files from ODVDVehicle.
INCLUDE_DIRECTORIES (SYSTEM ${ODVDVEHICLE_INCLUDE_DIRS})
# Set header files from ODVDOpenDLVData.
INCLUDE_DIRECTORIES (SYSTEM ${ODVDOPENDLVDATA_INCLUDE_DIRS})
# Set header files from Pruss.
INCLUDE_DIRECTORIES (SYSTEM ${PRUSS_INCLUDE_DIRS})
# Set header files from OpenDaVINCI.
INCLUDE_DIRECTORIES (SYSTEM ${OPENDAVINCI_INCLUDE_DIRS})
# Set include directory.
INCLUDE_DIRECTORIES(include)

# Set libraries to link against.
set(LIBRARIES ${OPENDAVINCI_LIBRARIES}
  ${PRUSS_LIBRARIES}
  ${ODVDMINIATURE_LIBRARIES}
  ${ODVDVEHICLE_LIBRARIES}
  ${ODVDOPENDLVDATA_LIBRARIES}
  ${ODVDMINIATURE_LIBRARIES}
  ${AUTOMOTIVEDATA_LIBRARIES})


###############################################################################
# Build this project.
FILE(GLOB_RECURSE thisproject-sources "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
ADD_LIBRARY (${PROJECT_NAME}-static STATIC ${thisproject-sources})
ADD_EXECUTABLE (${PROJECT_NAME} "${CMAKE_CURRENT_SOURCE_DIR}/apps/${PROJECT_NAME}.cpp")
TARGET_LINK_LIBRARIES (${PROJECT_NAME} ${PROJECT_NAME}-static ${LIBRARIES}) 


# Assemble firmware
file(GLOB FIRMWARE_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/firmware/*.p")

foreach(FIRMWARE_SRC ${FIRMWARE_SRCS})
  string(REPLACE ".p" ".bin" FIRMWARE_BIN ${FIRMWARE_SRC}) 
  get_filename_component(FIRMWARE_BIN ${FIRMWARE_BIN} NAME)

  add_custom_command(OUTPUT ${FIRMWARE_BIN}
    COMMAND ${PASM_ASSEMBLER} -b ${FIRMWARE_SRC}
    DEPENDS ${FIRMWARE_SRC}
  )

  add_custom_target(${PROJECT_NAME}-${FIRMWARE_BIN} DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${FIRMWARE_BIN})
  add_dependencies(${PROJECT_NAME} ${PROJECT_NAME}-${FIRMWARE_BIN})
  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${FIRMWARE_BIN} DESTINATION share/${PROJECT_NAME}/firmware COMPONENT opendlv-proxy-miniature)
endforeach()

###############################################################################
# Enable CxxTest for all available testsuites.
IF(CXXTEST_FOUND)
    FILE(GLOB thisproject-testsuites "${CMAKE_CURRENT_SOURCE_DIR}/testsuites/*.h")
    
    FOREACH(testsuite ${thisproject-testsuites})
        STRING(REPLACE "/" ";" testsuite-list ${testsuite})

        LIST(LENGTH testsuite-list len)
        MATH(EXPR lastItem "${len}-1")
        LIST(GET testsuite-list "${lastItem}" testsuite-short)

        SET(CXXTEST_TESTGEN_ARGS ${CXXTEST_TESTGEN_ARGS} --world=${PROJECT_NAME}-${testsuite-short})
        CXXTEST_ADD_TEST(${testsuite-short}-TestSuite ${testsuite-short}-TestSuite.cpp ${testsuite})
        IF(UNIX)
            IF( (   ("${CMAKE_SYSTEM_NAME}" STREQUAL "Linux")
                 OR ("${CMAKE_SYSTEM_NAME}" STREQUAL "FreeBSD")
                 OR ("${CMAKE_SYSTEM_NAME}" STREQUAL "DragonFly") )
                AND (NOT "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang") )
                SET_SOURCE_FILES_PROPERTIES(${testsuite-short}-TestSuite.cpp PROPERTIES COMPILE_FLAGS "-Wno-effc++ -Wno-float-equal -Wno-error=suggest-attribute=noreturn")
            ELSE()
                SET_SOURCE_FILES_PROPERTIES(${testsuite-short}-TestSuite.cpp PROPERTIES COMPILE_FLAGS "-Wno-effc++ -Wno-float-equal")
            ENDIF()
        ENDIF()
        IF(WIN32)
            SET_SOURCE_FILES_PROPERTIES(${testsuite-short}-TestSuite.cpp PROPERTIES COMPILE_FLAGS "")
        ENDIF()
        SET_TESTS_PROPERTIES(${testsuite-short}-TestSuite PROPERTIES TIMEOUT 3000)
        TARGET_LINK_LIBRARIES(${testsuite-short}-TestSuite ${PROJECT_NAME}-static ${LIBRARIES})
    ENDFOREACH()
ENDIF(CXXTEST_FOUND)

###############################################################################
# Install this project.
INSTALL(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin COMPONENT opendlv-proxy-miniature)
INSTALL(TARGETS ${PROJECT_NAME}-static DESTINATION lib COMPONENT opendlv-proxy-miniature)
INSTALL(FILES man/${PROJECT_NAME}.1 DESTINATION man/man1 COMPONENT opendlv-proxy-miniature)

# Install header files.
INSTALL(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include/" DESTINATION include/opendlv-proxy-miniature COMPONENT opendlv-proxy-miniature)
