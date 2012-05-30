//
// Copyright (C) 2011 Zoltan Bojthe
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


// Disable warnings about unused variables, empty switch stmts, etc:
#ifdef _MSC_VER
#  pragma warning(disable:4101)
#  pragma warning(disable:4065)
#endif

#include <iostream>
#include <sstream>

#include "aodv_msg_struct.h"

#if (OMNETPP_VERSION < 0x0403)
#define getFieldArraySize    getArraySize
#endif

// Template rule which fires if a struct or class doesn't have operator<<
template<typename T>
std::ostream& operator<<(std::ostream& out,const T&) {return out;}

// Another default rule (prevents compiler from choosing base class' doPacking())
template<typename T>
void doPacking(cCommBuffer *, T& t) {
    throw cRuntimeError("Parsim error: no doPacking() function for type %s or its base class (check .msg and _m.cc/h files!)",opp_typename(typeid(t)));
}

template<typename T>
void doUnpacking(cCommBuffer *, T& t) {
    throw cRuntimeError("Parsim error: no doUnpacking() function for type %s or its base class (check .msg and _m.cc/h files!)",opp_typename(typeid(t)));
}




class AODV_extDescriptor : public cClassDescriptor
{
  public:
    AODV_extDescriptor();
    virtual ~AODV_extDescriptor();

    virtual bool doesSupport(cObject *obj) const;
    virtual const char *getProperty(const char *propertyname) const;
    virtual int getFieldCount(void *object) const;
    virtual const char *getFieldName(void *object, int field) const;
    virtual int findField(void *object, const char *fieldName) const;
    virtual unsigned int getFieldTypeFlags(void *object, int field) const;
    virtual const char *getFieldTypeString(void *object, int field) const;
    virtual const char *getFieldProperty(void *object, int field, const char *propertyname) const;
    virtual int getFieldArraySize(void *object, int field) const;
    virtual std::string getFieldAsString(void *object, int field, int i) const;
    virtual bool setFieldAsString(void *object, int field, int i, const char *value) const;

    virtual const char *getFieldStructName(void *object, int field) const;
    virtual void *getFieldStructPointer(void *object, int field, int i) const;

  private:
    enum {
        FLD_type = 0,
        FLD_pointer,
        __FLD_NUM
    };
};

Register_ClassDescriptor(AODV_extDescriptor);

AODV_extDescriptor::AODV_extDescriptor() : cClassDescriptor("AODV_ext", "cObject")
{
}

AODV_extDescriptor::~AODV_extDescriptor()
{
}

bool AODV_extDescriptor::doesSupport(cObject *obj) const
{
    return dynamic_cast<AODV_ext *>(obj)!=NULL;
}

const char *AODV_extDescriptor::getProperty(const char *propertyname) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->getProperty(propertyname) : NULL;
}

int AODV_extDescriptor::getFieldCount(void *object) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? __FLD_NUM+basedesc->getFieldCount(object) : __FLD_NUM;
}

unsigned int AODV_extDescriptor::getFieldTypeFlags(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldTypeFlags(object, field);
        field -= basedesc->getFieldCount(object);
    }
    switch (field) {
        case FLD_type: return FD_ISEDITABLE;
        case FLD_pointer: return FD_ISARRAY | FD_ISEDITABLE;
        default: return 0;
    }
}

const char *AODV_extDescriptor::getFieldName(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldName(object, field);
        field -= basedesc->getFieldCount(object);
    }
    switch (field) {
        case FLD_type: return "type";
        case FLD_pointer: return "pointer";
        default: return NULL;
    }
}

int AODV_extDescriptor::findField(void *object, const char *fieldName) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    int base = basedesc ? basedesc->getFieldCount(object) : 0;
    if (fieldName[0]=='t' && strcmp(fieldName, "type")==0) return base+FLD_type;
    if (fieldName[0]=='p' && strcmp(fieldName, "pointer")==0) return base+FLD_pointer;
    return basedesc ? basedesc->findField(object, fieldName) : -1;
}

const char *AODV_extDescriptor::getFieldTypeString(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldTypeString(object, field);
        field -= basedesc->getFieldCount(object);
    }
    switch (field) {
        case FLD_type: return "uint8_t";
        case FLD_pointer: return "char";
        default: return NULL;
    }
}

const char *AODV_extDescriptor::getFieldProperty(void *object, int field, const char *propertyname) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldProperty(object, field, propertyname);
        field -= basedesc->getFieldCount(object);
    }
    switch (field) {
        default: return NULL;
    }
}

int AODV_extDescriptor::getFieldArraySize(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldArraySize(object, field);
        field -= basedesc->getFieldCount(object);
    }
    AODV_ext *pp = (AODV_ext *)object; (void)pp;
    switch (field) {
        case FLD_pointer: return pp->length;
        default: return 0;
    }
}

std::string AODV_extDescriptor::getFieldAsString(void *object, int field, int i) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldAsString(object,field,i);
        field -= basedesc->getFieldCount(object);
    }
    AODV_ext *pp = (AODV_ext *)object; (void)pp;
    switch (field) {
        case FLD_type: return ulong2string(pp->type);
        case FLD_pointer: return long2string(pp->pointer[i]);
        default: return "";
    }
}

bool AODV_extDescriptor::setFieldAsString(void *object, int field, int i, const char *value) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->setFieldAsString(object,field,i,value);
        field -= basedesc->getFieldCount(object);
    }
    AODV_ext *pp = (AODV_ext *)object; (void)pp;
    switch (field) {
        case FLD_type: pp->type = (string2ulong(value)); return true;
        case FLD_pointer: pp->pointer[i] = (string2long(value)); return true;
        default: return false;
    }
}

