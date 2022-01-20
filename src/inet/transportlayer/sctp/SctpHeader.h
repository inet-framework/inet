//
// Copyright (C) 2008-2009 Irene Ruengeler
// Copyright (C) 2009-2012 Thomas Dreibholz
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
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
    ~SctpHeader();
    SctpHeader& operator=(const SctpHeader& other);
    virtual SctpHeader *dup() const override { return new SctpHeader(*this); }

    virtual void setSctpChunksArraySize(size_t size) override;

    virtual void setSctpChunks(size_t k, SctpChunk * sctpChunks) override;
    /**
     * Returns the number of chunks in this SCTP packet
     */
    virtual size_t getSctpChunksArraySize() const override;

    /**
     * Returns the kth chunk in this SCTP packet
     */
 //   virtual SctpChunk *getSctpChunks(size_t k) const override;

    virtual const SctpChunk * getSctpChunks(size_t k) const override { return sctpChunkList.at(k); }

    /**
     * Adds a message object to the SCTP packet. The packet length will be adjusted
     */
#if (OMNETPP_BUILDNUM >= 1530)
    virtual void appendSctpChunks(SctpChunk *sctpChunks) override;
    using SctpHeader_Base::insertSctpChunks;
#else
    void appendSctpChunks(SctpChunk *sctpChunks);
    virtual void insertSctpChunks(SctpChunk *sctpChunks) override { appendSctpChunks(sctpChunks); }
#endif
    virtual void insertSctpChunks(size_t k, SctpChunk *sctpChunks) override;
    virtual void eraseSctpChunks(size_t k) override {}
    virtual void replaceSctpChunk(SctpChunk *msg, uint32_t k);


   // virtual void addChunk(SctpChunk * chunk);

    /**
     * Removes and returns the first message object in this SCTP packet.
     */
    virtual SctpChunk *removeFirstChunk();
    virtual SctpChunk *removeLastChunk();
    virtual SctpChunk *peekFirstChunk() const;
    virtual SctpChunk *peekLastChunk() const;

    virtual unsigned int getSourcePort() const override { return SctpHeader_Base::getSrcPort(); }
    virtual void setSourcePort(unsigned int port) override { SctpHeader_Base::setSrcPort(port); }
    virtual unsigned int getDestinationPort() const override { return SctpHeader_Base::getDestPort(); }
    virtual void setDestinationPort(unsigned int port) override { SctpHeader_Base::setDestPort(port); }
};

class INET_API SctpErrorChunk : public SctpErrorChunk_Base
{
  protected:
    std::vector<SctpParameter *> parameterList;

  private:
    void copy(const SctpErrorChunk& other);
    void clean();

  public:
    SctpErrorChunk(const char *name = nullptr, int32 kind = 0) : SctpErrorChunk_Base() {};
    SctpErrorChunk(const SctpErrorChunk& other) : SctpErrorChunk_Base(other) { copy(other); };
    SctpErrorChunk& operator=(const SctpErrorChunk& other);
    ~SctpErrorChunk();

    virtual SctpErrorChunk *dup() const override { return new SctpErrorChunk(*this); }
    virtual void setParametersArraySize(size_t size) override;
    virtual size_t getParametersArraySize() const override;
    /** Generated but unused method, should not be called. */
    virtual void setParameters(size_t k, SctpParameter * parameters) override;

#if (OMNETPP_BUILDNUM >= 1530)
    virtual void appendParameters(SctpParameter *parameters) override { throw cRuntimeError("Unimplemented function"); }
    using SctpErrorChunk_Base::insertParameters;
#else
    void appendParameters(SctpParameter *parameters) { throw cRuntimeError("Unimplemented function"); }
    virtual void insertParameters(SctpParameter *parameters) override { appendParameters(parameters); }
#endif
    virtual void insertParameters(size_t k, SctpParameter *parameters) override { throw cRuntimeError("Unimplemented function"); }
    virtual void eraseParameters(size_t k) override { throw cRuntimeError("Unimplemented function"); }
    /**
     * Returns the kth parameter in this SCTP Error Chunk
     */
     virtual SctpParameter * getParameters(size_t k) const override;
  //  virtual cPacketPtr& getParameters(uint32 k) override;

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
    SctpStreamResetChunk(const char *name = nullptr, int32 kind = 0) : SctpStreamResetChunk_Base() {};
    SctpStreamResetChunk(const SctpStreamResetChunk& other) : SctpStreamResetChunk_Base(other) { operator=(other); };
    SctpStreamResetChunk& operator=(const SctpStreamResetChunk& other);
    ~SctpStreamResetChunk();

    virtual SctpStreamResetChunk *dup() const override { return new SctpStreamResetChunk(*this); }

    virtual void setParametersArraySize(size_t size) override;
    virtual size_t getParametersArraySize() const override;

