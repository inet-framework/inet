//
// This program is property of its copyright holder. All rights reserved.
//
package inet.util;

//
// This channels adds suport for thruput meternig to the datarate channel
// A cDatarateChannel extended with throughput calculation. Values
// get displayed on the link, using the connection's "t=" display
// string tag.
//
// The display can be customized with the "format" attribute.
// In the format string, the following characters will get expanded:
//   - 'N': number of packets
//   - 'V': volume (in bytes)
//   - 'p': current packet/sec
//   - 'b': current bandwidth
//   - 'u': current channel utilization (%)
//   - 'P': average packet/sec on [0,now)
//   - 'B': average bandwidth on [0,now)
//   - 'U': average channel utilization (%) on [0,now)
// Other characters are copied verbatim.
//
// "Current" actually means the last measurement interval, which is
// 10 packets or 0.1s, whichever comes first.
//
channel ThruputMeteringChannel extends ned.DatarateChannel
{
    @class(ThruputMeteringChannel);
    string thruputDisplayFormat = default("");  // the format string for display on the link. Leave empty to disable display
}
