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
    int series;
    if (const char *lastColon = strrchr(figureSpec, ':')) {
        figureName = std::string(figureSpec, lastColon - figureSpec).c_str();
        series = utils::atoul(lastColon + 1);
    }
    else {
        figureName = figureSpec;
        series = 0;
    }
    cFigure *figure = module->getCanvas()->getFigureByPath(figureName.c_str());
    if (!figure)
        throw cRuntimeError("Figure '%s' in module '%s' not found", figureName.c_str(), module->getFullPath().c_str());
    indicatorFigure = check_and_cast<IIndicatorFigure *>(figure);
    if (series > indicatorFigure->getNumSeries())
        throw cRuntimeError("series :%d is out of bounds, figure '%s' supports %d series", series, figureName.c_str(), indicatorFigure->getNumSeries());
}

void FigureRecorder::collect(simtime_t_cref t, double value, cObject *details)
{
    indicatorFigure->setValue(series, t, value);
}

} // namespace inet

