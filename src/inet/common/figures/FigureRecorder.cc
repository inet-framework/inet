//
// Copyright (C) 2016 OpenSim Ltd.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
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

