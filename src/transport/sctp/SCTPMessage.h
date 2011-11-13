//
// Copyright (C) 2008 Irene Ruengeler
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


#ifndef _SCTPMESSAGE_H_
#define _SCTPMESSAGE_H_

#include <list>
#include "INETDefs.h"
#include "SCTPMessage_m.h"

/**
 * Represents a SCTP Message. More info in the SCTPMessage.msg file
 * (and the documentation generated from it).
 */
class INET_API SCTPMessage : public SCTPMessage_Base
{
    protected:
        std::list<cPacket*> chunkList;

    private:
      void copy(const SCTPMessage& other);
      void clean();

    public:
        SCTPMessage(const char *name = NULL, int32 kind = 0) : SCTPMessage_Base(name, kind) {}
        SCTPMessage(const SCTPMessage& other) : SCTPMessage_Base(other) { copy(other); }
        ~SCTPMessage();
        SCTPMessage& operator=(const SCTPMessage& other);
        virtual SCTPMessage *dup() const {return new SCTPMessage(*this);}
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
        virtual void addChunk(cPacket* msg);

        /**
        * Removes and returns the first message object in this SCTP packet.
        */
        virtual cPacket *removeChunk();
        virtual cPacket *removeLastChunk();
        virtual cPacket *peekFirstChunk();
        virtual cPacket *peekLastChunk();
        /**
         * Serializes SCTP packet for transmission on the wire,
         * writes source port into from structure and
         * returns length of sctp data written into buffer
         */


};

/*class SCTPErrorChunk : public SCTPErrorChunk_Base
{
    protected:
        std::list<cPacket*> parameterList;

    public:
            SCTPErrorChunk(const char *name=NULL, int32 kind=0) : SCTPErrorChunk_Base(name, kind) {};
            SCTPErrorChunk(const SCTPErrorChunk& other) : SCTPErrorChunk_Base(other.name()) {operator=(other);};
        SCTPErrorChunk& operator=(const SCTPErrorChunk& other);

        virtual cObject *dup() const {return new SCTPErrorChunk(*this);}
        virtual void setParametersArraySize(uint32 size);
            virtual uint32 getParametersArraySize() const;
        virtual void setParameters(uint32 k, const cPacketPtr& parameters_var);


        virtual cPacketPtr& getParameters(uint32 k);

        virtual void addParameter(cPacket* msg);

        virtual cPacket *removeParameter();
};*/

class INET_API SCTPErrorChunk : public SCTPErrorChunk_Base
{
    protected:
        std::list<cPacket*> parameterList;

    private:
        void copy(const SCTPErrorChunk& other);
        void clean();

    public:
        SCTPErrorChunk(const char *name = NULL, int32 kind = 0) : SCTPErrorChunk_Base(name, kind) {};
        SCTPErrorChunk(const SCTPErrorChunk& other) : SCTPErrorChunk_Base(other) { copy(other); };
        SCTPErrorChunk& operator=(const SCTPErrorChunk& other);
        ~SCTPErrorChunk();

        virtual SCTPErrorChunk *dup() const {return new SCTPErrorChunk(*this);}
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
        virtual void addParameters(cPacket* msg);

        /**
        * Removes and returns the first message object in this SCTP packet.
        */
        virtual cPacket *removeParameter();
};
#endif


