//
// This program is property of its copyright holder. All rights reserved.
// 

cplusplus{{
#include "inet/common/INETDefs.h"
}}

import inet.common.TagBase;

namespace inet;

//
// Some 8-bit SAP values for IEEE 802.x LLC headers.
//
enum SAPCode
{
    SAP_IBM_SNA = 0x04;
    SAP_IP = 0x06;
    SAP_3COM = 0x80;
    SAP_SNAP = 0xAA;
    SAP_BANYAN = 0xBC;
    SAP_NOVELL_IPX = 0xE0;
    SAP_LAN_MANAGER = 0xF4;
    SAP_CLNS = 0xFE;
    SAP_STP = 0x42;
}

class Ieee802SapTagBase extends TagBase
{
    int ssap;        // used with IEEE 802 LLC (see ~EtherFrameWithLLC)
    int dsap;        // used with IEEE 802 LLC (see ~EtherFrameWithLLC)
}

class Ieee802SapReq extends Ieee802SapTagBase
{
}

class Ieee802SapInd extends Ieee802SapTagBase
{
}
