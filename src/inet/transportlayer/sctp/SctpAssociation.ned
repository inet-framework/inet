//
// Copyright (C) 2005-2010 Irene Ruengeler
// Copyright (C) 2009-2012 Thomas Dreibholz
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.transportlayer.sctp;

import inet.common.SimpleModule;

//
// Represents a single SCTP association in the SCTP protocol implementation.
// Implements the SCTP state machine and handles association establishment,
// data transfer, flow control, congestion control, and association termination.
// Provides extensive statistics collection for various SCTP metrics.
//
simple SctpAssociation extends SimpleModule {
    parameters:
        @class(SctpAssociation);
        @display("i=block/socket");

        // ====== Path-specific signals (demux by remote address) ======
        @signal[pathRto](type=simtime_t);
        @signal[pathRtt](type=simtime_t);
        @signal[pathSsthresh](type="unsigned long");
        @signal[pathCwnd](type="unsigned long");
        @signal[pathBandwidth](type=double);
        @signal[pathOutstandingBytes](type="unsigned long");
        @signal[pathQueuedSentBytes](type="unsigned long");
        @signal[pathSenderBlockingFraction](type=double);
        @signal[pathReceiverBlockingFraction](type=double);
        @signal[pathTsnSent](type="unsigned long");
        @signal[pathTsnReceived](type="unsigned long");
        @signal[pathHeartbeatSent](type="unsigned long");
        @signal[pathHeartbeatAckSent](type="unsigned long");
        @signal[pathHeartbeatReceived](type="unsigned long");
        @signal[pathHeartbeatAckReceived](type="unsigned long");
        @signal[pathGapAckedChunksInLastSACK](type="unsigned long");
        @signal[pathGapNRAckedChunksInLastSACK](type="unsigned long");
        @signal[pathGapUnackedChunksInLastSACK](type="unsigned long");
        @signal[pathPartialBytesAcked](type="unsigned long");
        @signal[pathFastRecoveryState](type="unsigned long");
        @signal[pathTsnFastRTX](type="unsigned long");
        @signal[pathTsnTimerBasedRTX](type="unsigned long");
        @signal[pathTsnAckedCumAck](type="unsigned long");
        @signal[pathTsnAckedGapAck](type="unsigned long");
        @signal[pathPseudoCumAck](type="unsigned long");
        @signal[pathRTXPseudoCumAck](type="unsigned long");
        @signal[pathBlockingTsnsMoved](type="unsigned long");

        @statistic[pathRto](source=pathRto; record=vector(demux); interpolationmode=sample-hold);
        @statistic[pathRtt](source=pathRtt; record=vector(demux); interpolationmode=sample-hold);
        @statistic[pathSsthresh](source=pathSsthresh; record=vector(demux); interpolationmode=sample-hold);
        @statistic[pathCwnd](source=pathCwnd; record=vector(demux); interpolationmode=sample-hold);
        @statistic[pathBandwidth](source=pathBandwidth; record=vector(demux); interpolationmode=sample-hold);
        @statistic[pathOutstandingBytes](source=pathOutstandingBytes; record=vector(demux); interpolationmode=sample-hold);
        @statistic[pathQueuedSentBytes](source=pathQueuedSentBytes; record=vector(demux); interpolationmode=sample-hold);
        @statistic[pathSenderBlockingFraction](source=pathSenderBlockingFraction; record=vector(demux); interpolationmode=sample-hold);
        @statistic[pathReceiverBlockingFraction](source=pathReceiverBlockingFraction; record=vector(demux); interpolationmode=sample-hold);
        @statistic[pathTsnSent](source=pathTsnSent; record=vector(demux));
        @statistic[pathTsnReceived](source=pathTsnReceived; record=vector(demux));
        @statistic[pathHeartbeatSent](source=pathHeartbeatSent; record=vector(demux));
        @statistic[pathHeartbeatAckSent](source=pathHeartbeatAckSent; record=vector(demux));
        @statistic[pathHeartbeatReceived](source=pathHeartbeatReceived; record=vector(demux));
        @statistic[pathHeartbeatAckReceived](source=pathHeartbeatAckReceived; record=vector(demux));
        @statistic[pathGapAckedChunksInLastSACK](source=pathGapAckedChunksInLastSACK; record=vector(demux));
        @statistic[pathGapNRAckedChunksInLastSACK](source=pathGapNRAckedChunksInLastSACK; record=vector(demux));
        @statistic[pathGapUnackedChunksInLastSACK](source=pathGapUnackedChunksInLastSACK; record=vector(demux));
        @statistic[pathPartialBytesAcked](source=pathPartialBytesAcked; record=vector(demux));
        @statistic[pathFastRecoveryState](source=pathFastRecoveryState; record=vector(demux); interpolationmode=sample-hold);
        @statistic[pathTsnFastRTX](source=pathTsnFastRTX; record=vector(demux));
        @statistic[pathTsnTimerBasedRTX](source=pathTsnTimerBasedRTX; record=vector(demux));
        @statistic[pathTsnAckedCumAck](source=pathTsnAckedCumAck; record=vector(demux));
        @statistic[pathTsnAckedGapAck](source=pathTsnAckedGapAck; record=vector(demux));
        @statistic[pathPseudoCumAck](source=pathPseudoCumAck; record=vector(demux));
        @statistic[pathRTXPseudoCumAck](source=pathRTXPseudoCumAck; record=vector(demux));
        @statistic[pathBlockingTsnsMoved](source=pathBlockingTsnsMoved; record=vector(demux));

        // ====== Stream-specific signal (demux by stream ID) ======
        @signal[streamThroughput](type=double);
        @statistic[streamThroughput](source=streamThroughput; record=vector(demux));

        // ====== Association-level signals (no demux) ======
        @signal[advRwnd](type="unsigned long");
        @signal[cumTsnAck](type="unsigned long");
        @signal[sendQueue](type="unsigned long");
        @signal[numGapBlocks](type="unsigned long");
        @signal[outstandingBytes](type="unsigned long");
        @signal[queuedReceivedBytes](type="unsigned long");
        @signal[queuedSentBytes](type="unsigned long");
        @signal[totalSsthresh](type="unsigned long");
        @signal[totalCwnd](type="unsigned long");
        @signal[totalBandwidth](type=double);
        @signal[revokableGapBlocksInLastSACK](type="unsigned long");
        @signal[nonRevokableGapBlocksInLastSACK](type="unsigned long");
        @signal[arwndInLastSACK](type="unsigned long");
        @signal[peerRwnd](type="unsigned long");
        @signal[numTotalGapBlocksStored](type="unsigned long");
        @signal[numRevokableGapBlocksStored](type="unsigned long");
        @signal[numNonRevokableGapBlocksStored](type="unsigned long");
        @signal[numDuplicatesStored](type="unsigned long");
        @signal[numRevokableGapBlocksSent](type="unsigned long");
        @signal[numNonRevokableGapBlocksSent](type="unsigned long");
        @signal[numDuplicatesSent](type="unsigned long");
        @signal[sackLengthSent](type="unsigned long");
        @signal[advMsgRwnd](type="unsigned long");
        @signal[endToEndDelay](type=simtime_t);
        @signal[assocThroughput](type=double);

        @statistic[advRwnd](source=advRwnd; record=vector; interpolationmode=sample-hold);
        @statistic[cumTsnAck](source=cumTsnAck; record=vector);
        @statistic[sendQueue](source=sendQueue; record=vector; interpolationmode=sample-hold);
        @statistic[numGapBlocks](source=numGapBlocks; record=vector);
        @statistic[outstandingBytes](source=outstandingBytes; record=vector; interpolationmode=sample-hold);
        @statistic[queuedReceivedBytes](source=queuedReceivedBytes; record=vector; interpolationmode=sample-hold);
        @statistic[queuedSentBytes](source=queuedSentBytes; record=vector; interpolationmode=sample-hold);
        @statistic[totalSsthresh](source=totalSsthresh; record=vector; interpolationmode=sample-hold);
        @statistic[totalCwnd](source=totalCwnd; record=vector; interpolationmode=sample-hold);
        @statistic[totalBandwidth](source=totalBandwidth; record=vector; interpolationmode=sample-hold);
        @statistic[revokableGapBlocksInLastSACK](source=revokableGapBlocksInLastSACK; record=vector);
        @statistic[nonRevokableGapBlocksInLastSACK](source=nonRevokableGapBlocksInLastSACK; record=vector);
        @statistic[arwndInLastSACK](source=arwndInLastSACK; record=vector; interpolationmode=sample-hold);
        @statistic[peerRwnd](source=peerRwnd; record=vector; interpolationmode=sample-hold);
        @statistic[numTotalGapBlocksStored](source=numTotalGapBlocksStored; record=vector);
        @statistic[numRevokableGapBlocksStored](source=numRevokableGapBlocksStored; record=vector);
        @statistic[numNonRevokableGapBlocksStored](source=numNonRevokableGapBlocksStored; record=vector);
        @statistic[numDuplicatesStored](source=numDuplicatesStored; record=vector);
        @statistic[numRevokableGapBlocksSent](source=numRevokableGapBlocksSent; record=vector);
        @statistic[numNonRevokableGapBlocksSent](source=numNonRevokableGapBlocksSent; record=vector);
        @statistic[numDuplicatesSent](source=numDuplicatesSent; record=vector);
        @statistic[sackLengthSent](source=sackLengthSent; record=vector);
        @statistic[advMsgRwnd](source=advMsgRwnd; record=vector; interpolationmode=sample-hold);
        @statistic[endToEndDelay](source=endToEndDelay; record=vector);
        @statistic[assocThroughput](source=assocThroughput; record=vector; interpolationmode=sample-hold);
}
