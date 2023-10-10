#pragma once
#include <qparameterwidget_export.h>
#include <QTreeView>

#include <nlohmann/json.hpp>

#include <filesystem>
#include <memory>

namespace zen
{
namespace nl = nlohmann;

bool VerifyJsonSchema(const nl::json &schema, const nl::ordered_json &jv);

class QPARAMETERWIDGET_EXPORT QParameterWidget : public QTreeView
{
    Q_OBJECT
public:
    QParameterWidget(const std::filesystem::path &param_path,
                     const std::filesystem::path &schema_path,
                     QWidget *parent = nullptr);
    QParameterWidget(const std::shared_ptr<nl::ordered_json> &param,
                     const std::filesystem::path &schema_path,
                     QWidget *parent = nullptr);
    ~QParameterWidget();

    void SetHeaders(const QStringList &headers);

    const std::shared_ptr<nl::ordered_json> &GetJson() const;
    bool SaveJson(const std::filesystem::path &file_name) const;
    bool LoadJson(const std::shared_ptr<nl::ordered_json> &param);

signals:
    void SigParameterChanged(const std::string &param_json_pointer,
                             std::shared_ptr<nl::ordered_json> param);

private:
    void Init(const std::shared_ptr<nl::ordered_json> &param,
              const nl::json &schema);
    virtual void keyPressEvent(QKeyEvent *event) override;

    QAbstractItemDelegate *m_delegate{nullptr};
    nl::json m_schema;
};
} // namespace zen