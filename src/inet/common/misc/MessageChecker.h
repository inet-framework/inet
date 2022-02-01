//
// Copyright (C) 2010 Helene Lageber
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_MESSAGECHECKER_H
#define __INET_MESSAGECHECKER_H

#include "inet/common/INETDefs.h"

namespace inet {

#define BUFSIZE    4096

class INET_API MessageChecker : public cSimpleModule
{
  public:
    MessageChecker();

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

    cXMLElementList m_checkingInfo; // List of checking information
    cXMLElementList::iterator m_iterChk; // Interator of the list of chacking information
    unsigned forwardedMsg; // Number of received and forwarded messages
    unsigned checkedMsg; // Number of checked messages
};

} // namespace inet

#endif

