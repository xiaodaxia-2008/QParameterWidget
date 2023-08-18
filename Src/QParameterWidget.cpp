#include "QParameterWidget.h"
#include "ParameterItemDelegate.h"
#include "QJsonModel.h"

#include <QTreeView>

#include <fstream>

namespace zen {
QParameterWidget::QParameterWidget(const std::filesystem::path &param_path,
                                   const std::filesystem::path &schema_path,
                                   QWidget *parent)
    : QTreeView(parent) {

  std::ifstream f1(param_path);
  std::ifstream f2(schema_path);

  nl::ordered_json param;
  nl::json schema;
  f1 >> param;
  f2 >> schema;

  auto model = new QJsonModel(this);
  model->loadJson(param, schema);
  this->setModel(model);

  m_delegate = new ParameterItemDelegate(schema, this);
  this->setItemDelegate(m_delegate);
}

QParameterWidget::~QParameterWidget() { delete m_delegate; }
} // namespace zen