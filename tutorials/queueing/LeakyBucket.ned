
package inet.tutorials.queueing;

import inet.queueing.shaper.LeakyBucket;
import inet.queueing.sink.PassivePacketSink;
import inet.queueing.source.ActivePacketSource;

network LeakyBucketNetwork
{
    submodules:
        producer: ActivePacketSource {
            @display("p=100,100");
        }
        bucket: LeakyBucket {
            @display("p=200,100");
        }
        consumer: PassivePacketSink {
            @display("p=300,100");
        }
    connections allowunconnected:
        producer.out --> bucket.in;
        bucket.out --> consumer.in;
}
