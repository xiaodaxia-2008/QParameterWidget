file(GLOB_RECURSE srcs CONFIGURE_DEPENDS *.cpp *.h)
add_library(QParameterWidget ${srcs})
target_sources(QParameterWidget PUBLIC FILE_SET headers TYPE HEADERS BASE_DIRS "Include" FILES "Include/QParameterWidget")
target_link_libraries(QParameterWidget PUBLIC Qt::Widgets nlohmann_json::nlohmann_json spdlog::spdlog)
target_include_directories(QParameterWidget PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/Include>)
set_target_properties(QParameterWidget PROPERTIES AUTOMOC ON)

install(TARGETS QParameterWidget EXPORT QParameterWidgetConfig FILE_SET headers)
install(EXPORT QParameterWidgetConfig DESTINATION cmake NAMESPACE QParameterWidget::)