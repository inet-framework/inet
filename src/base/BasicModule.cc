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

/**
 * Subscription to NotificationBoard should be in stage==0, and firing
 * notifications in stage==1 or later.
 *
 * NOTE: You have to call this in the initialize() function of the
 * inherited class!
 */
void BasicModule::initialize(int stage)
{
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
