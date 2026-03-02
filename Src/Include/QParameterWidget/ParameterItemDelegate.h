/**
 * Copyright © 2026 Zen Shawn. All rights reserved.
 *
 * @file ParameterItemDelegate.h
 * @author: Zen Shawn
 * @email: xiaozisheng2008@hotmail.com
 * @date: 16:01:17, 2026-02-25
 */
#pragma once
#include "qparameterwidget_export.h"
#include <QStyledItemDelegate>
#include <nlohmann/json.hpp>

#include <filesystem>

namespace zen
{
namespace nl = nlohmann;

struct CustomizedDataEditor {
    std::string data_type;

    std::function<QWidget *(QWidget *, const QStyleOptionViewItem &,
                            const QModelIndex &, const nl::json &)>
        create_editor = nullptr;

    std::function<void(QWidget *, const QModelIndex &, const nl::json &)>
        set_editor_data = nullptr;

    std::function<void(QWidget *, QAbstractItemModel *, const QModelIndex &,
                       const nl::json &)>
        set_model_data = nullptr;

    std::function<void(QPainter *, const QStyleOptionViewItem &,
                       const QModelIndex &, const nl::json &)>
        paint = nullptr;
};

class QPARAMETERWIDGET_EXPORT ParameterItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    static void RegisterTypeEditor(CustomizedDataEditor editor);

    ParameterItemDelegate(const nl::json &schema, QObject *parent = nullptr,
                          const QLocale &locale = QLocale());

    virtual void paint(QPainter *painter, const QStyleOptionViewItem &option,
                       const QModelIndex &index) const override;

    virtual QWidget *createEditor(QWidget *parent,
                                  const QStyleOptionViewItem &option,
                                  const QModelIndex &index) const override;

    virtual void setEditorData(QWidget *editor,
                               const QModelIndex &index) const override;

    virtual void setModelData(QWidget *editor, QAbstractItemModel *model,
                              const QModelIndex &index) const override;

    void updateEditorGeometry(QWidget *editor,
                              const QStyleOptionViewItem &option,
                              const QModelIndex &index) const override;

    bool editorEvent(QEvent *event, QAbstractItemModel *model,
                     const QStyleOptionViewItem &option,
                     const QModelIndex &index) override;

private:
    void commitAndCloseEditor();

    nlohmann::json m_schema;
    QLocale m_locale;
};
} // namespace zen
