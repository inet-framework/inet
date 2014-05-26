//
// This file is part of an OMNeT++/OMNEST simulation example.
//
// Copyright (C) 1992-2008 Andras Varga
//
// This file is distributed WITHOUT ANY WARRANTY. See the file
// `license' for details on this and other legal matters.
//


package inet.util.fifo;


//
// Destroys the packets and collects statistics
//
simple FifoSink
{
    parameters:
        @display("i=block/sink");
        @signal[lifetime](type="simtime_t");
        @statistic[lifetime](title="lifetime of arrived jobs"; unit=s; record=vector,mean,max; interpolationmode=none);
        double timeScale = default(1.0);	// time scale factor for output vector recording
    gates:
        input in;
}

