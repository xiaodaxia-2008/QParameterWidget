#pragma once
#include <QStyledItemDelegate>
#include <nlohmann/json.hpp>
#include <qparameterwidget_export.h>

#include <filesystem>

namespace zen
{
namespace nl = nlohmann;
class QPARAMETERWIDGET_EXPORT ParameterItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
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
