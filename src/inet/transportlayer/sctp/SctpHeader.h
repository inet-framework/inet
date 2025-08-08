//
// Copyright (C) 2008-2009 Irene Ruengeler
// Copyright (C) 2009-2012 Thomas Dreibholz
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_SCTPHEADER_H
#define __INET_SCTPHEADER_H

#include <list>

#include "inet/common/INETDefs.h"
//#include "inet/transportlayer/contract/ITransportPacket.h"
#include "inet/transportlayer/sctp/SctpHeader_m.h"

namespace inet {
namespace sctp {

class INET_API SctpAsconfChunk : public SctpAsconfChunk_Base
{
  protected:
    std::vector<SctpParameter *> parameterList;

  public:
    SctpAsconfChunk(const char *name = nullptr, int32_t kind = 0) : SctpAsconfChunk_Base() {};
    SctpAsconfChunk(const SctpAsconfChunk& other) : SctpAsconfChunk_Base(other) { operator=(other); };
    SctpAsconfChunk& operator=(const SctpAsconfChunk& other);

    virtual SctpAsconfChunk *dup() const override { return new SctpAsconfChunk(*this); }

    virtual void setAsconfParamsArraySize(size_t size) override;
    virtual size_t getAsconfParamsArraySize() const override;

    /**
     * Returns the kth parameter in this SCTP Reset Chunk
     */
    virtual const SctpParameter *getAsconfParams(size_t k) const override;

    /** Generated but unused method, should not be called. */
    virtual void setAsconfParams(size_t k, SctpParameter *asconfParams) override;

    virtual void appendAsconfParams(SctpParameter *asconfParams) override { throw cRuntimeError("Unimplemented function"); }
    using SctpAsconfChunk_Base::insertAsconfParams;
    virtual void insertAsconfParams(size_t k, SctpParameter *asconfParams) override { throw cRuntimeError("Unimplemented function"); }
    virtual void eraseAsconfParams(size_t k) override { throw cRuntimeError("Unimplemented function"); }

    /**
     * Adds a message object to the SCTP packet. The packet length will be adjusted
     */
    virtual void addAsconfParam(SctpParameter *msg);

    /**
     * Removes and returns the first message object in this SCTP packet.
     */
    virtual SctpParameter *removeAsconfParam();
};

class INET_API SctpIncomingSsnResetRequestParameter : public SctpIncomingSsnResetRequestParameter_Base
{
  private:
    void copy(const SctpIncomingSsnResetRequestParameter& other);

  public:
    SctpIncomingSsnResetRequestParameter(const char *name = nullptr, int kind = 0) : SctpIncomingSsnResetRequestParameter_Base() {}
    SctpIncomingSsnResetRequestParameter(const SctpIncomingSsnResetRequestParameter& other) : SctpIncomingSsnResetRequestParameter_Base(other) { copy(other); }
    SctpIncomingSsnResetRequestParameter& operator=(const SctpIncomingSsnResetRequestParameter& other) { if (this == &other) return *this; SctpIncomingSsnResetRequestParameter_Base::operator=(other); copy(other); return *this; }
    virtual SctpIncomingSsnResetRequestParameter *dup() const override { return new SctpIncomingSsnResetRequestParameter(*this); }
};

class INET_API SctpAsconfAckChunk : public SctpAsconfAckChunk_Base
{
  protected:
    std::vector<SctpParameter *> parameterList;

  public:
    SctpAsconfAckChunk(const char *name = nullptr, int32_t kind = 0) : SctpAsconfAckChunk_Base() {};
    SctpAsconfAckChunk(const SctpAsconfAckChunk& other) : SctpAsconfAckChunk_Base(other) { operator=(other); };
    SctpAsconfAckChunk& operator=(const SctpAsconfAckChunk& other);

    virtual SctpAsconfAckChunk *dup() const override { return new SctpAsconfAckChunk(*this); }
    virtual void setAsconfResponseArraySize(size_t size) override;
    virtual size_t getAsconfResponseArraySize() const override;

    /** Generated but unused method, should not be called. */
    virtual void setAsconfResponse(size_t k, SctpParameter *asconfResponse) override;

    virtual void appendAsconfResponse(SctpParameter *asconfResponse) override { throw cRuntimeError("Unimplemented function"); }
    using SctpAsconfAckChunk_Base::insertAsconfResponse;
    virtual void insertAsconfResponse(size_t k, SctpParameter *asconfResponse) override { throw cRuntimeError("Unimplemented function"); }
    virtual void eraseAsconfResponse(size_t k) override { throw cRuntimeError("Unimplemented function"); }

    /**
     * Returns the kth parameter in this SCTP Reset Chunk
     */
    virtual SctpParameter *getAsconfResponse(size_t k) const override;

    /**
     * Adds a message object to the SCTP packet. The packet length will be adjusted
     */
    virtual void addAsconfResponse(SctpParameter *msg);

    /**
     * Removes and returns the first message object in this SCTP packet.
     */
    virtual SctpParameter *removeAsconfResponse();
};

} // namespace sctp
} // namespace inet

#endif

