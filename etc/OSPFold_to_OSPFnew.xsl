<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
<!-- transform old OSPF.xml to new OSPF.xml -->

<!--
usage:
    xsltproc this_xsl_file old_xml_file >new_xml_file
-->

<!-- AddressRange -->
<xsl:template match="Area/AddressRange/Address|Area/AddressRange/Mask|Area/AddressRange/Status" mode="toAttr">
  <xsl:attribute name="{name()}"><xsl:value-of select="." /></xsl:attribute>
</xsl:template>
<xsl:template match="Area/AddressRange/Address|Area/AddressRange/Mask|Area/AddressRange/Status">
</xsl:template>


<!-- BroadcastInterface -->
<xsl:template match="BroadcastInterface/AreaID|BroadcastInterface/InterfaceOutputCost|BroadcastInterface/RetransmissionInterval|BroadcastInterface/InterfaceTransmissionDelay|BroadcastInterface/RouterPriority|BroadcastInterface/HelloInterval|BroadcastInterface/RouterDeadInterval|BroadcastInterface/AuthenticationType|BroadcastInterface/AuthenticationKey" mode="toAttr">
  <xsl:attribute name="{name()}"><xsl:value-of select="." /></xsl:attribute>
</xsl:template>
<xsl:template match="BroadcastInterface/AreaID|BroadcastInterface/InterfaceOutputCost|BroadcastInterface/RetransmissionInterval|BroadcastInterface/InterfaceTransmissionDelay|BroadcastInterface/RouterPriority|BroadcastInterface/HelloInterval|BroadcastInterface/RouterDeadInterval|BroadcastInterface/AuthenticationType|BroadcastInterface/AuthenticationKey">
</xsl:template>


<!-- PointToPointInterface -->
<xsl:template match="PointToPointInterface/AreaID|PointToPointInterface/InterfaceOutputCost|PointToPointInterface/RetransmissionInterval|PointToPointInterface/InterfaceTransmissionDelay|PointToPointInterface/RouterPriority|PointToPointInterface/HelloInterval|PointToPointInterface/RouterDeadInterval|PointToPointInterface/AuthenticationType|PointToPointInterface/AuthenticationKey" mode="toAttr">
  <xsl:attribute name="{name()}"><xsl:value-of select="." /></xsl:attribute>
</xsl:template>
<xsl:template match="PointToPointInterface/AreaID|PointToPointInterface/InterfaceOutputCost|PointToPointInterface/RetransmissionInterval|PointToPointInterface/InterfaceTransmissionDelay|PointToPointInterface/RouterPriority|PointToPointInterface/HelloInterval|PointToPointInterface/RouterDeadInterval|PointToPointInterface/AuthenticationType|PointToPointInterface/AuthenticationKey">
</xsl:template>


<!-- PointToMultiPointInterface -->
<xsl:template match="PointToMultiPointInterface/AreaID|PointToMultiPointInterface/InterfaceOutputCost|PointToMultiPointInterface/RetransmissionInterval|PointToMultiPointInterface/InterfaceTransmissionDelay|PointToMultiPointInterface/RouterPriority|PointToMultiPointInterface/HelloInterval|PointToMultiPointInterface/RouterDeadInterval|PointToMultiPointInterface/AuthenticationType|PointToMultiPointInterface/AuthenticationKey" mode="toAttr">
  <xsl:attribute name="{name()}"><xsl:value-of select="." /></xsl:attribute>
</xsl:template>
<xsl:template match="PointToMultiPointInterface/AreaID|PointToMultiPointInterface/InterfaceOutputCost|PointToMultiPointInterface/RetransmissionInterval|PointToMultiPointInterface/InterfaceTransmissionDelay|PointToMultiPointInterface/RouterPriority|PointToMultiPointInterface/HelloInterval|PointToMultiPointInterface/RouterDeadInterval|PointToMultiPointInterface/AuthenticationType|PointToMultiPointInterface/AuthenticationKey">
</xsl:template>


<!-- NBMAInterface -->
<xsl:template match="NBMAInterface/AreaID|NBMAInterface/InterfaceOutputCost|NBMAInterface/RetransmissionInterval|NBMAInterface/InterfaceTransmissionDelay|NBMAInterface/RouterPriority|NBMAInterface/HelloInterval|NBMAInterface/RouterDeadInterval|NBMAInterface/AuthenticationType|NBMAInterface/AuthenticationKey" mode="toAttr">
  <xsl:attribute name="{name()}"><xsl:value-of select="." /></xsl:attribute>
