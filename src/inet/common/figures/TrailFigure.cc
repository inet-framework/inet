//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/common/figures/TrailFigure.h"

namespace inet {

TrailFigure::TrailFigure(int maxCount, bool fadeOut, const char *name) :
    cGroupFigure(name),
    maxCount(maxCount),
    fadeCounter(0),
    fadeOut(fadeOut)
{
}

void TrailFigure::addFigure(cFigure *figure)
{
    cGroupFigure::addFigure(figure);
    if (getNumFigures() > maxCount)
        delete removeFigure(0);
    if (fadeOut) {
        if (fadeCounter > 0)
            fadeCounter--;
        else {
            int count = getNumFigures();
            fadeCounter = count / 10;
            for (int i = 0; i < count; i++) {
                cFigure *figure = getFigure(i);
                cAbstractLineFigure *lineFigure = dynamic_cast<cAbstractLineFigure *>(figure);
                if (lineFigure)
                    lineFigure->setLineOpacity((double)i / count);
            }
        }
    }
}

} // namespace inet

