/* -*- mode:c++ -*- ********************************************************
 * file:        GilbertElliotSnr.h
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

#ifndef GE_SNR_H
#define GE_SNR_H

#include "SnrEval.h"


/**
 * @brief Keeps track of the different snir levels when receiving a
 * packet and switches between states GOOD and BAD (Gilbert-Elliot-Model)
 *
 * This module has exactly the same functionality as the SnrEval
 * module, but furthermore it puts a Gilbert-Elliot model on top of
 * the SNIR level computation. This means that it changes between the
 * states GOOD and BAD. If it is in state BAD a received frame will be
 * marked as corrupted.
 *
 * The Gilbert Elliot model is used to simulate outer system
 * disturbances.
 *
 * This module should be used in combination with the
 * GilbertElliotDecider moulde which evaluates the snr levels as well
 * as the marked frames.
 *
 * @author Marc Loebbers
 * @ingroup snrEval
 */
class INET_API GilbertElliotSnr : public SnrEval
{
  public:
    GilbertElliotSnr();
    virtual ~GilbertElliotSnr();

  protected:
    /** @brief Initialize variables and publish the radio status*/
    virtual void initialize(int);

    /** @brief Handle timers*/
    virtual void handleSelfMsg(cMessage*);

    /** @brief Buffer the frame and update noise levels and snr information...*/
    virtual void handleLowerMsgStart(AirFrame*);

    /** @brief Unbuffer the frame and update noise levels and snr information*/
    virtual void handleLowerMsgEnd(AirFrame*);

  protected:
    /** @brief Gilbert-Elliot state types*/
    enum State{
      GOOD,
      BAD
    };

    /** @brief System state*/
    State state;

    /** @brief mean value to compute the time the xystem stays in the GOOD state*/
    double meanGood;

    /** @brief mean value to compute the time the xystem stays in the BAD state*/
    double meanBad;

    /** @brief timer to indicate a change of state*/
    cMessage* stateChange;
};

#endif
