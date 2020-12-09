/*
 *  Copyright (c) 2018 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisRunnableStrokeJobDataBase.h"

KisRunnableStrokeJobDataBase::KisRunnableStrokeJobDataBase(KisStrokeJobData::Sequentiality sequentiality, KisStrokeJobData::Exclusivity exclusivity)
    : KisStrokeJobData(sequentiality, exclusivity)
{
}
