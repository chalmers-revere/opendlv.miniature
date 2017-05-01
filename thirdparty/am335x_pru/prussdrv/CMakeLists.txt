project(prussdrv)

include_directories(include)

add_library(prussdrv SHARED src/prussdrv.c)

install(TARGETS prussdrv DESTINATION lib)
install(DIRECTORY include/pruss DESTINATION include)
