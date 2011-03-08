/* -*- mode:c++ -*- *******************************************************
 * file:        BasicDecider.h
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


#ifndef  BASIC_DECIDER_H
#define  BASIC_DECIDER_H

#include <omnetpp.h>

#include "BasicModule.h"
#include "AirFrame_m.h"
#include "SnrControlInfo_m.h"


/**
 * @brief Module to decide whether a frame is received correctly or is
 * lost due to bit errors, interference...
 *
 * The decider module only handles messages from lower layers. All
 * messages from upper layers are directly passed to the snrEval layer
 * and cannot be processed in the decider module
 *
 * This is the basic decider module which does not really decide
 * anything. It only provides the basic functionality which all
 * decider modules should have, namely message de- & encapsulation
 * (For further information about the functionality of the physical
 * layer modules and the formats used for communication in between
 * them have a look at "The Design of a Mobility Framework in OMNeT++"
 * paper)
 *
 * Every own decider module class should be derived from this class
 * and only the handle*Msg functions may be redefined for your own
 * needs. The other functions should usually NOT be changed.
 *
 * All decider modules should assume bits as a unit for the length
 * fields.
 *
 * @ingroup decider
 * @ingroup basicModules
 * @author Marc Löbbers, Daniel Willkomm
 */
class INET_API BasicDecider : public BasicModule
{
  protected:
    /** @brief gate id*/
    /*@{*/
    int uppergateOut;
    int lowergateIn;
    /*@}*/

    /** @brief statistics*/
    /*@{*/
    unsigned long numRcvd;
    unsigned long numSentUp;
    /*@}*/

  public:
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
     * @brief Handle self messages such as timer...
     *
     * Define this function if you want to timer or other kinds of self
     * messages
     */
    virtual void handleSelfMsg(cMessage *msg){delete msg;};

    /**
     * @brief In this function the decision whether a frame is received
     * correctly or not is made.
     */
    virtual void  handleLowerMsg(AirFrame*, SnrList&);

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
    /** @brief Sends a message to the upper layer*/
    virtual void sendUp(AirFrame*);
    /*@}*/

};
#endif


