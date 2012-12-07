//
// This file is part of an OMNeT++/OMNEST simulation example.
//
// Copyright (C) 1992-2008 Andras Varga
//
// This file is distributed WITHOUT ANY WARRANTY. See the file
// `license' for details on this and other legal matters.
//

package inet.examples.test.database_test;

import inet.util.fifo.FifoQueue;
import inet.util.fifo.FifoSink;
import inet.util.fifo.FifoSource;

//
// Simple queueing network: generator + FIFO + sink.
//
network FifoNet
{
    submodules:
        gen: FifoSource {
            parameters:
                @display("p=89,100");
        }
        fifo: FifoQueue {
            parameters:
                @display("p=209,100");
        }
        sink: FifoSink {
            parameters:
                @display("p=329,100");
        }
    connections:
        gen.out --> fifo.in;
        fifo.out --> sink.in;
}
