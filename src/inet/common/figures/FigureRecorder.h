//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_FIGURERECORDER_H
#define __INET_FIGURERECORDER_H

#include "inet/common/figures/IIndicatorFigure.h"

namespace inet {
class INET_API FigureRecorder : public cNumericResultRecorder
{
  protected:
    IIndicatorFigure *indicatorFigure = nullptr;
    int series = 0;

  protected:
    virtual void init(Context *ctx) override;
    virtual void collect(simtime_t_cref t, double value, cObject *details) override;
};
} // namespace inet

#endif

