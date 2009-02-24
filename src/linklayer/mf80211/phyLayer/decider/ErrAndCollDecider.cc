/* -*- mode:c++ -*- ********************************************************
 * file:        ErrAndCollDecider.cc
 *
 * author:      Marc Loebbers
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


#include "ErrAndCollDecider.h"
#include "AirFrame_m.h"

Define_Module(ErrAndCollDecider);


void ErrAndCollDecider::handleLowerMsg(AirFrame *af, SnrList& receivedList)
{
    if (snrOverThreshold(receivedList))
    {
        if (af->hasBitError())
        {
            EV << "Message got lost due to digital channel model BAD state\n";
            delete af;
        }
        else
        {
            EV << "Message handed on to Mac\n";
            sendUp(af);
        }
    }
    else
    {
        EV << "COLLISION!\n";
        delete af;
    }
}
