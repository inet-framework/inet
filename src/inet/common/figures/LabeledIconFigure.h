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

#ifndef __INET_LABELEDICON_H
#define __INET_LABELEDICON_H

#include "inet/common/INETDefs.h"

namespace inet {

class INET_API LabeledIconFigure : public cGroupFigure
{
  protected:
    cIconFigure *iconFigure;
    cLabelFigure *labelFigure;

  public:
    LabeledIconFigure(const char *name = nullptr);

    cIconFigure *getIconFigure() const { return iconFigure; }
    cLabelFigure *getLabelFigure() const { return labelFigure; }

    void setTooltip(const char *tooltip);
    void setAssociatedObject(cObject * object);

    cFigure::Rectangle getBounds() const;

    void setOpacity(double opacity);

    void setPosition(cFigure::Point position);
};

} // namespace inet

#endif // ifndef __INET_LABELEDICON_H

