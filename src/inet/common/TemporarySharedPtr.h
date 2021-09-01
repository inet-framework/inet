//
// Copyright (C) 2020 OpenSim Ltd.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

#ifndef __INET_TEMPORARYSHAREDPTR_H
#define __INET_TEMPORARYSHAREDPTR_H

#include "inet/common/Ptr.h"

namespace inet {

template<typename T>
class TemporarySharedPtr;

/**
 * This class provides support for Qtenv inspectors for objects referenced by shared pointers.
 */
template<typename T>
class TemporarySharedPtrClassDescriptor : public cClassDescriptor
{
  protected:
    cClassDescriptor *classDescriptor;

  protected:
#if OMNETPP_VERSION >= 0x0600 && OMNETPP_BUILDNUM >= 1525
    const Ptr<const T> getSharedPtr(any_ptr object) const { return check_and_cast<TemporarySharedPtr<T> *>(fromAnyPtr<cObject>(object))->getObject(); }
    any_ptr getObjectPointer(any_ptr object) const { return toAnyPtr(const_cast<T *>(getSharedPtr(object).get())); }
#else
    const Ptr<const T> getSharedPtr(void *object) const { return static_cast<TemporarySharedPtr<T> *>(object)->getObject(); }
    T *getObjectPointer(void *object) const { return const_cast<T *>(getSharedPtr(object).get()); }
#endif

  public:
    TemporarySharedPtrClassDescriptor(cClassDescriptor *classDescriptor) : cClassDescriptor(classDescriptor->getClassName()), classDescriptor(classDescriptor) { }

#if OMNETPP_VERSION >= 0x0600 && OMNETPP_BUILDNUM >= 1525
    virtual bool doesSupport(cObject *object) const override { return classDescriptor->doesSupport(fromAnyPtr<cObject>(getObjectPointer(toAnyPtr(object)))); }
#else
    virtual bool doesSupport(cObject *object) const override { return classDescriptor->doesSupport(getObjectPointer(object)); }
#endif
    virtual cClassDescriptor *getBaseClassDescriptor() const override { return classDescriptor->getBaseClassDescriptor(); }
    virtual const char **getPropertyNames() const override { return classDescriptor->getPropertyNames(); }
    virtual const char *getProperty(const char *propertyname) const override { return classDescriptor->getProperty(propertyname); }
    virtual int getFieldCount() const override { return classDescriptor->getFieldCount(); }
    virtual const char *getFieldName(int field) const override { return classDescriptor->getFieldName(field); }
    virtual int findField(const char *fieldName) const override { return classDescriptor->findField(fieldName); }
    virtual unsigned int getFieldTypeFlags(int field) const override { return classDescriptor->getFieldTypeFlags(field); }
    virtual const char *getFieldDeclaredOn(int field) const override { return classDescriptor->getFieldDeclaredOn(field); }
    virtual const char *getFieldTypeString(int field) const override { return classDescriptor->getFieldTypeString(field); }
    virtual const char **getFieldPropertyNames(int field) const override { return classDescriptor->getFieldPropertyNames(field); }
    virtual const char *getFieldProperty(int field, const char *propertyname) const override { return classDescriptor->getFieldProperty(field, propertyname); }
    virtual int getFieldArraySize(any_ptr object, int field) const override { return classDescriptor->getFieldArraySize(getObjectPointer(object), field); }
#if OMNETPP_VERSION >= 0x0600
    virtual void setFieldArraySize(any_ptr object, int field, int size) const override { classDescriptor->setFieldArraySize(getObjectPointer(object), field, size); }
#endif
    virtual const char *getFieldDynamicTypeString(any_ptr object, int field, int i) const override { return classDescriptor->getFieldDynamicTypeString(getObjectPointer(object), field, i); }
    virtual std::string getFieldValueAsString(any_ptr object, int field, int i) const override { return classDescriptor->getFieldValueAsString(getObjectPointer(object), field, i); }
#if OMNETPP_VERSION >= 0x0600
    virtual void setFieldValueAsString(any_ptr object, int field, int i, const char *value) const override { classDescriptor->setFieldValueAsString(getObjectPointer(object), field, i, value); }
#else
    virtual bool setFieldValueAsString(void *object, int field, int i, const char *value) const override { return classDescriptor->setFieldValueAsString(getObjectPointer(object), field, i, value); }
#endif
    virtual const char *getFieldStructName(int field) const override { return classDescriptor->getFieldStructName(field); }
    virtual any_ptr getFieldStructValuePointer(any_ptr object, int field, int i) const override { return classDescriptor->getFieldStructValuePointer(getObjectPointer(object), field, i); }
#if OMNETPP_VERSION >= 0x0600
    virtual void setFieldStructValuePointer(any_ptr object, int field, int i, any_ptr ptr) const override { classDescriptor->setFieldStructValuePointer(getObjectPointer(object), field, i, ptr); }
#if OMNETPP_BUILDNUM >= 1526
    virtual cValue getFieldValue(any_ptr object, int field, int i) const override { return classDescriptor->getFieldValue(object, field, i); }
    virtual void setFieldValue(any_ptr object, int field, int i, const cValue& value) const override { classDescriptor->setFieldValue(object, field, i, value); }
#endif
#endif
};

/**
 * This class provides support for Qtenv inspectors for objects referenced by shared pointers.
 */
// TODO subclass from cTemporary when available to fix leaking memory from the inspector
template<typename T>
class TemporarySharedPtr : public cObject
{
  private:
    const Ptr<const T> object;
    TemporarySharedPtrClassDescriptor<T> *classDescriptor;

  public:
    TemporarySharedPtr(const Ptr<const T> object) : object(object), classDescriptor(new TemporarySharedPtrClassDescriptor<T>(object.get()->getDescriptor())) { }
    virtual ~TemporarySharedPtr() { delete classDescriptor; }

    const Ptr<const T>& getObject() const { return object; }
    virtual cClassDescriptor *getDescriptor() const override { return classDescriptor; }
};

} // namespace

#endif

