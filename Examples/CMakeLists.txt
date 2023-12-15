add_executable(ExampleParameterWidget ExampleParameterWidget.cpp)
target_link_libraries(ExampleParameterWidget PRIVATE QParameterWidget)
set_target_properties(ExampleParameterWidget PROPERTIES
    RESOURCE "Data/Parameters.json;Data/ParametersSchema.json")

add_custom_command(TARGET ExampleParameterWidget POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/Data $<TARGET_FILE_DIR:ExampleParameterWidget>/Data)

# add_custom_command(TARGET ExampleParameterWidget POST_BUILD
# COMMAND ${CMAKE_COMMAND} -E copy -t $<TARGET_FILE_DIR:ExampleParameterWidget> $<TARGET_RUNTIME_DLLS:ExampleParameterWidget>
# COMMAND_EXPAND_LISTS
# )
install(TARGETS ExampleParameterWidget RESOURCE DESTINATION "${CMAKE_INSTALL_BINDIR}/Data")
install(IMPORTED_RUNTIME_ARTIFACTS spdlog::spdlog fmt::fmt)

include(QtDeploy)
qt_deploy(ExampleParameterWidget)