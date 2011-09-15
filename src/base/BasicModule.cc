/* -*- mode:c++ -*- ********************************************************
 * file:        BasicModule.cc
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


#include <iostream>
#include "BasicModule.h"

#define coreEV (ev.isDisabled()||!coreDebug) ? (std::ostream&)ev : EV << loggingName << "::BasicModule: "

/**
 * Subscription to NotificationBoard should be in stage==0, and firing
 * notifications in stage==1 or later.
 *
 * NOTE: You have to call this in the initialize() function of the
 * inherited class!
 */
void BasicModule::initialize(int stage)
{
    cModule *host = findHost(false);

    if (stage == 0)
    {

        if (hasPar("coreDebug"))
            coreDebug = par("coreDebug").boolValue();
        else
            coreDebug = false;
        if (hasPar("debug"))
            debug = par("debug").boolValue();
        else
            debug = false;

        if (host) {
            // get the logging name of the host
            if (host->hasPar("logName"))
                loggingName = host->par("logName").stringValue();
            else
                loggingName = host->getName();
            char tmp[8];
            sprintf(&tmp[0], "[%d]", host->getIndex());
            loggingName += tmp;
        }
    }
}

cModule *BasicModule::findHost(bool errorIfNotFound) const
{
    cModule *mod;
    for (mod = getParentModule(); mod != 0; mod = mod->getParentModule()) {
        cProperties *properties = mod->getProperties();
        if (properties && properties->getAsBool("node"))
            break;
    }
    if (errorIfNotFound && !mod)
        error("findHost(): host module not found (it should have a property named node)");

    return mod;
}

/**
 * This function returns the logging name of the module with the
 * specified id. It can be used for logging messages to simplify
 * debugging in TKEnv.
 *
 * Only supports ids from simple module derived from the BasicModule
 * or the nic compound module id.
 *
 * @param id Id of the module for the desired logging name
 * @return logging name of module id or NULL if not found
 * @sa logName
 */
const char *BasicModule::getLogName(int id)
{
    BasicModule *mod;
    mod = (BasicModule *) simulation.getModule(id);
    if (mod->isSimple())
        return mod->logName();
    else if (mod->getSubmodule("snrEval"))
        return ((BasicModule *) mod->getSubmodule("snrEval"))->logName();
    else if (mod->getSubmodule("phy"))
        return ((BasicModule *) mod->getSubmodule("phy"))->logName();
    else
        return NULL;
};
