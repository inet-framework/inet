//
// Copyright (C) OpenSim Ltd.
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

#ifndef __INET_IEEE80211MGMTFRAMESERIALIZER_H
#define __INET_IEEE80211MGMTFRAMESERIALIZER_H

#include "inet/common/packet/serializer/FieldsChunkSerializer.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"

namespace inet {

namespace ieee80211 {

/**
 * Converts between Ieee80211AuthenticationFrame and binary network byte order IEEE 802.11 Authentication frame.
 */
class INET_API Ieee80211AuthenticationFrameSerializer : public FieldsChunkSerializer
{
  protected:
    virtual void serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const override;
    virtual const Ptr<Chunk> deserialize(MemoryInputStream& stream) const override;

  public:
    Ieee80211AuthenticationFrameSerializer() : FieldsChunkSerializer() {}
};

/**
 * Converts between Ieee80211DeauthenticationFrame and binary network byte order IEEE 802.11 Deauthentication frame.
 */
class INET_API Ieee80211DeauthenticationFrameSerializer : public FieldsChunkSerializer
{
  protected:
    virtual void serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const override;
    virtual const Ptr<Chunk> deserialize(MemoryInputStream& stream) const override;

  public:
    Ieee80211DeauthenticationFrameSerializer() : FieldsChunkSerializer() {}
};

/**
 * Converts between Ieee80211DisassociationFrame and binary network byte order IEEE 802.11 Disassociation frame.
 */
class INET_API Ieee80211DisassociationFrameSerializer : public FieldsChunkSerializer
{
  protected:
    virtual void serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const override;
    virtual const Ptr<Chunk> deserialize(MemoryInputStream& stream) const override;

  public:
    Ieee80211DisassociationFrameSerializer() : FieldsChunkSerializer() {}
};

/**
 * Converts between Ieee80211ProbeRequestFrame and binary network byte order IEEE 802.11 Probe Request frame.
 */
class INET_API Ieee80211ProbeRequestFrameSerializer : public FieldsChunkSerializer
{
  protected:
    virtual void serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const override;
    virtual const Ptr<Chunk> deserialize(MemoryInputStream& stream) const override;

  public:
    Ieee80211ProbeRequestFrameSerializer() : FieldsChunkSerializer() {}
};

/**
 * Converts between Ieee80211AssociationRequestFrame and binary network byte order IEEE 802.11 Association Request frame.
 */
class INET_API Ieee80211AssociationRequestFrameSerializer : public FieldsChunkSerializer
{
  protected:
    virtual void serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const override;
    virtual const Ptr<Chunk> deserialize(MemoryInputStream& stream) const override;

  public:
    Ieee80211AssociationRequestFrameSerializer() : FieldsChunkSerializer() {}
};

/**
 * Converts between Ieee80211ReassociationRequestFrame and binary network byte order IEEE 802.11 Reassotiation Request frame.
 */
class INET_API Ieee80211ReassociationRequestFrameSerializer : public FieldsChunkSerializer
{
  protected:
    virtual void serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const override;
    virtual const Ptr<Chunk> deserialize(MemoryInputStream& stream) const override;

  public:
    Ieee80211ReassociationRequestFrameSerializer() : FieldsChunkSerializer() {}
};

/**
 * Converts between Ieee80211AssociationResponseFrame and binary network byte order IEEE 802.11 Association Response frame.
 */
class INET_API Ieee80211AssociationResponseFrameSerializer : public FieldsChunkSerializer
{
  protected:
    virtual void serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const override;
    virtual const Ptr<Chunk> deserialize(MemoryInputStream& stream) const override;

  public:
    Ieee80211AssociationResponseFrameSerializer() : FieldsChunkSerializer() {}
};

/**
 * Converts between Ieee80211ReassociationResponseFrame and binary network byte order IEEE 802.11 Reassociation Response frame.
 */
class INET_API Ieee80211ReassociationResponseFrameSerializer : public FieldsChunkSerializer
{
  protected:
    virtual void serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const override;
    virtual const Ptr<Chunk> deserialize(MemoryInputStream& stream) const override;

  public:
    Ieee80211ReassociationResponseFrameSerializer() : FieldsChunkSerializer() {}
};

/**
 * Converts between Ieee80211BeaconFrame and binary network byte order IEEE 802.11 Beacon frame.
 */
class INET_API Ieee80211BeaconFrameSerializer : public FieldsChunkSerializer
{
  protected:
    virtual void serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const override;
    virtual const Ptr<Chunk> deserialize(MemoryInputStream& stream) const override;

  public:
    Ieee80211BeaconFrameSerializer() : FieldsChunkSerializer() {}
};

/**
 * Converts between Ieee80211ProbeResponseFrame and binary network byte order IEEE 802.11 Probe Response frame.
 */
class INET_API Ieee80211ProbeResponseFrameSerializer : public FieldsChunkSerializer
{
  protected:
    virtual void serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const override;
    virtual const Ptr<Chunk> deserialize(MemoryInputStream& stream) const override;

  public:
    Ieee80211ProbeResponseFrameSerializer() : FieldsChunkSerializer() {}
};

} // namespace ieee80211

} // namespace inet

#endif // ifndef __INET_IEEE80211MGMTFRAMESERIALIZER_H
