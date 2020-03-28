#include "propmodel.h"
#include "qdebug.h"

QVariant PropModel::data(const QModelIndex & index, int role) const
{
    //qDebug() << "Role" << role;
    if (!items.count()) // ordinary structure (only one object selected)
    {
        if (index.row() >= rows()) return QVariant();
        if (role == saveIndex)
        {
            save(index);
            return "index Saved";
        }
        if (role == loadIndexandInputMethodRole)
        {
            return data(load(), InputMethodRole);
        }
        if (role == loadIndexandDefaultValuesListRole)
        {
            return data(load(), DefaultValuesListRole);
        }
        if (role == loadIndexandVariableTypeRole)
        {
            return data(load(), VariableTypeRole);
        }

        if (!index.isValid())
            return QVariant();
        int row = index.row();
        int col = index.column();
        //qDebug() << QString("data for (%1, %2) role %3").arg(row).arg(col).arg(role);
        QString VariableName = parent->getmList(parent->ObjectType()).getaskables().VariableNames()[row];

        //qDebug() << QString("Variable Name %1").arg(VariableName); // .arg(col).arg(role).arg(VariableName);

        if (role == VariableNameRole) return VariableName;
        if (col == 0) {
            if (role == Qt::DisplayRole)
            {
                if (parent->parent->applicationShortName == "GWA" && VariableName == "SubType")return "Distribution";
                //			else if (parent->parent->applicationShortName == "GIFMod" && VariableName == "SubType")return "Model";

                else return VariableName;
            }
            if (role == Qt::ToolTipRole)
            {
                QString toolTip = (parent->getProp(VariableName, VariableNameToolTipRole) == "") ? VariableName : parent->getProp(VariableName, role).toString();
                if (parent->errors.keys().contains(VariableName)) toolTip.append("\n Error: ").append(parent->errors[VariableName]);
                if (parent->warnings.keys().contains(VariableName)) toolTip.append("\n Warning: ").append(parent->warnings[VariableName]);
                return toolTip;
            }
            if (role == DescriptionCodeRole)
                return parent->getProp(VariableName, role);
            return QVariant();
        }
        if (col == 1)
            return parent->getProp(VariableName, role);
    }
    else if (items.count()) // multiple objects selected
    {
        if (index.row() >= rows()) return QVariant();
        if (role == saveIndex)		{
            save(index);
            return "index Saved";
        }
        if (role == loadIndexandInputMethodRole)		{
            return data(load(), InputMethodRole);
        }
        if (role == loadIndexandDefaultValuesListRole)		{
            return data(load(), DefaultValuesListRole);
        }
        if (role == loadIndexandVariableTypeRole)		{
            return data(load(), VariableTypeRole);
        }

        if (!index.isValid())
            return QVariant();
        int row = index.row();
        int col = index.column();
        multiValues<mPropList> mList;
        QList<mProp> objectTypes;
        foreach (Node* n , items)	objectTypes.append(n->ObjectType());
        QString VariableName = parent->getmList(objectTypes).VariableNames()[row];

        if (role == VariableNameRole) return VariableName;
        if (col == 0) {
            if (role == Qt::DisplayRole)			{
                if (parent->parent->applicationShortName == "GWA" && VariableName == "SubType")return "Distribution";

                else return VariableName;
            }
            if (role == Qt::ToolTipRole)
            {
                QString toolTip = (parent->getProp(VariableName, items, VariableNameToolTipRole) == "") ? VariableName : parent->getProp(VariableName, items, role).toString();
                if (parent->errors.keys().contains(VariableName)) toolTip.append("\n Error: ").append(parent->errors[VariableName]);
                if (parent->warnings.keys().contains(VariableName)) toolTip.append("\n Warning: ").append(parent->warnings[VariableName]);
                return toolTip;
            }
            if (role == DescriptionCodeRole)
                return parent->getProp(VariableName, items, role);
            return QVariant();
        }
        if (col == 1)
            return parent->getProp(VariableName, items, role);

    }
}
template<>
bool PropModel<Node>::setData(const QModelIndex & index, const QVariant & value, int role)
{
    if (index.row() >= rows()) return false;
    int row = index.row();
    int col = index.column();
    QModelIndex idx;
    if (role == loadIndex) {
        idx = load();
        role = Qt::EditRole;
        QString VariableName = idx.sibling(idx.row(), 0).data().toString();
        bool r = parent->setProp(VariableName, value, role);
        //qDebug() << QString("set %1 to %2.").arg(VariableName).arg(value.toString());
        if (r) emit dataChanged(idx, idx);
        return r;
    }
    QString VariableName = index.sibling(row, 0).data().toString();
    bool r = parent->setProp(VariableName, value, role);
    if (r)
    {
        parent->parent->log(QString("%1.%2 changed to %3").arg(parent->Name()).arg(VariableName).arg(XString(value.toStringList()).toStringUnit()));
        if (parent->errors.keys().contains(VariableName))
        {
            parent->errors.remove(VariableName);
            parent->parent->log(QString("Error %1.%2 resolved.").arg(parent->Name()).arg(VariableName));
            parent->update();
        }

        if (parent->warnings.keys().contains(VariableName))
        {
            parent->warnings.remove(VariableName);
            parent->parent->log(QString("Warning %1.%2 resolved.").arg(parent->Name()).arg(VariableName));
        }

        emit dataChanged(index, index);
    }
    return r;
}
template<>
QVariant PropModel<Edge>::data(const QModelIndex & index, int role) const
{
    if (index.row() >= rows()) return QVariant();
    if (role == saveIndex)
    {
        save(index);
        return "index Saved";
    }
    if (role == loadIndexandInputMethodRole)
    {
        return data(load(), InputMethodRole);
    }
    if (role == loadIndexandDefaultValuesListRole)
    {
        return data(load(), DefaultValuesListRole);
    }
    if (role == loadIndexandVariableTypeRole)
    {
        return data(load(), VariableTypeRole);
    }
    if (!index.isValid())
        return QVariant();
    //	int rindex = index.row();// +Offset;
    int row = index.row();
    int col = index.column();
    ////qDebug() << QString("data for (%1, %2) role %3").arg(row).arg(col).arg(role);
    QString VariableName = parent->getmList(parent->ObjectType()).getaskables().VariableNames()[row];
    ////qDebug() << QString("data for (%1, %2) role %3: %4").arg(row).arg(col).arg(role).arg(VariableName);
    if (role == VariableNameRole) return VariableName;
    if (col == 0) {
        if (role == Qt::DisplayRole) return VariableName;
        if (role == Qt::ToolTipRole)
        {
            QString toolTip = (parent->getProp(VariableName, VariableNameToolTipRole) == "") ? VariableName : parent->getProp(VariableName, role).toString();
            if (parent->errors.keys().contains(VariableName)) toolTip.append("\n Error: ").append(parent->errors[VariableName]);
            if (parent->warnings.keys().contains(VariableName)) toolTip.append("\n Warning: ").append(parent->warnings[VariableName]);
            return toolTip;
        }
        if (role == DescriptionCodeRole)
            return parent->getProp(VariableName, role);

        return QVariant();
    }
    if (col == 1)
        return parent->getProp(VariableName, role);
}
template<>
bool PropModel<Edge>::setData(const QModelIndex & index, const QVariant & value, int role)
{
    if (index.row() >= rows()) return false;
    int row = index.row();
    int col = index.column();
    QModelIndex idx;
    if (role == loadIndex) {
        idx = load();
        role = Qt::EditRole;
        QString VariableName = idx.sibling(idx.row(), 0).data().toString();
        bool r = parent->setProp(VariableName, value, role);
        //qDebug() << QString("set %1 to %2.").arg(VariableName).arg(value.toString());
        if (r) emit dataChanged(idx, idx);
        return r;
    }
    QString VariableName = index.sibling(row, 0).data().toString();
    bool r = parent->setProp(VariableName, value, role);
    if (r)
    {
        parent->parent->log(QString("%1.%2 changed to %3").arg(parent->Name()).arg(VariableName).arg(XString(value.toStringList()).toStringUnit()));
        if (parent->errors.keys().contains(VariableName))
        {
            parent->errors.remove(VariableName);
            parent->parent->log(QString("Error %1.%2 resolved.").arg(parent->Name()).arg(VariableName));
            parent->update();
        }

        if (parent->warnings.keys().contains(VariableName))
        {
            parent->warnings.remove(VariableName);
            parent->parent->log(QString("Warning %1.%2 resolved.").arg(parent->Name()).arg(VariableName));
        }

        emit dataChanged(index, index);
    }
    return r;
}

