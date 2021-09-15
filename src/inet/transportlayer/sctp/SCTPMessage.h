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
#include "inet/transportlayer/contract/ITransportPacket.h"
#include "inet/transportlayer/sctp/SCTPMessage_m.h"

namespace inet {

namespace sctp {

/**
 * Represents a SCTP Message. More info in the SCTPMessage.msg file
 * (and the documentation generated from it).
 */
class INET_API SCTPMessage : public SCTPMessage_Base, public ITransportPacket
{
  public:
    SCTPMessage(const char *name = nullptr, int32 kind = 0) : SCTPMessage_Base(name, kind) {}
    SCTPMessage(const SCTPMessage& other) : SCTPMessage_Base(other) {}
    SCTPMessage& operator=(const SCTPMessage& other);
    virtual SCTPMessage *dup() const override { return new SCTPMessage(*this); }
    /** Generated but unused method, should not be called. */
    virtual void setChunksArraySize(size_t size) override;
    /** Generated but unused method, should not be called. */
    virtual void setChunks(size_t k, SCTPChunk *chunks_var) override;
    /** Adds a message object to the SCTP packet. The packet length will be adjusted */
    virtual void addChunk(SCTPChunk *msg);
    /** Removes and returns the first message object in this SCTP packet. */
    virtual SCTPChunk *removeChunk();
    virtual SCTPChunk *removeLastChunk();
    virtual SCTPChunk *peekFirstChunk();
    virtual SCTPChunk *peekLastChunk();
    virtual void replaceChunk(SCTPChunk *msg, size_t k);

    virtual unsigned int getSourcePort() const override { return SCTPMessage_Base::getSrcPort(); }
    virtual void setSourcePort(unsigned int port) override { SCTPMessage_Base::setSrcPort(port); }
    virtual unsigned int getDestinationPort() const override { return SCTPMessage_Base::getDestPort(); }
    virtual void setDestinationPort(unsigned int port) override { SCTPMessage_Base::setDestPort(port); }
};

class INET_API SCTPErrorChunk : public SCTPErrorChunk_Base
{
  public:
    SCTPErrorChunk(const char *name = nullptr, int32 kind = 0) : SCTPErrorChunk_Base(name, kind) {};
    SCTPErrorChunk(const SCTPErrorChunk& other) : SCTPErrorChunk_Base(other) {};
    SCTPErrorChunk& operator=(const SCTPErrorChunk& other);
    ~SCTPErrorChunk();

    virtual SCTPErrorChunk *dup() const override { return new SCTPErrorChunk(*this); }
    virtual void setParametersArraySize(size_t size) override;
    /** Generated but unused method, should not be called. */
    virtual void setParameters(size_t k, SCTPParameter *parameters_var) override;
    /** Adds a message object to the SCTP packet. The packet length will be adjusted */
    virtual void addParameters(SCTPParameter *msg);
    /** Removes and returns the first message object in this SCTP packet. */
    virtual SCTPParameter *removeParameter();
};

class INET_API SCTPStreamResetChunk : public SCTPStreamResetChunk_Base
{
  public:
    SCTPStreamResetChunk(const char *name = nullptr, int32 kind = 0) : SCTPStreamResetChunk_Base(name, kind) {};
    SCTPStreamResetChunk(const SCTPStreamResetChunk& other) : SCTPStreamResetChunk_Base(other.getName()) { operator=(other); };
    SCTPStreamResetChunk& operator=(const SCTPStreamResetChunk& other);
    ~SCTPStreamResetChunk();

    virtual SCTPStreamResetChunk *dup() const override { return new SCTPStreamResetChunk(*this); }
    virtual void setParametersArraySize(size_t size) override;

    /** Generated but unused method, should not be called. */
    virtual void setParameters(size_t k, SCTPParameter *parameters_var) override;

    /** Adds a message object to the SCTP packet. The packet length will be adjusted */
    virtual void addParameter(SCTPParameter *msg);

    /** Removes and returns the first message object in this SCTP packet. */
    virtual SCTPParameter *removeParameter();
};

class INET_API SCTPAsconfChunk : public SCTPAsconfChunk_Base
{
  public:
    SCTPAsconfChunk(const char *name = nullptr, int32 kind = 0) : SCTPAsconfChunk_Base(name, kind) {};
    SCTPAsconfChunk(const SCTPAsconfChunk& other) : SCTPAsconfChunk_Base(other.getName()) { operator=(other); };
    SCTPAsconfChunk& operator=(const SCTPAsconfChunk& other);

    virtual SCTPAsconfChunk *dup() const override { return new SCTPAsconfChunk(*this); };
    virtual void setAsconfParamsArraySize(size_t size) override;

    /** Generated but unused method, should not be called. */
    virtual void setAsconfParams(size_t k, SCTPParameter *parameters_var) override;
    /** Adds a message object to the SCTP packet. The packet length will be adjusted */
    virtual void addAsconfParam(SCTPParameter *msg);
    /** Removes and returns the first message object in this SCTP packet. */
    virtual SCTPParameter *removeAsconfParam();
};

class SCTPIncomingSSNResetRequestParameter : public SCTPIncomingSSNResetRequestParameter_Base
{
  private:
    void copy(const SCTPIncomingSSNResetRequestParameter& other);

  public:
    SCTPIncomingSSNResetRequestParameter(const char *name=nullptr, int kind=0) : SCTPIncomingSSNResetRequestParameter_Base(name,kind) {}
    SCTPIncomingSSNResetRequestParameter(const SCTPIncomingSSNResetRequestParameter& other) : SCTPIncomingSSNResetRequestParameter_Base(other) {copy(other);}
    SCTPIncomingSSNResetRequestParameter& operator=(const SCTPIncomingSSNResetRequestParameter& other) {if (this==&other) return *this; SCTPIncomingSSNResetRequestParameter_Base::operator=(other); copy(other); return *this;}
    virtual SCTPIncomingSSNResetRequestParameter *dup() const override {return new SCTPIncomingSSNResetRequestParameter(*this);}
};

class INET_API SCTPAsconfAckChunk : public SCTPAsconfAckChunk_Base
{
  public:
    SCTPAsconfAckChunk(const char *name = nullptr, int32 kind = 0) : SCTPAsconfAckChunk_Base(name, kind) {};
    SCTPAsconfAckChunk(const SCTPAsconfAckChunk& other) : SCTPAsconfAckChunk_Base(other.getName()) { operator=(other); };
    SCTPAsconfAckChunk& operator=(const SCTPAsconfAckChunk& other);

    virtual SCTPAsconfAckChunk *dup() const override { return new SCTPAsconfAckChunk(*this); }
    virtual void setAsconfResponseArraySize(size_t size) override;
    /** Generated but unused method, should not be called. */
    virtual void setAsconfResponse(const size_t k, SCTPParameter *parameters_var) override;
    /** Adds a message object to the SCTP packet. The packet length will be adjusted */
    virtual void addAsconfResponse(SCTPParameter *msg);
    /** Removes and returns the first message object in this SCTP packet. */
    virtual SCTPParameter *removeAsconfResponse();
};

} // namespace sctp

} // namespace inet

#endif // ifndef __INET_SCTPMESSAGE_H

