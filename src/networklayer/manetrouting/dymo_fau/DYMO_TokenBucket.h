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

#ifndef DYMO_TOKENBUCKET_H
#define DYMO_TOKENBUCKET_H

#include "INETDefs.h"

#include "IPv4Address.h"
#include "IInterfaceTable.h"
#include "InterfaceTableAccess.h"
#include "IRoutingTable.h"
#include "RoutingTableAccess.h"
#include "Ieee802Ctrl_m.h"
#include "ICMPMessage.h"
#include "IPv4Datagram.h"

#include "DYMO_Packet_m.h"
#include "DYMO_RoutingTable.h"
#include "DYMO_OutstandingRREQList.h"
#include "DYMO_RM_m.h"
#include "DYMO_RREQ_m.h"
#include "DYMO_RREP_m.h"
#include "DYMO_AddressBlock.h"
#include "DYMO_RERR_m.h"
#include "DYMO_UERR_m.h"
#include "DYMO_DataQueue.h"
#include "DYMO_Timeout_m.h"

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

#endif