const char *AODV_extDescriptor::getFieldStructName(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldStructName(object, field);
        field -= basedesc->getFieldCount(object);
    }
    switch (field) {
        default: return NULL;
    }
}

void *AODV_extDescriptor::getFieldStructPointer(void *object, int field, int i) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldStructPointer(object, field, i);
        field -= basedesc->getFieldCount(object);
    }
    AODV_ext *pp = (AODV_ext *)object; (void)pp;
    switch (field) {
        default: return NULL;
    }
}

Register_Class(AODV_msg);

class AODV_msgDescriptor : public cClassDescriptor
{
  public:
    AODV_msgDescriptor();
    virtual ~AODV_msgDescriptor();

    virtual bool doesSupport(cObject *obj) const;
    virtual const char *getProperty(const char *propertyname) const;
    virtual int getFieldCount(void *object) const;
    virtual const char *getFieldName(void *object, int field) const;
    virtual int findField(void *object, const char *fieldName) const;
    virtual unsigned int getFieldTypeFlags(void *object, int field) const;
    virtual const char *getFieldTypeString(void *object, int field) const;
    virtual const char *getFieldProperty(void *object, int field, const char *propertyname) const;
    virtual int getFieldArraySize(void *object, int field) const;
    virtual std::string getFieldAsString(void *object, int field, int i) const;
    virtual bool setFieldAsString(void *object, int field, int i, const char *value) const;

    virtual const char *getFieldStructName(void *object, int field) const;
    virtual void *getFieldStructPointer(void *object, int field, int i) const;

  private:
    enum {
        FLD_type = 0,
        FLD_ttl,
        FLD_extension,
        __FLD_NUM
    };
};

Register_ClassDescriptor(AODV_msgDescriptor);

AODV_msgDescriptor::AODV_msgDescriptor() : cClassDescriptor("AODV_msg", "cPacket")
{
}

AODV_msgDescriptor::~AODV_msgDescriptor()
{
}

bool AODV_msgDescriptor::doesSupport(cObject *obj) const
{
    return dynamic_cast<AODV_msg *>(obj)!=NULL;
}

const char *AODV_msgDescriptor::getProperty(const char *propertyname) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->getProperty(propertyname) : NULL;
}

int AODV_msgDescriptor::getFieldCount(void *object) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? __FLD_NUM+basedesc->getFieldCount(object) : __FLD_NUM;
}

unsigned int AODV_msgDescriptor::getFieldTypeFlags(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldTypeFlags(object, field);
        field -= basedesc->getFieldCount(object);
    }
    switch (field) {
        case FLD_type: return FD_ISEDITABLE;
        case FLD_ttl: return FD_ISEDITABLE;
        case FLD_extension: return FD_ISARRAY | FD_ISCOMPOUND;
        default: return 0;
    }
}

const char *AODV_msgDescriptor::getFieldName(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldName(object, field);
        field -= basedesc->getFieldCount(object);
    }
    switch (field) {
        case FLD_type: return "type";
        case FLD_ttl: return "ttl";
        case FLD_extension: return "extension";
        default: return NULL;
    }
}

int AODV_msgDescriptor::findField(void *object, const char *fieldName) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    int base = basedesc ? basedesc->getFieldCount(object) : 0;
    if (fieldName[0]=='t' && strcmp(fieldName, "type")==0) return base+FLD_type;
    if (fieldName[0]=='t' && strcmp(fieldName, "ttl")==0) return base+FLD_ttl;
    if (fieldName[0]=='e' && strcmp(fieldName, "extension")==0) return base+FLD_extension;
    return basedesc ? basedesc->findField(object, fieldName) : -1;
}

const char *AODV_msgDescriptor::getFieldTypeString(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldTypeString(object, field);
        field -= basedesc->getFieldCount(object);
    }
    switch (field) {
        case FLD_type: return "uint8_t";
        case FLD_ttl: return "uint8_t";
        case FLD_extension: return "AODV_ext";
        default: return NULL;
    }
}

const char *AODV_msgDescriptor::getFieldProperty(void *object, int field, const char *propertyname) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldProperty(object, field, propertyname);
        field -= basedesc->getFieldCount(object);
    }
    switch (field) {
        default: return NULL;
    }
}

int AODV_msgDescriptor::getFieldArraySize(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldArraySize(object, field);
        field -= basedesc->getFieldCount(object);
    }
    AODV_msg *pp = (AODV_msg *)object; (void)pp;
    switch (field) {
        case FLD_extension: return pp->getNumExtension();
        default: return 0;
    }
}

std::string AODV_msgDescriptor::getFieldAsString(void *object, int field, int i) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldAsString(object,field,i);
        field -= basedesc->getFieldCount(object);
    }
    AODV_msg *pp = (AODV_msg *)object; (void)pp;
    switch (field) {
        case FLD_type: return ulong2string(pp->type);
        case FLD_ttl: return ulong2string(pp->ttl);
        case FLD_extension: {std::stringstream out; out << *(pp->getExtension(i)); return out.str();}
        default: return "";
    }
}

bool AODV_msgDescriptor::setFieldAsString(void *object, int field, int i, const char *value) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->setFieldAsString(object,field,i,value);
        field -= basedesc->getFieldCount(object);
    }
    AODV_msg *pp = (AODV_msg *)object; (void)pp;
    switch (field) {
        case FLD_type: pp->type = (string2ulong(value)); return true;
        case FLD_ttl: pp->ttl = (string2ulong(value)); return true;
        default: return false;
    }
}

const char *AODV_msgDescriptor::getFieldStructName(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldStructName(object, field);
        field -= basedesc->getFieldCount(object);
    }
    switch (field) {
        case FLD_extension: return "AODV_ext";
        default: return NULL;
    }
}

