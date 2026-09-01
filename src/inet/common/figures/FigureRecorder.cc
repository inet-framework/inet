//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/common/figures/FigureRecorder.h"

#include "inet/common/INETUtils.h"

namespace inet {
Register_ResultRecorder("figure", FigureRecorder);

void FigureRecorder::init(Context *ctx)
{
    cNumericResultRecorder::init(ctx);

    cModule *module = check_and_cast<cModule *>(getComponent());
    const char *figureSpec = ctx->attrsProperty->getValue("targetFigure");
    if (!figureSpec)
        figureSpec = ctx->statisticName;
    std::string figureName;
    int index;
    if (const char *lastColon = strrchr(figureSpec, ':')) {
        figureName = std::string(figureSpec, lastColon - figureSpec).c_str();
        index = utils::atoul(lastColon + 1);
    }
    else {
        figureName = figureSpec;
        index = 0;
    }
    cFigure *figure = module->getCanvas()->getFigureByPath(figureName.c_str());
    if (!figure)
        throw cRuntimeError("Figure '%s' in module '%s' not found", figureName.c_str(), module->getFullPath().c_str());
    indicatorFigure = check_and_cast<IIndicatorFigure *>(figure);
    if (index < 0 || index >= indicatorFigure->getNumItems())
        throw cRuntimeError("Item index %d is out of bounds, figure '%s' displays %d items", index, figureName.c_str(), indicatorFigure->getNumItems());
}

void FigureRecorder::collect(simtime_t_cref t, double value, cObject *details)
{
    indicatorFigure->setValue(index, t, value);
}

} // namespace inet

