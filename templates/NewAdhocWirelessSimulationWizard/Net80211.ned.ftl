<@setoutput path=targetFileName?default("")/>
${bannerComment}

<#if nedPackageName!="">package ${nedPackageName};</#if>

// numOfHosts: ${numOfHosts}
// parametric: ${parametric?string}
// static:     ${static?string}

import inet.networklayer.autorouting.FlatNetworkConfigurator;
import inet.nodes.adhoc.MobileHost;
import inet.world.ChannelControl;


network ${targetTypeName}
{
    parameters:
<#if parametric>
        int numHosts;
</#if>
        double playgroundSizeX;
        double playgroundSizeY;

    submodules:
<#if parametric>
        host[numHosts]: MobileHost
        {
            parameters:
                @display("r=,,#707070");
        }
<#else>
    <#list 0..numOfHosts?number-1 as i>
        host${i}: MobileHost
        {
            parameters:
                @display("r=,,#707070");
        }
    </#list>
</#if>

        channelcontrol: ChannelControl
        {
            parameters:
                playgroundSizeX = playgroundSizeX;
                playgroundSizeY = playgroundSizeY;
                @display("p=60,50");
        }

        configurator: FlatNetworkConfigurator
        {
            @display("p=140,50");
        }
}