void *AODV_msgDescriptor::getFieldStructPointer(void *object, int field, int i) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldStructPointer(object, field, i);
        field -= basedesc->getFieldCount(object);
    }
    AODV_msg *pp = (AODV_msg *)object; (void)pp;
    switch (field) {
        case FLD_extension: return (void *)(pp->getExtension(i)); break;
        default: return NULL;
    }
}

class RERR_udestDescriptor : public cClassDescriptor
{
  public:
    RERR_udestDescriptor();
    virtual ~RERR_udestDescriptor();

    virtual bool doesSupport(cObject *obj) const;
    virtual const char *getProperty(const char *propertyname) const;
    virtual int getFieldCount(void *object) const;
    virtual const char *getFieldName(void *object, int field) const;
    virtual int findField(void *object, const char *fieldName) const;
    virtual unsigned int getFieldTypeFlags(void *object, int field) const;
    virtual const char *getFieldTypeString(void *object, int field) const;
    virtual const char *getFieldProperty(void *object, int field, const char *propertyname) const;
    virtual int getFieldArraySize(void *object, int field) const;

    virtual std::string getFieldAsString(void *object, int field, int i) const;
    virtual bool setFieldAsString(void *object, int field, int i, const char *value) const;

    virtual const char *getFieldStructName(void *object, int field) const;
    virtual void *getFieldStructPointer(void *object, int field, int i) const;

  private:
    enum {
        FLD_dest_addr = 0,
        FLD_dest_seqno,
        __FLD_NUM
    };
};

Register_ClassDescriptor(RERR_udestDescriptor);

RERR_udestDescriptor::RERR_udestDescriptor() : cClassDescriptor("RERR_udest", "")
{
}

RERR_udestDescriptor::~RERR_udestDescriptor()
{
}

bool RERR_udestDescriptor::doesSupport(cObject *obj) const
{
    return dynamic_cast<RERR_udest *>(obj)!=NULL;
}

const char *RERR_udestDescriptor::getProperty(const char *propertyname) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->getProperty(propertyname) : NULL;
}

int RERR_udestDescriptor::getFieldCount(void *object) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? __FLD_NUM+basedesc->getFieldCount(object) : __FLD_NUM;
}

unsigned int RERR_udestDescriptor::getFieldTypeFlags(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldTypeFlags(object, field);
        field -= basedesc->getFieldCount(object);
    }
    switch (field) {
        case FLD_dest_addr: return FD_ISCOMPOUND;
        case FLD_dest_seqno: return FD_ISEDITABLE;
        default: return 0;
    }
}

const char *RERR_udestDescriptor::getFieldName(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldName(object, field);
        field -= basedesc->getFieldCount(object);
    }
    switch (field) {
        case FLD_dest_addr: return "dest_addr";
        case FLD_dest_seqno: return "dest_seqno";
        default: return NULL;
    }
}

int RERR_udestDescriptor::findField(void *object, const char *fieldName) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    int base = basedesc ? basedesc->getFieldCount(object) : 0;
    if (fieldName[0]=='d' && strcmp(fieldName, "dest_addr")==0) return base+FLD_dest_addr;
    if (fieldName[0]=='d' && strcmp(fieldName, "dest_seqno")==0) return base+FLD_dest_seqno;
    return basedesc ? basedesc->findField(object, fieldName) : -1;
}

const char *RERR_udestDescriptor::getFieldTypeString(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldTypeString(object, field);
        field -= basedesc->getFieldCount(object);
    }
    switch (field) {
        case FLD_dest_addr: return "Uint128";
        case FLD_dest_seqno: return "uint32_t";
        default: return NULL;
    }
}

const char *RERR_udestDescriptor::getFieldProperty(void *object, int field, const char *propertyname) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldProperty(object, field, propertyname);
        field -= basedesc->getFieldCount(object);
    }
    switch (field) {
        default: return NULL;
    }
}

int RERR_udestDescriptor::getFieldArraySize(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldArraySize(object, field);
        field -= basedesc->getFieldCount(object);
    }
    RERR_udest *pp = (RERR_udest *)object; (void)pp;
    switch (field) {
        default: return 0;
    }
}

std::string RERR_udestDescriptor::getFieldAsString(void *object, int field, int i) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldAsString(object,field,i);
        field -= basedesc->getFieldCount(object);
    }
    RERR_udest *pp = (RERR_udest *)object; (void)pp;
    switch (field) {
        case FLD_dest_addr: {std::stringstream out; out << pp->dest_addr; return out.str();}
        case FLD_dest_seqno: return ulong2string(pp->dest_seqno);
        default: return "";
    }
}

bool RERR_udestDescriptor::setFieldAsString(void *object, int field, int i, const char *value) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->setFieldAsString(object,field,i,value);
        field -= basedesc->getFieldCount(object);
    }
    RERR_udest *pp = (RERR_udest *)object; (void)pp;
    switch (field) {
        case FLD_dest_seqno: pp->dest_seqno = string2ulong(value); return true;
        default: return false;
    }
}

const char *RERR_udestDescriptor::getFieldStructName(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldStructName(object, field);
        field -= basedesc->getFieldCount(object);
    }
    switch (field) {
        case FLD_dest_addr: return "Uint128";
        default: return NULL;
    }
}

void *RERR_udestDescriptor::getFieldStructPointer(void *object, int field, int i) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldStructPointer(object, field, i);
        field -= basedesc->getFieldCount(object);
    }
    RERR_udest *pp = (RERR_udest *)object; (void)pp;
    switch (field) {
        case FLD_dest_addr: return (void *)(&pp->dest_addr); break;
        default: return NULL;
    }
}

