//
// Copyright (C) OpenSim Ltd.
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

#ifndef __INET_PHYSICALENVIRONMENTOSGVISUALIZER_H
#define __INET_PHYSICALENVIRONMENTOSGVISUALIZER_H

#include "inet/environment/common/PhysicalEnvironment.h"
#include "inet/visualizer/base/PhysicalEnvironmentVisualizerBase.h"

namespace inet {

namespace visualizer {

class INET_API PhysicalEnvironmentOsgVisualizer : public PhysicalEnvironmentVisualizerBase
{
#ifdef WITH_OSG

  protected:
    bool enableObjectOpacity = false;

  protected:
    virtual void initialize(int stage) override;
    virtual void refreshDisplay() const override;

#endif // ifdef WITH_OSG
};

} // namespace visualizer

} // namespace inet

#endif // ifndef __INET_PHYSICALENVIRONMENTOSGVISUALIZER_H

