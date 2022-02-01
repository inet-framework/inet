//
// Copyright (C) 2005 Vojtech Janota
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_RSVPCLASSIFIER_H
#define __INET_RSVPCLASSIFIER_H

#include <string>
#include <vector>

#include "inet/common/ModuleRefByPar.h"
#include "inet/common/scenario/IScriptable.h"
#include "inet/networklayer/contract/ipv4/Ipv4Address.h"
#include "inet/networklayer/ipv4/Ipv4Header_m.h"
#include "inet/networklayer/mpls/ConstType.h"
#include "inet/networklayer/mpls/LibTable.h"
#include "inet/networklayer/rsvpte/IRsvpClassifier.h"
#include "inet/networklayer/rsvpte/IntServ_m.h"

namespace inet {

class RsvpTe;

/**
 * TODO documentation
 */
class INET_API RsvpClassifier : public cSimpleModule, public IScriptable, public IRsvpClassifier
{
  public:
    struct FecEntry {
        int id;

        Ipv4Address src;
        Ipv4Address dest;

        SessionObj session;
        SenderTemplateObj sender;

        int inLabel;
    };

  protected:
    Ipv4Address routerId;
    int maxLabel = 0;

    std::vector<FecEntry> bindings;
    ModuleRefByPar<LibTable> lt;
    ModuleRefByPar<RsvpTe> rsvp;

  public:
    RsvpClassifier() {}

  protected:
    virtual void initialize(int stage) override;
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void handleMessage(cMessage *msg) override;

    // IScriptable implementation
    virtual void processCommand(const cXMLElement& node) override;

    // IRsvpClassifier implementation
    virtual bool lookupLabel(Packet *ipdatagram, LabelOpVector& outLabel, std::string& outInterface, int& color) override;
    virtual void bind(const SessionObj& session, const SenderTemplateObj& sender, int inLabel) override;

  protected:
    virtual void readTableFromXML(const cXMLElement *fectable);
    virtual void readItemFromXML(const cXMLElement *fec);
    std::vector<FecEntry>::iterator findFEC(int fecid);
};

} // namespace inet

#endif