Register_Class(RERR);

class RERRDescriptor : public cClassDescriptor
{
  public:
    RERRDescriptor();
    virtual ~RERRDescriptor();

    virtual bool doesSupport(cObject *obj) const;
    virtual const char *getProperty(const char *propertyname) const;
    virtual int getFieldCount(void *object) const;
    virtual const char *getFieldName(void *object, int field) const;
    virtual int findField(void *object, const char *fieldName) const;
    virtual unsigned int getFieldTypeFlags(void *object, int field) const;
    virtual const char *getFieldTypeString(void *object, int field) const;
    virtual const char *getFieldProperty(void *object, int field, const char *propertyname) const;
    virtual int getFieldArraySize(void *object, int field) const;

    virtual std::string getFieldAsString(void *object, int field, int i) const;
    virtual bool setFieldAsString(void *object, int field, int i, const char *value) const;

    virtual const char *getFieldStructName(void *object, int field) const;
    virtual void *getFieldStructPointer(void *object, int field, int i) const;

  private:
    enum {
        FLD_res1 = 0,
        FLD_n,
        FLD_res2,
        FLD_udest,
        __FLD_NUM
    };
};

Register_ClassDescriptor(RERRDescriptor);

RERRDescriptor::RERRDescriptor() : cClassDescriptor("RERR", "AODV_msg")
{
}

RERRDescriptor::~RERRDescriptor()
{
}

bool RERRDescriptor::doesSupport(cObject *obj) const
{
    return dynamic_cast<RERR *>(obj)!=NULL;
}

const char *RERRDescriptor::getProperty(const char *propertyname) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->getProperty(propertyname) : NULL;
}

int RERRDescriptor::getFieldCount(void *object) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? 4+basedesc->getFieldCount(object) : 4;
}

unsigned int RERRDescriptor::getFieldTypeFlags(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldTypeFlags(object, field);
        field -= basedesc->getFieldCount(object);
    }
    switch (field) {
        case FLD_res1: return FD_ISEDITABLE;
        case FLD_n: return FD_ISEDITABLE;
        case FLD_res2: return FD_ISEDITABLE;
        case FLD_udest: return FD_ISARRAY | FD_ISCOMPOUND;
        default: return 0;
    }
}

const char *RERRDescriptor::getFieldName(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldName(object, field);
        field -= basedesc->getFieldCount(object);
    }
    switch (field) {
        case FLD_res1: return "res1";
        case FLD_n: return "n";
        case FLD_res2: return "res2";
        case FLD_udest: return "udest";
        default: return NULL;
    }
}

int RERRDescriptor::findField(void *object, const char *fieldName) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    int base = basedesc ? basedesc->getFieldCount(object) : 0;
    if (fieldName[0]=='r' && strcmp(fieldName, "res1")==0) return base+FLD_res1;
    if (fieldName[0]=='n' && strcmp(fieldName, "n")==0) return base+FLD_n;
    if (fieldName[0]=='r' && strcmp(fieldName, "res2")==0) return base+FLD_res2;
    if (fieldName[0]=='u' && strcmp(fieldName, "udest")==0) return base+FLD_udest;
    return basedesc ? basedesc->findField(object, fieldName) : -1;
}

const char *RERRDescriptor::getFieldTypeString(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldTypeString(object, field);
        field -= basedesc->getFieldCount(object);
    }
    switch (field) {
        case FLD_res1: return "unsigned short";
        case FLD_n: return "unsigned short";
        case FLD_res2: return "unsigned short";
        case FLD_udest: return "RERR_udest";
        default: return NULL;
    }
}

const char *RERRDescriptor::getFieldProperty(void *object, int field, const char *propertyname) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldProperty(object, field, propertyname);
        field -= basedesc->getFieldCount(object);
    }
    switch (field) {
        default: return NULL;
    }
}

int RERRDescriptor::getFieldArraySize(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldArraySize(object, field);
        field -= basedesc->getFieldCount(object);
    }
    RERR *pp = (RERR *)object; (void)pp;
    switch (field) {
        case FLD_udest: return pp->dest_count;
        default: return 0;
    }
}

std::string RERRDescriptor::getFieldAsString(void *object, int field, int i) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldAsString(object,field,i);
        field -= basedesc->getFieldCount(object);
    }
    RERR *pp = (RERR *)object; (void)pp;
    switch (field) {
        case FLD_res1: return ulong2string(pp->res1);
        case FLD_n: return ulong2string(pp->n);
        case FLD_res2: return ulong2string(pp->res2);
        case FLD_udest: {std::stringstream out; out << *(pp->getUdest(i)); return out.str();}
        default: return "";
    }
}

bool RERRDescriptor::setFieldAsString(void *object, int field, int i, const char *value) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->setFieldAsString(object,field,i,value);
        field -= basedesc->getFieldCount(object);
    }
    RERR *pp = (RERR *)object; (void)pp;
    switch (field) {
        case FLD_res1: pp->res1 = (string2ulong(value)); return true;
        case FLD_n: pp->n = (string2ulong(value)); return true;
        case FLD_res2: pp->res2 = (string2ulong(value)); return true;
        default: return false;
    }
}

const char *RERRDescriptor::getFieldStructName(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldStructName(object, field);
        field -= basedesc->getFieldCount(object);
    }
    switch (field) {
        case FLD_udest: return "RERR_udest";
        default: return NULL;
    }
}

void *RERRDescriptor::getFieldStructPointer(void *object, int field, int i) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldStructPointer(object, field, i);
        field -= basedesc->getFieldCount(object);
    }
    RERR *pp = (RERR *)object; (void)pp;
    switch (field) {
        case 3: return (void *)(pp->getUdest(i)); break;
        default: return NULL;
    }
}