</xsl:template>
<xsl:template match="NBMAInterface/AreaID|NBMAInterface/InterfaceOutputCost|NBMAInterface/RetransmissionInterval|NBMAInterface/InterfaceTransmissionDelay|NBMAInterface/RouterPriority|NBMAInterface/HelloInterval|NBMAInterface/RouterDeadInterval|NBMAInterface/AuthenticationType|NBMAInterface/AuthenticationKey">
</xsl:template>


<!-- HostInterface -->
<xsl:template match="HostInterface/AreaID|HostInterface/AttachedHost|HostInterface/LinkCost" mode="toAttr">
  <xsl:attribute name="{name()}"><xsl:value-of select="." /></xsl:attribute>
</xsl:template>
<xsl:template match="HostInterface/AreaID|HostInterface/AttachedHost|HostInterface/LinkCost">
</xsl:template>


<!-- VirtualLink -->
<xsl:template match="VirtualLink/TransitAreaID|VirtualLink/RetransmissionInterval|VirtualLink/InterfaceTransmissionDelay|VirtualLink/HelloInterval|VirtualLink/RouterDeadInterval|VirtualLink/AuthenticationType|VirtualLink/AuthenticationKey" mode="toAttr">
  <xsl:attribute name="{name()}"><xsl:value-of select="." /></xsl:attribute>
</xsl:template>
<xsl:template match="VirtualLink/TransitAreaID|VirtualLink/RetransmissionInterval|VirtualLink/InterfaceTransmissionDelay|VirtualLink/HelloInterval|VirtualLink/RouterDeadInterval|VirtualLink/AuthenticationType|VirtualLink/AuthenticationKey">
</xsl:template>


<!-- ExternalInterface -->
<xsl:template match="ExternalInterface/ForwardingAddress|ExternalInterface/ExternalRouteTag" mode="toAttr">
  <xsl:attribute name="{local-name()}"><xsl:value-of select="." /></xsl:attribute>
</xsl:template>
<xsl:template match="ExternalInterface/ForwardingAddress|ExternalInterface/ExternalRouteTag">
</xsl:template>

<xsl:template match="ExternalInterface/AdvertisedExternalNetwork" mode="toAttr">
  <xsl:for-each select="Address|Mask">
    <xsl:attribute name="AdvertisedExternalNetwork{name()}"><xsl:value-of select="." /></xsl:attribute>
  </xsl:for-each>
</xsl:template>
<xsl:template match="ExternalInterface/AdvertisedExternalNetwork">
  <xsl:for-each select="*[local-name()!='Address' and local-name()!='Mask']">
    <xsl:apply-templates select="."/>
  </xsl:for-each>
</xsl:template>

<xsl:template match="ExternalInterface/ExternalInterfaceOutputParameters" mode="toAttr">
  <xsl:for-each select="ExternalInterfaceOutputType|ExternalInterfaceOutputCost">
    <xsl:attribute name="{name()}"><xsl:value-of select="." /></xsl:attribute>
  </xsl:for-each>
</xsl:template>
<xsl:template match="ExternalInterface/ExternalInterfaceOutputParameters">
  <xsl:for-each select="*[local-name()!='ExternalInterfaceOutputType' and local-name()!='ExternalInterfaceOutputCost']">
    <xsl:apply-templates select="."/>
  </xsl:for-each>
</xsl:template>


<!-- do not convert other nodes to attribute -->
<xsl:template match="*" mode="toAttr">
</xsl:template>


<!-- -->
<xsl:template match="AddressRange|BroadcastInterface|PointToPointInterface|PointToMultiPointInterface|NBMAInterface|HostInterface|VirtualLink|ExternalInterface">
<!--
<xsl:template match="*">
-->
  <xsl:element name="{name()}">
    <xsl:for-each select="@*">
      <xsl:attribute name="{local-name()}"><xsl:value-of select="." /></xsl:attribute>
    </xsl:for-each>
    <xsl:for-each select="*">
      <xsl:apply-templates select="." mode="toAttr"/>
    </xsl:for-each>
    <xsl:for-each select="*">
      <xsl:apply-templates select="."/>
    </xsl:for-each>
  </xsl:element>
</xsl:template>


<xsl:template match="@*" mode="toAttr">
  <xsl:copy>
    <xsl:apply-templates select="@*"/>
  </xsl:copy>
</xsl:template>

<xsl:template match="@*|node()">
  <xsl:copy>
    <xsl:apply-templates select="@*|node()"/>
  </xsl:copy>
</xsl:template>

</xsl:stylesheet>

