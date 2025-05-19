//
// Copyright (C) 2019-2024 Timo Völker, Ekaterina Volodina
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef INET_APPLICATIONS_QUIC_STREAM_H_
#define INET_APPLICATIONS_QUIC_STREAM_H_

#include "inet/common/packet/ChunkQueue.h"
#include "../packet/QuicFrame.h"
#include "../flowcontroller/FlowController.h"
#include "../flowcontroller/FlowControlResponder.h"
#include "../Connection.h"
#include "StreamRcvQueue.h"
#include "StreamSndQueue.h"
#include "../Statistics.h"

namespace inet {
namespace quic {

class Connection;
class StreamRcvQueue;

// TODO: Split into SendStream and ReceiveStream?
class Stream {
public:
    Stream(uint64_t id, Connection *connection, Statistics *connStats);
    virtual ~Stream();

    uint64_t id;

    // ReceiveStream
    void bufferReceivedData(Ptr<const Chunk> data, uint64_t offset);
    void updateHighestRecievedOffset(uint64_t offset);
    void processFlowControlResponder(uint64_t offset);
    void onDataBlockedFrameReceived(uint64_t streamDataLimit);
    void onMaxStreamDataFrameReceived(uint64_t maxStreamData);
    void onMaxDataFrameReceived(uint64_t maxStreamData);
    void onMaxStreamDataFrameLost();
    bool hasDataForApp();
    bool isAllowedToReceivedData(uint64_t dataSize);
    Ptr<const Chunk> getDataForApp(B expectedDataSize);
    uint64_t getAvailableDataSizeForApp();

    // SendStream
    void enqueueDataFromApp(Ptr<const Chunk> data);
    uint64_t getSendQueueLength();

    /**
     * Checks the connection and stream flow control. If one is 0, it queues a DATA_BLOCKED or STREAM_DATA_BLOCKED frame.
     *
     * @return The minimum of both flow control available receiver windows
     */
    uint64_t checkAndGetAvailableRwnd();

    /**
     * Calculates the size in bytes of the next stream frame from this stream.
     *
     * @param maxFrameSize Gives the limit for the frame.
     * @return The size in bytes equal or smaller the given maxFrameSize.
     * Returns 0 if the stream is unable to generate a stream frame with the given maxFrameSize.
     */
    uint64_t getNextStreamFrameSize(uint64_t maxFrameSize);

    QuicFrame *generateStreamFrame(uint64_t offset, uint64_t length);

    /**
     * Generates the next stream frame from this stream.
     *
     * @param maxFrameSize Gives the limit for the size in bytes of the frame.
     * @return The generated stream frame.
     * @exception cRuntimeError If this stream is unable to generate a stream frame.
     */
    QuicFrame *generateNextStreamFrame(uint64_t maxFrameSize);

    /**
     * While a QuicStreamFrame stores only offset and length, this function returns the actual data.
     *
     * @param offset Offset in bytes.
     * @param length Length in bytes.
     * @return data chunk
     * @exception cRuntimeError If the data are not available.
     */
    const Ptr<const Chunk> getDataToSend(uint64_t offset, uint64_t length);
    void streamDataLost(uint64_t offset, uint64_t length);
    void streamDataAcked(uint64_t offset, uint64_t length);

    //Statistic
    void measureStreamRcvDataBytes(uint64_t dataLength);

protected:
    Connection *connection;

private:
    StreamSndQueue sendQueue;
    StreamRcvQueue *receiveQueue = nullptr;
    StreamFlowController *streamFlowController = nullptr;
    StreamFlowControlResponder *streamFlowControlResponder = nullptr;

    Statistics *stats;
    simsignal_t sendBufferUnsentAppDataStat;
    simsignal_t streamRcvAppDataStat;
    simtime_t streamCreatedTime;

    Ptr<StreamFrameHeader> createHeader(StreamSndQueue::Region region);
};

} /* namespace quic */
} /* namespace inet */

#endif /* INET_APPLICATIONS_QUIC_STREAM_H_ */
