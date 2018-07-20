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

#ifndef __INET_IEEE8022LLC_H
#define __INET_IEEE8022LLC_H

#include "inet/common/packet/Packet.h"
#include "inet/linklayer/ieee8022/Ieee8022LlcHeader_m.h"

namespace inet {

class INET_API Ieee8022Llc : public cSimpleModule
{
  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *message) override;

    virtual void encapsulate(Packet *frame);
    virtual void decapsulate(Packet *frame);

  public:
    static const Protocol *getProtocol(const Ptr<const Ieee8022LlcHeader>& header);
};

} // namespace inet

#endif // ifndef __INET_IEEE8022LLC_H

