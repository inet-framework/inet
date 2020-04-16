//
// Copyright (C) 2014 OpenSim Ltd.
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

#ifndef __INET_IEEE80211PHYHEADERSERIALIZER_H
#define __INET_IEEE80211PHYHEADERSERIALIZER_H

#include "inet/common/packet/serializer/FieldsChunkSerializer.h"

namespace inet {

namespace physicallayer {

/**
 * Converts between Ieee80211FhssPhyHeader and binary network byte order IEEE 802.11 FHSS PHY header.
 */
class INET_API Ieee80211FhssPhyHeaderSerializer : public FieldsChunkSerializer
{
  protected:
    virtual void serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const override;
    virtual const Ptr<Chunk> deserialize(MemoryInputStream& stream) const override;

  public:
    Ieee80211FhssPhyHeaderSerializer() : FieldsChunkSerializer() {}
};

/**
 * Converts between Ieee80211IrPhyHeader and binary network byte order IEEE 802.11 IR PHY header.
 */
class INET_API Ieee80211IrPhyHeaderSerializer : public FieldsChunkSerializer
{
  protected:
    virtual void serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const override;
    virtual const Ptr<Chunk> deserialize(MemoryInputStream& stream) const override;

  public:
    Ieee80211IrPhyHeaderSerializer() : FieldsChunkSerializer() {}
};

/**
 * Converts between Ieee80211DsssPhyHeader and binary network byte order IEEE 802.11 DSSS PHY header.
 */
class INET_API Ieee80211DsssPhyHeaderSerializer : public FieldsChunkSerializer
{
  protected:
    virtual void serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const override;
    virtual const Ptr<Chunk> deserialize(MemoryInputStream& stream) const override;

  public:
    Ieee80211DsssPhyHeaderSerializer() : FieldsChunkSerializer() {}
};

/**
 * Converts between Ieee80211HrDsssPhyHeader and binary network byte order IEEE 802.11 HR/DSSS PHY header.
 */
class INET_API Ieee80211HrDsssPhyHeaderSerializer : public FieldsChunkSerializer
{
  protected:
    virtual void serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const override;
    virtual const Ptr<Chunk> deserialize(MemoryInputStream& stream) const override;

  public:
    Ieee80211HrDsssPhyHeaderSerializer() : FieldsChunkSerializer() {}
};

/**
 * Converts between Ieee80211OfdmPhyHeader and binary network byte order IEEE 802.11 OFDM PHY header.
 */
class INET_API Ieee80211OfdmPhyHeaderSerializer : public FieldsChunkSerializer
{
  protected:
    virtual void serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const override;
    virtual const Ptr<Chunk> deserialize(MemoryInputStream& stream) const override;

  public:
    Ieee80211OfdmPhyHeaderSerializer() : FieldsChunkSerializer() {}
};

/**
 * Converts between Ieee80211ErpOfdmPhyHeader and binary network byte order IEEE 802.11 ERP OFDM PHY header.
 */
class INET_API Ieee80211ErpOfdmPhyHeaderSerializer : public FieldsChunkSerializer
{
  protected:
    virtual void serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const override;
    virtual const Ptr<Chunk> deserialize(MemoryInputStream& stream) const override;

  public:
    Ieee80211ErpOfdmPhyHeaderSerializer() : FieldsChunkSerializer() {}
};

/**
 * Converts between Ieee80211HtPhyHeader and binary network byte order IEEE 802.11 HT PHY header.
 */
class INET_API Ieee80211HtPhyHeaderSerializer : public FieldsChunkSerializer
{
  protected:
    virtual void serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const override;
    virtual const Ptr<Chunk> deserialize(MemoryInputStream& stream) const override;

  public:
    Ieee80211HtPhyHeaderSerializer() : FieldsChunkSerializer() {}
};

/**
 * Converts between Ieee80211VhtPhyHeader and binary network byte order IEEE 802.11 VHT PHY header.
 */
class INET_API Ieee80211VhtPhyHeaderSerializer : public FieldsChunkSerializer
{
  protected:
    virtual void serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const override;
    virtual const Ptr<Chunk> deserialize(MemoryInputStream& stream) const override;

  public:
    Ieee80211VhtPhyHeaderSerializer() : FieldsChunkSerializer() {}
};

} // namespace physicallayer

} // namespace inet

#endif // ifndef __INET_IEEE80211PHYHEADERSERIALIZER_H
