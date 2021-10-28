cmake_minimum_required(VERSION 3.8.0)

cmake_policy(SET CMP0015 NEW)

project(DXFeed)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED on)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED on)

set(DXFEED_INCLUDE_DIR ../dxfeed-c-api/include)
set(DXFEED_SRC_DIR ../dxfeed-c-api/src)
set(DXFEED_DEF_FILE ../dxfeed-c-api/DXFeed.def)

set(DXFEED_EXPORT_HEADERS
        ${DXFEED_INCLUDE_DIR}/DXErrorCodes.h
        ${DXFEED_INCLUDE_DIR}/DXFeed.h
        ${DXFEED_INCLUDE_DIR}/DXTypes.h
        ${DXFEED_INCLUDE_DIR}/EventData.h
        ${DXFEED_INCLUDE_DIR}/RecordData.h
        )

set(DXFEED_HEADER_FILES
        ${DXFEED_SRC_DIR}/BinaryQTPComposer.hpp
        ${DXFEED_SRC_DIR}/BinaryQTPParser.hpp
        ${DXFEED_SRC_DIR}/Candle.h
        ${DXFEED_SRC_DIR}/ClientMessageProcessor.h
        ${DXFEED_SRC_DIR}/Configuration.h
        ${DXFEED_SRC_DIR}/Configuration.hpp
        ${DXFEED_SRC_DIR}/Connection.h
        ${DXFEED_SRC_DIR}/Connection.hpp
        ${DXFEED_SRC_DIR}/ConnectionContextData.h
        ${DXFEED_SRC_DIR}/DXAddressParser.h
        ${DXFEED_SRC_DIR}/DXAlgorithms.h
        ${DXFEED_SRC_DIR}/DXErrorHandling.h
        ${DXFEED_SRC_DIR}/DXMemory.h
        ${DXFEED_SRC_DIR}/DXNetwork.h
        ${DXFEED_SRC_DIR}/DXPMessageData.h
        ${DXFEED_SRC_DIR}/DXProperties.h
        ${DXFEED_SRC_DIR}/DXSockets.h
        ${DXFEED_SRC_DIR}/DXThreads.h
        ${DXFEED_SRC_DIR}/EventManager.h
        ${DXFEED_SRC_DIR}/EventSubscription.h
        ${DXFEED_SRC_DIR}/EventSubscription.hpp
        ${DXFEED_SRC_DIR}/HeartbeatPayload.hpp
        ${DXFEED_SRC_DIR}/Logger.h
        ${DXFEED_SRC_DIR}/ObjectArray.h
        ${DXFEED_SRC_DIR}/PriceLevelBook.h
        ${DXFEED_SRC_DIR}/RegionalBook.h
        ${DXFEED_SRC_DIR}/PrimitiveTypes.h
        ${DXFEED_SRC_DIR}/resource.h
        ${DXFEED_SRC_DIR}/RecordTranscoder.h
        ${DXFEED_SRC_DIR}/ServerMessageProcessor.h
        ${DXFEED_SRC_DIR}/Snapshot.h
        ${DXFEED_SRC_DIR}/TaskQueue.h
        ${DXFEED_SRC_DIR}/TimeMarkUtil.hpp
        ${DXFEED_SRC_DIR}/Version.h
        ${DXFEED_SRC_DIR}/WideDecimal.h
        )

set(DXFEED_PARSER_HEADERS
        ${DXFEED_SRC_DIR}/BufferedInput.h
        ${DXFEED_SRC_DIR}/BufferedIOCommon.h
        ${DXFEED_SRC_DIR}/BufferedOutput.h
        ${DXFEED_SRC_DIR}/ConfigurationDeserializer.h
        ${DXFEED_SRC_DIR}/DataStructures.h
        ${DXFEED_SRC_DIR}/Decimal.h
        ${DXFEED_SRC_DIR}/RecordBuffers.h
        ${DXFEED_SRC_DIR}/RecordFieldSetters.h
        ${DXFEED_SRC_DIR}/SymbolCodec.h
        )

