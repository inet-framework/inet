//**************************************************************************
// * file:        Mac80211PktXXX.msg
// *
// * author:      Marc Löbbers
// * copyright:   (c) by 'the Glimmer Twins'
// *              Telecommunication Networks Group
// *              TU-Berlin
// * email:       loebbers@tkn.tu-berlin.de
// **************************************************************************
// * part of:     framework implementation developed by tkn
// *************************************************************************/

cplusplus {{
#include "MACAddress.h"
}};

class noncobject MACAddress;


message MacPktXXX
{
    fields:
        MACAddress destAddr; // destination mac address
        MACAddress srcAddr; // source mac address
        int channelId=1; // channel to send / receive on
}


//
// Defines all fields of an 802.11 MAC frame
//
message Mac80211PktXXX extends MacPktXXX
{
   fields:
       int address3;
       int address4;
       double duration;
       int fragmentation; //part of the Frame Control field
       int informationDS; //part of the Frame Control field
       int sequenceControl;
};
