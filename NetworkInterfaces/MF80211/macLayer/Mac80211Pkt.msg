//**************************************************************************
// * file:        Mac80211Pkt.msg
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
#include "MacPkt_m.h"
}};

class MacPkt;

//
// Defines all fields of an 802.11 MAC frame
//
message Mac80211Pkt extends MacPkt
{
   fields:
       int address3;
       int address4;
       double duration;
       int fragmentation; //part of the Frame Control field
       int informationDS; //part of the Frame Control field
       int sequenceControl;
};
