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

#ifndef __INET_BOXEDLABELFIGURE_H
#define __INET_BOXEDLABELFIGURE_H

#include "inet/common/INETDefs.h"

namespace inet {

class INET_API BoxedLabelFigure : public cGroupFigure
{
  protected:
    double inset = 3;
    cLabelFigure *labelFigure;
    cRectangleFigure *rectangleFigure;

  public:
    BoxedLabelFigure(const char *name = nullptr);

    cLabelFigure *getLabelFigure() const { return labelFigure; }
    cRectangleFigure *getRectangleFigure() const { return rectangleFigure; }

    double getInset() const { return inset; }
    void setInset(double inset);

    void setTags(const char *tags);
    void setTooltip(const char *tooltip);
    void setAssociatedObject(cObject * object);

    const cFigure::Rectangle& getBounds() const;

    const cFigure::Font& getFont() const;
    void setFont(cFigure::Font font);

    const cFigure::Color& getLabelColor() const;
    void setLabelColor(cFigure::Color color);

    const cFigure::Color& getBackgroundColor() const;
    void setBackgroundColor(cFigure::Color color);

    const char *getText() const;
    void setText(const char *text);

    double getOpacity() const;
    void setOpacity(double opacity);
};

} // namespace inet

#endif // ifndef __INET_BOXEDLABELFIGURE_H

