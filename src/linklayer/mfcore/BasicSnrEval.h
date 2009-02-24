/* -*- mode:c++ -*- ********************************************************
 * file:        BasicSnrEval.h
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


#ifndef BASIC_SNREVAL_H
#define BASIC_SNREVAL_H

#include <map>

#include "ChannelAccess.h"
#include "AirFrame_m.h"
#include "SnrControlInfo_m.h"


/**
 * @brief The basic class for all snrEval modules
 *
 * The BasicSnrEval module provides functionality like en- and
 * decapsulation of messages. If you use the standard message formats
 * everything should work fine. Before a packet is sent some
 * information, e.g. transmission power, can be written to the
 * AirFrame header. If you write your own snrEval, just subclass and
 * redefine the handleUpperMsg function (see description of the
 * function). After receiving a message it can be processed in
 * handleLowerMsgStart. Then it is buffered for the time the
 * transmission would last in reality, and then can be handled
 * again. Again you can redefine the 1. handleLowerMsgStart and
 * 2. handleLowerMsgEnd for your own needs (see description). So, the
 * call of these functions represent the following events: 1. received
 * a message (i.e. transmission startet) 2. message will be handed on
 * to the upper layer (i.e. transmission time is over)
 *
 * This supports a single radio channel only
 *
 * @author Marc Loebbers
 * @ingroup snrEval
 * @ingroup basicModules
 */
class INET_API BasicSnrEval : public ChannelAccess
{
  protected:
    /** @brief a parameter that has to be read in from omnetpp.ini*/
    double bitrate;

    /** @brief a parameter that has to be read in from omnetpp.ini*/
    int headerLength;

    /** @brief power used to transmit messages */
    double transmitterPower;

    /** @brief gate id*/
    /*@{*/
    int uppergateOut;
    int uppergateIn;
    /*@}*/

  protected:
    /** @brief Initialization of the module and some variables*/
    virtual void initialize(int);

    /** @brief Called every time a message arrives*/
    virtual void handleMessage( cMessage* );

  protected:
    /**
     * @name Handle Messages
     * @brief Functions to redefine by the programmer
     */
    /*@{*/
    /**
     * @brief Fill the header fields, redefine for your own needs...
     */
    virtual void handleUpperMsg(AirFrame*);

    /**
     * @brief Handle self messages such as timer...
     *
     * Define this function if you want to process timer or other kinds
     * of self messages
     */
    virtual void handleSelfMsg(cMessage *msg){delete msg;};

    /** @brief Calculate Snr Information before buffering.*/
    virtual void handleLowerMsgStart(AirFrame*);

    /**
     * @brief Calculate SnrInfo after buffering and add the PhySnrList
     * to the message
     */
    virtual void handleLowerMsgEnd(AirFrame*);
    /*@}*/

    /**
     * @name Convenience Functions
     * @brief Functions for convenience - NOT to be modified
     *
     * These are functions taking care of message encapsulation and
     * message sending. Normally you should not need to alter these but
     * should use them to handle message encasulation and sending. They
     * will wirte all necessary information into packet headers and add
     * or strip the appropriate headers for each layer.
     *
     */
    /*@{*/

    /** @brief Buffers message for 'transmission time'*/
    virtual void bufferMsg(AirFrame *frame);

    /** @brief Unbuffers a message after 'transmission time'*/
    virtual AirFrame* unbufferMsg(cMessage *msg);

    /** @brief Sends a message to the upper layer*/
    virtual void sendUp(AirFrame*, SnrList&);

    /** @brief Sends a message to the channel*/
    virtual void sendDown(AirFrame *msg);

    /** @brief Encapsulates a MAC frame into an Air Frame*/
    virtual AirFrame *encapsMsg(cPacket *msg);
    /*@}*/

    /** @brief This function calculates the duration of the AirFrame */
    virtual double calcDuration(cPacket *);

    /** @brief Returns the channel we're listening on. This version always returns 0 (single radio channel supported) */
    virtual int getChannelNumber() const {return 0;}

    /**
     * @name Abstraction layer
     * @brief Factory function to create AirFrame into which a MAC frame is encapsulated.
     *
     * SnrEval authors should be able to use their own SnrEval packets. The
     * framework does not need to know about them, but must be able to
     * produce new ones. Both goals can be reached by using a factory
     * method.
     *
     * Overwrite this function in derived classes to use your
     * own AirFrames
     */
    /*@{*/

    /** @brief Create a new AirFrame */
    virtual AirFrame* createCapsulePkt() {
        return new AirFrame();
    };
    /*@}*/

};

#endif
