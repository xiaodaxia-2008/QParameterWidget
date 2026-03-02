add_library(QParameterWidget )
generate_export_header(QParameterWidget
    EXPORT_FILE_NAME ${CMAKE_CURRENT_LIST_DIR}/Include/QParameterWidget/qparameterwidget_export.h
)

file(GLOB_RECURSE srcs CONFIGURE_DEPENDS *.cpp)
file(GLOB_RECURSE incs CONFIGURE_DEPENDS *.h)

target_sources(QParameterWidget PRIVATE ${srcs} ${incs})
target_sources(QParameterWidget PUBLIC FILE_SET headers TYPE HEADERS BASE_DIRS Include FILES ${incs})

target_link_libraries(QParameterWidget PUBLIC Qt::Widgets nlohmann_json::nlohmann_json spdlog::spdlog)

target_include_directories(QParameterWidget PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/Include>
)

set_target_properties(QParameterWidget PROPERTIES AUTOMOC ON)
set_target_properties(QParameterWidget PROPERTIES AUTOUIC ON)
set_target_properties(QParameterWidget PROPERTIES AUTORCC ON)

install(TARGETS QParameterWidget EXPORT QParameterWidgetConfig FILE_SET headers)
install(EXPORT QParameterWidgetConfig DESTINATION cmake NAMESPACE QParameterWidget::)
