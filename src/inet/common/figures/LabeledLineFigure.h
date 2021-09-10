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

#ifndef __INET_LABELEDLINEFIGURE_H
#define __INET_LABELEDLINEFIGURE_H

#include "inet/common/INETDefs.h"

namespace inet {

class INET_API LabeledLineFigure : public cGroupFigure
{
  protected:
    cLineFigure *lineFigure;
    cPanelFigure *panelFigure;
    cTextFigure *labelFigure;

  protected:
    void updateLabelPosition();

  public:
    LabeledLineFigure(const char *name = nullptr);

    cLineFigure *getLineFigure() const { return lineFigure; }
    cTextFigure *getLabelFigure() const { return labelFigure; }

    void setStart(cFigure::Point point);
    void setEnd(cFigure::Point point);
};

} // namespace inet

#endif // ifndef __INET_LABELEDLINEFIGURE_H

