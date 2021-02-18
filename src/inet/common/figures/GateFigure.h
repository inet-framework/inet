//
// Copyright (C) 2020 OpenSim Ltd.
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

#ifndef __INET_GATEFIGURE_H
#define __INET_GATEFIGURE_H

#include "inet/common/INETMath.h"

namespace inet {

class INET_API GateFigure : public cRectangleFigure
{
  protected:
    double spacing = 2;
    double position = 0;
    cLabelFigure *labelFigure = nullptr;
    cLineFigure *positionFigure = nullptr;
    std::vector<cRectangleFigure *> scheduleFigures;

  public:
    GateFigure(const char *name = nullptr);

    virtual void setBounds(const Rectangle& bounds) override;

    const char *getLabel() const { return labelFigure->getText(); }
    void setLabel(const char *text);

    double getSpacing() const { return spacing; }
    void setSpacing(double spacing) { this->spacing = spacing; }

    double getPosition() const { return position; }
    void setPosition(double position);

    void addSchedule(double start, double end, bool open);
    void clearSchedule();
};

} // namespace inet

#endif

