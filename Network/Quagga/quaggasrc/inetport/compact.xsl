<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">

<xsl:output method="xml" indent="yes" encoding="iso-8859-1"/>

<xsl:template match="/doxygenindex">
  <xsl:message>Extracting sources...</xsl:message>
  <xsl:document href="_source.xml">
    <sources>
    <xsl:call-template name="newline"/>
    <xsl:apply-templates select="compound[@kind='file']"/>
    <xsl:apply-templates select="compound[@kind='struct']"/>
    </sources>
    <xsl:call-template name="newline"/>
  </xsl:document>
</xsl:template>

<xsl:template match="compound">
  <xsl:variable name="fname">
    <xsl:value-of select="concat('xml/', concat(@refid, '.xml'))"/>
  </xsl:variable>
  <xsl:copy-of select="document($fname)"/>
  <xsl:call-template name="newline"/>
</xsl:template>

<xsl:template name="newline"><xsl:text>
</xsl:text></xsl:template>

</xsl:stylesheet>
