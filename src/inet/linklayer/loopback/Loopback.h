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
// Author: Zoltan Bojthe
//

#ifndef __INET_LOOPBACK_H
#define __INET_LOOPBACK_H

#include "inet/common/INETDefs.h"
#include "inet/linklayer/base/MacProtocolBase.h"

namespace inet {

class InterfaceEntry;

/**
 * Loopback interface implementation.
 */
class INET_API Loopback : public MacProtocolBase
{
  protected:
    // statistics
    long numSent = 0;
    long numRcvdOK = 0;

  protected:
    virtual void configureInterfaceEntry() override;

  public:
    Loopback() {}
    virtual ~Loopback();

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleUpperPacket(Packet *packet) override;
    virtual void refreshDisplay() const override;
};

} // namespace inet

#endif // ifndef __INET_LOOPBACK_H

