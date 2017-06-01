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

#ifndef __INET_SIGNALSOURCE_H
#define __INET_SIGNALSOURCE_H

#include "inet/common/INETDefs.h"

namespace inet {

class INET_API SignalSource : public cSimpleModule
{
  protected:
    simtime_t startTime, endTime;
    simsignal_t signal = -1;

  public:
    SignalSource() {}

  protected:
    void initialize() override;
    void handleMessage(cMessage *msg) override;
    void finish() override;
};

} // namespace inet

#endif // ifndef __INET_SERIESSIGNALSOURCE_H