Register_Class(RREP);

class RREPDescriptor : public cClassDescriptor
{
  public:
    RREPDescriptor();
    virtual ~RREPDescriptor();

    virtual bool doesSupport(cObject *obj) const;
    virtual const char *getProperty(const char *propertyname) const;
    virtual int getFieldCount(void *object) const;
    virtual const char *getFieldName(void *object, int field) const;
    virtual int findField(void *object, const char *fieldName) const;
    virtual unsigned int getFieldTypeFlags(void *object, int field) const;
    virtual const char *getFieldTypeString(void *object, int field) const;
    virtual const char *getFieldProperty(void *object, int field, const char *propertyname) const;
    virtual int getFieldArraySize(void *object, int field) const;

    virtual std::string getFieldAsString(void *object, int field, int i) const;
    virtual bool setFieldAsString(void *object, int field, int i, const char *value) const;

    virtual const char *getFieldStructName(void *object, int field) const;
    virtual void *getFieldStructPointer(void *object, int field, int i) const;

  private:
    enum {
        FLD_res1 = 0,
        FLD_a,
        FLD_r,
        FLD_prefix,
        FLD_res2,
        FLD_hcnt,
        FLD_dest_addr,
        FLD_dest_seqno,
        FLD_orig_addr,
        FLD_lifetime,
        __FLD_NUM
    };
};

Register_ClassDescriptor(RREPDescriptor);

RREPDescriptor::RREPDescriptor() : cClassDescriptor("RREP", "AODV_msg")
{
}

RREPDescriptor::~RREPDescriptor()
{
}

bool RREPDescriptor::doesSupport(cObject *obj) const
{
    return dynamic_cast<RREP *>(obj)!=NULL;
}

const char *RREPDescriptor::getProperty(const char *propertyname) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->getProperty(propertyname) : NULL;
}

int RREPDescriptor::getFieldCount(void *object) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? 10+basedesc->getFieldCount(object) : 10;
}

unsigned int RREPDescriptor::getFieldTypeFlags(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldTypeFlags(object, field);
        field -= basedesc->getFieldCount(object);
    }
    switch (field) {
        case FLD_res1: return FD_ISEDITABLE;
        case FLD_a: return FD_ISEDITABLE;
        case FLD_r: return FD_ISEDITABLE;
        case FLD_prefix: return FD_ISEDITABLE;
        case FLD_res2: return FD_ISEDITABLE;
        case FLD_hcnt: return FD_ISEDITABLE;
        case FLD_dest_addr: return FD_ISCOMPOUND;
        case FLD_dest_seqno: return FD_ISEDITABLE;
        case FLD_orig_addr: return FD_ISCOMPOUND;
        case FLD_lifetime: return FD_ISEDITABLE;
        default: return 0;
    }
}

const char *RREPDescriptor::getFieldName(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldName(object, field);
        field -= basedesc->getFieldCount(object);
    }
    switch (field) {
        case FLD_res1: return "res1";
        case FLD_a: return "a";
        case FLD_r: return "r";
        case FLD_prefix: return "prefix";
        case FLD_res2: return "res2";
        case FLD_hcnt: return "hcnt";
        case FLD_dest_addr: return "dest_addr";
        case FLD_dest_seqno: return "dest_seqno";
        case FLD_orig_addr: return "orig_addr";
        case FLD_lifetime: return "lifetime";
        default: return NULL;
    }
}

int RREPDescriptor::findField(void *object, const char *fieldName) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    int base = basedesc ? basedesc->getFieldCount(object) : 0;
    if (fieldName[0]=='r' && strcmp(fieldName, "res1")==0) return base+FLD_res1;
    if (fieldName[0]=='a' && strcmp(fieldName, "a")==0) return base+FLD_a;
    if (fieldName[0]=='r' && strcmp(fieldName, "r")==0) return base+FLD_r;
    if (fieldName[0]=='p' && strcmp(fieldName, "prefix")==0) return base+FLD_prefix;
    if (fieldName[0]=='r' && strcmp(fieldName, "res2")==0) return base+FLD_res2;
    if (fieldName[0]=='h' && strcmp(fieldName, "hcnt")==0) return base+FLD_hcnt;
    if (fieldName[0]=='d' && strcmp(fieldName, "dest_addr")==0) return base+FLD_dest_addr;
    if (fieldName[0]=='d' && strcmp(fieldName, "dest_seqno")==0) return base+FLD_dest_seqno;
    if (fieldName[0]=='o' && strcmp(fieldName, "orig_addr")==0) return base+FLD_orig_addr;
    if (fieldName[0]=='l' && strcmp(fieldName, "lifetime")==0) return base+FLD_lifetime;
    return basedesc ? basedesc->findField(object, fieldName) : -1;
}

const char *RREPDescriptor::getFieldTypeString(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldTypeString(object, field);
        field -= basedesc->getFieldCount(object);
    }
    switch (field) {
        case FLD_res1: return "uint16_t";
        case FLD_a: return "uint16_t";
        case FLD_r: return "uint16_t";
        case FLD_prefix: return "uint16_t";
        case FLD_res2: return "uint16_t";
        case FLD_hcnt: return "uint8_t";
        case FLD_dest_addr: return "Uint128";
        case FLD_dest_seqno: return "uint32_t";
        case FLD_orig_addr: return "Uint128";
        case FLD_lifetime: return "uint32_t";
        default: return NULL;
    }
}

const char *RREPDescriptor::getFieldProperty(void *object, int field, const char *propertyname) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldProperty(object, field, propertyname);
        field -= basedesc->getFieldCount(object);
    }
    switch (field) {
        default: return NULL;
    }
}

