<xsl:stylesheet version="1.0"
		xmlns:xsl="http://www.w3.org/1999/XSL/Transform" 
		xmlns:str="http://exslt.org/strings" 
		extension-element-prefixes="str">

<xsl:output method="text" indent="yes" encoding="iso-8859-1"/>

<xsl:key name='findvar' match="//compounddef[@kind='file']/sectiondef[@kind='var']/memberdef[@kind='variable']" use="name/text()"/> 

<xsl:template match="/sources">
  <xsl:message>Processing problematic structures...</xsl:message>
  <xsl:document href="02_struct.patch" method="text">
    <xsl:text>@;S;&gt;structs.h</xsl:text>
    <xsl:call-template name="newline"/>
    <xsl:text>w;S;#ifndef __STRUCTS_H__</xsl:text>
    <xsl:call-template name="newline"/>
    <xsl:text>w;S;#define __STRUCTS_H__</xsl:text>
    <xsl:call-template name="newline"/>
    <xsl:for-each select="//compounddef[@kind='struct']">
      <xsl:sort select="compoundname"/>
      <xsl:if test="key('findvar', compoundname/text())">
	<xsl:text>S;</xsl:text>
	<xsl:value-of select="compoundname"/>
	<xsl:call-template name="newline"/>
      </xsl:if>
    </xsl:for-each>
    <xsl:for-each select="//compounddef[@kind='struct']/sectiondef/memberdef[@kind='variable']">
      <xsl:sort select="name"/>
      <xsl:if test="key('findvar', name/text())">
	<xsl:value-of select="concat('T;', name, ';', location/@file, ';', location/@line)"/>
	<xsl:call-template name="newline"/>
      </xsl:if>
    </xsl:for-each>
    <xsl:text>w;S;#endif</xsl:text>
    <xsl:call-template name="newline"/>
  </xsl:document>
</xsl:template>

<xsl:template name="newline"><xsl:text>
</xsl:text></xsl:template>

</xsl:stylesheet>
