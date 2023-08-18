#include <QParameterWidget.h>

#include <QApplication>
#include <filesystem>

int main(int argc, char **argv) {
  using zen::QParameterWidget;
  std::filesystem::path src_dir = std::filesystem::path(__FILE__).parent_path();

  QApplication app(argc, argv);
  QParameterWidget pw(src_dir / "Parameters/GlobalParameters.json",
                      src_dir / "Parameters/GlobalParametersSchema.json");
  pw.show();
  app.exec();
  return 0;
}