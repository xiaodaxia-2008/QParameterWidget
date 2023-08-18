#pragma once
#include <QAbstractItemModel>

#include <nlohmann/json.hpp>

#include <filesystem>
#include <string>

namespace zen {
namespace nl = nlohmann;

class QJsonTreeItem {
  friend void to_json(nl::ordered_json &obj, const QJsonTreeItem &p);

public:
  QJsonTreeItem(QJsonTreeItem *parent = nullptr);
  ~QJsonTreeItem();
  const std::string &jsonPointer() const;
  void appendChild(QJsonTreeItem *item);
  QJsonTreeItem *child(int row);
  QJsonTreeItem *parent();
  int childCount() const;
  int row() const;
  void setKey(const QString &key);
  void setValue(const QVariant &value);
  void setType(const nl::ordered_json::value_t &type);
  QString key() const;
  QString title() const;
  QVariant value() const;
  nl::ordered_json::value_t type() const;

  static QJsonTreeItem *load(const nl::ordered_json &obj,
                             const nl::json &schema, const std::string &key = "",
                             QJsonTreeItem *parent = nullptr);

private:
  std::string mJsonPointer;
  QString mTitle;
  QString mKey;
  QVariant mValue;
  nl::ordered_json::value_t mType;
  QList<QJsonTreeItem *> mChilds;
  QJsonTreeItem *mParent;
};

//---------------------------------------------------

class QJsonModel : public QAbstractItemModel {
  Q_OBJECT
public:
  explicit QJsonModel(QObject *parent = nullptr);
  ~QJsonModel();

  bool load(const std::filesystem::path &json_file,
            const std::filesystem::path &schema_file);
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
};
} // namespace zen
