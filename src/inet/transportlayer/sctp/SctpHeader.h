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

/**
 * Represents an SCTP Message. More info in the SctpHeader.msg file
 * (and the documentation generated from it).
 */
class INET_API SctpHeader : public SctpHeader_Base
{
  protected:
    typedef std::vector<SctpChunk *> SctpChunkList;
    SctpChunkList sctpChunkList;

  private:
    void copy(const SctpHeader& other);
    void clean();

  public:
    SctpHeader() : SctpHeader_Base() {}
    SctpHeader(const SctpHeader& other) : SctpHeader_Base(other) { copy(other); }
    ~SctpHeader() override;
    SctpHeader& operator=(const SctpHeader& other);
    SctpHeader *dup() const override { return new SctpHeader(*this); }

    void setSctpChunksArraySize(size_t size) override;

    void setSctpChunks(size_t k, SctpChunk *sctpChunks) override;
    /**
     * Returns the number of chunks in this SCTP packet
     */
    size_t getSctpChunksArraySize() const override;

    /**
     * Returns the kth chunk in this SCTP packet
     */
//    virtual SctpChunk *getSctpChunks(size_t k) const override;

    const SctpChunk *getSctpChunks(size_t k) const override { return sctpChunkList.at(k); }

    /**
     * Adds a message object to the SCTP packet. The packet length will be adjusted
     */
    void appendSctpChunks(SctpChunk *sctpChunks) override;
    using SctpHeader_Base::insertSctpChunks;
    void insertSctpChunks(size_t k, SctpChunk *sctpChunks) override;
    void eraseSctpChunks(size_t k) override {}
    virtual void replaceSctpChunk(SctpChunk *msg, uint32_t k);

//    virtual void addChunk(SctpChunk * chunk);

    /**
     * Removes and returns the first message object in this SCTP packet.
     */
    virtual SctpChunk *removeFirstChunk();
    virtual SctpChunk *removeLastChunk();
    virtual SctpChunk *peekFirstChunk() const;
    virtual SctpChunk *peekLastChunk() const;

    unsigned int getSourcePort() const override { return SctpHeader_Base::getSrcPort(); }
    void setSourcePort(unsigned int port) override { SctpHeader_Base::setSrcPort(port); }
    unsigned int getDestinationPort() const override { return SctpHeader_Base::getDestPort(); }
    void setDestinationPort(unsigned int port) override { SctpHeader_Base::setDestPort(port); }
};

class INET_API SctpErrorChunk : public SctpErrorChunk_Base
{
  protected:
    std::vector<SctpParameter *> parameterList;

  private:
    void copy(const SctpErrorChunk& other);
    void clean();

  public:
    SctpErrorChunk(const char *name = nullptr, int32_t kind = 0) : SctpErrorChunk_Base() {};
    SctpErrorChunk(const SctpErrorChunk& other) : SctpErrorChunk_Base(other) { copy(other); };
    SctpErrorChunk& operator=(const SctpErrorChunk& other);
    ~SctpErrorChunk() override;

    SctpErrorChunk *dup() const override { return new SctpErrorChunk(*this); }
    void setParametersArraySize(size_t size) override;
    size_t getParametersArraySize() const override;
    /** Generated but unused method, should not be called. */
    void setParameters(size_t k, SctpParameter *parameters) override;

    void appendParameters(SctpParameter *parameters) override { throw cRuntimeError("Unimplemented function"); }
    using SctpErrorChunk_Base::insertParameters;
    void insertParameters(size_t k, SctpParameter *parameters) override { throw cRuntimeError("Unimplemented function"); }
    void eraseParameters(size_t k) override { throw cRuntimeError("Unimplemented function"); }
    /**
     * Returns the kth parameter in this SCTP Error Chunk
     */
    SctpParameter *getParameters(size_t k) const override;
//    virtual cPacketPtr& getParameters(uint32_t k) override;

    /**
     * Adds a message object to the SCTP packet. The packet length will be adjusted
     */
    virtual void addParameters(SctpParameter *msg);

    /**
     * Removes and returns the first message object in this SCTP packet.
     */
    virtual SctpParameter *removeParameter();
};

class INET_API SctpStreamResetChunk : public SctpStreamResetChunk_Base
{
  protected:
    std::vector<SctpParameter *> parameterList;

  private:
    void copy(const SctpStreamResetChunk& other);
    void clean();

