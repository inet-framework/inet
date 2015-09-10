/*
 *  Copyright (C) 2007 Christoph Sommer <christoph.sommer@informatik.uni-erlangen.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef __INET_DYMO_TOKENBUCKET_H
#define __INET_DYMO_TOKENBUCKET_H

#include "inet/common/INETDefs.h"

#include "inet/networklayer/contract/ipv4/IPv4Address.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/networklayer/ipv4/IIPv4RoutingTable.h"
#include "inet/linklayer/common/Ieee802Ctrl.h"
#include "inet/networklayer/ipv4/ICMPMessage.h"
#include "inet/networklayer/ipv4/IPv4Datagram.h"

#include "inet/routing/extras/dymo_fau/DYMO_Packet_m.h"
#include "inet/routing/extras/dymo_fau/DYMO_RoutingTable.h"
#include "inet/routing/extras/dymo_fau/DYMO_OutstandingRREQList.h"
#include "inet/routing/extras/dymo_fau/DYMO_RM_m.h"
#include "inet/routing/extras/dymo_fau/DYMO_RREQ_m.h"
#include "inet/routing/extras/dymo_fau/DYMO_RREP_m.h"
#include "inet/routing/extras/dymo_fau/DYMO_AddressBlock.h"
#include "inet/routing/extras/dymo_fau/DYMO_RERR_m.h"
#include "inet/routing/extras/dymo_fau/DYMO_UERR_m.h"
#include "inet/routing/extras/dymo_fau/DYMO_DataQueue.h"
#include "inet/routing/extras/dymo_fau/DYMO_Timeout_m.h"

namespace inet {

namespace inetmanet {

//===========================================================================================
// class DYMO_TokenBucket: Simple rate limiting mechanism
//===========================================================================================
class DYMO_TokenBucket
{
  public:
    DYMO_TokenBucket(double tokensPerTick, double maxTokens, simtime_t currentTime);

    bool consumeTokens(double tokens, simtime_t currentTime);

  protected:
    double tokensPerTick;
    double maxTokens;

    double availableTokens;
    simtime_t lastUpdate;

};

} // namespace inetmanet

} // namespace inet

#endif

