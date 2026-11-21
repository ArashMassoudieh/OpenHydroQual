/*
 * OpenHydroQual - Environmental Modeling Platform
 * Copyright (C) 2025 Arash Massoudieh
 *
 * This file is part of OpenHydroQual.
 *
 * OpenHydroQual is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * If you use this file in a commercial product, you must purchase a
 * commercial license. Contact arash.massoudieh@enviroinformatics.co for details.
 */

#include "arrayconnector.h"
#include "mainwindow.h"
#include "XString.h"
#include <QTabWidget>
#include <QListWidget>
#include <QComboBox>
#include <QLabel>
#include <QFormLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLineEdit>
#include <QScrollArea>
#include <QRegularExpression>
#include <set>
#include <vector>

ArrayConnector::ArrayConnector(MainWindow *parent) : QDialog(parent), mainwindow(parent)
{
    setWindowTitle(tr("Connect Two Arrays"));
    resize(760, 560);
    QVBoxLayout *main = new QVBoxLayout(this);
    tabs = new QTabWidget(this);
    main->addWidget(tabs);

    // ---- tab 1: the two arrays, the link type and the direction
    QWidget *pick = new QWidget(this);
    QVBoxLayout *pl = new QVBoxLayout(pick);
    pl->addWidget(new QLabel(tr("Check the two arrays to connect (arrays of the same size; blocks named Name(i:j)):"), pick));
    arrayList = new QListWidget(pick);
    pl->addWidget(arrayList, 1);
    QFormLayout *fl = new QFormLayout();
    linkType = new QComboBox(pick);
    direction = new QComboBox(pick);
    fl->addRow(tr("Link type:"), linkType);
    fl->addRow(tr("Direction:"), direction);
    pl->addLayout(fl);
    status = new QLabel(pick);
    status->setWordWrap(true);
    status->setMinimumHeight(status->fontMetrics().lineSpacing() * 2 + 6);   // the message may take two lines
    pl->addWidget(status);
    tabs->addTab(pick, tr("Arrays and link type"));

    // ---- tab 2: the link properties asked from the user
    QScrollArea *scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    QWidget *props = new QWidget(scroll);
    propertyForm = new QFormLayout(props);
    scroll->setWidget(props);
    tabs->addTab(scroll, tr("Link properties"));

    QHBoxLayout *buttons = new QHBoxLayout();
    buttons->addStretch(1);
    QPushButton *cancel = new QPushButton(tr("Cancel"), this);
    okButton = new QPushButton(tr("OK"), this);
    cancel->setAutoDefault(false); okButton->setAutoDefault(false);
    buttons->addWidget(cancel);
    buttons->addWidget(okButton);
    main->addLayout(buttons);

    FindArrays();
    for (const ArrayInfo &a : arrays)
    {
        QString type = QString::fromStdString(a.blocktype);
        if (system()->GetModel(a.blocktype)) type = QString::fromStdString(system()->GetModel(a.blocktype)->Description());
        QListWidgetItem *item = new QListWidgetItem(QString("%1   (%2 x %3, %4)").arg(a.name).arg(a.nx).arg(a.ny).arg(a.mixed ? tr("mixed block types") : type), arrayList);
        item->setData(Qt::UserRole, a.name);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(Qt::Unchecked);
    }
    for (unsigned int i = 0; i < system()->GetAllLinkTypes().size(); i++)
    {
        const std::string t = system()->GetAllLinkTypes()[i];
        linkType->addItem(QString::fromStdString(system()->GetModel(t)->Description()), QString::fromStdString(t));
    }

    connect(arrayList, &QListWidget::itemChanged, this, [this](QListWidgetItem *) { UpdateDirections(); QString m; Validate(&m); });
    connect(linkType, &QComboBox::currentIndexChanged, this, [this](int) { PopulateProperties(); QString m; Validate(&m); });
    connect(direction, &QComboBox::currentIndexChanged, this, [this](int) { QString m; Validate(&m); });
    connect(cancel, &QPushButton::clicked, this, &QDialog::reject);
    connect(okButton, &QPushButton::clicked, this, [this]() { QString m; if (Validate(&m)) { Connect(); accept(); } });

    UpdateDirections();
    PopulateProperties();
    QString m; Validate(&m);
    if (arrays.size() < 2)
        status->setText(tr("The model needs at least two arrays of the same size (made with Add Objects > Create 2D Array)."));
}

System *ArrayConnector::system()
{
    return mainwindow ? mainwindow->GetSystem() : nullptr;
}

