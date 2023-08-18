#include "QParameterWidget.h"
#include "ParameterItemDelegate.h"
#include "QJsonModel.h"

#include <spdlog/spdlog.h>

#include <QFileDialog>
#include <QKeyEvent>

#include <fstream>

namespace zen
{
QParameterWidget::QParameterWidget(const std::filesystem::path &param_path,
                                   const std::filesystem::path &schema_path,
                                   QWidget *parent)
    : QTreeView(parent)
{
    std::ifstream f1(param_path);
    std::ifstream f2(schema_path);

    f1 >> m_param;
    f2 >> m_schema;

    if (!VerifyJsonSchema(m_schema, m_param)) {
        SPDLOG_ERROR("parameter and schema don't match");
        return;
    }

    auto model = new QJsonModel(this);
    model->LoadJson(m_param, m_schema);
    this->setModel(model);

    m_delegate = new ParameterItemDelegate(m_schema, this);
    this->setItemDelegate(m_delegate);
}

QParameterWidget::~QParameterWidget() { delete m_delegate; }

void QParameterWidget::keyPressEvent(QKeyEvent *event)
{
    if (event->keyCombination()
        == QKeyCombination(Qt::ControlModifier, Qt::Key_S)) {
        auto fname = QFileDialog::getSaveFileName(this, tr("Save File Name"),
                                                  "", "Json(*.json)");
        if (!fname.isEmpty()) {
            qobject_cast<QJsonModel *>(model())->SaveJson(
                std::filesystem::path(fname.toStdString()));
        }
        event->accept();
    }
    else if (event->keyCombination()
             == QKeyCombination(Qt::ControlModifier, Qt::Key_O)) {
        auto fname = QFileDialog::getOpenFileName(this, tr("Open File Name"),
                                                  "", "Json(*.json)");
        if (!fname.isEmpty()) {
            std::ifstream f(fname.toStdString());
            try {
                nl::ordered_json jv;
                f >> jv;
                qobject_cast<QJsonModel *>(model())->LoadJson(jv, m_schema);
            }
            catch (std::exception &e) {
                SPDLOG_WARN("{}", e.what());
            }
        }
        event->accept();
    }
    else {
        QTreeView::keyPressEvent(event);
    }
}

bool VerifyJsonSchema(const nl::json &schema, const nl::ordered_json &jv)
{
    return true;
}
} // namespace zen