#pragma once
#include <QTreeView>

#include <nlohmann/json.hpp>

#include <filesystem>

namespace zen
{
namespace nl = nlohmann;

bool VerifyJsonSchema(const nl::json &schema, const nl::ordered_json &jv);

class QParameterWidget : public QTreeView
{
    Q_OBJECT
public:
    QParameterWidget(const std::filesystem::path &param_path,
                     const std::filesystem::path &schema_path,
                     QWidget *parent = nullptr);
    ~QParameterWidget();

    const nl::ordered_json &GetJson() const;

signals:
    void SigParameterChanged(const std::string &json_pointer,
                             const QVariant &val);

private:
    virtual void keyPressEvent(QKeyEvent *event) override;

    QAbstractItemDelegate *m_delegate{nullptr};
    nl::json m_schema;
    nl::ordered_json m_param;
};
} // namespace zen