// Arrays are recognized by their block names: Name(i:j); an array counts when all nx x ny cells exist.
void ArrayConnector::FindArrays()
{
    static const QRegularExpression re("^(.*)\\((\\d+):(\\d+)\\)$");
    QMap<QString, int> cells;
    QMap<QString, std::set<std::string>> types;
    for (const std::string &name : system()->GetAllBlockNames())
    {
        QRegularExpressionMatch m = re.match(QString::fromStdString(name));
        if (!m.hasMatch()) continue;
        const QString base = m.captured(1);
        ArrayInfo &a = arrays[base];
        a.name = base;
        a.nx = qMax(a.nx, m.captured(2).toInt());
        a.ny = qMax(a.ny, m.captured(3).toInt());
        cells[base]++;
        if (system()->block(name)) types[base].insert(system()->block(name)->GetType());
    }
    for (auto it = arrays.begin(); it != arrays.end();)
    {
        if (cells[it.key()] != it->nx * it->ny) { it = arrays.erase(it); continue; }
        it->blocktype = types[it.key()].empty() ? "" : *types[it.key()].begin();
        it->mixed = types[it.key()].size() > 1;
        ++it;
    }
}

QStringList ArrayConnector::CheckedArrays() const
{
    QStringList out;
    for (int i = 0; i < arrayList->count(); i++)
        if (arrayList->item(i)->checkState() == Qt::Checked) out << arrayList->item(i)->data(Qt::UserRole).toString();
    return out;
}

void ArrayConnector::UpdateDirections()
{
    const QStringList c = CheckedArrays();
    const int keep = direction->currentIndex();
    direction->blockSignals(true);
    direction->clear();
    if (c.size() == 2)
    {
        direction->addItem(c[0] + "  →  " + c[1]);
        direction->addItem(c[1] + "  →  " + c[0]);
        direction->setCurrentIndex(keep == 1 ? 1 : 0);
    }
    direction->blockSignals(false);
}

// One row per link property asked from the user (except the name, which is made from the two blocks).
void ArrayConnector::PopulateProperties()
{
    while (propertyForm->rowCount() > 0) propertyForm->removeRow(0);
    editors.clear();
    const std::string type = linkType->currentData().toString().toStdString();
    QuanSet *qs = system()->GetModel(type);
    if (!qs) return;
    for (unsigned int i = 0; i < qs->AskableSize(); i++)
    {
        Quan *q = qs->GetVarAskable(i);
        if (!q || q->GetName() == "name") continue;
        const QString delegate = QString::fromStdString(q->Delegate());
        if (delegate.contains("Browser") || delegate.contains("ComboBox") || delegate.contains("ListBox")
            || delegate.contains("expressionEditor") || delegate.contains("CheckBox") || delegate.contains("Memo")) continue;
        PropertyEditor e;
        e.delegate = q->Delegate();
        QWidget *row = new QWidget();
        QHBoxLayout *h = new QHBoxLayout(row);
        h->setContentsMargins(0, 0, 0, 0);
        e.value = new QLineEdit(QString::fromStdString(q->Default()), row);
        h->addWidget(e.value, 1);
        if (delegate.contains("UnitBox"))
        {
            e.unit = new QComboBox(row);
            const QStringList units = QString::fromStdString(q->Units()).split(';', Qt::SkipEmptyParts);
            for (const QString &u : units) e.unit->addItem(XString::reform(u), u);
            const int k = e.unit->findData(QString::fromStdString(q->Unit()));
            if (k >= 0) e.unit->setCurrentIndex(k);
            h->addWidget(e.unit);
        }
        QLabel *label = new QLabel(QString::fromStdString(q->Description()));
        label->setToolTip(QString::fromStdString(q->GetName()));
        propertyForm->addRow(label, row);
        editors[QString::fromStdString(q->GetName())] = e;
    }
    if (editors.isEmpty())
        propertyForm->addRow(new QLabel(tr("This link type has no properties to set.")));
}

// Quantities the link type's expressions read from its source (suffix .s) or end (.e) block that the block type
// does not have.
QStringList ArrayConnector::MissingVariables(const std::string &linktype, const std::string &blocktype, const QString &suffix)
{
    QStringList missing;
    QuanSet *bq = system()->GetModel(blocktype);
    Link link;
    if (!bq || !link.SetQuantities(system()->GetMetaModel(), linktype)) return missing;
    const std::vector<std::string> needed = suffix == "s" ? link.GetAllRequieredStartingBlockProperties()
                                                          : link.GetAllRequieredDestinationBlockProperties();
    for (const std::string &p : needed)
        if (bq->Count(p) == 0 && !missing.contains(QString::fromStdString(p))) missing << QString::fromStdString(p);
    return missing;
}

