cmake_minimum_required (VERSION 3.8)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(TARGET_NAME MangaBackend)

if(NOT EXISTS "${CMAKE_BINARY_DIR}/conan.cmake")
  message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
  file(DOWNLOAD "https://raw.githubusercontent.com/conan-io/cmake-conan/v0.16.1/conan.cmake"
                "${CMAKE_BINARY_DIR}/conan.cmake"
                EXPECTED_HASH SHA256=396e16d0f5eabdc6a14afddbcfff62a54a7ee75c6da23f32f7a31bc85db23484
                TLS_VERIFY ON)
endif()

include(${CMAKE_BINARY_DIR}/conan.cmake)

list(APPEND CONAN_DEPENDENCIES
        spdlog/1.8.5
        sqlite_orm/1.6
        nlohmann_json/3.9.1
        OPTIONS
        spdlog:header_only=True
        fmt:header_only=True
        )

if (NOT $ENV{DOCKER_BUILD})
message("Non docker build. Downloading build fron conan repositories...")
list(APPEND CONAN_DEPENDENCIES boost/1.75.0)

endif()

conan_cmake_run(REQUIRES ${CONAN_DEPENDENCIES} 
                BASIC_SETUP CMAKE_TARGETS BUILD missing)

find_package(Boost REQUIRED)

set(HEADERS Server.h
            QueryRouter.h
            Query.h
            QueryRouterBase.h
            Session.h
            Structs.h
            Database.h
            Utils.h
            MangaDatabase.h
            utility/Base64.h
            )

set(SOURCES Server.cpp
            QueryRouter.cpp
            Query.cpp
            Structs.cpp
            Session.cpp
            MangaDatabase.cpp
            )

add_executable (${TARGET_NAME} main.cpp ${HEADERS} ${SOURCES})

target_include_directories(${TARGET_NAME} PUBLIC ${Boost_INCLUDE_DIR})
target_compile_definitions(${TARGET_NAME} PRIVATE BOOST_DATE_TIME_NO_LIB)
target_link_libraries(${TARGET_NAME} PRIVATE CONAN_PKG::spdlog
                                             CONAN_PKG::sqlite_orm
                                             CONAN_PKG::nlohmann_json)
