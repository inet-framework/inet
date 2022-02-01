//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/common/packet/chunk/FieldsChunk.h"

#include "inet/common/packet/chunk/EmptyChunk.h"
#include "inet/common/packet/chunk/SliceChunk.h"

namespace inet {

FieldsChunk::FieldsChunk() :
    Chunk(),
    chunkLength(b(-1)),
    serializedBytes(nullptr)
{
}

FieldsChunk::FieldsChunk(const FieldsChunk& other) :
    Chunk(other),
    chunkLength(other.chunkLength),
    serializedBytes(other.serializedBytes != nullptr ? new std::vector<uint8_t>(*other.serializedBytes) : nullptr)
{
}

FieldsChunk::~FieldsChunk()
{
    delete serializedBytes;
}

void FieldsChunk::parsimPack(cCommBuffer *buffer) const
{
    Chunk::parsimPack(buffer);
    buffer->pack(chunkLength.get());
}

void FieldsChunk::parsimUnpack(cCommBuffer *buffer)
{
    Chunk::parsimUnpack(buffer);
    int64_t l;
    buffer->unpack(l);
    chunkLength = b(l);
    delete serializedBytes;
    serializedBytes = nullptr;
}

void FieldsChunk::handleChange()
{
    Chunk::handleChange();
    delete serializedBytes;
    serializedBytes = nullptr;
}

bool FieldsChunk::containsSameData(const Chunk& other) const
{
    if (&other == this)
        return true;
    else if (!Chunk::containsSameData(other))
        return false;
    else {
        // KLUDGE should we generate this method from the MSG compiler?
        // this implementation returns false if it cannot determine the result correctly
        auto thisDescriptor = getDescriptor();
        auto otherDescriptor = other.getDescriptor();
        if (thisDescriptor != otherDescriptor)
            return false;
        auto thisVoidPtr = toAnyPtr(this);
        auto otherVoidPtr = toAnyPtr(static_cast<const FieldsChunk *>(&other));
        for (int field = 0; field < thisDescriptor->getFieldCount(); field++) {
            auto declaredOn = thisDescriptor->getFieldDeclaredOn(field);
            if (!strcmp(declaredOn, "omnetpp::cObject") || !strcmp(declaredOn, "inet::Chunk") || !strcmp(declaredOn, "inet::FieldsChunk"))
                continue;
            auto flags = thisDescriptor->getFieldTypeFlags(field);
            if ((flags & cClassDescriptor::FD_ISARRAY) || (flags & cClassDescriptor::FD_ISCOMPOUND) || (flags & cClassDescriptor::FD_ISPOINTER))
                return false;
            auto thisValue = thisDescriptor->getFieldValueAsString(thisVoidPtr, field, 0);
            auto otherValue = otherDescriptor->getFieldValueAsString(otherVoidPtr, field, 0);
            if (thisValue != otherValue)
                return false;
        }
        return true;
    }
}

const Ptr<Chunk> FieldsChunk::peekUnchecked(PeekPredicate predicate, PeekConverter converter, const Iterator& iterator, b length, int flags) const
{
    b chunkLength = getChunkLength();
    CHUNK_CHECK_USAGE(b(0) <= iterator.getPosition() && iterator.getPosition() <= chunkLength, "iterator is out of range");
    // 1. peeking an empty part returns nullptr
    if (length == b(0) || (iterator.getPosition() == chunkLength && length < b(0))) {
        if (predicate == nullptr || predicate(nullptr))
            return EmptyChunk::getEmptyChunk(flags);
    }
    // 2. peeking the whole part returns this chunk
    if (iterator.getPosition() == b(0) && (-length >= chunkLength || length == chunkLength)) {
        auto result = const_cast<FieldsChunk *>(this)->shared_from_this();
        if (predicate == nullptr || predicate(result))
            return result;
    }
    // 3. peeking a part from the beginning without conversion returns an incomplete copy of this chunk
    if (predicate != nullptr && predicate(const_cast<FieldsChunk *>(this)->shared_from_this()) && iterator.getPosition() == b(0)) {
        auto copy = staticPtrCast<FieldsChunk>(dupShared());
        copy->setChunkLength(length);
        copy->regionTags.copyTags(regionTags, iterator.getPosition(), b(0), length);
        copy->markIncomplete();
        return copy;
    }
    // 4. peeking without conversion returns a SliceChunk
    if (converter == nullptr)
        return peekConverted<SliceChunk>(iterator, length, flags);
    // 5. peeking with conversion
    return converter(const_cast<FieldsChunk *>(this)->shared_from_this(), iterator, length, flags);
}

std::ostream& FieldsChunk::printFieldsToStream(std::ostream& stream, int level, int evFlags) const
{
    auto className = getClassName();
    auto descriptor = getDescriptor();
    // TODO make this more sophisticated, e.g. add properties to fields to control what is printed
    if (level <= PRINT_LEVEL_DETAIL)
        for (int i = 0; i < descriptor->getFieldCount(); i++)
            if (!descriptor->getFieldIsArray(i) && !strcmp(className, descriptor->getFieldDeclaredOn(i)))
                stream << ", " << EV_BOLD << descriptor->getFieldName(i) << EV_NORMAL << " = " << descriptor->getFieldValueAsString(toAnyPtr(this), i, 0);
    return stream;
}

} // namespace