int RREPDescriptor::getFieldArraySize(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldArraySize(object, field);
        field -= basedesc->getFieldCount(object);
    }
    RREP *pp = (RREP *)object; (void)pp;
    switch (field) {
        default: return 0;
    }
}

std::string RREPDescriptor::getFieldAsString(void *object, int field, int i) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldAsString(object,field,i);
        field -= basedesc->getFieldCount(object);
    }
    RREP *pp = (RREP *)object; (void)pp;
    switch (field) {
        case FLD_res1: return ulong2string(pp->res1);
        case FLD_a: return ulong2string(pp->a);
        case FLD_r: return ulong2string(pp->r);
        case FLD_prefix: return ulong2string(pp->prefix);
        case FLD_res2: return ulong2string(pp->res2);
        case FLD_hcnt: return ulong2string(pp->hcnt);
        case FLD_dest_addr: {std::stringstream out; out << pp->dest_addr; return out.str();}
        case FLD_dest_seqno: return ulong2string(pp->dest_seqno);
        case FLD_orig_addr: {std::stringstream out; out << pp->orig_addr; return out.str();}
        case FLD_lifetime: return ulong2string(pp->lifetime);
        default: return "";
    }
}

bool RREPDescriptor::setFieldAsString(void *object, int field, int i, const char *value) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->setFieldAsString(object,field,i,value);
        field -= basedesc->getFieldCount(object);
    }
    RREP *pp = (RREP *)object; (void)pp;
    switch (field) {
        case FLD_res1: pp->res1 = (string2ulong(value)); return true;
        case FLD_a: pp->a = (string2ulong(value)); return true;
        case FLD_r: pp->r = (string2ulong(value)); return true;
        case FLD_prefix: pp->prefix = (string2ulong(value)); return true;
        case FLD_res2: pp->res2 = (string2ulong(value)); return true;
        case FLD_hcnt: pp->hcnt = (string2ulong(value)); return true;
        case FLD_dest_seqno: pp->dest_seqno = (string2ulong(value)); return true;
        case FLD_lifetime: pp->lifetime = (string2ulong(value)); return true;
        default: return false;
    }
}

const char *RREPDescriptor::getFieldStructName(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldStructName(object, field);
        field -= basedesc->getFieldCount(object);
    }
    switch (field) {
        case FLD_dest_addr: return "Uint128";
        case FLD_orig_addr: return "Uint128";
        default: return NULL;
    }
}

void *RREPDescriptor::getFieldStructPointer(void *object, int field, int i) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldStructPointer(object, field, i);
        field -= basedesc->getFieldCount(object);
    }
    RREP *pp = (RREP *)object; (void)pp;
    switch (field) {
        case FLD_dest_addr: return (void *)(&pp->dest_addr); break;
        case FLD_orig_addr: return (void *)(&pp->orig_addr); break;
        default: return NULL;
    }
}

Register_Class(RREP_ack);

class RREP_ackDescriptor : public cClassDescriptor
{
  public:
    RREP_ackDescriptor();
    virtual ~RREP_ackDescriptor();

    virtual bool doesSupport(cObject *obj) const;
    virtual const char *getProperty(const char *propertyname) const;
    virtual int getFieldCount(void *object) const;
    virtual const char *getFieldName(void *object, int field) const;
    virtual int findField(void *object, const char *fieldName) const;
    virtual unsigned int getFieldTypeFlags(void *object, int field) const;
    virtual const char *getFieldTypeString(void *object, int field) const;
    virtual const char *getFieldProperty(void *object, int field, const char *propertyname) const;
    virtual int getFieldArraySize(void *object, int field) const;

    virtual std::string getFieldAsString(void *object, int field, int i) const;
    virtual bool setFieldAsString(void *object, int field, int i, const char *value) const;

    virtual const char *getFieldStructName(void *object, int field) const;
    virtual void *getFieldStructPointer(void *object, int field, int i) const;

  private:
    enum {
        FLD_reserved = 0,
        __FLD_NUM
    };
};

Register_ClassDescriptor(RREP_ackDescriptor);

RREP_ackDescriptor::RREP_ackDescriptor() : cClassDescriptor("RREP_ack", "AODV_msg")
{
}

RREP_ackDescriptor::~RREP_ackDescriptor()
{
}

bool RREP_ackDescriptor::doesSupport(cObject *obj) const
{
    return dynamic_cast<RREP_ack *>(obj)!=NULL;
}

const char *RREP_ackDescriptor::getProperty(const char *propertyname) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->getProperty(propertyname) : NULL;
}

int RREP_ackDescriptor::getFieldCount(void *object) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? __FLD_NUM+basedesc->getFieldCount(object) : __FLD_NUM;
}

unsigned int RREP_ackDescriptor::getFieldTypeFlags(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldTypeFlags(object, field);
        field -= basedesc->getFieldCount(object);
    }
    switch (field) {
        case FLD_reserved: return FD_ISEDITABLE;
        default: return 0;
    }
}

const char *RREP_ackDescriptor::getFieldName(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldName(object, field);
        field -= basedesc->getFieldCount(object);
    }
    switch (field) {
        case FLD_reserved: return "reserved";
        default: return NULL;
    }
}

int RREP_ackDescriptor::findField(void *object, const char *fieldName) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    int base = basedesc ? basedesc->getFieldCount(object) : 0;
    if (fieldName[0]=='r' && strcmp(fieldName, "reserved")==0) return base+FLD_reserved;
    return basedesc ? basedesc->findField(object, fieldName) : -1;
}

