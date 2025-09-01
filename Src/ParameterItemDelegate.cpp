#include <QParameterWidget/ParameterItemDelegate.h>
#include <QParameterWidget/QJsonModel.h>

#include <fmt/std.h>
#include <spdlog/spdlog.h>

#include <QAbstractItemView>
#include <QApplication>
#include <QColorDialog>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QMouseEvent>
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
        (void)e;
        SPDLOG_DEBUG("failed to get property from {}, exception: {}",
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

QRect GetCheckBoxRect(const QStyleOptionViewItem &option)
{
    QStyleOptionButton checkBoxOption;
    checkBoxOption.rect = QApplication::style()->subElementRect(
        QStyle::SE_CheckBoxIndicator, &option, option.widget);

    int centerX = option.rect.left() + checkBoxOption.rect.width() / 2;
    int centerY = option.rect.top() + option.rect.height() / 2
                  - checkBoxOption.rect.height() / 2;
    checkBoxOption.rect.moveTo(centerX, centerY);
    return checkBoxOption.rect;
}

void ParameterItemDelegate::paint(QPainter *painter,
                                  const QStyleOptionViewItem &option,
                                  const QModelIndex &index) const
{
    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);

    if (opt.state & QStyle::State_Editing) {
        return;
    }

    auto item = static_cast<QJsonTreeItem *>(index.internalPointer());
    auto jp = item->schema_json_pointer;
    auto item_type = GetProperty<std::string>(m_schema, jp + "/type", "");
    if (item_type == "color" && index.column() == 1) {
        opt.text = "";
        QStyledItemDelegate::paint(painter, opt, index);
        // inset a little for padding
        QRect colorRect = opt.rect.adjusted(2, 2, -2, -2);
        painter->fillRect(option.rect, QColor(item->value.toString()));
        return;
    }

    if (item_type == "number" && index.column() == 1) {
        int decimals = GetProperty<int>(m_schema, jp + "/decimals", 2);
        auto suffix = QString::fromStdString(
            GetProperty<std::string>(m_schema, jp + "/suffix"));
        opt.text = QString("%1 %2")
                       .arg(item->value.toDouble(), 0, 'g', decimals)
                       .arg(suffix);
    } else if (item_type == "integer" && index.column() == 1) {
        auto suffix = QString::fromStdString(
            GetProperty<std::string>(m_schema, jp + "/suffix"));
        opt.text = QString("%1 %2").arg(item->value.toInt()).arg(suffix);
    } else if (item_type == "boolean" && index.column() == 1) {
        opt.text.clear();
        bool checked = item->value.toBool();

        // 准备一个 QStyleOptionButton 来描述复选框
        QStyleOptionButton checkBoxOption;
        checkBoxOption.rect = GetCheckBoxRect(opt);

        checkBoxOption.state = QStyle::State_Enabled;
        if (checked) {
            checkBoxOption.state |= QStyle::State_On;
        } else {
            checkBoxOption.state |= QStyle::State_Off;
        }

        // 让 style 绘制背景和复选框
        QApplication::style()->drawControl(QStyle::CE_ItemViewItem, &opt,
                                           painter);
        QApplication::style()->drawControl(QStyle::CE_CheckBox, &checkBoxOption,
                                           painter);
        return;
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
        opt.text = title;
    }
    QStyle *style =
        option.widget ? option.widget->style() : QApplication::style();
    opt.text.prepend("  ");
    style->drawControl(QStyle::CE_ItemViewItem, &opt, painter, option.widget);
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
        int minimum = GetProperty<int>(m_schema, jp + "/minimum",
                                       std::numeric_limits<int>::min());
        int maximum = GetProperty<int>(m_schema, jp + "/maximum",
                                       std::numeric_limits<int>::max());
        int step = GetProperty<int>(m_schema, jp + "/singleStep", 1);
        auto suffix = GetProperty<std::string>(m_schema, jp + "/suffix", "");
        editor->setMinimum(minimum);
        editor->setMaximum(maximum);
        editor->setSingleStep(step);
        editor->setSuffix(QString::fromStdString(suffix));
        return editor;
    } else if (item_type == "number") {
        auto editor = new QDoubleSpinBox(parent);
        double minimum = GetProperty<double>(
            m_schema, jp + "/minimum", std::numeric_limits<double>::min());
        double maximum = GetProperty<double>(
            m_schema, jp + "/maximum", std::numeric_limits<double>::max());
        double step = GetProperty<double>(m_schema, jp + "/singleStep", 1.0);
        auto suffix = GetProperty<std::string>(m_schema, jp + "/suffix", "");
        auto decimals = GetProperty<int>(m_schema, jp + "/decimals", 6);
        editor->setMinimum(minimum);
        editor->setMaximum(maximum);
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

void ParameterItemDelegate::updateEditorGeometry(
    QWidget *editor, const QStyleOptionViewItem &option,
    const QModelIndex &index) const
{
    editor->setGeometry(option.rect);
}

bool ParameterItemDelegate::editorEvent(QEvent *event,
                                        QAbstractItemModel *model,
                                        const QStyleOptionViewItem &option,
                                        const QModelIndex &index)
{
    // 确保是布尔类型并且是第二列
    auto item = static_cast<QJsonTreeItem *>(index.internalPointer());
    if (item
        && GetProperty<std::string>(m_schema,
                                    item->schema_json_pointer + "/type", "")
               == "boolean"
        && index.column() == 1) {
        // 只处理鼠标释放事件
        if (event->type() == QEvent::MouseButtonRelease) {
            // 获取复选框的区域，以便判断点击是否在它上面
            QStyleOptionButton checkBoxOption;
            checkBoxOption.rect = GetCheckBoxRect(option);

            // 如果点击位置在复选框的矩形区域内
            if (checkBoxOption.rect.contains(
                    static_cast<QMouseEvent *>(event)->pos())) {
                // 切换模型中的值
                bool currentValue = model->data(index).toBool();
                model->setData(index, !currentValue);
                return true; // 事件已处理
            }
        }
    }

    // 对于其他所有情况，调用基类的默认实现
    return QStyledItemDelegate::editorEvent(event, model, option, index);
}

void ParameterItemDelegate::commitAndCloseEditor()
{
    QWidget *editor = qobject_cast<QWidget *>(sender());
    emit commitData(editor);
    emit closeEditor(editor);
}
} // namespace zen