  public:
    SctpStreamResetChunk(const char *name = nullptr, int32_t kind = 0) : SctpStreamResetChunk_Base() {};
    SctpStreamResetChunk(const SctpStreamResetChunk& other) : SctpStreamResetChunk_Base(other) { operator=(other); };
    SctpStreamResetChunk& operator=(const SctpStreamResetChunk& other);
    ~SctpStreamResetChunk() override;

    SctpStreamResetChunk *dup() const override { return new SctpStreamResetChunk(*this); }

    void setParametersArraySize(size_t size) override;
    size_t getParametersArraySize() const override;

    /** Generated but unused method, should not be called. */
    void setParameters(size_t k, SctpParameter *parameters) override;

    void appendParameters(SctpParameter *parameters) override { throw cRuntimeError("Unimplemented function"); }
    using SctpStreamResetChunk_Base::insertParameters;
    void insertParameters(size_t k, SctpParameter *parameters) override { throw cRuntimeError("Unimplemented function"); }
    void eraseParameters(size_t k) override { throw cRuntimeError("Unimplemented function"); }
    /**
     * Returns the kth parameter in this SCTP Reset Chunk
     */
    const SctpParameter *getParameters(size_t k) const override;

    /**
     * Adds a message object to the SCTP packet. The packet length will be adjusted
     */
    virtual void addParameter(SctpParameter *msg);

    /**
     * Removes and returns the first message object in this SCTP packet.
     */
    virtual cPacket *removeParameter();
};

class INET_API SctpAsconfChunk : public SctpAsconfChunk_Base
{
  protected:
    std::vector<SctpParameter *> parameterList;

  public:
    SctpAsconfChunk(const char *name = nullptr, int32_t kind = 0) : SctpAsconfChunk_Base() {};
    SctpAsconfChunk(const SctpAsconfChunk& other) : SctpAsconfChunk_Base(other) { operator=(other); };
    SctpAsconfChunk& operator=(const SctpAsconfChunk& other);

    SctpAsconfChunk *dup() const override { return new SctpAsconfChunk(*this); }

    void setAsconfParamsArraySize(size_t size) override;
    size_t getAsconfParamsArraySize() const override;

    /**
     * Returns the kth parameter in this SCTP Reset Chunk
     */
    const SctpParameter *getAsconfParams(size_t k) const override;

    /** Generated but unused method, should not be called. */
    void setAsconfParams(size_t k, SctpParameter *asconfParams) override;

    void appendAsconfParams(SctpParameter *asconfParams) override { throw cRuntimeError("Unimplemented function"); }
    using SctpAsconfChunk_Base::insertAsconfParams;
    void insertAsconfParams(size_t k, SctpParameter *asconfParams) override { throw cRuntimeError("Unimplemented function"); }
    void eraseAsconfParams(size_t k) override { throw cRuntimeError("Unimplemented function"); }

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
    SctpIncomingSsnResetRequestParameter *dup() const override { return new SctpIncomingSsnResetRequestParameter(*this); }
};

class INET_API SctpAsconfAckChunk : public SctpAsconfAckChunk_Base
{
  protected:
    std::vector<SctpParameter *> parameterList;

  public:
    SctpAsconfAckChunk(const char *name = nullptr, int32_t kind = 0) : SctpAsconfAckChunk_Base() {};
    SctpAsconfAckChunk(const SctpAsconfAckChunk& other) : SctpAsconfAckChunk_Base(other) { operator=(other); };
    SctpAsconfAckChunk& operator=(const SctpAsconfAckChunk& other);

    SctpAsconfAckChunk *dup() const override { return new SctpAsconfAckChunk(*this); }
    void setAsconfResponseArraySize(size_t size) override;
    size_t getAsconfResponseArraySize() const override;

    /** Generated but unused method, should not be called. */
    void setAsconfResponse(size_t k, SctpParameter *asconfResponse) override;

    void appendAsconfResponse(SctpParameter *asconfResponse) override { throw cRuntimeError("Unimplemented function"); }
    using SctpAsconfAckChunk_Base::insertAsconfResponse;
    void insertAsconfResponse(size_t k, SctpParameter *asconfResponse) override { throw cRuntimeError("Unimplemented function"); }
    void eraseAsconfResponse(size_t k) override { throw cRuntimeError("Unimplemented function"); }

    /**
     * Returns the kth parameter in this SCTP Reset Chunk
     */
    SctpParameter *getAsconfResponse(size_t k) const override;

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