const char *RREP_ackDescriptor::getFieldTypeString(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldTypeString(object, field);
        field -= basedesc->getFieldCount(object);
    }
    switch (field) {
        case FLD_reserved: return "uint8_t";
        default: return NULL;
    }
}

const char *RREP_ackDescriptor::getFieldProperty(void *object, int field, const char *propertyname) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldProperty(object, field, propertyname);
        field -= basedesc->getFieldCount(object);
    }
    switch (field) {
        default: return NULL;
    }
}

int RREP_ackDescriptor::getFieldArraySize(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldArraySize(object, field);
        field -= basedesc->getFieldCount(object);
    }
    RREP_ack *pp = (RREP_ack *)object; (void)pp;
    switch (field) {
        default: return 0;
    }
}

std::string RREP_ackDescriptor::getFieldAsString(void *object, int field, int i) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldAsString(object,field,i);
        field -= basedesc->getFieldCount(object);
    }
    RREP_ack *pp = (RREP_ack *)object; (void)pp;
    switch (field) {
        case FLD_reserved: return ulong2string(pp->reserved);
        default: return "";
    }
}

bool RREP_ackDescriptor::setFieldAsString(void *object, int field, int i, const char *value) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->setFieldAsString(object,field,i,value);
        field -= basedesc->getFieldCount(object);
    }
    RREP_ack *pp = (RREP_ack *)object; (void)pp;
    switch (field) {
        case FLD_reserved: pp->reserved = (string2ulong(value)); return true;
        default: return false;
    }
}

const char *RREP_ackDescriptor::getFieldStructName(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldStructName(object, field);
        field -= basedesc->getFieldCount(object);
    }
    switch (field) {
        case FLD_reserved: return NULL;
        default: return NULL;
    }
}

void *RREP_ackDescriptor::getFieldStructPointer(void *object, int field, int i) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldStructPointer(object, field, i);
        field -= basedesc->getFieldCount(object);
    }
    RREP_ack *pp = (RREP_ack *)object; (void)pp;
    switch (field) {
        default: return NULL;
    }
}

Register_Class(RREQ);

class RREQDescriptor : public cClassDescriptor
{
  public:
    RREQDescriptor();
    virtual ~RREQDescriptor();

    virtual bool doesSupport(cObject *obj) const;
    virtual const char *getProperty(const char *propertyname) const;
    virtual int getFieldCount(void *object) const;
    virtual const char *getFieldName(void *object, int field) const;
    virtual int findField(void *object, const char *fieldName) const;
    virtual unsigned int getFieldTypeFlags(void *object, int field) const;
    virtual const char *getFieldTypeString(void *object, int field) const;
    virtual const char *getFieldProperty(void *object, int field, const char *propertyname) const;
    virtual int getFieldArraySize(void *object, int field) const;

    virtual std::string getFieldAsString(void *object, int field, int i) const;
    virtual bool setFieldAsString(void *object, int field, int i, const char *value) const;

    virtual const char *getFieldStructName(void *object, int field) const;
    virtual void *getFieldStructPointer(void *object, int field, int i) const;

  private:
    enum {
        FLD_j = 0,
        FLD_r,
        FLD_g,
        FLD_d,
        FLD_res1,
        FLD_res2,
        FLD_hcnt,
        FLD_rreq_id,
        FLD_dest_addr,
        FLD_dest_seqno,
        FLD_orig_addr,
        FLD_orig_seqno,
        __FLD_NUM
    };
};

Register_ClassDescriptor(RREQDescriptor);

RREQDescriptor::RREQDescriptor() : cClassDescriptor("RREQ", "AODV_msg")
{
}

RREQDescriptor::~RREQDescriptor()
{
}

bool RREQDescriptor::doesSupport(cObject *obj) const
{
    return dynamic_cast<RREQ *>(obj)!=NULL;
}

const char *RREQDescriptor::getProperty(const char *propertyname) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->getProperty(propertyname) : NULL;
}

int RREQDescriptor::getFieldCount(void *object) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? __FLD_NUM+basedesc->getFieldCount(object) : __FLD_NUM;
}

unsigned int RREQDescriptor::getFieldTypeFlags(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldTypeFlags(object, field);
        field -= basedesc->getFieldCount(object);
    }
    switch (field) {
        case FLD_j: return FD_ISEDITABLE;
        case FLD_r: return FD_ISEDITABLE;
        case FLD_g: return FD_ISEDITABLE;
        case FLD_d: return FD_ISEDITABLE;
        case FLD_res1: return FD_ISEDITABLE;
        case FLD_res2: return FD_ISEDITABLE;
        case FLD_hcnt: return FD_ISEDITABLE;
        case FLD_rreq_id: return FD_ISEDITABLE;
        case FLD_dest_addr: return FD_ISCOMPOUND;
        case FLD_dest_seqno: return FD_ISEDITABLE;
        case FLD_orig_addr: return FD_ISCOMPOUND;
        case FLD_orig_seqno: return FD_ISEDITABLE;
        default: return 0;
    }
}

const char *RREQDescriptor::getFieldName(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldName(object, field);
        field -= basedesc->getFieldCount(object);
    }
    switch (field) {
        case FLD_j: return "j";
        case FLD_r: return "r";
        case FLD_g: return "g";
        case FLD_d: return "d";
        case FLD_res1: return "res1";
        case FLD_res2: return "res2";
        case FLD_hcnt: return "hcnt";
        case FLD_rreq_id: return "rreq_id";
        case FLD_dest_addr: return "dest_addr";
        case FLD_dest_seqno: return "dest_seqno";
        case FLD_orig_addr: return "orig_addr";
        case FLD_orig_seqno: return "orig_seqno";
        default: return NULL;
    }
}