    /** Generated but unused method, should not be called. */
    virtual void setParameters(size_t k, SctpParameter * parameters) override;

#if (OMNETPP_BUILDNUM >= 1530)
    virtual void appendParameters(SctpParameter *parameters) override { throw cRuntimeError("Unimplemented function"); }
    using SctpStreamResetChunk_Base::insertParameters;
#else
    void appendParameters(SctpParameter *parameters) { throw cRuntimeError("Unimplemented function"); }
    virtual void insertParameters(SctpParameter *parameters) override { appendParameters(parameters); }
#endif
    virtual void insertParameters(size_t k, SctpParameter *parameters) override { throw cRuntimeError("Unimplemented function"); }
    virtual void eraseParameters(size_t k) override { throw cRuntimeError("Unimplemented function"); }
    /**
     * Returns the kth parameter in this SCTP Reset Chunk
     */
    virtual const SctpParameter * getParameters(size_t k) const override;

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
    SctpAsconfChunk(const char *name = nullptr, int32 kind = 0) : SctpAsconfChunk_Base() {};
    SctpAsconfChunk(const SctpAsconfChunk& other) : SctpAsconfChunk_Base(other) { operator=(other); };
    SctpAsconfChunk& operator=(const SctpAsconfChunk& other);

    virtual SctpAsconfChunk *dup() const override { return new SctpAsconfChunk(*this); };

    virtual void setAsconfParamsArraySize(size_t size) override;
    virtual size_t getAsconfParamsArraySize() const override;

    /**
     * Returns the kth parameter in this SCTP Reset Chunk
     */
    virtual const SctpParameter * getAsconfParams(size_t k) const override;

    /** Generated but unused method, should not be called. */
    virtual void setAsconfParams(size_t k, SctpParameter * asconfParams) override;

#if (OMNETPP_BUILDNUM >= 1530)
    virtual void appendAsconfParams(SctpParameter *asconfParams) override { throw cRuntimeError("Unimplemented function"); }
    using SctpAsconfChunk_Base::insertAsconfParams;
#else
    void appendAsconfParams(SctpParameter *asconfParams) { throw cRuntimeError("Unimplemented function"); }
    virtual void insertAsconfParams(SctpParameter *asconfParams) override { appendAsconfParams(asconfParams); }
#endif
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

class SctpIncomingSsnResetRequestParameter : public SctpIncomingSsnResetRequestParameter_Base
{
  private:
    void copy(const SctpIncomingSsnResetRequestParameter& other);

  public:
    SctpIncomingSsnResetRequestParameter(const char *name=nullptr, int kind=0) : SctpIncomingSsnResetRequestParameter_Base() {}
    SctpIncomingSsnResetRequestParameter(const SctpIncomingSsnResetRequestParameter& other) : SctpIncomingSsnResetRequestParameter_Base(other) {copy(other);}
    SctpIncomingSsnResetRequestParameter& operator=(const SctpIncomingSsnResetRequestParameter& other) {if (this==&other) return *this; SctpIncomingSsnResetRequestParameter_Base::operator=(other); copy(other); return *this;}
    virtual SctpIncomingSsnResetRequestParameter *dup() const override {return new SctpIncomingSsnResetRequestParameter(*this);}
};

class INET_API SctpAsconfAckChunk : public SctpAsconfAckChunk_Base
{
  protected:
    std::vector<SctpParameter *> parameterList;

  public:
    SctpAsconfAckChunk(const char *name = nullptr, int32 kind = 0) : SctpAsconfAckChunk_Base() {};
    SctpAsconfAckChunk(const SctpAsconfAckChunk& other) : SctpAsconfAckChunk_Base(other) { operator=(other); };
    SctpAsconfAckChunk& operator=(const SctpAsconfAckChunk& other);

    virtual SctpAsconfAckChunk *dup() const override { return new SctpAsconfAckChunk(*this); }
    virtual void setAsconfResponseArraySize(size_t size) override;
    virtual size_t getAsconfResponseArraySize() const override;

    /** Generated but unused method, should not be called. */
    virtual void setAsconfResponse(size_t k, SctpParameter * asconfResponse) override;

#if (OMNETPP_BUILDNUM >= 1530)
    virtual void appendAsconfResponse(SctpParameter *asconfResponse) override { throw cRuntimeError("Unimplemented function"); }
    using SctpAsconfAckChunk_Base::insertAsconfResponse;
#else
    void appendAsconfResponse(SctpParameter *asconfResponse) { throw cRuntimeError("Unimplemented function"); }
    virtual void insertAsconfResponse(SctpParameter *asconfResponse) override { appendAsconfResponse(asconfResponse); }
#endif
    virtual void insertAsconfResponse(size_t k, SctpParameter *asconfResponse) override { throw cRuntimeError("Unimplemented function"); }
    virtual void eraseAsconfResponse(size_t k) override { throw cRuntimeError("Unimplemented function"); }

    /**
     * Returns the kth parameter in this SCTP Reset Chunk
     */
    virtual SctpParameter * getAsconfResponse(size_t k) const override;

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

#endif // ifndef __INET_SCTPHEADER_H

