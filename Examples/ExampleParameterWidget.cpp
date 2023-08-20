#include <QParameterWidget/QParameterWidget.h>

#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

#include <QApplication>

#include <filesystem>
#include <fstream>

int main(int argc, char **argv)
{
    using zen::QParameterWidget;
    std::filesystem::path src_dir =
        std::filesystem::path(argv[0]).parent_path();
    std::ifstream f(src_dir / "Data/Parameters.json");
    auto param = std::make_shared<nlohmann::ordered_json>();
    f >> *param;

    QApplication app(argc, argv);
    QParameterWidget pw(param, src_dir / "Data/ParametersSchema.json");
    pw.expandAll();
    pw.resizeColumnToContents(0);

    QObject::connect(
        &pw, &QParameterWidget::SigParameterChanged,
        [](const std::string &json_pointer,
           std::shared_ptr<nlohmann::ordered_json> jv) {
            fmt::println(
                "{}: {}", json_pointer,
                jv->at(nlohmann::ordered_json::json_pointer(json_pointer))
                    .dump());
        });
    pw.show();
    app.exec();

    // param is changed with QParameterWidget
    fmt::println("{}", param->dump(4));
    return 0;
}