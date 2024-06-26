#include <QParameterWidget/ParameterItemDelegate.h>
#include <QParameterWidget/QJsonModel.h>

#include <fmt/std.h>
#include <spdlog/spdlog.h>

#include <QColorDialog>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QPainter>
#include <QSpinBox>

#include <fstream>

namespace zen
{

template <typename T>
T GetProperty(const nl::json &schema, const std::string &json_pointer,
              const T &default_value = T{})
{
    try {
        return schema.at(nl::json::json_pointer(json_pointer)).get<T>();
    } catch (std::exception &e) {
        SPDLOG_WARN("failed to get property from {}, exception: {}",
                    json_pointer, e.what());
        return default_value;
    }
}

ParameterItemDelegate::ParameterItemDelegate(const nl::json &schema,
                                             QObject *parent,
                                             const QLocale &locale)
    : QStyledItemDelegate(parent), m_schema(schema), m_locale(locale)
{
}

void ParameterItemDelegate::paint(QPainter *painter,
                                  const QStyleOptionViewItem &option,
                                  const QModelIndex &index) const
{
    auto item = static_cast<QJsonTreeItem *>(index.internalPointer());
    auto jp = item->schema_json_pointer;
    auto item_type = GetProperty<std::string>(m_schema, jp + "/type", "");
    if (item_type == "color" && index.column() == 1) {
        painter->fillRect(option.rect, QColor(item->value.toString()));
    } else if (item_type == "number" && index.column() == 1) {
        int decimals = GetProperty<int>(m_schema, jp + "/decimals", 2);
        auto suffix = QString::fromStdString(
            GetProperty<std::string>(m_schema, jp + "/suffix"));
        painter->drawText(option.rect, Qt::AlignLeft | Qt::AlignVCenter,
                          QString("%1 %2")
                              .arg(item->value.toDouble(), 0, 'g', decimals)
                              .arg(suffix));
    } else if (item_type == "integer" && index.column() == 1) {
        auto suffix = QString::fromStdString(
            GetProperty<std::string>(m_schema, jp + "/suffix"));
        painter->drawText(
            option.rect, Qt::AlignLeft | Qt::AlignVCenter,
            QString("%1 %2").arg(item->value.toInt()).arg(suffix));
    } else if (item_type == "enum" && index.column() == 1) {
        auto lang = LanguageToCode(m_locale);
        auto enum_items =
            m_schema.at(nl::ordered_json::json_pointer(jp + "/items"));
        auto title = item->value.toString();
        // assuming that there will not be too many enum items
        for (const auto &enum_item : enum_items) {
            auto name = enum_item.at("name").get<std::string>();
            if (name == item->value.toString().toStdString()) {
                if (lang != "en" && enum_item.contains("title_" + lang)) {
                    title = QString::fromStdString(
                        enum_item.at("title_" + lang).get<std::string>());
                }
                break;
            }
        }
        painter->drawText(option.rect, Qt::AlignLeft | Qt::AlignVCenter, title);
    } else {
        QStyledItemDelegate::paint(painter, option, index);
    }
}

QWidget *ParameterItemDelegate::createEditor(QWidget *parent,
                                             const QStyleOptionViewItem &option,
                                             const QModelIndex &index) const
{
    auto item = static_cast<QJsonTreeItem *>(index.internalPointer());
    auto jp = item->schema_json_pointer;
    auto item_type = GetProperty<std::string>(m_schema, jp + "/type", "");
    if (item_type == "integer") {
        auto editor = new QSpinBox(parent);
        int minimium = GetProperty<int>(m_schema, jp + "/minimium",
                                        std::numeric_limits<int>::min());
        int maximium = GetProperty<int>(m_schema, jp + "/maximium",
                                        std::numeric_limits<int>::max());
        int step = GetProperty<int>(m_schema, jp + "/singleStep", 1);
        auto suffix = GetProperty<std::string>(m_schema, jp + "/suffix", "");
        editor->setMinimum(minimium);
        editor->setMaximum(maximium);
        editor->setSingleStep(step);
        editor->setSuffix(QString::fromStdString(suffix));
        return editor;
    } else if (item_type == "number") {
        auto editor = new QDoubleSpinBox(parent);
        double minimium = GetProperty<double>(
            m_schema, jp + "/minimium", std::numeric_limits<double>::min());
        double maximium = GetProperty<double>(
            m_schema, jp + "/maximium", std::numeric_limits<double>::max());
        double step = GetProperty<double>(m_schema, jp + "/singleStep", 1.0);
        auto suffix = GetProperty<std::string>(m_schema, jp + "/suffix", "");
        auto decimals = GetProperty<int>(m_schema, jp + "/decimals", 6);
        editor->setMinimum(minimium);
        editor->setMaximum(maximium);
        editor->setSingleStep(step);
        editor->setSuffix(QString::fromStdString(suffix));
        editor->setDecimals(decimals);
        return editor;
    } else if (item_type == "enum") {
        auto lang = LanguageToCode(m_locale);
        auto editor = new QComboBox(parent);
        auto items = m_schema.at(nl::ordered_json::json_pointer(jp + "/items"));
        for (const auto &item : items) {
            auto name = item.at("name").get<std::string>();
            auto title = name;
            if (lang != "en" && item.contains("title_" + lang)) {
                title = item.at("title_" + lang).get<std::string>();
            }
            editor->addItem(QString::fromStdString(title),
                            QString::fromStdString(name));
        }
        return editor;
    } else if (item_type == "color") {
        auto editor = new QColorDialog(QColor(item->value.toString()), parent);
        return editor;
    } else {
        return QStyledItemDelegate::createEditor(parent, option, index);
    }
}

void ParameterItemDelegate::setEditorData(QWidget *editor,
                                          const QModelIndex &index) const
{
    auto item = static_cast<QJsonTreeItem *>(index.internalPointer());
    auto jp = item->schema_json_pointer;
    auto item_type = GetProperty<std::string>(m_schema, jp + "/type", "");
    if (item_type == "color") {
        auto color_dialog = qobject_cast<QColorDialog *>(editor);
        color_dialog->setCurrentColor(QColor(item->value.toString()));
    } else {
        QStyledItemDelegate::setEditorData(editor, index);
    }
}

void ParameterItemDelegate::setModelData(QWidget *editor,
                                         QAbstractItemModel *model,
                                         const QModelIndex &index) const
{
    auto item = static_cast<QJsonTreeItem *>(index.internalPointer());
    auto jp = item->schema_json_pointer;
    auto item_type = GetProperty<std::string>(m_schema, jp + "/type", "");
    if (item_type == "color") {
        auto color_dialog = qobject_cast<QColorDialog *>(editor);
        QString color =
            color_dialog->selectedColor().name(QColor::NameFormat::HexRgb);
        model->setData(index, QVariant::fromValue(color));
    } else if (item_type == "enum") {
        auto combox = qobject_cast<QComboBox *>(editor);
        model->setData(index, combox->currentData());
    } else {
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