#pragma once
#include <QAbstractItemModel>

#include <nlohmann/json.hpp>

#include <filesystem>
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

    std::string json_pointer;
    QString title;
    QString key;
    QVariant value;
    nl::ordered_json::value_t type;
    QList<QJsonTreeItem *> children;
    QJsonTreeItem *parent;
};

//---------------------------------------------------

class QJsonModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit QJsonModel(QObject *parent = nullptr);
    ~QJsonModel();
    bool loadJson(const nl::ordered_json &jv, const nl::json &schema);
    bool saveJson(const std::filesystem::path &fileName) const;
    nl::ordered_json getJson() const;
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

private:
    QJsonTreeItem *mRootItem;
    QStringList mHeaders;
    nl::ordered_json m_json;
};
} // namespace zen
