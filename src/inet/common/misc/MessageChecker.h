//
// Copyright (C) 2010 Helene Lageber
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_MESSAGECHECKER_H
#define __INET_MESSAGECHECKER_H

#include "inet/common/IModuleInterfaceLookup.h"
#include "inet/queueing/common/PassivePacketSinkRef.h"
#include "inet/common/INETDefs.h"

namespace inet {

using namespace inet::queueing;

#define BUFSIZE    4096

class INET_API MessageChecker : public cSimpleModule, public IPassivePacketSink, public IModuleInterfaceLookup
{
  public:
    MessageChecker();

    virtual bool canPushSomePacket(const cGate *gate) const override { return gate->isName("in"); }
    virtual bool canPushPacket(Packet *packet, const cGate *gate) const override { return gate->isName("in"); }
    virtual void pushPacket(Packet *packet, const cGate *gate) override;
    virtual void pushPacketStart(Packet *packet, const cGate *gate, bps datarate) override { throw cRuntimeError("TODO"); }
    virtual void pushPacketEnd(Packet *packet, const cGate *gate) override { throw cRuntimeError("TODO"); }
    virtual void pushPacketProgress(Packet *packet, const cGate *gate, bps datarate, b position, b extraProcessableLength = b(0)) override { throw cRuntimeError("TODO"); }

    virtual cGate *lookupModuleInterface(cGate *gate, const std::type_info& type, const cObject *arguments, int direction) override;

  protected:
    void initialize() override;
    void handleMessage(cMessage *msg) override;
    void checkMessage(cMessage *msg);
    void forwardMessage(cMessage *msg);
    void finish() override;

    void checkFields(any_ptr object, cClassDescriptor *descriptor, const cXMLElementList& msgPattern) const;

    void checkFieldValue(any_ptr object, cClassDescriptor *descriptor, int field, cXMLAttributeMap& attr, int i = 0) const;
    void checkFieldObject(any_ptr object, cClassDescriptor *descriptor, int field, cXMLAttributeMap& attr, const cXMLElement& pattern, int i = 0) const;
    int checkFieldArray(any_ptr object, cClassDescriptor *descriptor, int field, cXMLAttributeMap& attr) const;
    void checkFieldValueInArray(any_ptr object, cClassDescriptor *descriptor, int field, cXMLAttributeMap& attr) const;
    void checkFieldObjectInArray(any_ptr object, cClassDescriptor *descriptor, int field, cXMLAttributeMap& attr, const cXMLElement& pattern) const;

    void checkFieldType(any_ptr object, cClassDescriptor *descriptor, int field, cXMLAttributeMap& attrList, int i = 0) const;
    int findFieldIndex(any_ptr object, cClassDescriptor *descriptor, const std::string& fieldName) const;

    PassivePacketSinkRef outSink;
    cXMLElementList m_checkingInfo; // List of checking information
    cXMLElementList::iterator m_iterChk; // Interator of the list of chacking information
    unsigned forwardedMsg; // Number of received and forwarded messages
    unsigned checkedMsg; // Number of checked messages
};

} // namespace inet

#endif

