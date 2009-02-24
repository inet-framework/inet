/* -*- mode:c++ -*- ********************************************************
 * file:        BasicModule.h
 *
 * author:      Steffen Sroka
 *              Andreas Koepke
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


#ifndef BASIC_MODULE_H
#define BASIC_MODULE_H

#include <omnetpp.h>
#include "NotificationBoard.h"
#include "NotifierConsts.h"

#ifndef EV
#define EV (ev.isDisabled()||!debug) ? (std::ostream&)ev : ev << logName() << "::" << getClassName() << ": "
#endif


/**
 * @brief Base class for all simple modules of a host that want to have
 * access to the NotificationBoard module.
 *
 * The basic module additionally provides a function findHost() which
 * returns a pointer to the host module.
 *
 * There is no Define_Module() for this class because we use
 * BasicModule only as a base class to derive all other
 * module. There will never be a stand-alone BasicModule module
 * (and that is why there is no Define_Module() and no .ned file for
 * BasicModule).
 *
 * @see NotificationBoard
 * @ingroup basicModules
 *
 * @author Steffen Sroka
 * @author Andreas Koepke
 */
class INET_API BasicModule: public cSimpleModule, public INotifiable
{
  protected:
    /** @brief Cached pointer to the NotificationBoard module*/
    NotificationBoard *nb;

    /** @brief Debug switch for the core modules*/
    bool coreDebug;

    /** @brief Debug switch for all other modules*/
    bool debug;

    /** @brief Log name of the host module*/
    std::string loggingName;

  protected:
    /** @brief Function to get a pointer to the host module*/
    virtual cModule *findHost(void) const;

    /** @brief Function to get the logging name of id*/
    const char* getLogName(int);

  protected:
    /** @brief Basic initialization for all modules */
    virtual void initialize(int);

    /**
     * @brief Divide initialization into two stages
     *
     * In the first stage (stage==0), modules subscribe to notification
     * categories at NotificationBoard. The first notifications
     * (e.g. about the initial values of some variables such as RadioState)
     * should take place earliest in the second stage (stage==1),
     * when everyone interested in them has already subscribed.
     */
    virtual int numInitStages() const {return 2;}

    /**
     * @brief Function to get the logging name of the host
     *
     * The logging name is the ned module name of the host (unless the
     * host ned variable loggingName is specified). It can be used for
     * logging messages to simplify debugging in TKEnv.
     */
    const char* logName(void) const
    {
      return loggingName.c_str();
    };

    /**
     * @brief Called by the NotificationBoard whenever a change of a category occurs
     * to which we have subscribed. Redefined from INotifiable.
     */
    virtual void receiveChangeNotification(int category, const cPolymorphic *details) {}
};

#endif


