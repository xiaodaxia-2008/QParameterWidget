add_executable(ExampleParameterWidget ExampleParameterWidget.cpp)
target_link_libraries(ExampleParameterWidget PRIVATE QParameterWidget)
set_target_properties(ExampleParameterWidget PROPERTIES
    RESOURCE "Data/GlobalParameters.json;Data/GlobalParametersSchema.json")

install(TARGETS ExampleParameterWidget RESOURCE DESTINATION "${CMAKE_INSTALL_BINDIR}/Data")

qt_generate_deploy_app_script(TARGET ExampleParameterWidget FILENAME_VARIABLE deploy_script NO_UNSUPPORTED_PLATFORM_ERROR)
install(SCRIPT ${deploy_script})

install(IMPORTED_RUNTIME_ARTIFACTS spdlog::spdlog fmt::fmt)