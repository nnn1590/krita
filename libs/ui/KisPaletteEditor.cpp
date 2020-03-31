/*
 *  Copyright (c) 2018 Michael Zhou <simeirxh@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include <QHash>
#include <QString>
#include <QScopedPointer>
#include <QPointer>
#include <QFormLayout>
#include <QCheckBox>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QSpinBox>
#include <QComboBox>
#include <QMessageBox>

#include <KoDialog.h>
#include <KoFileDialog.h>
#include <KoColorSet.h>
#include <KisSwatchGroup.h>
#include <kis_signal_compressor.h>
#include <KisViewManager.h>
#include <KisDocument.h>
#include <KoResourceServer.h>
#include <KoResourceServerProvider.h>
#include <KisPaletteModel.h>
#include <kis_color_button.h>

#include "KisPaletteEditor.h"
#include <KisGlobalResourcesInterface.h>

struct KisPaletteEditor::PaletteInfo {
    QString name;
    QString filename;
    int columnCount;
    bool isReadOnly;
    bool isGlobal;
    QHash<QString, KisSwatchGroup> groups;
};

struct KisPaletteEditor::Private
{
    bool isNameModified {false};
    bool isColumnCountModified {false};
    QSet<QString> modifiedGroupNames; // key is original group name
    QSet<QString> newGroupNames;
    QSet<QString> keepColorGroups;
    QSet<QString> pathsToRemove;
    QString groupBeingRenamed;
    QPointer<KisPaletteModel> model;
    QPointer<KisViewManager> view;
    PaletteInfo modified;
    QPointer<KoDialog> query;
    KoResourceServer<KoColorSet> *rServer {0};

    QPalette normalPalette;
    QPalette warnPalette;
};

KisPaletteEditor::KisPaletteEditor(QObject *parent)
    : QObject(parent)
    , m_d(new Private)
{
    m_d->rServer = KoResourceServerProvider::instance()->paletteServer();
    m_d->warnPalette.setColor(QPalette::Text, Qt::red);
}

KisPaletteEditor::~KisPaletteEditor()
{ }

void KisPaletteEditor::setPaletteModel(KisPaletteModel *model)
{
    if (!model) { return; }
    m_d->model = model;
    slotPaletteChanged();
    connect(model, SIGNAL(sigPaletteChanged()), SLOT(slotPaletteChanged()));
    connect(model, SIGNAL(sigPaletteModified()), SLOT(slotSetDocumentModified()));
}

void KisPaletteEditor::setView(KisViewManager *view)
{
    m_d->view = view;
}

void KisPaletteEditor::addPalette()
{
    if (!m_d->view) { return; }
    if (!m_d->view->document()) { return; }

    KoColorSetSP colorSet(new KoColorSet());

    KoDialog dlg;
    QFormLayout layout;
    dlg.mainWidget()->setLayout(&layout);
    QLabel lbl(i18nc("Label for line edit to set a palette name.","Name"));
    QLineEdit le(i18nc("Default name for a new palette","New Palette"));
    layout.addRow(&lbl, &le);

    QString saveLocation = m_d->rServer->saveLocation();


    QCheckBox chkSaveInDocument(i18n("Save Palette in the Current Document"));
    chkSaveInDocument.setChecked(false);
    layout.addRow(&chkSaveInDocument);

    if (dlg.exec() != QDialog::Accepted) { return; }

    QString name = le.text();
    colorSet->setPaletteType(KoColorSet::KPL);
    colorSet->setIsEditable(true);
    colorSet->setValid(true);
    colorSet->setName(name);
    colorSet->setFilename(name.split(" ").join("_")+colorSet->defaultFileExtension());

    QString resourceLocation = "";
    if (chkSaveInDocument.isChecked()) {
        resourceLocation = m_d->view->document()->uniqueID();
    }
    m_d->rServer->resourceModel()->addResource(colorSet, resourceLocation);
}

void KisPaletteEditor::importPalette()
{
    KoFileDialog dialog(0, KoFileDialog::OpenFile, "Open Palette");

    dialog.setDefaultDir(QDir::homePath());
    dialog.setMimeTypeFilters(QStringList() << "krita/x-colorset" << "application/x-gimp-color-palette");

    QString filename = dialog.filename();
    if (filename.isEmpty()) { return; }

    QMessageBox messageBox;
    messageBox.setText(i18n("Do you want to store this palette in your current image?"));
    messageBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    QString storageLocation = "";
    if (messageBox.exec() == QMessageBox::Yes) {
        storageLocation = m_d->view->document()->uniqueID();
    }

    KoColorSetSP colorSet(new KoColorSet(filename));
    colorSet->load(KisGlobalResourcesInterface::instance());
    m_d->rServer->resourceModel()->addResource(colorSet, storageLocation);
}

void KisPaletteEditor::removePalette(KoColorSetSP cs)
{
    if (!m_d->view) { return; }
    if (!m_d->view->document()) { return; }
    if (!cs || !cs->isEditable()) {
        return;
    }
    m_d->rServer->removeResourceFromServer(cs);
}

int KisPaletteEditor::rowNumberOfGroup(const QString &oriName) const
{
    if (!m_d->modified.groups.contains(oriName)) { return 0; }
    return m_d->modified.groups[oriName].rowCount();
}

bool KisPaletteEditor::duplicateExistsGroupName(const QString &name) const
{
    if (name == m_d->groupBeingRenamed) { return false; }
    Q_FOREACH (const KisSwatchGroup &g, m_d->modified.groups.values()) {
        if (name == g.name()) { return true; }
    }
    return false;
}

bool KisPaletteEditor::duplicateExistsOriginalGroupName(const QString &name) const
{
    return m_d->modified.groups.contains(name);
}

QString KisPaletteEditor::oldNameFromNewName(const QString &newName) const
{
    Q_FOREACH (const QString &oldGroupName, m_d->modified.groups.keys()) {
        if (m_d->modified.groups[oldGroupName].name() == newName) {
            return oldGroupName;
        }
    }
    return QString();
}

void KisPaletteEditor::rename(const QString &newName)
{
    if (newName.isEmpty()) { return; }
    m_d->isNameModified = true;
    m_d->modified.name = newName;
}

void KisPaletteEditor::changeColCount(int newCount)
{
    m_d->isColumnCountModified = true;
    m_d->modified.columnCount = newCount;
}

QString KisPaletteEditor::addGroup()
{
    KoDialog dlg;
    m_d->query = &dlg;

    QVBoxLayout layout(&dlg);
    dlg.mainWidget()->setLayout(&layout);

    QLabel lblName(i18n("Name"), &dlg);
    layout.addWidget(&lblName);
    QLineEdit leName(&dlg);
    leName.setText(newGroupName());
    connect(&leName, SIGNAL(textChanged(QString)), SLOT(slotGroupNameChanged(QString)));
    layout.addWidget(&leName);
    QLabel lblRowCount(i18n("Row count"), &dlg);
    layout.addWidget(&lblRowCount);
    QSpinBox spxRow(&dlg);
    spxRow.setValue(20);
    layout.addWidget(&spxRow);

    if (dlg.exec() != QDialog::Accepted) { return QString(); }
    if (duplicateExistsGroupName(leName.text())) { return QString(); }

    QString realName = leName.text();
    QString name = realName;
    if (duplicateExistsOriginalGroupName(name)) {
        name = newGroupName();
    }
    m_d->modified.groups[name] = KisSwatchGroup();
    KisSwatchGroup &newGroup = m_d->modified.groups[name];
    newGroup.setName(realName);
    m_d->newGroupNames.insert(name);
    newGroup.setRowCount(spxRow.value());
    return realName;
}

bool KisPaletteEditor::removeGroup(const QString &name)
{
    KoDialog window;
    window.setWindowTitle(i18nc("@title:window", "Removing Group"));
    QFormLayout editableItems(&window);
    QCheckBox chkKeep(&window);
    window.mainWidget()->setLayout(&editableItems);
    editableItems.addRow(i18nc("Shows up when deleting a swatch group", "Keep the Colors"), &chkKeep);
    if (window.exec() != KoDialog::Accepted) { return false; }

    m_d->modified.groups.remove(name);
    m_d->newGroupNames.remove(name);
    if (chkKeep.isChecked()) {
        m_d->keepColorGroups.insert(name);
    }
    return true;
}

QString KisPaletteEditor::renameGroup(const QString &oldName)
{
    if (oldName.isEmpty() || oldName == KoColorSet::GLOBAL_GROUP_NAME) { return QString(); }

    KoDialog dlg;
    m_d->query = &dlg;
    m_d->groupBeingRenamed = m_d->modified.groups[oldName].name();

    QFormLayout form(&dlg);
    dlg.mainWidget()->setLayout(&form);

    QLineEdit leNewName;
    connect(&leNewName, SIGNAL(textChanged(QString)), SLOT(slotGroupNameChanged(QString)));
    leNewName.setText(m_d->modified.groups[oldName].name());

    form.addRow(i18nc("Renaming swatch group", "New name"), &leNewName);

    if (dlg.exec() != KoDialog::Accepted) { return QString(); }
    if (leNewName.text().isEmpty()) { return QString(); }
    if (duplicateExistsGroupName(leNewName.text())) { return QString(); }

    m_d->modified.groups[oldName].setName(leNewName.text());
    m_d->modifiedGroupNames.insert(oldName);

    return leNewName.text();
}

void KisPaletteEditor::slotGroupNameChanged(const QString &newName)
{
    QLineEdit *leGroupName = qobject_cast<QLineEdit*>(sender());
    if (duplicateExistsGroupName(newName) || newName == QString()) {
        leGroupName->setPalette(m_d->warnPalette);
        if (m_d->query->button(KoDialog::Ok)) {
            m_d->query->button(KoDialog::Ok)->setEnabled(false);
        }
        return;
    }
    leGroupName->setPalette(m_d->normalPalette);
    if (m_d->query->button(KoDialog::Ok)) {
        m_d->query->button(KoDialog::Ok)->setEnabled(true);
    }
}

void KisPaletteEditor::changeGroupRowCount(const QString &name, int newRowCount)
{
    if (!m_d->modified.groups.contains(name)) { return; }
    m_d->modified.groups[name].setRowCount(newRowCount);
    m_d->modifiedGroupNames.insert(name);
}

void KisPaletteEditor::setGlobal(bool isGlobal)
{
    m_d->modified.isGlobal = isGlobal;
}

void KisPaletteEditor::setEntry(const KoColor &color, const QModelIndex &index)
{
    Q_ASSERT(m_d->model);
    if (!m_d->model->colorSet()->isEditable()) { return; }
    if (!m_d->view) { return; }
    if (!m_d->view->document()) { return; }
    KisSwatch c = KisSwatch(color);
    c.setId(QString::number(m_d->model->colorSet()->colorCount() + 1));
    c.setName(i18nc("Default name for a color swatch","Color %1", QString::number(m_d->model->colorSet()->colorCount()+1)));
    m_d->model->setEntry(c, index);
}

void KisPaletteEditor::slotSetDocumentModified()
{
    m_d->rServer->resourceModel()->addResource(m_d->model->colorSet(), m_d->model->colorSet()->storageLocation());
    m_d->view->document()->setModified(true);
}

void KisPaletteEditor::removeEntry(const QModelIndex &index)
{
    Q_ASSERT(m_d->model);
    if (!m_d->model->colorSet()->isEditable()) { return; }
    if (!m_d->view) { return; }
    if (!m_d->view->document()) { return; }
    if (qvariant_cast<bool>(index.data(KisPaletteModel::IsGroupNameRole))) {
        removeGroup(qvariant_cast<QString>(index.data(KisPaletteModel::GroupNameRole)));
    } else {
        m_d->model->removeEntry(index, false);
    }
    updatePalette();
}

void KisPaletteEditor::modifyEntry(const QModelIndex &index)
{
    if (!m_d->model->colorSet()->isEditable()) { return; }
    if (!m_d->view) { return; }
    if (!m_d->view->document()) { return; }

    KoDialog dlg;
    dlg.setCaption(i18nc("@title:window", "Add a Color"));
    QFormLayout *editableItems = new QFormLayout(&dlg);
    dlg.mainWidget()->setLayout(editableItems);

    QString groupName = qvariant_cast<QString>(index.data(Qt::DisplayRole));
    if (qvariant_cast<bool>(index.data(KisPaletteModel::IsGroupNameRole))) {
        renameGroup(groupName);
        updatePalette();
    }
    else {

        QLineEdit *lnIDName = new QLineEdit(&dlg);
        QLineEdit *lnGroupName = new QLineEdit(&dlg);
        KisColorButton *bnColor = new KisColorButton(&dlg);
        QCheckBox *chkSpot = new QCheckBox(&dlg);
        chkSpot->setToolTip(i18nc("@info:tooltip", "A spot color is a color that the printer is able to print without mixing the paints it has available to it. The opposite is called a process color."));

        KisSwatch entry = m_d->model->getEntry(index);

        editableItems->addRow(i18n("ID"), lnIDName);
        editableItems->addRow(i18nc("Name for a swatch group", "Swatch group name"), lnGroupName);
        editableItems->addRow(i18n("Color"), bnColor);
        editableItems->addRow(i18n("Spot color"), chkSpot);

        lnGroupName->setText(entry.name());
        lnIDName->setText(entry.id());
        bnColor->setColor(entry.color());
        chkSpot->setChecked(entry.spotColor());

        if (dlg.exec() == KoDialog::Accepted) {
            entry.setName(lnGroupName->text());
            entry.setId(lnIDName->text());
            entry.setColor(bnColor->color());
            entry.setSpotColor(chkSpot->isChecked());
            m_d->model->setEntry(entry, index);
        }
    }
}

void KisPaletteEditor::addEntry(const KoColor &color)
{
    Q_ASSERT(m_d->model);
    if (!m_d->view) { return; }
    if (!m_d->view->document()) { return; }
    if (!m_d->model->colorSet()->isEditable()) { return; }
    KoDialog window;
    window.setWindowTitle(i18nc("@title:window", "Add a new Colorset Entry"));
    QFormLayout editableItems(&window);
    window.mainWidget()->setLayout(&editableItems);
    QComboBox cmbGroups(&window);
    cmbGroups.addItems(m_d->model->colorSet()->getGroupNames());
    QLineEdit lnIDName(&window);
    QLineEdit lnName(&window);
    KisColorButton bnColor(&window);
    QCheckBox chkSpot(&window);
    chkSpot.setToolTip(i18nc("@info:tooltip", "A spot color is a color that the printer is able to print without mixing the paints it has available to it. The opposite is called a process color."));
    editableItems.addRow(i18n("Group"), &cmbGroups);
    editableItems.addRow(i18n("ID"), &lnIDName);
    editableItems.addRow(i18n("Name"), &lnName);
    editableItems.addRow(i18n("Color"), &bnColor);
    editableItems.addRow(i18nc("Spot color", "Spot"), &chkSpot);
    cmbGroups.setCurrentIndex(0);
    lnName.setText(i18nc("Default name for a color swatch","Color %1", QString::number(m_d->model->colorSet()->colorCount()+1)));
    lnIDName.setText(QString::number(m_d->model->colorSet()->colorCount() + 1));
    bnColor.setColor(color);
    chkSpot.setChecked(false);

    if (window.exec() != KoDialog::Accepted) { return; }

    QString groupName = cmbGroups.currentText();

    KisSwatch newEntry;
    newEntry.setColor(bnColor.color());
    newEntry.setName(lnName.text());
    newEntry.setId(lnIDName.text());
    newEntry.setSpotColor(chkSpot.isChecked());
    m_d->model->addEntry(newEntry, groupName);
    m_d->rServer->resourceModel()->updateResource(m_d->model->colorSet());
    m_d->modifiedGroupNames.insert(groupName);
    m_d->modified.groups[groupName].addEntry(newEntry);
}

void KisPaletteEditor::updatePalette()
{
    Q_ASSERT(m_d->model);
    Q_ASSERT(m_d->model->colorSet());
    if (!m_d->model->colorSet()->isEditable()) { return; }
    if (!m_d->view) { return; }
    if (!m_d->view->document()) { return; }
    KoColorSetSP palette = m_d->model->colorSet();
    PaletteInfo &modified = m_d->modified;

    if (m_d->isColumnCountModified) {
        palette->setColumnCount(modified.columnCount);
    }
    if (m_d->isNameModified) {
        m_d->rServer->resourceModel()->renameResource(palette, modified.name);
    }
    QString resourceLocation = m_d->model->colorSet()->storageLocation();
    if (m_d->modified.isGlobal) {
        resourceLocation = QString();
    } else {
        resourceLocation = m_d->view->document()->uniqueID();
    }
    Q_FOREACH (const QString &groupName, palette->getGroupNames()) {
        if (!modified.groups.contains(groupName)) {
            m_d->model->removeGroup(groupName, m_d->keepColorGroups.contains(groupName));
        }
    }
    m_d->keepColorGroups.clear();
    Q_FOREACH (const QString &groupName, palette->getGroupNames()) {
        if (m_d->modifiedGroupNames.contains(groupName)) {
            m_d->model->setRowNumber(groupName, modified.groups[groupName].rowCount());
            if (groupName != modified.groups[groupName].name()) {
                m_d->model->renameGroup(groupName, modified.groups[groupName].name());
                modified.groups[modified.groups[groupName].name()] = modified.groups[groupName];
                modified.groups.remove(groupName);
            }
        }
    }
    m_d->modifiedGroupNames.clear();
    Q_FOREACH (const QString &newGroupName, m_d->newGroupNames) {
        m_d->model->addGroup(modified.groups[newGroupName]);
    }
    m_d->newGroupNames.clear();
    qDebug() << "updating palette from updatePalette" << m_d->model->colorSet()->filename() << resourceLocation;
    m_d->rServer->resourceModel()->addResource(m_d->model->colorSet(), resourceLocation);
}

void KisPaletteEditor::slotPaletteChanged()
{
    Q_ASSERT(m_d->model);
    if (!m_d->model->colorSet()) { return; }
    KoColorSetSP palette = m_d->model->colorSet();
    m_d->modified.groups.clear();
    m_d->keepColorGroups.clear();
    m_d->newGroupNames.clear();
    m_d->modifiedGroupNames.clear();

    m_d->modified.name = palette->name();
    //hack alert! needs better solution.
    m_d->modified.isGlobal = !palette->storageLocation().contains("/");
    m_d->modified.columnCount = palette->columnCount();
    m_d->modified.isReadOnly = !palette->isEditable();

    Q_FOREACH (const QString &groupName, palette->getGroupNames()) {
        KisSwatchGroup *cs = palette->getGroup(groupName);
        m_d->modified.groups[groupName] = KisSwatchGroup(*cs);
    }
}

QString KisPaletteEditor::relativePathFromSaveLocation() const
{
    return filenameFromPath(m_d->modified.filename);
}

QString KisPaletteEditor::newGroupName() const
{
    int i = 1;
    QString groupname = i18nc("Default new group name", "New Group %1", QString::number(i));
    while (m_d->modified.groups.contains(groupname)) {
        i++;
        groupname = i18nc("Default new group name", "New Group %1", QString::number(i));
    }
    return groupname;
}

QString KisPaletteEditor::filenameFromPath(const QString &path) const
{
    return QDir::fromNativeSeparators(path).section('/', -1, -1);
}
