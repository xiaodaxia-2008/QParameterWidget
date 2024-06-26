#include <QParameterWidget/ParameterItemDelegate.h>
#include <QParameterWidget/QJsonModel.h>
#include <QParameterWidget/QParameterWidget.h>

#include <spdlog/spdlog.h>

#include <QFileDialog>
#include <QKeyEvent>

#include <fstream>

namespace zen
{
QParameterWidget::QParameterWidget(
    const std::shared_ptr<nl::ordered_json> &param,
    const std::filesystem::path &schema_path, QWidget *parent,
    const QLocale &locale)
    : QTreeView(parent)
{
    std::ifstream f(schema_path);
    nl::json schema;
    f >> schema;
    Init(param, schema, locale);
}

QParameterWidget::QParameterWidget(const std::filesystem::path &param_path,
                                   const std::filesystem::path &schema_path,
                                   QWidget *parent, const QLocale &locale)
{
    std::ifstream f1(param_path);
    auto param = std::make_shared<nl::ordered_json>();
    f1 >> *param;

    std::ifstream f2(schema_path);
    nl::json schema;
    f2 >> schema;
    Init(param, schema, locale);
}

void QParameterWidget::Init(const std::shared_ptr<nl::ordered_json> &param,
                            const nl::json &schema, const QLocale &locale)
{
    m_schema = schema;
    if (!VerifyJsonSchema(m_schema, *param)) {
        SPDLOG_ERROR("parameter and schema don't match");
        return;
    }

    auto model = new QJsonModel(this, locale);
    model->LoadJson(param, m_schema);
    this->setModel(model);

    m_delegate = new ParameterItemDelegate(m_schema, this, locale);
    this->setItemDelegate(m_delegate);

    connect(model, &QJsonModel::SigParameterChanged, this,
            &QParameterWidget::SigParameterChanged);
}

QParameterWidget::~QParameterWidget() { delete m_delegate; }

void QParameterWidget::SetHeaders(const QStringList &headers)
{
    return qobject_cast<QJsonModel *>(model())->SetHeaders(headers);
}

const std::shared_ptr<nl::ordered_json> &QParameterWidget::GetJson() const
{
    return qobject_cast<QJsonModel *>(model())->GetJson();
}

bool QParameterWidget::SaveJson(const std::filesystem::path &fileName) const
{
    std::ofstream f(fileName);
    if (f) {
        f << *GetJson();
        return true;
    }
    return false;
}

bool QParameterWidget::LoadJson(const std::shared_ptr<nl::ordered_json> &param)
{
    if (!VerifyJsonSchema(m_schema, *param)) {
        SPDLOG_ERROR("json and schema don't match!");
        return false;
    }
    return qobject_cast<QJsonModel *>(model())->LoadJson(param, m_schema);
}

void QParameterWidget::keyPressEvent(QKeyEvent *event)
{
    if (event->matches(QKeySequence::Save)) {
        auto fname = QFileDialog::getSaveFileName(this, tr("Save File Name"),
                                                  "", "Json(*.json)");
        if (!fname.isEmpty()) {
            SaveJson(std::filesystem::path(fname.toStdString()));
        }
        event->accept();
    } else if (event->matches(QKeySequence::Open)) {
        auto fname = QFileDialog::getOpenFileName(this, tr("Open File Name"),
                                                  "", "Json(*.json)");
        if (!fname.isEmpty()) {
            std::ifstream f(fname.toStdString());
            try {
                auto jv = std::make_shared<nl::ordered_json>();
                f >> *jv;
                LoadJson(jv);
            } catch (std::exception &e) {
                SPDLOG_WARN("{}", e.what());
            }
        }
        event->accept();
    } else {
        QTreeView::keyPressEvent(event);
    }
}

bool VerifyJsonSchema(const nl::json &schema, const nl::ordered_json &jv)
{
    return true;
}
} // namespace zen