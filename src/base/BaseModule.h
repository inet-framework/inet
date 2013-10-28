/* -*- mode:c++ -*- ********************************************************
 * file:        BaseModule.h
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

#ifndef BASE_MODULE_H
#define BASE_MODULE_H

#include <sstream>
#include <omnetpp.h>

#include "INETDefs.h"

/**
 * @brief Base class for all simple modules of a host.
 *
 * The base module additionally provides a function findHost which
 * returns a pointer to the host module and a function hostIndex to
 * return the index of the host module.
 *
 * There will never be a stand-alone BaseModule module.
 *
 * @ingroup baseModules
 *
 * @author Steffen Sroka
 * @author Andreas Koepke
 */
class INET_API BaseModule: public cSimpleModule, public cListener {
protected:

    /** @brief Function to get a pointer to the host module*/
    cModule* findHost(void);
    const cModule* findHost(void) const;
    /** @brief Function to get the logging name of id*/
    //std::string getLogName(int);

private:
    /** @brief Copy constructor is not allowed.
     */
    BaseModule(const BaseModule&);
    /** @brief Assignment operator is not allowed.
     */
    BaseModule& operator=(const BaseModule&);

  public:

    BaseModule();
    BaseModule(unsigned stacksize);

    /** @brief Basic initialization for all modules */
    virtual void initialize(int);

    /**
     * @brief Divide initialization into two stages
     *
     * In the first stage (stage==0), modules subscribe to notification.
     * The first notifications (e.g. about the initial
     * values of some variables such as RadioState) should take place earliest
     * in the second stage (stage==1), when everyone interested in them has
     * already subscribed.
     * Further one should try to keep calls to other modules out of stage 0 to
     * assure that the other module had at least once the chance to initialize
     * itself in stage 0.
     */
    virtual int numInitStages() const {
    	return 2;
    }

    /**
     * @brief Get a reference to the local node module
     */
    const cModule* getNode() const {
    	return findHost();
    }

    /**
     * @brief Called by the signaling mechanism whenever a change of a category occurs
     * to which we have subscribed.
     * In this base class just handle the host state switching and
     * some debug notifications
     */
    virtual void receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj);
};

#endif
