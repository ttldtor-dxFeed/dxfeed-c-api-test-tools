cmake_minimum_required(VERSION 3.8.0)

cmake_policy(SET CMP0015 NEW)

set(PROJECT_NAME bench)
project(${PROJECT_NAME} LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED on)

add_executable(${PROJECT_NAME}
        src/main.cpp
        )

add_dependencies(${PROJECT_NAME} DXFeed)

set(ADDITIONAL_LIBRARIES "")

if (WIN32)
else ()
    set(ADDITIONAL_LIBRARIES ${ADDITIONAL_LIBRARIES} pthread)
endif ()

target_link_libraries(${PROJECT_NAME} DXFeed ${ADDITIONAL_LIBRARIES})
