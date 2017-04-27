# Copyright (C) 2017 Chalmers Revere
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

if(NOT ("${PRUSS_DIR}" STREQUAL ""))
  find_path(PRUSS_INCLUDE_DIR 
    NAMES prussdrv.h 
    PATHS ${PRUSS_DIR}/include/pruss
    NO_DEFAULT_PATH)

  find_library(PRUSS_LIBRARIES
    NAMES prussdrv
    PATHS ${PRUSS_DIR}/lib
    NO_DEFAULT_PATH)
  
  find_file(PASM_ASSEMBLER 
    NAMES pasm
    PATHS ${PRUSS_DIR}/bin
    NO_DEFAULT_PATH)
endif()

if(("${PRUSS_INCLUDE_DIR}" STREQUAL "PRUSS_INCLUDE_DIR-NOTFOUND")
  OR ("${PRUSS_DIR}" STREQUAL "") )
  find_path(PRUSS_INCLUDE_DIR 
    NAMES prussdrv.h
    PATHS /usr/include
      /usr/local/include)

  find_library(PRUSS_LIBRARIES
    NAMES prussdrv
    PATHS /usr/lib
      /usr/lib64
      /usr/local/lib
      /usr/local/lib64)
  
  find_file(PASM_ASSEMBLER 
    NAMES pasm
    PATHS /usr/bin
      /usr/local/bin)
endif()

if("${PRUSS_INCLUDE_DIR}" STREQUAL "")
  message(FATAL_ERROR "Could not find pruss library.")
else()
  set(FOUND_PRUSS 1)
endif()