template<>
QVariant PropModel<Entity>::data(const QModelIndex & index, int role) const
{
    if (index.row() >= rows()) return QVariant();
    if (role == saveIndex)
    {
        save(index);
        return "index Saved";
    }
    if (role == loadIndexandInputMethodRole)
    {
        return data(load(), InputMethodRole);
    }
    if (role == loadIndexandDefaultValuesListRole)
    {
        return data(load(), DefaultValuesListRole);
    }
    if (role == loadIndexandVariableTypeRole)
    {
        return data(load(), VariableTypeRole);
    }
    if (role == allowableWordsRole)
    {
        QStringList words;
        words << parent->parent->EntityNames("Constituent") <<
            parent->parent->EntityNames("Reaction parameter") <<
            parent->parent->PhysicalCharacteristicsList <<
            parent->parent->functionList << "q";

        return words;
    }

    if (!index.isValid())
        return QVariant();
    int row = index.row();
    int col = index.column();
    ////qDebug() << QString("data for (%1, %2) role %3").arg(row).arg(col).arg(role);
    QString VariableName = parent->getmList(parent->ObjectType()).getaskables().VariableNames()[row];
    ////qDebug() << QString("data for (%1, %2) role %3: %4").arg(row).arg(col).arg(role).arg(VariableName);
    if (VariableName.contains("mum"))// && role == 6)
        int i = 0;
    if (role == VariableNameRole) return VariableName;
    if (col == 0) {
        if (role == Qt::DisplayRole)
        {
            if (parent->parent->applicationShortName == "GIFMod" && VariableName == "SubType" && parent->objectType.ObjectType == "Evapotranspiration")
                return "Model";
            if (parent->parent->applicationShortName == "GIFMod" && VariableName == "SubType" && parent->objectType.ObjectType == "Observation")
                return "Block/connector";
            if (parent->parent->applicationShortName == "GIFMod" && VariableName == "SubType" && parent->objectType.ObjectType == "Constituent")
                return "Settling model";
            return VariableName;
        }
        if (role == Qt::ToolTipRole)
        {
//			if (parent->getProp(VariableName, VariableTypeRole).toString().toLower().contains("datetime"))
//				return parent->getProp(VariableName);
            QString toolTip = (parent->getProp(VariableName, VariableNameToolTipRole) == "") ? VariableName : parent->getProp(VariableName, role).toString();
            if (parent->errors.keys().contains(VariableName)) toolTip.append("\n Error: ").append(parent->errors[VariableName]);
            if (parent->warnings.keys().contains(VariableName)) toolTip.append("\n Warning: ").append(parent->warnings[VariableName]);
            return toolTip;
        }
        if (role == DescriptionCodeRole)
            return parent->getProp(VariableName, role);


        return QVariant();
    }
//	if (role == fullFileNameRole)
//		//qDebug() << QString("data for (%1, %2) role %3").arg(row).arg(col).arg(role);
    if (col == 1)
        return parent->getProp(VariableName, role);

}
template<>
bool PropModel<Entity>::setData(const QModelIndex & index, const QVariant & value, int role)
{
    if (index.row() >= rows()) return false;
    int row = index.row();
    int col = index.column();

    QString VariableName;
    if (role == loadIndex) {
        QModelIndex idx = load();
        role = Qt::EditRole;
        VariableName = idx.sibling(idx.row(), 0).data().toString();
    }
    else
        VariableName = index.sibling(row, 0).data().toString();

    bool r = parent->setProp(VariableName, value, role);
    //qDebug() << QString("set %1 to %2.").arg(VariableName).arg(value.toString());
    if (r)
    {
        if (value.toStringList().size())
            parent->parent->log(QString("%1.%2 changed to %3").arg(parent->Name()).arg(VariableName).arg(XString(value.toStringList()).toStringUnit()));
        else
            parent->parent->log(QString("%1.%2 changed to %3").arg(parent->Name()).arg(VariableName).arg(value.toString()));

        if (parent->errors.keys().contains(VariableName))
        {
            parent->errors.remove(VariableName);
            parent->parent->log(QString("Error %1.%2 resolved.").arg(parent->Name()).arg(VariableName));
        }
        if (parent->warnings.keys().contains(VariableName))
        {
            parent->warnings.remove(VariableName);
            parent->parent->log(QString("Warning %1.%2 resolved.").arg(parent->Name()).arg(VariableName));
        }
        emit dataChanged(index, index);
    }
    return r;


}
