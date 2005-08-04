/* -*- mode:c++ -*- ********************************************************
 * file:        SnrDecider.cc
 *
 * author:      Marc Loebbers, Andreas Koepke
 *
 * copyright:   (C) 2004 Telecommunication Networks Group (TKN) at
 *              Technische Universitaet Berlin, Germany.
 *
 *              This program is free software; you can redistribute it
 *              and/or modify it under the terms of the GNU General Public
 *              License as published by the Free Software Foundation; either
 *              version 2 of the License, or (at your option) any later
 *              version.
 *              For further information see file COPYING
 *              in the top level directory
 ***************************************************************************
 * part of:     framework implementation developed by tkn
 **************************************************************************/

#include "SnrDecider.h"
#include "AirFrame_m.h"
#include "FWMath.h"


Define_Module(SnrDecider);


void SnrDecider::initialize(int stage)
{
    BasicDecider::initialize(stage);

    if (stage == 0)
    {
        snrThresholdLevel = FWMath::dBm2mW(par("snrThresholdLevel"));
    }
}


/**
 *  Checks the received sduList (from the PhySDU header) if it contains an
 *  snr level above the threshold
 */
bool SnrDecider::snrOverThreshold(SnrList& snrlist) const
{
    //check the entries in the sduList if a level is lower than the
    //acceptable minimum:

    //check
    for (SnrList::const_iterator iter = snrlist.begin(); iter != snrlist.end(); iter++)
    {
        if (iter->snr <= snrThresholdLevel)
        {
            EV << "Message got lost. MessageSnr: " << iter->
                snr << " Threshold: " << snrThresholdLevel << endl;
            return false;
        }
    }
    return true;
}

void SnrDecider::handleLowerMsg(AirFrame *af, SnrList& receivedList)
{
    if (snrOverThreshold(receivedList))
    {
        EV << "Message handed on to Mac\n";
        sendUp(af);
    }
    else
    {
        delete af;
    }
}
