//
// Copyright (C) 2010 Helene Lageber
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
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

    cXMLElementList m_checkingInfo;    /// List of checking information
    cXMLElementList::iterator m_iterChk;    /// Interator of the list of chacking information
    unsigned forwardedMsg;    /// Number of received and forwarded messages
    unsigned checkedMsg;    /// Number of checked messages
};

} // namespace inet

#endif // ifndef __INET_MESSAGECHECKER_H

