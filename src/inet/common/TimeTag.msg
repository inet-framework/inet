//
// This program is property of its copyright holder. All rights reserved.
//

import inet.common.INETDefs;
import inet.common.TagBase;

namespace inet;

class CreationTimeTag extends TagBase
{
    simtime_t creationTime = simTime();
}

class TimeTagBase extends TagBase
{
    string flowNames[];
    simtime_t totalTimes[];
}

class ElapsedTimeTag extends TimeTagBase
{
}

class DelayingTimeTag extends TimeTagBase
{
}

class QueueingTimeTag extends TimeTagBase
{
}

class ProcessingTimeTag extends TimeTagBase
{
}

class TransmissionTimeTag extends TimeTagBase
{
}

class PropagationTimeTag extends TimeTagBase
{
}
