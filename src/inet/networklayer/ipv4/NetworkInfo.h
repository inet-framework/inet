//
// Copyright (C) 2007 Vojtech Janota
//
// SPDX-License-Identifier: GPL-2.0-or-later
//
//

#ifndef __INET_NETWORKINFO_H
#define __INET_NETWORKINFO_H

#include "inet/common/scenario/IScriptable.h"

namespace inet {

/**
 * TODO documentation
 */
class INET_API NetworkInfo : public cSimpleModule, public IScriptable
{
  protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;

    // IScriptable implementation
    virtual void processCommand(const cXMLElement& node) override;

  protected:
    virtual void dumpRoutingInfo(cModule *target, const char *filename, bool append, bool compat);
};

} // namespace inet

#endif

