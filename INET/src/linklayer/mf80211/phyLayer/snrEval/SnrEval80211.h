/* -*- mode:c++ -*- ********************************************************
 * file:        SnrEval80211.h
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

#ifndef SNR_EVAL_80211H
#define SNR_EVAL_80211H

#include "SnrEval.h"


/**
 * @brief A SnrEval for the 802.11b protocol
 *
 * Subclass of SnrEval. Basically the same except for some extra
 * parameters of 802.11 and the duration of the packet that has to be
 * computed differently as the modulation of header and data part of the
 * packet are different. This module forms a physical layer together with
 * the Decider80211 module. The resulting physical layer is intended to
 * be used together with the Mac80211 module.
 *
 * @author Marc Löbbers
 *
 * @ingroup snrEval
 */
class INET_API SnrEval80211 : public SnrEval
{
  protected:
    /** @brief Some extra parameters have to be read in */
    virtual void initialize(int);

  protected:
    /** @brief computes the duration of a 802.11 frame in seconds */
    virtual double calcDuration(cPacket *);
};

#endif
