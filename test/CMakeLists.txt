enable_testing()

if(NOT TARGET SDL2::SDL2main)
    find_package(SDL2main)
endif()

if(NOT TARGET SDL2::SDL2test)
    find_package(SDL2test REQUIRED)
endif()

add_executable(testimage main.c)

set(ALL_TESTS
    testimage
)
set(RESOURCE_FILES
    palette.bmp
    palette.gif
    sample.avif
    sample.bmp
    sample.cur
    sample.ico
    sample.jpg
    sample.jxl
    sample.pcx
    sample.png
    sample.pnm
    sample.qoi
    sample.tga
    sample.tif
    sample.webp
    sample.xcf
    sample.xpm
    svg-class.bmp
    svg-class.svg
    svg.bmp
    svg.svg
    svg64.bmp
)

set(TESTS_ENVIRONMENT
    "SDL_TEST_SRCDIR=${CMAKE_CURRENT_SOURCE_DIR}"
    "SDL_TEST_BUILDDIR=${CMAKE_CURRENT_BINARY_DIR}"
    "SDL_VIDEODRIVER=dummy"
)

foreach(prog ${ALL_TESTS})
    target_compile_definitions(${prog} PRIVATE $<TARGET_PROPERTY:SDL2_image,COMPILE_DEFINITIONS>)
    target_link_libraries(${prog} PRIVATE SDL2_image::${sdl2_image_export_name})
    if(TARGET SDL2::SDL2main)
        target_link_libraries(${prog} PRIVATE SDL2::SDL2main)
    endif()
    target_link_libraries(${prog} PRIVATE SDL2::SDL2test ${sdl2_target_name})

    add_test(
        NAME ${prog}
        COMMAND ${prog}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )
    set_tests_properties(
        ${prog}
        PROPERTIES
        ENVIRONMENT "${TESTS_ENVIRONMENT}"
        TIMEOUT 30
    )
    if(SDL2IMAGE_TESTS_INSTALL)
        set(exe ${prog})
        set(installedtestsdir "${CMAKE_INSTALL_FULL_LIBEXECDIR}/installed-tests/${PROJECT_NAME}")
        configure_file(template.test.in "${exe}.test" @ONLY)
        install(
            FILES "${CMAKE_CURRENT_BINARY_DIR}/${exe}.test"
            DESTINATION "${CMAKE_INSTALL_DATADIR}/installed-tests/${PROJECT_NAME}"
        )
    endif()
endforeach()

if(SDL2IMAGE_TESTS_INSTALL)
    install(
        TARGETS ${ALL_TESTS}
        DESTINATION "${CMAKE_INSTALL_LIBEXECDIR}/installed-tests/${PROJECT_NAME}"
    )
    install(
        FILES ${RESOURCE_FILES}
        DESTINATION "${CMAKE_INSTALL_LIBEXECDIR}/installed-tests/${PROJECT_NAME}"
    )
endif()

# Check for Dreamcast and adjust accordingly
if(__DREAMCAST__)
    # # Disable LTO for Dreamcast
    # string(REPLACE "-flto=auto" "" CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
    # string(REPLACE "-flto=auto" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    # add_compile_options(-fno-lto)

    # Set up resources for Dreamcast
    set(DREAMCAST_APPS
        testimage
        # Add other apps here if needed
    )

    foreach(APP IN LISTS DREAMCAST_APPS)
        if(${APP} STREQUAL "testimage")
            set(APP_RESOURCE_FILES "palette.bmp" "sample.png" "sample.jpg" "sample.bmp" "sample.tga" "sample.cur" "palette.gif" "sample.ico")
        endif()

        if(APP_RESOURCE_FILES)
            # Set up romdisk directory for the app
            set(KOS_ROMDISK_DIR "${CMAKE_CURRENT_BINARY_DIR}/romdisk_${APP}")
            
            # Ensure resources directory exists
            file(MAKE_DIRECTORY "${KOS_ROMDISK_DIR}")

            # Copy resource files to the romdisk directory
            foreach(RESOURCE_FILE ${APP_RESOURCE_FILES})
                file(COPY ${RESOURCE_FILE} DESTINATION "${KOS_ROMDISK_DIR}")
            endforeach()

            # Use kos_add_romdisk function from KOS Dreamcast CMake utilities
            kos_add_romdisk(${APP} "${KOS_ROMDISK_DIR}" ${APP})
        else()
            message(WARNING "App ${APP} does not have resource files defined!")
        endif()
    endforeach()
endif()
