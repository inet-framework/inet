//
// Copyright (C) 2009 - today Brno University of Technology, Czech Republic
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//
/**
* @file EigrpPrint.h
* @author Jan Zavrel (honza.zavrel96@gmail.com)
* @author Vit Rek (rek@kn.vutbr.cz)
* @author Vladimir Vesely (ivesely@fit.vutbr.cz)
* @copyright Brno University of Technology (www.fit.vutbr.cz) under GPLv3
* @date 6. 11. 2014
* @brief EIGRP Print functions header file
* @detail File contains useful functions for printing EIGRP informations (CUT + PASTE from EigrpIpv4Pdm.cc)
*/

#ifndef EIGRPPRINT_H_
#define EIGRPPRINT_H_

#include <iostream>

#include "inet/routing/eigrp/EigrpDualStack.h"
#include "inet/routing/eigrp/messages/EigrpMessage_m.h"
#include "inet/routing/eigrp/tables/EigrpNetworkTable.h"
namespace inet {
std::ostream& operator<<(std::ostream& os, const eigrp::EigrpNetwork<Ipv4Address>& network);
std::ostream& operator<<(std::ostream& os, const eigrp::EigrpNetwork<Ipv6Address>& network);
std::ostream& operator<<(std::ostream& os, const EigrpKValues& kval);
std::ostream& operator<<(std::ostream& os, const EigrpStub& stub);


namespace eigrp
{

// User message codes
enum UserMsgCodes
{
  M_OK = 0,                             // no message
  M_UPDATE_SEND = EIGRP_UPDATE_MSG,     // send Update message
  M_REQUEST_SEND = EIGRP_REQUEST_MSG,   // send Request message
  M_QUERY_SEND = EIGRP_QUERY_MSG,       // send Query message
  M_REPLY_SEND = EIGRP_REPLY_MSG,       // send Query message
  M_HELLO_SEND = EIGRP_HELLO_MSG,       // send Hello message
  M_DISABLED_ON_IF,                     // EIGRP is disabled on interface
  M_NEIGH_BAD_AS,                       // neighbor has bad AS number
  M_NEIGH_BAD_KVALUES,                  // neighbor has bad K-values
  M_NEIGH_BAD_SUBNET,                   // neighbor isn't on common subnet
  M_SIAQUERY_SEND = EIGRP_SIAQUERY_MSG, // send SIA Query message
  M_SIAREPLY_SEND = EIGRP_SIAREPLY_MSG, // send SIA Reply message
};

// User messages
extern const char *UserMsgs[];
}; // end of namespace eigrp
}
#endif /* EIGRPPRINT_H_ */
