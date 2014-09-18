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

#ifndef __INET_SCTPMESSAGE_H
#define __INET_SCTPMESSAGE_H

#include <list>
#include "inet/common/INETDefs.h"
#include "inet/transportlayer/sctp/SCTPMessage_m.h"

namespace inet {

namespace sctp {

/**
 * Represents a SCTP Message. More info in the SCTPMessage.msg file
 * (and the documentation generated from it).
 */
class INET_API SCTPMessage : public SCTPMessage_Base
{
  protected:
    std::list<cPacket *> chunkList;

  private:
    void copy(const SCTPMessage& other);
    void clean();

  public:
    SCTPMessage(const char *name = NULL, int32 kind = 0) : SCTPMessage_Base(name, kind) {}
    SCTPMessage(const SCTPMessage& other) : SCTPMessage_Base(other) { copy(other); }
    ~SCTPMessage();
    SCTPMessage& operator=(const SCTPMessage& other);
    virtual SCTPMessage *dup() const { return new SCTPMessage(*this); }
    /** Generated but unused method, should not be called. */
    virtual void setChunksArraySize(uint32 size);
    /** Generated but unused method, should not be called. */
    virtual void setChunks(uint32 k, const cPacketPtr& chunks_var);
    /**
     * Returns the number of chunks in this SCTP packet
     */
    virtual uint32 getChunksArraySize() const;

    /**
     * Returns the kth chunk in this SCTP packet
     */
    virtual cPacketPtr& getChunks(uint32 k);
    /**
     * Adds a message object to the SCTP packet. The packet length will be adjusted
     */
    virtual void addChunk(cPacket *msg);

    /**
     * Removes and returns the first message object in this SCTP packet.
     */
    virtual cPacket *removeChunk();
    virtual cPacket *removeLastChunk();
    virtual cPacket *peekFirstChunk();
    virtual cPacket *peekLastChunk();
};

class INET_API SCTPErrorChunk : public SCTPErrorChunk_Base
{
  protected:
    std::list<cPacket *> parameterList;

  private:
    void copy(const SCTPErrorChunk& other);
    void clean();

  public:
    SCTPErrorChunk(const char *name = NULL, int32 kind = 0) : SCTPErrorChunk_Base(name, kind) {};
    SCTPErrorChunk(const SCTPErrorChunk& other) : SCTPErrorChunk_Base(other) { copy(other); };
    SCTPErrorChunk& operator=(const SCTPErrorChunk& other);
    ~SCTPErrorChunk();

    virtual SCTPErrorChunk *dup() const { return new SCTPErrorChunk(*this); }
    virtual void setParametersArraySize(uint32 size);
    virtual uint32 getParametersArraySize() const;
    /** Generated but unused method, should not be called. */
    virtual void setParameters(uint32 k, const cPacketPtr& parameters_var);

    /**
     * Returns the kth parameter in this SCTP Reset Chunk
     */
    virtual cPacketPtr& getParameters(uint32 k);

    /**
     * Adds a message object to the SCTP packet. The packet length will be adjusted
     */
    virtual void addParameters(cPacket *msg);

    /**
     * Removes and returns the first message object in this SCTP packet.
     */
    virtual cPacket *removeParameter();
};

class INET_API SCTPStreamResetChunk : public SCTPStreamResetChunk_Base
{
  protected:
    std::list<cPacket *> parameterList;

  public:
    SCTPStreamResetChunk(const char *name = NULL, int32 kind = 0) : SCTPStreamResetChunk_Base(name, kind) {};
    SCTPStreamResetChunk(const SCTPStreamResetChunk& other) : SCTPStreamResetChunk_Base(other.getName()) { operator=(other); };
    SCTPStreamResetChunk& operator=(const SCTPStreamResetChunk& other);

    virtual SCTPStreamResetChunk *dup() const { return new SCTPStreamResetChunk(*this); }
    virtual void setParametersArraySize(const uint32 size);
    virtual uint32 getParametersArraySize() const;

    /** Generated but unused method, should not be called. */
    virtual void setParameters(const uint32 k, const cPacketPtr& parameters_var);

    /**
     * Returns the kth parameter in this SCTP Reset Chunk
     */
    virtual cPacketPtr& getParameters(uint32 k);

    /**
     * Adds a message object to the SCTP packet. The packet length will be adjusted
     */
    virtual void addParameter(cPacket *msg);

    /**
     * Removes and returns the first message object in this SCTP packet.
     */
    virtual cPacket *removeParameter();
};

class INET_API SCTPAsconfChunk : public SCTPAsconfChunk_Base
{
  protected:
    std::list<cPacket *> parameterList;

  public:
    SCTPAsconfChunk(const char *name = NULL, int32 kind = 0) : SCTPAsconfChunk_Base(name, kind) {};
    SCTPAsconfChunk(const SCTPAsconfChunk& other) : SCTPAsconfChunk_Base(other.getName()) { operator=(other); };
    SCTPAsconfChunk& operator=(const SCTPAsconfChunk& other);

    virtual SCTPAsconfChunk *dup() const { return new SCTPAsconfChunk(*this); };
    virtual void setAsconfParamsArraySize(const uint32 size);
    virtual uint32 getAsconfParamsArraySize() const;

    /** Generated but unused method, should not be called. */
    virtual void setAsconfParams(const uint32 k, const cPacketPtr& parameters_var);

    /**
     * Returns the kth parameter in this SCTP Reset Chunk
     */
    virtual cPacketPtr& getAsconfParams(uint32 k);

    /**
     * Adds a message object to the SCTP packet. The packet length will be adjusted
     */
    virtual void addAsconfParam(cPacket *msg);

    /**
     * Removes and returns the first message object in this SCTP packet.
     */
    virtual cPacket *removeAsconfParam();
};

class INET_API SCTPAsconfAckChunk : public SCTPAsconfAckChunk_Base
{
  protected:
    std::list<cPacket *> parameterList;

  public:
    SCTPAsconfAckChunk(const char *name = NULL, int32 kind = 0) : SCTPAsconfAckChunk_Base(name, kind) {};
    SCTPAsconfAckChunk(const SCTPAsconfAckChunk& other) : SCTPAsconfAckChunk_Base(other.getName()) { operator=(other); };
    SCTPAsconfAckChunk& operator=(const SCTPAsconfAckChunk& other);

    virtual SCTPAsconfAckChunk *dup() const { return new SCTPAsconfAckChunk(*this); }
    virtual void setAsconfResponseArraySize(const uint32 size);
    virtual uint32 getAsconfResponseArraySize() const;

    /** Generated but unused method, should not be called. */
    virtual void setAsconfResponse(const uint32 k, const cPacketPtr& parameters_var);

    /**
     * Returns the kth parameter in this SCTP Reset Chunk
     */
    virtual cPacketPtr& getAsconfResponse(uint32 k);

    /**
     * Adds a message object to the SCTP packet. The packet length will be adjusted
     */
    virtual void addAsconfResponse(cPacket *msg);

    /**
     * Removes and returns the first message object in this SCTP packet.
     */
    virtual cPacket *removeAsconfResponse();
};

} // namespace sctp

} // namespace inet

#endif // ifndef __INET_SCTPMESSAGE_H

