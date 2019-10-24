
package inet.tutorials.queueing;

import inet.queueing.shaper.TokenBucket;
import inet.queueing.sink.PassivePacketSink;
import inet.queueing.source.ActivePacketSource;

network TokenBucketNetwork
{
    submodules:
        producer: ActivePacketSource {
            @display("p=100,100");
        }
        bucket: TokenBucket {
            @display("p=200,100");
        }
        consumer: PassivePacketSink {
            @display("p=300,100");
        }
    connections allowunconnected:
        producer.out --> bucket.in;
        bucket.out --> consumer.in;
}
