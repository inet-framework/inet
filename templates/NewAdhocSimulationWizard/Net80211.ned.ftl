<@setoutput path=targetFileName?default("")/>
${bannerComment}

<#if nedPackageName!="">package ${nedPackageName};</#if>

// numOfHosts: ${numOfHosts}

import inet.networklayer.autorouting.FlatNetworkConfigurator;
import inet.nodes.adhoc.MobileHost;
import inet.world.ChannelControl;


network ${targetTypeName}
{
    parameters:
        int numHosts;
        double playgroundSizeX;
        double playgroundSizeY;
    submodules:
        host[numHosts]: MobileHost {
            parameters:
                @display("r=,,#707070");
        }
        channelcontrol: ChannelControl {
            parameters:
                playgroundSizeX = playgroundSizeX;
                playgroundSizeY = playgroundSizeY;
                @display("p=60,50");
        }
        configurator: FlatNetworkConfigurator {
            @display("p=140,50");
        }
}
