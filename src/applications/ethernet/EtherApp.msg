//
// Copyright (C) 2003 Andras Varga; CTIE, Monash University, Australia
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

cplusplus {{
#include "INETDefs.h"
#define ETHERAPP_CLI_SAP  0xf0
#define ETHERAPP_SRV_SAP  0xf1
}}



//
// Request packet sent by EtherAppCli. Contains the number of bytes to be
// sent back as response; if this value is greater than ~1400, the reply
// will consist of several Ethernet frames. See also EtherAppResp.
//
packet EtherAppReq
{
    long requestId;
    long responseBytes;
}

//
// Packet sent back by EtherAppSrv in response to an EtherAppReq.
//
packet EtherAppResp
{
    int requestId;
    int numFrames;
}


