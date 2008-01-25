//
// Copyright (C) 2003 CTIE, Monash University
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

cplusplus {{
#include "INETDefs.h"
#define ETHERAPP_CLI_SAP  0xf0
#define ETHERAPP_SRV_SAP  0xf1
}};


//
// Request packet sent by EtherAppCli. Contains the number of bytes to be
// sent back as response; if this value is greater than ~1400, the reply
// will consist of several Ethernet frames. See also EtherAppResp.
//
message EtherAppReq
{
    fields:
        long requestId;
        long responseBytes;
};

//
// Packet sent back by EtherAppSrv in response to an EtherAppReq.
//
message EtherAppResp
{
    fields:
        int requestId;
        int numFrames;
};


