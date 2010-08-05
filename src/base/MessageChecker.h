#pragma once

#include "INETDefs.h"

#define BUFSIZE	4096

class INET_API MessageChecker : public cSimpleModule
/// Module for message checkout purpose
/// Check any fields of any messages
///
{
public:
	MessageChecker();

protected:
	void initialize();
	void handleMessage(cMessage *msg);
	void checkMessage(cMessage* msg);
	void forwardMessage(cMessage* msg);
	void finish();

	void checkFields(void* object, cClassDescriptor* descriptor, const cXMLElementList& msgPattern) const;

	void checkFieldValue(void* object, cClassDescriptor* descriptor, int field, cXMLAttributeMap& attr, int i = 0) const;
	void checkFieldObject(void* object, cClassDescriptor* descriptor, int field, cXMLAttributeMap& attr, const cXMLElement& pattern, int i = 0) const;
	int checkFieldArray(void* object, cClassDescriptor* descriptor, int field, cXMLAttributeMap& attr) const;
	void checkFieldValueInArray(void* object, cClassDescriptor* descriptor, int field, cXMLAttributeMap& attr) const;
	void checkFieldObjectInArray(void* object, cClassDescriptor* descriptor, int field, cXMLAttributeMap& attr, const cXMLElement& pattern) const;
	
	void checkFieldType(void* object, cClassDescriptor* descriptor, int field, cXMLAttributeMap& attrList, int i = 0) const;
	int findFieldIndex(void* object, cClassDescriptor* descriptor, const std::string& fieldName) const;

	cXMLElementList				m_checkingInfo;	/// List of checking information
	cXMLElementList::iterator	m_iterChk;		/// Interator of the list of chacking information
	unsigned					forwardedMsg;	/// Number of received and forwarded messages
	unsigned					checkedMsg;		/// Number of checked messages
};
