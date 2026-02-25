#include <QParameterWidget/ParameterItemDelegate.h>
#include <QParameterWidget/QJsonModel.h>

#include <fmt/std.h>
#include <nlohmann/json_fwd.hpp>
#include <qbrush.h>
#include <qcolor.h>
#include <qpoint.h>
#include <qstyleditemdelegate.h>
#include <spdlog/spdlog.h>

#include <QAbstractItemView>
#include <QApplication>
#include <QColorDialog>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QFileDialog>
#include <QMouseEvent>
#include <QPainter>
#include <QSpinBox>
#include <string>

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

bool IsFilepath(const QJsonTreeItem *item, const nl::json &schema)
{
    if (GetProperty<std::string>(schema, item->schema_json_pointer + "/type")
        == "filepath") {
        return true;
    }

    if (item->type != nl::json::value_t::string) {
        return false;
    }

    QString str = item->value.toString();
    QFileInfo file(str);
    if (file.exists()) {
        return true;
    }

    if (str.contains('/') || str.contains('\\')) {

        if (file.absoluteDir().exists()) {
            return true;
        }
    }

    return false;
}

bool IsColorType(const QJsonTreeItem *item, const nl::json &schema)
{
    if (GetProperty<std::string>(schema, item->schema_json_pointer + "/type")
        == "color") {
        return true;
    }

    if (item->type == nl::json::value_t::string) {
        return QColor(item->value.toString()).isValid();
    }

    return false;
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
    if (index.column() == 0) {
        QStyledItemDelegate::paint(painter, option, index);
        return;
    }

    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);

    if (opt.state & QStyle::State_Editing) {
        return;
    }

    auto item = static_cast<QJsonTreeItem *>(index.internalPointer());
    auto jp = item->schema_json_pointer;
    auto item_type = GetProperty<std::string>(m_schema, jp + "/type", "");
    using value_t = nl::ordered_json::value_t;
    if (IsColorType(item, m_schema)) {
        // 1. Draw the standard selection/hover background first
        QApplication::style()->drawControl(QStyle::CE_ItemViewItem, &opt,
                                           painter);

        QString colorStr = item->value.toString();
        QColor bgColor(colorStr);

        // 2. Fill the cell with the item's color
        // We adjust the rect slightly to not overlap the grid lines if they
        // exist
        QRect colorRect = opt.rect.adjusted(1, 1, -1, -1);
        painter->fillRect(colorRect, bgColor);

        // 3. Determine "Contrast Color" (Black or White)
        // Formula: (R * 299 + G * 587 + B * 114) / 1000
        // Or use Qt's built-in luminance check:
        QColor textColor = (bgColor.lightness() > 128) ? Qt::black : Qt::white;

        // 4. Draw the text centered
        painter->setPen(textColor);
        // Use the option's font (it handles bold/italic state changes)
        painter->setFont(opt.font);
        painter->drawText(colorRect, Qt::AlignVCenter | Qt::AlignLeft,
                          colorStr);

        return;
    }

    if (item->type == value_t::number_float) {
        int decimals = GetProperty<int>(m_schema, jp + "/decimals", 2);
        auto suffix = QString::fromStdString(
            GetProperty<std::string>(m_schema, jp + "/suffix"));
        opt.text = QString("%1 %2")
                       .arg(item->value.toDouble(), 0, 'g', decimals)
                       .arg(suffix);
    } else if (item->type == value_t::number_integer
               || item->type == value_t::number_unsigned) {
        auto suffix = QString::fromStdString(
            GetProperty<std::string>(m_schema, jp + "/suffix"));
        opt.text = QString("%1 %2").arg(item->value.toInt()).arg(suffix);
    } else if (item->type == value_t::boolean) {
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
    } else if (item_type == "enum") {
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
    style->drawControl(QStyle::CE_ItemViewItem, &opt, painter, option.widget);
}

