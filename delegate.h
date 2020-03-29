#ifndef DELEGATE_H
#define DELEGATE_H
#include <qstyleditemdelegate.h>
#include <qfiledialog.h>

class Delegate : public QStyledItemDelegate
{
    Q_OBJECT

public:

    Delegate(QObject *parent) : QStyledItemDelegate(parent) {

    }

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                          const QModelIndex &index) const Q_DECL_OVERRIDE;

    void setEditorData(QWidget *editor, const QModelIndex &index) const Q_DECL_OVERRIDE;
    void setModelData(QWidget *editor, QAbstractItemModel *model,
                      const QModelIndex &index) const Q_DECL_OVERRIDE;

    void updateEditorGeometry(QWidget *editor,
        const QStyleOptionViewItem &option, const QModelIndex &index) const Q_DECL_OVERRIDE;

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
        const QModelIndex &index) const;


public slots:
//    void browserClicked();
//    void dirBrowserClicked();
//    void browserCheck(QString);// const;

private:
    QString browserFileName;
    QModelIndex *tempIndex;
    QFileDialog *fileDialog;
    QStyleOptionViewItem option;
};
#endif // DELEGATE_H
