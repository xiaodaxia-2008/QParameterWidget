macro(qt_deploy target_name)
    get_target_property(_moc_executable Qt::moc IMPORTED_LOCATION)
    cmake_path(GET _moc_executable PARENT_PATH qt_bin_dir)
    find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS ${qt_bin_dir})
    message(STATUS "windeployqt: ${WINDEPLOYQT_EXECUTABLE}")
    add_custom_command(TARGET ${target_name} POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E env PATH="${qt_bin_dir}"
        "${WINDEPLOYQT_EXECUTABLE}" --verbose 0 --no-compiler-runtime "$<TARGET_FILE:${target_name}>"
        COMMENT "Deploying Qt..."
    )

    if(Qt6_FOUND)
        qt_generate_deploy_app_script(TARGET ${target_name} FILENAME_VARIABLE deploy_script NO_UNSUPPORTED_PLATFORM_ERROR)
        install(SCRIPT ${deploy_script})
    else()
        install(CODE "
        message(STATUS CMAKE_INSTALL_PREFIX: $<INSTALL_PREFIX>)
        message(STATUS CMAKE_INSTALL_BINDIR: ${CMAKE_INSTALL_BINDIR})
        set(target_path $<INSTALL_PREFIX>/${CMAKE_INSTALL_BINDIR}/$<TARGET_FILE_NAME:${target_name}>)
        message(STATUS target_path: \${target_path})
        message(STATUS WINDEPLOYQT_EXECUTABLE: ${WINDEPLOYQT_EXECUTABLE})
        execute_process(COMMAND ${WINDEPLOYQT_EXECUTABLE} --verbose 1 --no-compiler-runtime \${target_path}
            )
        "
        )
    endif()
endmacro()