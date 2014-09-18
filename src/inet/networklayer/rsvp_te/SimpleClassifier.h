//
// (C) 2005 Vojtech Janota
//
// This library is free software, you can redistribute it
// and/or modify
// it under  the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation;
// either version 2 of the License, or any later version.
// The library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//

#ifndef __INET_SIMPLECLASSIFIER_H
#define __INET_SIMPLECLASSIFIER_H

#include <vector>
#include <string>

#include "inet/common/INETDefs.h"

#include "inet/networklayer/mpls/ConstType.h"
#include "inet/networklayer/contract/ipv4/IPv4Address.h"
#include "inet/networklayer/ipv4/IPv4Datagram.h"
#include "inet/common/scenario/IScriptable.h"
#include "inet/networklayer/rsvp_te/IRSVPClassifier.h"
#include "inet/networklayer/mpls/LIBTable.h"
#include "inet/networklayer/rsvp_te/IntServ.h"

namespace inet {

class RSVP;

/**
 * TODO documentation
 */
class INET_API SimpleClassifier : public cSimpleModule, public IScriptable, public IRSVPClassifier
{
  public:
    struct FECEntry
    {
        int id;

        IPv4Address src;
        IPv4Address dest;

        SessionObj_t session;
        SenderTemplateObj_t sender;

        int inLabel;
    };

  protected:
    IPv4Address routerId;
    int maxLabel;

    std::vector<FECEntry> bindings;
    LIBTable *lt;
    RSVP *rsvp;

  public:
    SimpleClassifier() {}

  protected:
    virtual void initialize(int stage);
    virtual int numInitStages() const { return NUM_INIT_STAGES; }
    virtual void handleMessage(cMessage *msg);

    // IScriptable implementation
    virtual void processCommand(const cXMLElement& node);

    // IRSVPClassifier implementation
    virtual bool lookupLabel(IPv4Datagram *ipdatagram, LabelOpVector& outLabel, std::string& outInterface, int& color);
    virtual void bind(const SessionObj_t& session, const SenderTemplateObj_t& sender, int inLabel);

  protected:
    virtual void readTableFromXML(const cXMLElement *fectable);
    virtual void readItemFromXML(const cXMLElement *fec);
    std::vector<FECEntry>::iterator findFEC(int fecid);
};

} // namespace inet

#endif // ifndef __INET_SIMPLECLASSIFIER_H

