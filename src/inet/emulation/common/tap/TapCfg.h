//
// Copyright (C) 2018 OpenSimLtd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

// This file is based on the Ppp.h of INET written by Andras Varga.

#ifndef __INET_TAPCFG_H
#define __INET_TAPCFG_H

#include "inet/common/INETDefs.h"
#include "inet/common/IInterfaceRegistrationListener.h"
#include "inet/common/scheduler/RealTimeScheduler.h"
#include "inet/networklayer/common/InterfaceEntry.h"

namespace inet {

/**
 * Implements an interface that corresponds to a real interface
 * on the host running the simulation. Suitable for hardware-in-the-loop
 * simulations.
 *
 * See NED file for more details.
 */
class INET_API TapCfg : public cSimpleModule
{
  public:
    virtual ~TapCfg();
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *msg) override;
};

} // namespace inet

#endif // ifndef __INET_TAPCFG_H

