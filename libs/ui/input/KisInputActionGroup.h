/*
 *  Copyright (c) 2018 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISINPUTACTIONGROUP_H
#define KISINPUTACTIONGROUP_H

#include <QFlags>

enum KisInputActionGroup {
    NoActionGroup = 0x0,
    ViewTransformActionGroup = 0x1,
    ModifyingActionGroup = 0x2,
    AllActionGroup = ViewTransformActionGroup | ModifyingActionGroup
};

Q_DECLARE_FLAGS(KisInputActionGroupsMask, KisInputActionGroup)
Q_DECLARE_OPERATORS_FOR_FLAGS(KisInputActionGroupsMask)


/**
 * A special interface class for accessing masking properties using
 * KisInputActionGroupsMaskGuard
 */
struct KisInputActionGroupsMaskInterface
{
    virtual ~KisInputActionGroupsMaskInterface();

    /**
     * Return the mask of currently available input action groups
     */
    virtual KisInputActionGroupsMask inputActionGroupsMask() const = 0;

    /**
     * Set the mask of currently available action groups
     */
    virtual void setInputActionGroupsMask(KisInputActionGroupsMask mask) = 0;
};

/**
 * A RAII wrapper for setting the input actions mask on the masking interface
 * (which is usually a canvas). In constructor the guard saves the previous mask
 * value and resets it to the new one. In destructor the guard restores the old
 * mask value.
 */
class KisInputActionGroupsMaskGuard
{
public:
    /**
     * Create a guard and set a new mask \p mask onto \p object. The old mask value is
     * saved in the guard itself.
     */
    KisInputActionGroupsMaskGuard(KisInputActionGroupsMaskInterface *object, KisInputActionGroupsMask mask);

    /**
     * Destroy the guard and reset the mask value to the old value
     */
    ~KisInputActionGroupsMaskGuard();

private:
    KisInputActionGroupsMaskInterface *m_object;
    KisInputActionGroupsMask m_oldMask;
};

#endif // KISINPUTACTIONGROUP_H
