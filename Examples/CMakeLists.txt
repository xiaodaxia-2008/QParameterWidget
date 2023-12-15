add_executable(ExampleParameterWidget ExampleParameterWidget.cpp)
target_link_libraries(ExampleParameterWidget PRIVATE QParameterWidget)
set_target_properties(ExampleParameterWidget PROPERTIES
    RESOURCE "Data/Parameters.json;Data/ParametersSchema.json")

install(TARGETS ExampleParameterWidget RESOURCE DESTINATION "${CMAKE_INSTALL_BINDIR}/Data")

if(Qt6_FOUND)
    qt_generate_deploy_app_script(TARGET ExampleParameterWidget FILENAME_VARIABLE deploy_script NO_UNSUPPORTED_PLATFORM_ERROR)
    install(SCRIPT ${deploy_script})
else()
    get_target_property(_moc_executable Qt::moc IMPORTED_LOCATION)
    cmake_path(GET _moc_executable PARENT_PATH qt_bin_dir)
    find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS ${qt_bin_dir})
    message(STATUS "windeployqt: ${WINDEPLOYQT_EXECUTABLE}")
    add_custom_command(TARGET ExampleParameterWidget POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E env PATH="${qt_bin_dir}"
        "${WINDEPLOYQT_EXECUTABLE}" --verbose 0 --no-compiler-runtime \"$<TARGET_FILE:ExampleParameterWidget>\"
        COMMENT "Deploying Qt..."
    )
endif()

install(IMPORTED_RUNTIME_ARTIFACTS spdlog::spdlog fmt::fmt)