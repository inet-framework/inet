// file:        SnrEval80211.cc
//
//  author:      Marc Löbbers
// copyright:   (c) by Tralafitty
//              Telecommunication Networks Group
//              TU-Berlin
// email:       loebbers@tkn.tu-berlin.de

// part of: framework implementation developed by tkn description: - a
// snrEval extension for the use with the other 802.11 modules


#include "SnrEval80211.h"
#include "Consts80211.h"
#include "AirFrame_m.h"


Define_Module(SnrEval80211);

void SnrEval80211::initialize(int stage)
{
    SnrEval::initialize(stage);

    if (stage == 0)
    {
        EV << "initializing stage 0\n";
        if (bitrate != 1E+6 && bitrate != 2E+6 && bitrate != 5.5E+6 && bitrate != 11E+6)
            error("Wrong bit rate for 802.11, valid values are 1E+6, 2E+6, 5.5E+6 or 11E+6");
        headerLength = 192;     //has to be 192; this makes sure it is!
    }
}


/**
 * The duration of the packet is computed, with respect to the
 * different bitrates of header and data. The header is sent with
 * 1Mbit/s and the rest with the bitrate read in in initialize().
 */
double SnrEval80211::calcDuration(cPacket *frame)
{
    EV << "bits without header: " << frame->getBitLength() -
        headerLength << ", bits header: " << headerLength << endl;
    return ((frame->getBitLength() - headerLength) / bitrate + headerLength / BITRATE_HEADER);
}
