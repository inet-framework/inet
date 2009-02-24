/* -*-  mode:c++ -*- *******************************************************
 * file:        SnrDecider.cc
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


#ifndef SNR_DECIDER_H
#define SNR_DECIDER_H

#include <BasicDecider.h>

/**
 * @brief a simple snr decider...
 *
 * This decider simply takes a look at the sduList contained in the
 * received PhySDU packet and checks whether one of the mentioned snr
 * levels is lower than the snrThresholdLevel which has to be read in at
 * the beginning of a simulation. (suggestion: from the omnetpp.ini
 * file!)
 *
 * @author Marc Löbbers, Andreas Koepke
 *
 * @ingroup decider
 */
class INET_API SnrDecider : public BasicDecider
{
  protected:
    /** @brief Level for decision [mW]
     *
     * When a packet contains an snr level higher than snrThresholdLevel it
     * will be considered as lost. This parameter has to be specified at the
     * beginning of a simulation (omnetpp.ini) in dBm.
     */
    double snrThresholdLevel;

  protected:
    virtual void initialize(int);

  protected:
    virtual bool snrOverThreshold(SnrList &) const;
    virtual void handleLowerMsg(AirFrame*, SnrList &);
};

#endif
