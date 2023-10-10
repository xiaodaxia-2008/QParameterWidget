file(GLOB_RECURSE srcs CONFIGURE_DEPENDS *.cpp *.h)
add_library(QParameterWidget ${srcs})
generate_export_header(QParameterWidget)
target_sources(QParameterWidget PUBLIC FILE_SET headers TYPE HEADERS BASE_DIRS "Include" FILES "Include/QParameterWidget")
target_link_libraries(QParameterWidget PUBLIC Qt::Widgets nlohmann_json::nlohmann_json spdlog::spdlog)
target_include_directories(QParameterWidget PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/Include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
)

set_target_properties(QParameterWidget PROPERTIES AUTOMOC ON)
set_target_properties(QParameterWidget PROPERTIES AUTOUIC ON)
set_target_properties(QParameterWidget PROPERTIES AUTORCC ON)

# qt_add_lupdate(QParameterWidget TS_FILES QParameterWidget_en.ts QParameterWidget_zh.ts
# INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR})
# qt_add_lrelease(QParameterWidget TS_FILES QParameterWidget_en.ts QParameterWidget_zh.ts
# QM_FILES_OUTPUT_VARIABLE qm_files)
install(TARGETS QParameterWidget EXPORT QParameterWidgetConfig FILE_SET headers)
install(EXPORT QParameterWidgetConfig DESTINATION cmake NAMESPACE QParameterWidget::)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/qparameterwidget_export.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/QParameterWidget)

# install(FILES ${qm_files} DESTINATION translations)