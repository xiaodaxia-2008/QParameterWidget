#pragma once
#include <QAbstractItemModel>

#include <nlohmann/json.hpp>

#include <filesystem>
#include <memory>
#include <string>

namespace zen
{
namespace nl = nlohmann;


struct QJsonTreeItem
{
    QJsonTreeItem(QJsonTreeItem *parent = nullptr);
    ~QJsonTreeItem();
    static QJsonTreeItem *load(const nl::ordered_json &obj,
                               const nl::json &schema,
                               const std::string &key = "",
                               QJsonTreeItem *parent = nullptr);
    int row() const;

    QJsonTreeItem *parent;
    std::string schema_json_pointer;
    std::string param_json_pointer;
    QString title;
    QString key;
    QVariant value;
    nl::ordered_json::value_t type;
    QList<QJsonTreeItem *> children;
};
void to_json(nl::ordered_json &j, const QJsonTreeItem &p);

class QJsonModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit QJsonModel(QObject *parent = nullptr);
    ~QJsonModel();
    void SetHeaders(const QStringList &headers);
    bool LoadJson(const std::shared_ptr<nl::ordered_json> &param,
                  const nl::json &schema);
    const std::shared_ptr<nl::ordered_json> &GetJson() const;
    const QJsonTreeItem *RootItem() const;

    QVariant data(const QModelIndex &index, int role) const override;
    bool setData(const QModelIndex &index, const QVariant &value,
                 int role = Qt::EditRole) override;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role) const override;
    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

signals:
    void SigParameterChanged(const std::string &param_json_pointer,
                             std::shared_ptr<nl::ordered_json> param);

private:
    QJsonTreeItem *m_root_item;
    std::shared_ptr<nl::ordered_json> m_param;
    QStringList m_headers;
};
} // namespace zen
