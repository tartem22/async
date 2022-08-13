cmake_minimum_required(VERSION 3.2)

set(PATCH_VERSION "1" CACHE INTERNAL "Patch version")
set(PROJECT_VERSION 0.0.${PATCH_VERSION})

project(Command VERSION ${PROJECT_VERSION})

option(WITH_GTEST "Whether to build Gtest test" ON)

add_library(async SHARED                 
            command_handler.h
            command_handler.cpp
            command.h
            command_block.h
            command_block.cpp
            static_command_block.h
            dynamic_command_block.h)

target_link_libraries(async PRIVATE -pthread)

set_target_properties(async
                        PROPERTIES
                        CXX_STANDARD 17
                        CXX_STANDARD_REQUIRED ON)

add_executable(${PROJECT_NAME}
                main.cpp
                async.cpp
                async.h
                )

set_target_properties(${PROJECT_NAME}
                        PROPERTIES
                        CXX_STANDARD 17
                        CXX_STANDARD_REQUIRED ON)

target_link_libraries(${PROJECT_NAME} async)

if(WITH_GTEST)
    include(FetchContent)
    FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/609281088cfefc76f9d0ce82e1ff6c30cc3591e5.zip
    )
    # For Windows: Prevent overriding the parent project's compiler/linker settings
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
    add_executable(tests 
                    test.cpp
                    async.cpp
                    async.h)
    set_target_properties(tests PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
    )
    target_link_libraries(tests
        gtest_main
        async
    )
endif()


if (MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE
        /W4
)
else ()
    target_compile_options(${PROJECT_NAME} PRIVATE
        -Wall
    )
endif()

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)

set(CPACK_GENERATOR DEB)
set(CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${PROJECT_VERSION_PATCH}")
set(CPACK_PACKAGE_CONTACT example@example.com)
include(CPack)