# CMake script for building SDL_image tests

enable_testing()

set(CMAKE_POSITION_INDEPENDENT_CODE OFF)

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

function(add_sdl_image_test_executable TARGET)
    add_executable(${TARGET} ${ARGN})
    target_compile_definitions(${TARGET}
        PRIVATE
            $<TARGET_PROPERTY:${sdl3_image_target_name},COMPILE_DEFINITIONS>
            "SDL_IMAGE_SAVE_AVIF=$<BOOL:${SDLIMAGE_AVIF_SAVE}>"
            "SDL_IMAGE_SAVE_JPG=$<BOOL:${SDLIMAGE_JPG_SAVE}>"
            "SDL_IMAGE_SAVE_PNG=$<BOOL:${SDLIMAGE_PNG_SAVE}>"
    )
    sdl_add_warning_options(${TARGET} WARNING_AS_ERROR ${SDLIMAGE_WERROR})
    target_link_libraries(${TARGET} PRIVATE SDL3_image::SDL3_image SDL3::SDL3_test SDL3::SDL3)

    # Add Dreamcast-specific libraries
    if(__DREAMCAST__)
        target_link_libraries(${TARGET} PRIVATE GL pthread)
    endif()

    add_test(
        NAME ${TARGET}
        COMMAND ${TARGET}
        WORKING_DIRECTORY "$<TARGET_FILE_DIR:${TARGET}>"
    )

    set(TESTS_ENVIRONMENT
        "SDL_TEST_SRCDIR=${CMAKE_CURRENT_SOURCE_DIR}"
        "SDL_TEST_BUILDDIR=$<TARGET_FILE_DIR:${TARGET}>"
        "SDL_VIDEO_DRIVER=dummy"
    )
    set_tests_properties(${TARGET}
        PROPERTIES
            ENVIRONMENT "${TESTS_ENVIRONMENT}"
            TIMEOUT 30
    )
    if(SDLIMAGE_TESTS_INSTALL)
        set(installedtestsdir "${CMAKE_INSTALL_FULL_LIBEXECDIR}/installed-tests/${PROJECT_NAME}")
        configure_file(template.test.in "${TARGET}.test" @ONLY)
        install(
            FILES "${CMAKE_CURRENT_BINARY_DIR}/${TARGET}.test"
            DESTINATION "${CMAKE_INSTALL_DATADIR}/installed-tests/${PROJECT_NAME}"
        )
    endif()
    if(SDLIMAGE_TESTS_INSTALL)
        install(
            TARGETS ${TARGET}
            DESTINATION "${CMAKE_INSTALL_LIBEXECDIR}/installed-tests/${PROJECT_NAME}"
        )
        if(MSVC)
            SDL_install_pdb("${TARGET}" "${CMAKE_INSTALL_LIBEXECDIR}/installed-tests/${PROJECT_NAME}")
        endif()
    endif()
endfunction()

add_sdl_image_test_executable(testimage main.c)

if(SDLIMAGE_TESTS_INSTALL)
    install(
        FILES ${RESOURCE_FILES}
        DESTINATION "${CMAKE_INSTALL_LIBEXECDIR}/installed-tests/${PROJECT_NAME}"
    )
endif()

# ----------------------
# Dreamcast-specific setup
# ----------------------
if(__DREAMCAST__)
    # Collect resource files for the romdisk
    set(APP_RESOURCE_FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/palette.bmp"
        "${CMAKE_CURRENT_SOURCE_DIR}/palette.gif"
        "${CMAKE_CURRENT_SOURCE_DIR}/sample.avif"
        "${CMAKE_CURRENT_SOURCE_DIR}/sample.bmp"
        "${CMAKE_CURRENT_SOURCE_DIR}/sample.cur"
        "${CMAKE_CURRENT_SOURCE_DIR}/sample.ico"
        "${CMAKE_CURRENT_SOURCE_DIR}/sample.jpg"
        "${CMAKE_CURRENT_SOURCE_DIR}/sample.jxl"
        "${CMAKE_CURRENT_SOURCE_DIR}/sample.pcx"
        "${CMAKE_CURRENT_SOURCE_DIR}/sample.png"
        "${CMAKE_CURRENT_SOURCE_DIR}/sample.pnm"
        "${CMAKE_CURRENT_SOURCE_DIR}/sample.qoi"
        "${CMAKE_CURRENT_SOURCE_DIR}/sample.tga"
        "${CMAKE_CURRENT_SOURCE_DIR}/sample.tif"
        "${CMAKE_CURRENT_SOURCE_DIR}/sample.webp"
        "${CMAKE_CURRENT_SOURCE_DIR}/sample.xcf"
        "${CMAKE_CURRENT_SOURCE_DIR}/sample.xpm"
        "${CMAKE_CURRENT_SOURCE_DIR}/svg-class.bmp"
        "${CMAKE_CURRENT_SOURCE_DIR}/svg-class.svg"
        "${CMAKE_CURRENT_SOURCE_DIR}/svg.bmp"
        "${CMAKE_CURRENT_SOURCE_DIR}/svg.svg"
        "${CMAKE_CURRENT_SOURCE_DIR}/svg64.bmp"
    )

    # Create the ROMDISK directory
    set(KOS_ROMDISK_DIR "${CMAKE_CURRENT_BINARY_DIR}/romdisk")
    file(MAKE_DIRECTORY "${KOS_ROMDISK_DIR}")

    # Copy the resource files to the romdisk directory
    foreach(RESOURCE_FILE ${APP_RESOURCE_FILES})
        if(EXISTS ${RESOURCE_FILE})
            file(COPY ${RESOURCE_FILE} DESTINATION "${KOS_ROMDISK_DIR}")
        else()
            message(WARNING "Resource file ${RESOURCE_FILE} not found.")
        endif()
    endforeach()

    # Add a romdisk for the testimage executable
    kos_add_romdisk(testimage "${KOS_ROMDISK_DIR}" testimage)
endif()

