//
// Copyright (C) 2016 OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
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

