#pragma once
#include <QStyledItemDelegate>
#include <nlohmann/json.hpp>

#include <filesystem>

namespace zen {
namespace nl = nlohmann;
class ParameterItemDelegate : public QStyledItemDelegate {
  Q_OBJECT
public:
  ParameterItemDelegate(const nl::json &schema, QObject *parent = nullptr);
  virtual void paint(QPainter *painter, const QStyleOptionViewItem &option,
                     const QModelIndex &index) const override;
  virtual QWidget *createEditor(QWidget *parent,
                                const QStyleOptionViewItem &option,
                                const QModelIndex &index) const override;
  virtual void setEditorData(QWidget *editor,
                             const QModelIndex &index) const override;
  virtual void setModelData(QWidget *editor, QAbstractItemModel *model,
                            const QModelIndex &index) const override;

private:
  void commitAndCloseEditor();

  nlohmann::json m_schema;
};
} // namespace zen
