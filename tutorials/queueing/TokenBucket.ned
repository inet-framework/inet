
package inet.tutorials.queueing;

import inet.queueing.shaper.TokenBucket;
import inet.queueing.sink.PassivePacketSink;
import inet.queueing.source.ActivePacketSource;
import inet.queueing.tokengenerator.TimeBasedTokenGenerator;

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
        tokenGenerator: TimeBasedTokenGenerator {
            @display("p=100,200");
        }
    connections allowunconnected:
        producer.out --> bucket.in;
        bucket.out --> consumer.in;
}

