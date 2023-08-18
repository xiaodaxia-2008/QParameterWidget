#include "QJsonModel.h"

#include <fmt/std.h>
#include <spdlog/spdlog.h>

#include <filesystem>
#include <fstream>

namespace zen
{
void to_json(nl::ordered_json &j, const QJsonTreeItem &p);

std::string GetJsonPath(QJsonTreeItem *item)
{
    std::string key = item->key.toStdString();
    if (auto parent = item->parent; parent && parent->key != "root") {
        return fmt::format("{}/properties/{}", GetJsonPath(parent), key);
    }
    return "/properties/" + key;
}

QJsonTreeItem::QJsonTreeItem(QJsonTreeItem *parent) : parent(parent) {}

QJsonTreeItem::~QJsonTreeItem() { qDeleteAll(children); }

int QJsonTreeItem::row() const
{
    if (this->parent) {
        return this->parent->children.indexOf(
            const_cast<QJsonTreeItem *>(this));
    }
    return 0;
}

QJsonTreeItem *QJsonTreeItem::load(const nl::ordered_json &jv,
                                   const nl::json &schema,
                                   const std::string &key,
                                   QJsonTreeItem *parent)
{
    QJsonTreeItem *item = new QJsonTreeItem(parent);
    item->key = QString::fromStdString(key);
    item->type = jv.type();
    if (parent) {
        item->schema_json_pointer =
            parent->schema_json_pointer + "/properties/" + key;
        item->param_json_pointer = parent->param_json_pointer + "/" + key;
    }
    else if (!key.empty()) {
        item->schema_json_pointer = "/properties/" + key;
        item->param_json_pointer = "/" + key;
    }

    switch (jv.type()) {
    case nl::ordered_json::value_t::array:
    case nl::ordered_json::value_t::object: {
        for (auto &[k, v] : jv.items()) {
            QJsonTreeItem *child = load(v, schema, k, item);
            try {
                nl::json::json_pointer jp(child->schema_json_pointer
                                          + "/title");
                auto title = schema.at(jp).get<std::string>();
                child->title = QString::fromStdString(title);
            }
            catch (std::exception &e) {
                SPDLOG_WARN(
                    "failed to get item title from {}/title, exception: {}",
                    child->schema_json_pointer, e.what());
                child->title = child->key;
            }
            item->children.append(child);
        }
    } break;
    case nl::ordered_json::value_t::boolean: {
        item->value = jv.get<bool>();
    } break;
    case nl::ordered_json::value_t::number_unsigned: {
        item->value = jv.get<unsigned>();
    } break;
    case nl::ordered_json::value_t::number_integer: {
        item->value = jv.get<signed int>();
    } break;
    case nl::ordered_json::value_t::number_float: {
        item->value = jv.get<double>();
    } break;
    case nl::ordered_json::value_t::string: {
        item->value = QString::fromStdString(jv.get<std::string>());
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

void to_json(nl::ordered_json &obj, const QJsonTreeItem &p)
{
    switch (p.type) {
    case nl::ordered_json::value_t::boolean:
        obj[p.key.toStdString()] = p.value.toBool();
        break;
    case nl::ordered_json::value_t::number_float:
        obj[p.key.toStdString()] = p.value.toDouble();
        break;
    case nl::ordered_json::value_t::number_integer:
        obj[p.key.toStdString()] = p.value.toInt();
        break;
    case nl::ordered_json::value_t::number_unsigned:
        obj[p.key.toStdString()] = p.value.toUInt();
        break;
    case nl::ordered_json::value_t::string:
        obj[p.key.toStdString()] = p.value.toString().toStdString();
        break;
    case nl::ordered_json::value_t::object: {
        for (auto child : p.children) {
            if (p.parent) {
                to_json(obj[p.key.toStdString()], *child);
            }
            else {
                to_json(obj, *child);
            }
        }
    } break;
    case nl::ordered_json::value_t::array: {
        nl::ordered_json child_json;
        for (auto child : p.children) {
            to_json(child_json, *child);
        }
        for (auto &[k, v] : child_json.items()) {
            obj[p.key.toStdString()].push_back(v);
        }
    } break;
    default:
        break;
    }
}

//=========================================================================

QJsonModel::QJsonModel(QObject *parent)
    : QAbstractItemModel(parent), m_root_item{new QJsonTreeItem}
{
}

QJsonModel::~QJsonModel() { delete m_root_item; }

bool QJsonModel::LoadJson(const nl::ordered_json &jv, const nl::json &schema)
{
    m_json = jv;
    beginResetModel();
    delete m_root_item;
    m_root_item = QJsonTreeItem::load(jv, schema);
    m_root_item->type = jv.type();
    endResetModel();
    return false;
}

QVariant QJsonModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) return QVariant();
    QJsonTreeItem *item = static_cast<QJsonTreeItem *>(index.internalPointer());
    if (role == Qt::DisplayRole) {
        if (index.column() == 0) return QString("%1").arg(item->title);
        if (index.column() == 1) return item->value;
    }
    else if (Qt::EditRole == role) {
        if (index.column() == 1) {
            return item->value;
        }
    }
    return QVariant();
}

bool QJsonModel::setData(const QModelIndex &index, const QVariant &value,
                         int role)
{
    int col = index.column();
    if (Qt::EditRole == role) {
        if (col == 1) {
            QJsonTreeItem *item =
                static_cast<QJsonTreeItem *>(index.internalPointer());
            item->value = value;
            // TODO: update back to json
            // m_json.at(item->param_json_pointer) = *item;
            emit dataChanged(index, index, {Qt::EditRole});
            return true;
        }
    }

    return false;
}

QVariant QJsonModel::headerData(int section, Qt::Orientation orientation,
                                int role) const
{
    if (role != Qt::DisplayRole) return QVariant();
    if (orientation == Qt::Horizontal) {
        if (section == 0) {
            return tr("Name");
        }
        else if (section == 1) {
            return tr("Value");
        }
    }
    return QVariant();
}

QModelIndex QJsonModel::index(int row, int column,
                              const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent)) return QModelIndex();
    QJsonTreeItem *parentItem;
    if (!parent.isValid()) {
        parentItem = m_root_item;
    }
    else {
        parentItem = static_cast<QJsonTreeItem *>(parent.internalPointer());
    }

