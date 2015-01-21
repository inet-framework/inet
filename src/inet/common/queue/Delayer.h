//
// Copyright (C) 2012 OpenSim Ltd
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
// @author: Zoltan Bojthe
//

#ifndef __INET_DELAYER_H
#define __INET_DELAYER_H

#include "inet/common/INETDefs.h"

namespace inet {

/**
 * Delayer module.
 */
class INET_API Delayer : public cSimpleModule
{
  public:
    Delayer() { delayPar = nullptr; }

  protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;

    cPar *delayPar;

    static simsignal_t rcvdPkSignal;
    static simsignal_t sentPkSignal;
    static simsignal_t delaySignal;
};

} // namespace inet

#endif // ifndef __INET_DELAYER_H

