#pragma once
#include <QTreeView>

#include <filesystem>

namespace zen {
class QParameterWidget : public QTreeView {
  Q_OBJECT
public:
  QParameterWidget(const std::filesystem::path &param_path,
                   const std::filesystem::path &schema_path,
                   QWidget *parent = nullptr);
  ~QParameterWidget();

private:
  QAbstractItemDelegate *m_delegate{nullptr};
};
} // namespace zen