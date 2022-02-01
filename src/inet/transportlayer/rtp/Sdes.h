//
// Copyright (C) 2001 Matthias Oppitz <Matthias.Oppitz@gmx.de>
// Copyright (C) 2010 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_SDES_H
#define __INET_SDES_H

#include "inet/common/INETDefs.h"

namespace inet {

namespace rtp {

/**
 * The class SdesItem is used for storing a source description item
 * (type of description, description string) for an Rtp end system.
 */
class INET_API SdesItem : public cObject
{
  public:
    /**
     * This enumeration holds the types of source description items
     * as defined in the RFC. In this implementation only SDES_UNDEF
     * and SDES_CNAME are usable.
     */
    enum SdesItemType {
        SDES_UNDEF = 0,
        SDES_CNAME = 1,
        SDES_NAME  = 2,
        SDES_EMAIL = 3,
        SDES_PHONE = 4,
        SDES_LOC   = 5,
        SDES_TOOL  = 6,
        SDES_NOTE  = 7,
        SDES_PRIV  = 8
    };

    /**
     * Default constructor.
     */
    SdesItem();

    /**
     * Constructor which sets the entry.
     */
    SdesItem(SdesItemType type, const char *content);

    /**
     * Copy constructor.
     */
    SdesItem(const SdesItem& sdesItem);

    /**
     * Destructor.
     */
    virtual ~SdesItem();

    /**
     * Assignment operator.
     */
    SdesItem& operator=(const SdesItem& sdesItem);

    /**
     * Duplicates theis SdesItem by calling the copy constructor.
     */
    virtual SdesItem *dup() const override;

    /**
     * Writes a short info about this SdesItem into the given string.
     */
    virtual std::string str() const override;

    /**
     * Writes an info about this SdesItem into the give output stream.
     */
    virtual void dump(std::ostream& os) const;

    /**
     * Returns the type of this sdes item.
     */
    virtual SdesItemType getType() const;

    /**
     * Returns the stored sdes string.
     */
    virtual const char *getContent() const;

    /**
     * This method returns the length of Value part.
     */
    virtual int getLengthField() const;

    /**
     * This method returns the size of this SdesItem in bytes as it
     * would be in the real world.
     */
    virtual int getSdesTotalLength() const;

  private:
    void copy(const SdesItem& other);
    void clean() {} // FIXME The `_content' sometimes allocated, sometimes not allocated pointer.

  protected:
    /**
     * The type of this SdesItem.
     */
    SdesItemType _type;

    /**
     * The length of this SdesItem.
     */
    int _length;

    /**
     * The sdes string.
     */
    std::string _content;
};

/**
 * The class SdesChunk is used for storing SdesItem objects
 * for one rtp end system.
 */
class INET_API SdesChunk : public cArray
{
  public:
    /**
     * Default constructor.
     */
    SdesChunk(const char *name = nullptr, uint32_t ssrc = 0);

    /**
     * Copy constructor.
     */
    SdesChunk(const SdesChunk& sdesChunk);

    /**
     * Destructor.
     */
    virtual ~SdesChunk();

    /**
     * Operator equal.
     */
    SdesChunk& operator=(const SdesChunk& sdesChunk);

    /**
     * Duplicates this SdesChunk by calling the copy constructor.
     */
    virtual SdesChunk *dup() const override;

    /**
     * Writes a short info about this SdesChunk into the given string.
     */
    virtual std::string str() const override;

    /**
     * Writes a longer info about this SdesChunk into the given stream.
     */
    virtual void dump(std::ostream& os) const;

    /**
     * Adds an SdesItem to this SdesChunk. If there is already an SdesItem
     * of the same type in this SdesChunk it is replaced by the new one.
     */
    virtual void addSDESItem(SdesItem *item);

    /**
     * Returns the ssrc identifier this SdesChunk is for.
     */
    virtual uint32_t getSsrc() const;

    /**
     * Sets the ssrc identifier this SdesChunk is for.
     */
    virtual void setSsrc(uint32_t ssrc);

    /**
     * Returns the length in bytes of this SdesChunk.
     */
    virtual int getLength() const;

  private:
    void copy(const SdesChunk& other);

  protected:
    /**
     * The ssrc identifier this SdesChunk is for.
     */
    uint32_t _ssrc;

    /**
     * The length in bytes of this SdesChunk.
     */
    int _length;
};

} // namespace rtp

} // namespace inet

#endif

