//
// Copyright (C) 2004 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//


#include <stdio.h>
#include "stlwatch.h"


//
// Internal
//
class cVectorWatchDescriptor : public cStructDescriptor
{
  public:
    cVectorWatchDescriptor(void *p=NULL);
    virtual ~cVectorWatchDescriptor();
    cVectorWatchDescriptor& operator=(const cVectorWatchDescriptor& other);
    virtual cObject *dup() const {return new cVectorWatchDescriptor(*this);}

    virtual int getFieldCount();
    virtual const char *getFieldName(int field);
    virtual int getFieldType(int field);
    virtual const char *getFieldTypeString(int field);
    virtual const char *getFieldEnumName(int field);
    virtual int getArraySize(int field);

    virtual bool getFieldAsString(int field, int i, char *resultbuf, int bufsize);
    virtual bool setFieldAsString(int field, int i, const char *value);

    virtual const char *getFieldStructName(int field);
    virtual void *getFieldStructPointer(int field, int i);
    virtual sFieldWrapper *getFieldWrapper(int field, int i);
};

Register_Class(cVectorWatchDescriptor);

cVectorWatchDescriptor::cVectorWatchDescriptor(void *p) : cStructDescriptor(p)
{
}

cVectorWatchDescriptor::~cVectorWatchDescriptor()
{
}

int cVectorWatchDescriptor::getFieldCount()
{
    return 1;
}

int cVectorWatchDescriptor::getFieldType(int field)
{
    switch (field) {
        case 0: return FT_BASIC_ARRAY;
        default: return FT_INVALID;
    }
}

const char *cVectorWatchDescriptor::getFieldName(int field)
{
    cVectorWatcherBase *pp = (cVectorWatcherBase *)p;
    switch (field) {
        case 0: return pp->name();
        default: return NULL;
    }
}

const char *cVectorWatchDescriptor::getFieldTypeString(int field)
{
    cVectorWatcherBase *pp = (cVectorWatcherBase *)p;
    switch (field) {
        case 0: return pp->elemTypeName();
        default: return NULL;
    }
}

const char *cVectorWatchDescriptor::getFieldEnumName(int field)
{
    switch (field) {
        default: return NULL;
    }
}

int cVectorWatchDescriptor::getArraySize(int field)
{
    cVectorWatcherBase *pp = (cVectorWatcherBase *)p;
    switch (field) {
        case 0: return pp->size();
        default: return 0;
    }
}

bool cVectorWatchDescriptor::getFieldAsString(int field, int i, char *resultbuf, int bufsize)
{
    cVectorWatcherBase *pp = (cVectorWatcherBase *)p;
    switch (field) {
        case 0: oppstring2string(pp->at(i).c_str(),resultbuf,bufsize); return true;
        default: return false;
    }
}

bool cVectorWatchDescriptor::setFieldAsString(int field, int i, const char *value)
{
    cVectorWatcherBase *pp = (cVectorWatcherBase *)p;
    switch (field) {
        case 0: return false; // not supported for now
        default: return false;
    }
}

const char *cVectorWatchDescriptor::getFieldStructName(int field)
{
    switch (field) {
        default: return NULL;
    }
}

void *cVectorWatchDescriptor::getFieldStructPointer(int field, int i)
{
    cVectorWatcherBase *pp = (cVectorWatcherBase *)p;
    switch (field) {
        default: return NULL;
    }
}

sFieldWrapper *cVectorWatchDescriptor::getFieldWrapper(int field, int i)
{
    return NULL;
}

//--------------------------------

void cVectorWatcherBase::info(char *buf)
{
    sprintf(buf,"size=%d", size());
}

string cVectorWatcherBase::detailedInfo() const
{
    stringstream out;
    int n = size()<=3 ? size() : 3;
    for (int i=0; i<n; i++)
        out << fullName() << "[" << i << "]=" << at(i) << "\n";
    if (size()>3)
        out << "...\n";
    return out.str();
}

cStructDescriptor *cVectorWatcherBase::createDescriptor()
{
    cStructDescriptor *sd = new cVectorWatchDescriptor();
    sd->setStruct(this);
    return sd;
}