QWidget *ParameterItemDelegate::createEditor(QWidget *parent,
                                             const QStyleOptionViewItem &option,
                                             const QModelIndex &index) const
{
    auto item = static_cast<QJsonTreeItem *>(index.internalPointer());
    auto jp = item->schema_json_pointer;
    auto item_type = GetProperty<std::string>(m_schema, jp + "/type", "");
    using value_t = nl::ordered_json::value_t;
    if (item->type == value_t::number_integer
        || item->type == value_t::number_unsigned) {
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
    } else if (item->type == value_t::number_float) {
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
    } else if (IsColorType(item, m_schema)) {
        auto editor = new QColorDialog(QColor(item->value.toString()), parent);
        return editor;
    } else if (IsFilepath(item, m_schema)) {
        auto editor =
            new QFileDialog(parent, "File Select", item->value.toString());
        editor->setAcceptMode(QFileDialog::AcceptSave);
        editor->setFileMode(QFileDialog::AnyFile);
        editor->setMinimumSize(QSize(600, 400));
        return editor;
    } else {
        // default editor is used when there is no schema specified for this
        // item
        auto editor = QStyledItemDelegate::createEditor(parent, option, index);
        if (auto spinbox = qobject_cast<QDoubleSpinBox *>(editor)) {
            spinbox->setDecimals(4);
            spinbox->setMinimum(std::numeric_limits<double>::lowest());
            spinbox->setMaximum(std::numeric_limits<double>::max());
            spinbox->setSingleStep(0.1);
        } else if (auto spinbox = qobject_cast<QSpinBox *>(editor)) {
            spinbox->setMinimum(std::numeric_limits<int>::lowest());
            spinbox->setMaximum(std::numeric_limits<int>::max());
        }
        return editor;
    }
}

void ParameterItemDelegate::setEditorData(QWidget *editor,
                                          const QModelIndex &index) const
{
    auto item = static_cast<QJsonTreeItem *>(index.internalPointer());
    if (IsColorType(item, m_schema)) {
        auto color_dialog = qobject_cast<QColorDialog *>(editor);
        color_dialog->setCurrentColor(QColor(item->value.toString()));
    } else if (IsFilepath(item, m_schema)) {
        auto file_dialog = qobject_cast<QFileDialog *>(editor);
        file_dialog->selectFile(item->value.toString());
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
    try {
        SPDLOG_INFO("value: {}", item->value.toString().toStdString());
    } catch (...) {
    }
    if (IsColorType(item, m_schema)) {
        auto color_dialog = qobject_cast<QColorDialog *>(editor);
        QColor color = color_dialog->selectedColor();
        if (color.isValid()) {
            QString color_str = color.name(QColor::NameFormat::HexRgb);
            model->setData(index, QVariant::fromValue(color_str));
        }
    } else if (IsFilepath(item, m_schema)) {
        auto file_dialog = qobject_cast<QFileDialog *>(editor);
        if (file_dialog->acceptMode() == QDialog::Accepted) {
            QString file = file_dialog->selectedFiles().first();
            SPDLOG_INFO("file: {}", file.toStdString());
            model->setData(index, file);
        }
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
    using value_t = nl::ordered_json::value_t;
    auto item = static_cast<QJsonTreeItem *>(index.internalPointer());
    if (item && item->type == value_t::boolean && index.column() == 1) {
        if (event->type() == QEvent::MouseButtonRelease) {
            QStyleOptionButton checkBoxOption;
            checkBoxOption.rect = GetCheckBoxRect(option);
            if (checkBoxOption.rect.contains(
                    static_cast<QMouseEvent *>(event)->pos())) {
                bool currentValue = model->data(index).toBool();
                model->setData(index, !currentValue);
                return true;
            }
        }
    }

    return QStyledItemDelegate::editorEvent(event, model, option, index);
}

void ParameterItemDelegate::commitAndCloseEditor()
{
    QWidget *editor = qobject_cast<QWidget *>(sender());
    emit commitData(editor);
    emit closeEditor(editor);
}
} // namespace zen
