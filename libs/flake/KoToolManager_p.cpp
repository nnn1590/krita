/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2006 Thomas Zander <zander@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "KoToolManager_p.h"

#include <KoShapeManager.h>
#include <KoSelection.h>
#include <KoToolBase.h>
#include <KoToolFactoryBase.h>
#include "kis_action_registry.h"

static int newUniqueToolHelperId()
{
    static int idCounter = 0;
    return ++idCounter;
}

/*    ************ ToolHelper **********
 * This class wrangles the tool factory, toolbox button and switch-tool action
 * for a single tool. It assumes the  will continue to live once it is created.
 * (Hiding the toolbox is OK.)
 */

ToolHelper::ToolHelper(KoToolFactoryBase *tool)
    : m_toolFactory(tool),
      m_uniqueId(newUniqueToolHelperId()),
      m_hasCustomShortcut(false),
      m_toolAction(0)
{
    // TODO: how to get an existing custom shortcut in the beginning here?
    // Once the first ShortcutToolAction is added to the actionCollection,
    // it will get any custom shortcut set by the actionCollection and
    // by that trigger shortcutToolActionUpdated().
    // But until then shortcut() will report a wrong shortcut and e.g. show
    // that in the tooltips of the KoToolBox.
}

KoToolAction *ToolHelper::toolAction()
{
    // create lazily
    if (!m_toolAction) {
        m_toolAction = new KoToolAction(this);
    }
    return m_toolAction;
}

QString ToolHelper::id() const
{
    return m_toolFactory->id();
}

QString ToolHelper::activationShapeId() const
{
    return m_toolFactory->activationShapeId();
}

QString ToolHelper::iconName() const
{
    return m_toolFactory->iconName();
}

QString ToolHelper::text() const
{
    // TODO: add text property to KoToolFactoryBase
    return m_toolFactory->toolTip();
}

QString ToolHelper::iconText() const
{
    // TODO: add text iconText to KoToolFactoryBase
    return m_toolFactory->toolTip();
}

QString ToolHelper::toolTip() const
{
    return m_toolFactory->toolTip();
}

void ToolHelper::activate()
{
    emit toolActivated(this);
}

void ToolHelper::shortcutToolActionUpdated()
{
    ShortcutToolAction *action = static_cast<ShortcutToolAction*>(sender());
    // check if shortcut changed
    const QKeySequence actionShortcut = action->shortcut();
    const QKeySequence currentShortcut = shortcut();
    if (actionShortcut != currentShortcut) {
        m_hasCustomShortcut = true;
        m_customShortcut = actionShortcut;
        if (m_toolAction) {
            emit m_toolAction->changed();
        }
        // no need to forward the new shortcut to the other ShortcutToolAction objects,
        // they are synchronized behind the scenes
        // Thus they will also trigger this method, but due to them having
        // the same shortcut not result in any further action.
    }
}

KoToolBase *ToolHelper::createTool(KoCanvasBase *canvas) const
{
    KoToolBase *tool = m_toolFactory->createTool(canvas);
    if (tool) {
        tool->setToolId(id());
    }
    return tool;
}

ShortcutToolAction* ToolHelper::createShortcutToolAction(QObject *parent)
{
    ShortcutToolAction* action = new ShortcutToolAction(id(), text(), parent);

    KisActionRegistry::instance()->propertizeAction(id(), action);

    connect(action, SIGNAL(changed()), SLOT(shortcutToolActionUpdated()));

    return action;
}

QString ToolHelper::section() const
{
    return m_toolFactory->section();
}

int ToolHelper::priority() const
{
    return m_toolFactory->priority();
}

QKeySequence ToolHelper::shortcut() const
{
    if (m_hasCustomShortcut) {
        return m_customShortcut;
    }

    return m_toolFactory->shortcut();
}


//   ************ KoToolAction::Private **********

class Q_DECL_HIDDEN KoToolAction::Private
{
public:
    ToolHelper* toolHelper;
};

KoToolAction::KoToolAction(ToolHelper* toolHelper)
    : QObject(toolHelper)
    , d(new Private)
{
    d->toolHelper = toolHelper;
}

KoToolAction::~KoToolAction()
{
    delete d;
}

void KoToolAction::trigger()
{
    d->toolHelper->activate();
}


QString KoToolAction::iconText() const
{
    return d->toolHelper->iconText();
}

QString KoToolAction::toolTip() const
{
    return d->toolHelper->toolTip();
}

QString KoToolAction::id() const
{
    return d->toolHelper->id();
}

QString KoToolAction::iconName() const
{
    return d->toolHelper->iconName();
}

QKeySequence KoToolAction::shortcut() const
{
    return d->toolHelper->shortcut();
}


QString KoToolAction::section() const
{
    return d->toolHelper->section();
}

int KoToolAction::priority() const
{
    return d->toolHelper->priority();
}

int KoToolAction::buttonGroupId() const
{
    return d->toolHelper->uniqueId();
}

QString KoToolAction::visibilityCode() const
{
    return d->toolHelper->activationShapeId();
}



//   ************ Connector **********
Connector::Connector(KoShapeManager *parent)
        : QObject(parent),
        m_shapeManager(parent)
{
    connect(m_shapeManager, SIGNAL(selectionChanged()), this, SLOT(selectionChanged()));
}

void Connector::selectionChanged()
{
    emit selectionChanged(m_shapeManager->selection()->selectedShapes());
}

//   ************ ShortcutToolAction **********
ShortcutToolAction::ShortcutToolAction(const QString &id, const QString &name, QObject *parent)
    : QAction(name, parent)
    , m_toolID(id)
{
    connect(this, SIGNAL(triggered(bool)), this, SLOT(actionTriggered()));
}

ShortcutToolAction::~ShortcutToolAction()
{
}

void ShortcutToolAction::actionTriggered()
{
    // todo: why not ToolHelper::activate(); and thus a slightly different behaviour?
    // Answering the todo item: switchToolRequested
    KoToolManager::instance()->switchToolRequested(m_toolID);
}

