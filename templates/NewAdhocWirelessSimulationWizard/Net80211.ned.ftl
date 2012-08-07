<@setoutput path=targetFileName?default("")/>
${bannerComment}

<#if nedPackageName!="">package ${nedPackageName};</#if>

// numOfHosts: ${numOfHosts}
// parametric: ${parametric?string}
// static:     ${static?string}

import inet.networklayer.autorouting.ipv4.IPv4NetworkConfigurator;
import inet.nodes.inet.AdhocHost;
import inet.world.radio.ChannelControl;


network ${targetTypeName}
{
    parameters:
<#if parametric>
        int numHosts;
</#if>
    submodules:
<#if parametric>
        host[numHosts]: AdhocHost
        {
            parameters:
                @display("r=,,#707070");
        }
<#else>
    <#list 0..numOfHosts?number-1 as i>
        host${i}: AdhocHost
        {
            parameters:
                @display("r=,,#707070");
        }
    </#list>
</#if>

        channelControl: ChannelControl
        {
            parameters:
                @display("p=60,50");
        }

        configurator: IPv4NetworkConfigurator
        {
            @display("p=140,50");
        }
}
