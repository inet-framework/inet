/* -*-  mode:c++ -*- *******************************************************
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
 ***************************************************************************/


#ifndef GE_DECIDER_H
#define GE_DECIDER_H

#include "SnrDecider.h"

/**
 * @brief decider module for the GilbertElliotSnr module
 *
 * This decider simply takes a look at the sduList contained in the
 * received PhySDU packet and checks whether one of the mentioned snr
 * levels is lower than the snrThresholdLevel which has to be read in
 * at the beginning of a simulation (suggestion: from the omnetpp.ini
 * file!). If there is such a level the packet is considered to be
 * lost due to a collision. On top of that the packet can also be lost
 * due to the BAD state of the digital channel model model (see e.g.
 * GilbertElliotSnr module). This module should be used in combination
 * with digital channel modules like GilbertElliotSnr.
 *
 * @author Marc Löbbers
 *
 * @ingroup decider
 */
class INET_API ErrAndCollDecider : public SnrDecider
{
  protected:
    virtual void handleLowerMsg(AirFrame*, SnrList &);
};

#endif
