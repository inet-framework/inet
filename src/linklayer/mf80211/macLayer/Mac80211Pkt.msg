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
}}


class MacPkt;


//
// Defines all fields of an 802.11 MAC frame
//
// This component has been taken over from Mobility Framework 1.0a5.
//
packet Mac80211Pkt extends MacPkt
{
    int address3;
    int address4;
    simtime_t duration;
    int fragmentation; //part of the Frame Control field
    int informationDS; //part of the Frame Control field
    int sequenceControl;
}