set(DXFEED_PARSER_SOURCES
        ${DXFEED_SRC_DIR}/BufferedInput.c
        ${DXFEED_SRC_DIR}/BufferedIOCommon.c
        ${DXFEED_SRC_DIR}/BufferedOutput.c
        ${DXFEED_SRC_DIR}/ConfigurationDeserializer.c
        ${DXFEED_SRC_DIR}/DataStructures.c
        ${DXFEED_SRC_DIR}/Decimal.c
        ${DXFEED_SRC_DIR}/RecordBuffers.c
        ${DXFEED_SRC_DIR}/RecordFieldSetters.c
        ${DXFEED_SRC_DIR}/SymbolCodec.c
        )

set(DXFEED_PARSER_TESTS
        ${DXFEED_SRC_DIR}/TestParser.c
        )

set(DXFEED_SOURCE_FILES
        ${DXFEED_SRC_DIR}/BinaryQTPComposer.cpp
        ${DXFEED_SRC_DIR}/BinaryQTPParser.cpp
        ${DXFEED_SRC_DIR}/Candle.c
        ${DXFEED_SRC_DIR}/ClientMessageProcessor.c
        ${DXFEED_SRC_DIR}/Configuration.cpp
        ${DXFEED_SRC_DIR}/Connection.cpp
        ${DXFEED_SRC_DIR}/ConnectionContextData.c
        ${DXFEED_SRC_DIR}/DXAddressParser.c
        ${DXFEED_SRC_DIR}/DXAlgorithms.c
        ${DXFEED_SRC_DIR}/DXErrorCodes.c
        ${DXFEED_SRC_DIR}/DXErrorHandling.c
        ${DXFEED_SRC_DIR}/DXFeed.c
        ${DXFEED_SRC_DIR}/DXMemory.c
        ${DXFEED_SRC_DIR}/DXNetwork.c
        ${DXFEED_SRC_DIR}/DXPMessageData.c
        ${DXFEED_SRC_DIR}/DXProperties.c
        ${DXFEED_SRC_DIR}/DXSockets.c
        ${DXFEED_SRC_DIR}/DXThreads.c
        ${DXFEED_SRC_DIR}/EventData.c
        ${DXFEED_SRC_DIR}/EventManager.c
        ${DXFEED_SRC_DIR}/EventSubscription.cpp
        ${DXFEED_SRC_DIR}/HeartbeatPayload.cpp
        ${DXFEED_SRC_DIR}/Logger.c
        ${DXFEED_SRC_DIR}/ObjectArray.c
        ${DXFEED_SRC_DIR}/PriceLevelBook.c
        ${DXFEED_SRC_DIR}/RegionalBook.c
        ${DXFEED_SRC_DIR}/RecordTranscoder.c
        ${DXFEED_SRC_DIR}/ServerMessageProcessor.c
        ${DXFEED_SRC_DIR}/Snapshot.c
        ${DXFEED_SRC_DIR}/TaskQueue.c
        ${DXFEED_SRC_DIR}/Version.c
        ${DXFEED_SRC_DIR}/WideDecimal.cpp
        ${DXFEED_DEF_FILE})

set(DXFEED_RESOURCE_FILES
        ${DXFEED_SRC_DIR}/DXFeed.rc
        )

set(DXFEED_ALL_HEADERS
        ${DXFEED_HEADER_FILES}
        ${DXFEED_EXPORT_HEADERS}
        ${DXFEED_PARSER_HEADERS}
        )

set(DXFEED_ALL_SOURCES
        ${DXFEED_SOURCE_FILES}
        ${DXFEED_PARSER_SOURCES}
        ${DXFEED_PARSER_TESTS}
        ${DXFEED_RESOURCE_FILES}
        )

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(MACOSX TRUE)
endif ()

include(CheckCSourceCompiles)

check_c_source_compiles("_Thread_local int g; int main() { g = 1; return g; }" DXFEED_HAVE__THREAD_LOCAL)

set(DXFEED_ADDITIONAL_LIBRARIES "")

if (WIN32)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS -D_CRT_NONSTDC_NO_DEPRECATE -D_USRDLL -DDXFEED_EXPORTS)
    set(DXFEED_SOURCE_FILES
            ${DXFEED_SOURCE_FILES}
            ${DXFEED_SRC_DIR}/Win32.c
            )
    set(DXFEED_ALL_SOURCES
            ${DXFEED_ALL_SOURCES}
            ${DXFEED_SRC_DIR}/Win32.c
            )
    if (MSVC)
