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

#ifndef __INET_BARFIGURE_H
#define __INET_BARFIGURE_H

#include "inet/common/INETMath.h"

namespace inet {

class INET_API BarFigure : public cGroupFigure
{
  protected:
    double value = NaN;
    double minValue = NaN;
    double maxValue = NaN;
    double width = 3;
    double height = 16;
    cFigure::Point position;

  protected:
    virtual void refreshDisplay();

  public:
    BarFigure(double value, double minValue, double maxValue, const char *name = nullptr);

    virtual double getWidth() const { return width; }
    virtual double getHeight() const { return height; }

    virtual void setPosition(const cFigure::Point& position);
    virtual void setValue(double value);
};

} // namespace inet

#endif // ifndef __INET_BARFIGURE_H

