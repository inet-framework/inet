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

#ifndef __INET_FIGURERECORDER_H
#define __INET_FIGURERECORDER_H

#include "inet/common/INETDefs.h"
#include "inet/common/figures/IMeterFigure.h"

namespace inet {

class INET_API FigureRecorder : public cNumericResultRecorder
{
    protected:
        IMeterFigure *meterFigure = nullptr;
        int series = 0;
    protected:
        virtual void init(cComponent *component, const char *statisticName, const char *recordingMode, cProperty *attrsProperty, opp_string_map *manualAttrs=nullptr) override;
        virtual void collect(simtime_t_cref t, double value DETAILS_ARG) override;
};

} // namespace inet

#endif

