#include "QJsonModel.h"

#include <fmt/std.h>
#include <spdlog/spdlog.h>

#include <filesystem>
#include <fstream>

namespace zen {
std::string GetJsonPath(QJsonTreeItem *item) {
  std::string key = item->key().toStdString();
  if (auto parent = item->parent(); parent && parent->key() != "root") {
    return fmt::format("{}/properties/{}", GetJsonPath(parent), key);
  }
  return "/properties/" + key;
}

void to_json(nl::ordered_json &j, const QJsonTreeItem &p);

QJsonTreeItem::QJsonTreeItem(QJsonTreeItem *parent)
    : mJsonPointer(""), mParent(parent) {}

QJsonTreeItem::~QJsonTreeItem() { qDeleteAll(mChilds); }

const std::string &QJsonTreeItem::jsonPointer() const { return mJsonPointer; }

void QJsonTreeItem::appendChild(QJsonTreeItem *item) { mChilds.append(item); }
QJsonTreeItem *QJsonTreeItem::child(int row) { return mChilds.value(row); }
QJsonTreeItem *QJsonTreeItem::parent() { return mParent; }
int QJsonTreeItem::childCount() const { return mChilds.count(); }
int QJsonTreeItem::row() const {
  if (mParent) {
    return mParent->mChilds.indexOf(const_cast<QJsonTreeItem *>(this));
  }
  return 0;
}
void QJsonTreeItem::setKey(const QString &key) { mKey = key; }
void QJsonTreeItem::setValue(const QVariant &value) { mValue = value; }
void QJsonTreeItem::setType(const nl::ordered_json::value_t &type) {
  mType = type;
}
QString QJsonTreeItem::key() const { return mKey; }
QString QJsonTreeItem::title() const { return mTitle; }
QVariant QJsonTreeItem::value() const { return mValue; }
nl::ordered_json::value_t QJsonTreeItem::type() const { return mType; }

QJsonTreeItem *QJsonTreeItem::load(const nl::ordered_json &jv,
                                   const nl::json &schema,
                                   const std::string &key,
                                   QJsonTreeItem *parent) {
  QJsonTreeItem *item = new QJsonTreeItem(parent);
  item->setKey(QString::fromStdString(key));
  item->setType(jv.type());
  if (parent) {
    item->mJsonPointer = parent->mJsonPointer + "/properties/" + key;
  } else if (!key.empty()) {
    item->mJsonPointer = "/properties/" + key;
  }

  switch (jv.type()) {
  case nl::ordered_json::value_t::array:
  case nl::ordered_json::value_t::object: {
    for (auto &[k, v] : jv.items()) {
      QJsonTreeItem *child = load(v, schema, k, item);
      try {
        nl::json::json_pointer jp(child->mJsonPointer + "/title");
        auto title = schema.at(jp).get<std::string>();
        child->mTitle = QString::fromStdString(title);
      } catch (std::exception &e) {
        SPDLOG_WARN("failed to get item title from {}/title, exception: {}",
                    child->mJsonPointer, e.what());
        child->mTitle = child->mKey;
      }
      item->appendChild(child);
    }
  } break;
  case nl::ordered_json::value_t::boolean: {
    item->setValue(QVariant(jv.get<bool>()));
  } break;
  case nl::ordered_json::value_t::number_unsigned: {
    item->setValue(QVariant(jv.get<unsigned>()));
  } break;
  case nl::ordered_json::value_t::number_integer: {
    item->setValue(jv.get<signed int>());
  } break;
  case nl::ordered_json::value_t::number_float: {
    item->setValue(QVariant(jv.get<double>()));
  } break;
  case nl::ordered_json::value_t::string: {
    item->setValue(QVariant(QString::fromStdString(jv.get<std::string>())));
  } break;
  case nl::ordered_json::value_t::null:
  case nl::ordered_json::value_t::binary:
  case nl::ordered_json::value_t::discarded: {
    SPDLOG_DEBUG("null/binary/discarded type is ignored!");
  } break;
  default: {
    SPDLOG_WARN("This type is ignored!");
  } break;
  }
  return item;
}

void to_json(nl::ordered_json &obj, const QJsonTreeItem &p) {
  switch (p.type()) {
  case nl::ordered_json::value_t::boolean:
    obj[p.key().toStdString()] = p.value().toBool();
    break;
  case nl::ordered_json::value_t::number_float:
    obj[p.key().toStdString()] = p.value().toDouble();
    break;
  case nl::ordered_json::value_t::number_integer:
    obj[p.key().toStdString()] = p.value().toInt();
    break;
  case nl::ordered_json::value_t::number_unsigned:
    obj[p.key().toStdString()] = p.value().toUInt();
    break;
  case nl::ordered_json::value_t::string:
    obj[p.key().toStdString()] = p.value().toString().toStdString();
    break;
  case nl::ordered_json::value_t::object: {
    for (auto child : p.mChilds) {
      if (p.mParent) {
        to_json(obj[p.key().toStdString()], *child);
      } else {
        to_json(obj, *child);
      }
    }
  } break;
  case nl::ordered_json::value_t::array: {
    nl::ordered_json child_json;
    for (auto child : p.mChilds) {
      to_json(child_json, *child);
    }
    for (auto &[k, v] : child_json.items()) {
      obj[p.key().toStdString()].push_back(v);
    }
  } break;
  default:
    break;
  }
}

//=========================================================================

QJsonModel::QJsonModel(QObject *parent)
    : QAbstractItemModel(parent), mRootItem{new QJsonTreeItem} {
  mHeaders.append(tr("Name"));
  mHeaders.append(tr("Value"));
}

QJsonModel::~QJsonModel() { delete mRootItem; }

bool QJsonModel::load(const std::filesystem::path &json_file,
                      const std::filesystem::path &schema_file) {
  std::ifstream f1(json_file);
  std::ifstream f2(schema_file);
  try {
    nl::ordered_json jv;
    nl::json schema;
    f1 >> jv;
    f2 >> schema;
    return loadJson(jv, schema);
  } catch (std::exception &e) {
    SPDLOG_ERROR("failed to load, exception: {}", json_file, e.what());
    return false;
  }
}

bool QJsonModel::loadJson(const nl::ordered_json &jv, const nl::json &schema) {
  beginResetModel();
  delete mRootItem;
  mRootItem = QJsonTreeItem::load(jv, schema);
  mRootItem->setType(jv.type());
  endResetModel();
  return false;
}

QVariant QJsonModel::data(const QModelIndex &index, int role) const {
  if (!index.isValid())
    return QVariant();
  QJsonTreeItem *item = static_cast<QJsonTreeItem *>(index.internalPointer());
  if (role == Qt::DisplayRole) {
    if (index.column() == 0)
      return QString("%1").arg(item->title());
    if (index.column() == 1)
      return item->value();
  } else if (Qt::EditRole == role) {
    if (index.column() == 1) {
      return item->value();
    }
  }
  return QVariant();
}

bool QJsonModel::setData(const QModelIndex &index, const QVariant &value,
                         int role) {
  int col = index.column();
  if (Qt::EditRole == role) {
    if (col == 1) {
      QJsonTreeItem *item =
          static_cast<QJsonTreeItem *>(index.internalPointer());
      item->setValue(value);
      emit dataChanged(index, index, {Qt::EditRole});
      return true;
    }
  }

  return false;
}

QVariant QJsonModel::headerData(int section, Qt::Orientation orientation,
                                int role) const {
  if (role != Qt::DisplayRole)
    return QVariant();
  if (orientation == Qt::Horizontal) {
    return mHeaders.value(section);
  } else {
    return QVariant();
  }
}

QModelIndex QJsonModel::index(int row, int column,
                              const QModelIndex &parent) const {
  if (!hasIndex(row, column, parent))
    return QModelIndex();
  QJsonTreeItem *parentItem;
  if (!parent.isValid()) {
    parentItem = mRootItem;
  } else {
    parentItem = static_cast<QJsonTreeItem *>(parent.internalPointer());
  }

  QJsonTreeItem *childItem = parentItem->child(row);
  if (childItem) {
    return createIndex(row, column, childItem);
  } else {
    return QModelIndex();
  }
}

QModelIndex QJsonModel::parent(const QModelIndex &index) const {
  if (!index.isValid())
    return QModelIndex();

  QJsonTreeItem *childItem =
      static_cast<QJsonTreeItem *>(index.internalPointer());
  QJsonTreeItem *parentItem = childItem->parent();
  if (parentItem == mRootItem)
    return QModelIndex();
  return createIndex(parentItem->row(), 0, parentItem);
}

int QJsonModel::rowCount(const QModelIndex &parent) const {
  QJsonTreeItem *parentItem;
  if (parent.column() > 0)
    return 0;
  if (!parent.isValid()) {
    parentItem = mRootItem;
  } else {
    parentItem = static_cast<QJsonTreeItem *>(parent.internalPointer());
  }
  return parentItem->childCount();
}

int QJsonModel::columnCount(const QModelIndex &parent) const {
  Q_UNUSED(parent)
  return 2;
}

Qt::ItemFlags QJsonModel::flags(const QModelIndex &index) const {
  int col = index.column();
  auto item = static_cast<QJsonTreeItem *>(index.internalPointer());
  auto isArray = nl::ordered_json::value_t::array == item->type();
  auto isObject = nl::ordered_json::value_t::object == item->type();
  if ((col == 1) && !(isArray || isObject)) {
    return Qt::ItemIsEditable | QAbstractItemModel::flags(index);
  } else {
    return QAbstractItemModel::flags(index);
  }
}

nl::ordered_json QJsonModel::getJson() const {
  return nl::ordered_json(*mRootItem);
}

bool QJsonModel::saveJson(const std::filesystem::path &fileName) const {
  std::ofstream f(fileName);
  if (f) {
    f << nl::ordered_json(*mRootItem);
    return true;
  }
  return false;
}
} // namespace zen