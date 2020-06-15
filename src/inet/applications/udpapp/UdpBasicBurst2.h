//
// Copyright (C) 2004 Andras Varga
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
// Copyright (C) 2011 Zoltan Bojthe
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
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//

#ifndef __INET_UDPBASICBURST2_H
#define __INET_UDPBASICBURST2_H

#include <map>
#include <vector>

#include "inet/common/INETDefs.h"
#include "inet/applications/udpapp/UdpBasicBurst.h"

namespace inet {

/**
 * UDP application. See NED for more info.
 */
class INET_API UdpBasicBurst2 : public UdpBasicBurst
{

    static int numStatics;
    static int numMobiles;

    static std::vector<L3Address> staticNodes;
    static int packetStatic;
    static int packetMob;
    static int packetStaticRec;
    static int packetMobRec;
    static int stablePaths;
    static cHistogram * delay;
    bool isStatic = false;


  protected:
    // chooses random destination address
    virtual void processPacket(Packet *msg) override;
    virtual void initialize(int stage) override;
    virtual void finish() override;
    virtual void generateBurst() override;

  public:
    UdpBasicBurst2() {}
    virtual ~UdpBasicBurst2() {if (delay != nullptr) {delete delay; delay = nullptr;}}

};

} // namespace inet

#endif // ifndef __INET_UDPBASICBURST_H