#        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /Gd /Zc:wchar_t /Zc:forScope /Gm- /W3 /Ob0")
#        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Gd /Zc:wchar_t /Zc:forScope /Gm- /W3 /Ob0")
#        set(CMAKE_C_FLAGS_DEBUG "/RTC1 /MTd /Od")
#        set(CMAKE_CXX_FLAGS_DEBUG "/MTd /Od")
#        set(CMAKE_C_FLAGS_RELEASE "/O2 /MT /Zi")
#        set(CMAKE_CXX_FLAGS_RELEASE "/O2 /MT /Zi")
        set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} /DEBUG /OPT:REF /OPT:ICF")
        set(DXFEED_ADDITIONAL_LIBRARIES ${DXFEED_ADDITIONAL_LIBRARIES} ws2_32.lib)

        if (CMAKE_BUILD_TYPE STREQUAL "Debug")
            set(DXFEED_ADDITIONAL_LIBRARIES ${DXFEED_ADDITIONAL_LIBRARIES} dbghelp.lib)
        endif (CMAKE_BUILD_TYPE STREQUAL "Debug")

        # Hack for remove standard libraries from linking
        set(CMAKE_C_STANDARD_LIBRARIES "" CACHE STRING "" FORCE)
        # End hack
    elseif (("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU") OR ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang"))
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -pedantic")
        set(CMAKE_C_FLAGS_DEBUG "-g -O0")
        set(CMAKE_C_FLAGS_RELEASE "-O2")

        set(DXFEED_ADDITIONAL_LIBRARIES
                ${DXFEED_ADDITIONAL_LIBRARIES}
                ws2_32
                )
        if (CMAKE_BUILD_TYPE STREQUAL "Debug")
            set(DXFEED_ADDITIONAL_LIBRARIES
                    ${DXFEED_ADDITIONAL_LIBRARIES}
                    dbghelp
                    )
        endif (CMAKE_BUILD_TYPE STREQUAL "Debug")
    else ()
        message("Unknown compiler")
    endif ()
elseif (MACOSX)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -pedantic -Wl,-install_name,@executable_path/libDXFeed.dylib")
    set(CMAKE_C_FLAGS_DEBUG "-g -O0 -fPIC")
    set(CMAKE_C_FLAGS_RELEASE "-O2 -fPIC")
    add_definitions(-DMACOSX -DUSE_PTHREADS)
    set(DXFEED_SOURCE_FILES
            ${DXFEED_SOURCE_FILES}
            ${DXFEED_SRC_DIR}/Linux.c
            )
    set(DXFEED_ALL_SOURCES
            ${DXFEED_ALL_SOURCES}
            ${DXFEED_SRC_DIR}/Linux.c
            )
    set(DXFEED_ADDITIONAL_LIBRARIES
            ${DXFEED_ADDITIONAL_LIBRARIES}
            pthread
            )
else ()
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -pedantic")
    set(CMAKE_C_FLAGS_DEBUG "-g -O0 -fPIC")
    set(CMAKE_C_FLAGS_RELEASE "-O2 -fPIC")
    add_definitions(-DUSE_PTHREADS)

    set(DXFEED_SOURCE_FILES
            ${DXFEED_SOURCE_FILES}
            ${DXFEED_SRC_DIR}/Linux.c
            )
    set(DXFEED_ALL_SOURCES
            ${DXFEED_ALL_SOURCES}
            ${DXFEED_SRC_DIR}/Linux.c
            )
    set(DXFEED_ADDITIONAL_LIBRARIES
            ${DXFEED_ADDITIONAL_LIBRARIES}
            pthread rt
            )
endif (WIN32)

add_library(DXFeed SHARED ${DXFEED_ALL_HEADERS} ${DXFEED_ALL_SOURCES})
add_library(DXFeed::DXFeed ALIAS DXFeed)

target_include_directories(DXFeed PUBLIC ${DXFEED_INCLUDE_DIR} PRIVATE ../dxfeed-c-api/thirdparty/toml11 ../dxfeed-c-api/thirdparty/boost-1_75)
target_link_libraries(DXFeed PRIVATE ${DXFEED_ADDITIONAL_LIBRARIES})

if (WIN32)
    set_target_properties(DXFeed PROPERTIES PREFIX "")
elseif (MACOSX)
    set_target_properties(
            DXFeed
            PROPERTIES
            INSTALL_RPATH "@executable_path/."
    )
endif ()