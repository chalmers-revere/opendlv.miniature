project(pasm)

set(CMAKE_C_FLAGS "-Wall")

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  add_definitions("-D_UNIX_")
endif()

file(GLOB SRC "src/*.c")

add_executable(pasm ${SRC})

install(TARGETS pasm RUNTIME DESTINATION bin)
install(FILES man/pasm.1 DESTINATION man/man1)
