#include "ParameterItemDelegate.h"

#include "QJsonModel.h"

#include <fmt/std.h>
#include <spdlog/spdlog.h>

#include <QColorDialog>
#include <QDoubleSpinBox>
#include <QPainter>
#include <QSpinBox>

#include <fstream>

namespace zen
{

template <typename T>
T GetProperty(const nl::json &schema, const std::string &json_pointer,
              const T &default_value)
{
    try {
        return schema.at(nl::json::json_pointer(json_pointer)).get<T>();
    }
    catch (std::exception &e) {
        SPDLOG_WARN("failed to get property from {}, exception: {}",
                    json_pointer, e.what());
        return default_value;
    }
}

ParameterItemDelegate::ParameterItemDelegate(const nl::json &schema,
                                             QObject *parent)
    : QStyledItemDelegate(parent), m_schema(schema)
{
}

void ParameterItemDelegate::paint(QPainter *painter,
                                  const QStyleOptionViewItem &option,
                                  const QModelIndex &index) const
{
    auto item = static_cast<QJsonTreeItem *>(index.internalPointer());
    auto jp = item->json_pointer;
    auto item_type = GetProperty<std::string>(m_schema, jp + "/type", "");
    if (item_type == "color" && index.column() == 1) {
        painter->fillRect(option.rect, QColor(item->value.toString()));
    }
    else {
        QStyledItemDelegate::paint(painter, option, index);
    }
}

QWidget *ParameterItemDelegate::createEditor(QWidget *parent,
                                             const QStyleOptionViewItem &option,
                                             const QModelIndex &index) const
{
    auto item = static_cast<QJsonTreeItem *>(index.internalPointer());
    auto jp = item->json_pointer;
    auto item_type = GetProperty<std::string>(m_schema, jp + "/type", "");
    if (item_type == "integer") {
        auto editor = new QSpinBox(parent);
        int minimium = GetProperty<int>(m_schema, jp + "/minimium",
                                        std::numeric_limits<int>::min());
        int maximium = GetProperty<int>(m_schema, jp + "/maximium",
                                        std::numeric_limits<int>::max());
        int step = GetProperty<int>(m_schema, jp + "/singleStep", 1);
        editor->setMinimum(minimium);
        editor->setMaximum(maximium);
        editor->setSingleStep(step);
        return editor;
    }
    else if (item_type == "number") {
        auto editor = new QDoubleSpinBox(parent);
        double minimium = GetProperty<double>(
            m_schema, jp + "/minimium", std::numeric_limits<double>::min());
        double maximium = GetProperty<double>(
            m_schema, jp + "/maximium", std::numeric_limits<double>::max());
        double step = GetProperty<double>(m_schema, jp + "/singleStep", 1.0);
        editor->setMinimum(minimium);
        editor->setMaximum(maximium);
        editor->setSingleStep(step);
        return editor;
    }
    else if (item_type == "color") {
        auto editor = new QColorDialog(QColor(item->value.toString()), parent);
        return editor;
    }
    else {
        return QStyledItemDelegate::createEditor(parent, option, index);
    }
}

void ParameterItemDelegate::setEditorData(QWidget *editor,
                                          const QModelIndex &index) const
{
    auto item = static_cast<QJsonTreeItem *>(index.internalPointer());
    auto jp = item->json_pointer;
    auto item_type = GetProperty<std::string>(m_schema, jp + "/type", "");
    if (item_type == "color") {
        auto color_dialog = qobject_cast<QColorDialog *>(editor);
        color_dialog->setCurrentColor(QColor(item->value.toString()));
    }
    else {
        QStyledItemDelegate::setEditorData(editor, index);
    }
}

void ParameterItemDelegate::setModelData(QWidget *editor,
                                         QAbstractItemModel *model,
                                         const QModelIndex &index) const
{
    auto item = static_cast<QJsonTreeItem *>(index.internalPointer());
    auto jp = item->json_pointer;
    auto item_type = GetProperty<std::string>(m_schema, jp + "/type", "");
    if (item_type == "color") {
        auto color_dialog = qobject_cast<QColorDialog *>(editor);
        QString color =
            color_dialog->selectedColor().name(QColor::NameFormat::HexRgb);
        model->setData(index, QVariant::fromValue(color));
    }
    else {
        QStyledItemDelegate::setModelData(editor, model, index);
    }
}

void ParameterItemDelegate::commitAndCloseEditor()
{
    QWidget *editor = qobject_cast<QWidget *>(sender());
    emit commitData(editor);
    emit closeEditor(editor);
}
} // namespace zen