//
// Copyright (C) 2012 Andras Varga
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

#ifndef __INET_DSCPMARKER_H
#define __INET_DSCPMARKER_H

#include "inet/common/INETDefs.h"

namespace inet {

/**
 * DSCP Marker.
 */
class INET_API DSCPMarker : public cSimpleModule
{
  protected:
    std::vector<int> dscps;

    int numRcvd = 0;
    int numMarked = 0;

    static simsignal_t markPkSignal;

  public:
    DSCPMarker() {}

  protected:
    virtual void initialize() override;

    virtual void handleMessage(cMessage *msg) override;

    virtual bool markPacket(cPacket *msg, int dscp);
};

} // namespace inet

#endif // ifndef __INET_DSCPMARKER_H

