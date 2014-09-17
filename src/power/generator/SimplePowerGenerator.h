//
// Copyright (C) 2013 OpenSim Ltd.
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

#ifndef __INET_SIMPLEPOWERGENERATOR_H
#define __INET_SIMPLEPOWERGENERATOR_H

#include "inet/power/contract/IPowerGenerator.h"
#include "inet/power/contract/IPowerSink.h"

namespace inet {

namespace power {

/**
 * This class implements a simple power generator.
 *
 * @author Levente Meszaros
 */
class INET_API SimplePowerGenerator : public cSimpleModule, public IPowerGenerator
{
  protected:
    bool isSleeping;
    int powerGeneratorId;
    IPowerSink *powerSink;
    W powerGeneration;
    cMessage *timer;

  protected:
    virtual void initialize(int stage);

    virtual void handleMessage(cMessage *message);

    virtual void updatePowerGeneration();

    virtual void scheduleIntervalTimer();

  public:
    SimplePowerGenerator();
    virtual ~SimplePowerGenerator();

    virtual W getPowerGeneration() { return powerGeneration; }
};

} // namespace power

} // namespace inet

#endif // ifndef __INET_SIMPLEPOWERGENERATOR_H