bool ArrayConnector::Validate(QString *message)
{
    const QStringList c = CheckedArrays();
    QString msg;
    bool ok = false;
    if (c.size() != 2)
        msg = tr("Check two arrays (%1 checked).").arg(c.size());
    else if (arrays[c[0]].nx != arrays[c[1]].nx || arrays[c[0]].ny != arrays[c[1]].ny)
        msg = tr("The two arrays have different sizes: %1 is %2 x %3, %4 is %5 x %6.")
                  .arg(c[0]).arg(arrays[c[0]].nx).arg(arrays[c[0]].ny).arg(c[1]).arg(arrays[c[1]].nx).arg(arrays[c[1]].ny);
    else if (arrays[c[0]].mixed || arrays[c[1]].mixed)
        msg = tr("An array has blocks of different types; its cells cannot all be checked against the link type.");
    else
    {
        const bool flip = direction->currentIndex() == 1;
        const ArrayInfo &src = arrays[flip ? c[1] : c[0]], &dst = arrays[flip ? c[0] : c[1]];
        const std::string type = linkType->currentData().toString().toStdString();
        const QStringList ms = MissingVariables(type, src.blocktype, "s"), me = MissingVariables(type, dst.blocktype, "e");
        if (!ms.isEmpty() || !me.isEmpty())
        {
            msg = tr("'%1' cannot connect these arrays:").arg(linkType->currentText());
            if (!ms.isEmpty()) msg += tr(" the source blocks (%1) lack %2.").arg(src.name, ms.join(", "));
            if (!me.isEmpty()) msg += tr(" the end blocks (%1) lack %2.").arg(dst.name, me.join(", "));
        }
        else
        {
            msg = tr("OK: %1 links, %2(i:j) → %3(i:j), type '%4'.").arg(src.nx * src.ny).arg(src.name, dst.name, linkType->currentText());
            ok = true;
        }
    }
    status->setText(msg);
    status->setStyleSheet(ok ? "color: #1b6e2b;" : "color: #b03a2e;");
    okButton->setEnabled(ok);
    if (message) *message = msg;
    return ok;
}

void ArrayConnector::Connect()
{
    const QStringList c = CheckedArrays();
    const bool flip = direction->currentIndex() == 1;
    const ArrayInfo &src = arrays[flip ? c[1] : c[0]], &dst = arrays[flip ? c[0] : c[1]];
    const std::string type = linkType->currentData().toString().toStdString();
    mainwindow->undoData.SetActiveSystem(system());
    int made = 0;
    for (int i = 1; i <= src.nx; i++)
        for (int j = 1; j <= src.ny; j++)
        {
            const QString cell = QString("(%1:%2)").arg(i).arg(j);
            const std::string s = (src.name + cell).toStdString(), e = (dst.name + cell).toStdString();
            const std::string name = s + " - " + e;            // the pattern of a hand-drawn link
            if (system()->object(name) != nullptr) continue;
            Link link;
            if (!link.SetQuantities(system()->GetMetaModel(), type))
            {
                mainwindow->LogError(QString::fromStdString(link.lasterror()));
                return;
            }
            link.SetType(type);
            link.SetName(name);
            Object *obj = system()->AddLink(link, s, e) ? system()->object(name) : nullptr;
            if (!obj)
            {
                mainwindow->LogError("Link '" + QString::fromStdString(name) + "' was not added: " + QString::fromStdString(system()->lasterror()));
                continue;
            }
            obj->SetName(name);
            system()->AddAllConstituentRelateProperties(system()->link(name));
            for (auto it = editors.cbegin(); it != editors.cend(); ++it)
            {
                const std::string q = it.key().toStdString();
                const QString text = it->value->text().trimmed();
                if (text.isEmpty() || !obj->Variable(q)) continue;
                if (it->unit)
                {
                    const QString unit = it->unit->currentData().toString();
                    obj->Variable(q)->SetUnit(unit.toStdString());
                    obj->SetProperty(q, aquiutils::numbertostring(text.toDouble() * XString::coefficient(unit)), true);
                }
                else
                    obj->SetProperty(q, text.toStdString(), true);
            }
            obj->AssignRandomPrimaryKey();
            system()->SetVariableParents();
            mainwindow->LogAddDelete("Link '" + QString::fromStdString(name) + "' was added!");
            made++;
        }
    mainwindow->RefreshTreeView();
    mainwindow->undoData.AppendAfterActive(system());
    mainwindow->RecreateGraphicItemsFromSystem(false);
    mainwindow->SetModified();
    mainwindow->LogAddDelete(QString("Connect Two Arrays: %1 links of type '%2' added").arg(made).arg(QString::fromStdString(type)));
}