    QJsonTreeItem *childItem = parentItem->children.at(row);
    if (childItem) {
        return createIndex(row, column, childItem);
    }
    else {
        return QModelIndex();
    }
}

QModelIndex QJsonModel::parent(const QModelIndex &index) const
{
    if (!index.isValid()) return QModelIndex();

    QJsonTreeItem *childItem =
        static_cast<QJsonTreeItem *>(index.internalPointer());
    QJsonTreeItem *parentItem = childItem->parent;
    if (parentItem == m_root_item) return QModelIndex();
    return createIndex(parentItem->row(), 0, parentItem);
}

int QJsonModel::rowCount(const QModelIndex &parent) const
{
    QJsonTreeItem *parentItem;
    if (parent.column() > 0) return 0;
    if (!parent.isValid()) {
        parentItem = m_root_item;
    }
    else {
        parentItem = static_cast<QJsonTreeItem *>(parent.internalPointer());
    }
    return parentItem->children.count();
}

int QJsonModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return 2;
}

Qt::ItemFlags QJsonModel::flags(const QModelIndex &index) const
{
    int col = index.column();
    auto item = static_cast<QJsonTreeItem *>(index.internalPointer());
    auto isArray = nl::ordered_json::value_t::array == item->type;
    auto isObject = nl::ordered_json::value_t::object == item->type;
    if ((col == 1) && !(isArray || isObject)) {
        return Qt::ItemIsEditable | QAbstractItemModel::flags(index);
    }
    else {
        return QAbstractItemModel::flags(index);
    }
}

const nl::ordered_json &QJsonModel::GetJson() const
{
    m_json = *m_root_item; //
    return m_json;
}

bool QJsonModel::SaveJson(const std::filesystem::path &fileName) const
{
    std::ofstream f(fileName);
    if (f) {
        f << nl::ordered_json(*m_root_item);
        return true;
    }
    return false;
}
} // namespace zen