int RREQDescriptor::findField(void *object, const char *fieldName) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    int base = basedesc ? basedesc->getFieldCount(object) : 0;
    if (fieldName[0]=='j' && strcmp(fieldName, "j")==0) return base+FLD_j;
    if (fieldName[0]=='r' && strcmp(fieldName, "r")==0) return base+FLD_r;
    if (fieldName[0]=='g' && strcmp(fieldName, "g")==0) return base+FLD_g;
    if (fieldName[0]=='d' && strcmp(fieldName, "d")==0) return base+FLD_d;
    if (fieldName[0]=='r' && strcmp(fieldName, "res1")==0) return base+FLD_res1;
    if (fieldName[0]=='r' && strcmp(fieldName, "res2")==0) return base+FLD_res2;
    if (fieldName[0]=='h' && strcmp(fieldName, "hcnt")==0) return base+FLD_hcnt;
    if (fieldName[0]=='r' && strcmp(fieldName, "rreq_id")==0) return base+FLD_rreq_id;
    if (fieldName[0]=='d' && strcmp(fieldName, "dest_addr")==0) return base+FLD_dest_addr;
    if (fieldName[0]=='d' && strcmp(fieldName, "dest_seqno")==0) return base+FLD_dest_seqno;
    if (fieldName[0]=='o' && strcmp(fieldName, "orig_addr")==0) return base+FLD_orig_addr;
    if (fieldName[0]=='o' && strcmp(fieldName, "orig_seqno")==0) return base+FLD_orig_seqno;
    return basedesc ? basedesc->findField(object, fieldName) : -1;
}

const char *RREQDescriptor::getFieldTypeString(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldTypeString(object, field);
        field -= basedesc->getFieldCount(object);
    }
    switch (field) {
        case FLD_j: return "uint8_t";
        case FLD_r: return "uint8_t";
        case FLD_g: return "uint8_t";
        case FLD_d: return "uint8_t";
        case FLD_res1: return "uint8_t";
        case FLD_res2: return "uint8_t";
        case FLD_hcnt: return "uint8_t";
        case FLD_rreq_id: return "uint32_t";
        case FLD_dest_addr: return "Uint128";
        case FLD_dest_seqno: return "uint32_t";
        case FLD_orig_addr: return "Uint128";
        case FLD_orig_seqno: return "uint32_t";
        default: return NULL;
    }
}

const char *RREQDescriptor::getFieldProperty(void *object, int field, const char *propertyname) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldProperty(object, field, propertyname);
        field -= basedesc->getFieldCount(object);
    }
    switch (field) {
        default: return NULL;
    }
}

int RREQDescriptor::getFieldArraySize(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldArraySize(object, field);
        field -= basedesc->getFieldCount(object);
    }
    RREQ *pp = (RREQ *)object; (void)pp;
    switch (field) {
        default: return 0;
    }
}

std::string RREQDescriptor::getFieldAsString(void *object, int field, int i) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldAsString(object,field,i);
        field -= basedesc->getFieldCount(object);
    }
    RREQ *pp = (RREQ *)object; (void)pp;
    switch (field) {
        case FLD_j: return ulong2string(pp->j);
        case FLD_r: return ulong2string(pp->r);
        case FLD_g: return ulong2string(pp->g);
        case FLD_d: return ulong2string(pp->d);
        case FLD_res1: return ulong2string(pp->res1);
        case FLD_res2: return ulong2string(pp->res2);
        case FLD_hcnt: return ulong2string(pp->hcnt);
        case FLD_rreq_id: return ulong2string(pp->rreq_id);
        case FLD_dest_addr: {std::stringstream out; out << pp->dest_addr; return out.str();}
        case FLD_dest_seqno: return ulong2string(pp->dest_seqno);
        case FLD_orig_addr: {std::stringstream out; out << pp->orig_addr; return out.str();}
        case FLD_orig_seqno: return ulong2string(pp->orig_seqno);
        default: return "";
    }
}

bool RREQDescriptor::setFieldAsString(void *object, int field, int i, const char *value) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->setFieldAsString(object,field,i,value);
        field -= basedesc->getFieldCount(object);
    }
    RREQ *pp = (RREQ *)object; (void)pp;
    switch (field) {
        case FLD_j: pp->j = (string2ulong(value)); return true;
        case FLD_r: pp->r = (string2ulong(value)); return true;
        case FLD_g: pp->g = (string2ulong(value)); return true;
        case FLD_d: pp->d = (string2ulong(value)); return true;
        case FLD_res1: pp->res1 = (string2ulong(value)); return true;
        case FLD_res2: pp->res2 = (string2ulong(value)); return true;
        case FLD_hcnt: pp->hcnt = (string2ulong(value)); return true;
        case FLD_rreq_id: pp->rreq_id = (string2ulong(value)); return true;
        case FLD_dest_seqno: pp->dest_seqno = (string2ulong(value)); return true;
        case FLD_orig_seqno: pp->orig_seqno = (string2ulong(value)); return true;
        default: return false;
    }
}

const char *RREQDescriptor::getFieldStructName(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldStructName(object, field);
        field -= basedesc->getFieldCount(object);
    }
    switch (field) {
        case FLD_dest_addr: return "Uint128";
        case FLD_orig_addr: return "Uint128";
        default: return NULL;
    }
}

void *RREQDescriptor::getFieldStructPointer(void *object, int field, int i) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldStructPointer(object, field, i);
        field -= basedesc->getFieldCount(object);
    }
    RREQ *pp = (RREQ *)object; (void)pp;
    switch (field) {
        case FLD_dest_addr: return (void *)(&pp->dest_addr); break;
        case FLD_orig_addr: return (void *)(&pp->orig_addr); break;
        default: return NULL;
    }
}

