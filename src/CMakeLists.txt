
file(GLOB SOURCE *.cpp)
add_executable(img_proc ${SOURCE})
message(STATUS ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(img_proc PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}\\..\\include)
target_compile_options(img_proc PRIVATE /ZI /DEBUG /Qopt-report:max)
