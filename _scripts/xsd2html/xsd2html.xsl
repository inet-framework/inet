<?xml version="1.0"?>
<!--
 Author: Andras Varga, based on xsd2html.xsl by Christopher R. Maden
 http://crism.maden.org/consulting/pub/xsl/xsd2html.xsl
-->
<xsl:stylesheet version="1.0"
  xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
  xmlns:xs="http://www.w3.org/2001/XMLSchema"
  xmlns:date="http://exslt.org/dates-and-times"
  exclude-result-prefixes="xs">

<xsl:output method="html"/>

<!-- directory to generate output -->
<xsl:param name="outputdir" select="'html'"/>

<!-- to enable generating diagrams via .dot files, set this to 'yes' -->
<xsl:param name="have-dot" select="'yes'"/>

<xsl:template match="xs:schema">
   <xsl:document href="{$outputdir}/index.html">
     <html>
        <head>
           <title><xsl:value-of select="xs:annotation[xs:documentation][1]/xs:documentation"/></title>
        </head>
        <frameset cols="30%,70%">
           <frame src="toc.html" name="indexframe"/>
           <frame src="overview.html" name="mainframe"/>
        </frameset>
        <noframes>
           <h2>Frame Alert</h2>
           <p>This document is designed to be viewed using HTML frames. If you see this message,
           you are using a non-frame-capable browser.</p>
        </noframes>
     </html>
   </xsl:document>

   <xsl:document href="{$outputdir}/style.css" method="text">
      body,td,p,ul,ol,li,h1,h2,h3,h4 {font-family:arial,sans-serif }
      body,td,p,ul,ol,li { font-size:10pt }
      h1 { font-size:18pt; text-align:center }
      pre.comment { font-size:10pt; padding-left:5pt }
      pre.src { font-size:8pt; background:#E0E0E0; padding-left:5pt }
      th { font-size:10pt; text-align:left; vertical-align:top; background:#E0E0f0 }
      td { font-size:10pt; text-align:left; vertical-align:top }
      tt { font-family:Courier,Courier New,Fixed,Terminal }
      img          { border:none }
      .navbar     { font-size:8pt; }
      .navbarlink { font-size:8pt; }
      .indextitle { font-size:12pt; }
      .comptitle  { font-size:14pt; }
      .subtitle   { font-size:12pt; margin-bottom: 3px}
      .footer     { font-size:8pt; margin-top:0px; text-align:center; color:#303030; }
      FIXME.paramtable { border:2px ridge; border-collapse:collapse;}
      .src-keyword { font-weight:bold }
      .src-comment { font-style:italic; color:#404040 }
      .src-string  { color:#006000 }
      .src-number  { color:#0000c0 }
   </xsl:document>

   <xsl:document href="{$outputdir}/usage-diagram.dot" method="text">
      <xsl:call-template name="create-usage-diagram"/>
   </xsl:document>

   <xsl:call-template name="write-html-page">
      <xsl:with-param name="href" select="'toc.html'"/>
      <xsl:with-param name="content">
         <xsl:call-template name="toc"/>
      </xsl:with-param>
   </xsl:call-template>

   <!-- generate html -->
   <xsl:call-template name="write-html-page">
      <xsl:with-param name="href" select="'overview.html'"/>
      <xsl:with-param name="content">
         <xsl:apply-templates/>
      </xsl:with-param>
   </xsl:call-template>

   <xsl:call-template name="write-html-page">
      <xsl:with-param name="href" select="'usage-diagram.html'"/>
      <xsl:with-param name="content">
         <h2 class="comptitle">Usage Diagram</h2>
         <p>The following diagram shows usage relationships between elements.</p>
         <img src="usage-diagram.gif" ismap="yes" usemap="#usage-diagram"/>
         <map name="usage-diagram">@INSERTFILE(usage-diagram.map)</map>
      </xsl:with-param>
   </xsl:call-template>
</xsl:template>

<xsl:template name="toc">
  <xsl:if test="/xs:schema/xs:element">
    <h3>XML Elements:</h3>
    <ul>
      <li><a href="usage-diagram.html" target="mainframe">Usage diagram</a></li>
    </ul>
    <ul>
      <xsl:for-each select="/xs:schema/xs:element">
        <xsl:sort select="@name"/>
        <li>
          <a href="{@name}.html" target="mainframe">
            <xsl:value-of select="@name"/>
          </a>
        </li>
      </xsl:for-each>
    </ul>
    <h3>Attribute Groups:</h3>
    <ul>
      <xsl:for-each select="/xs:schema/xs:attributeGroup">
        <xsl:sort select="@name"/>
        <li>
          <a href="{@name}.html" target="mainframe">
            <xsl:value-of select="@name"/>
          </a>
        </li>
      </xsl:for-each>
    </ul>
  </xsl:if>
  <xsl:if test="/xs:schema/xs:group">
    <h3>Content Model Groups:</h3>
    <ul>
      <xsl:for-each select="/xs:schema/xs:group">
        <xsl:sort select="@name"/>
        <li>
          <a href="{@name}.html" target="mainframe">
            <xsl:value-of select="@name"/>
          </a>
        </li>
      </xsl:for-each>
    </ul>
  </xsl:if>
</xsl:template>

<xsl:template name="write-html-page">
   <xsl:param name="href"/>
   <xsl:param name="content"/>
   <xsl:document href="{$outputdir}/{$href}" method="html" indent="yes">
      <html>
         <head>
            <link rel="stylesheet" type="text/css" href="style.css" />
         </head>
         <body>
            <xsl:copy-of select="$content"/>
            <xsl:if test="$href!='toc.html'">
              <p>Generated on <xsl:value-of select="date:date-time()"/></p>
            </xsl:if>
         </body>
      </html>
   </xsl:document>
</xsl:template>

<xsl:template name="create-element-usage-diagram">
   digraph opp {
      node [fontsize=10,fontname=helvetica,shape=box,height=.25,style=filled];
      <xsl:variable name="name" select="@name"/>
      <xsl:value-of select="@name"/> [URL="<xsl:value-of select="@name"/>.html",fillcolor="#fff700",tooltip="element <xsl:value-of select="@name"/>"];
      <xsl:for-each select=".//xs:element[@ref]">
         <xsl:value-of select="@ref"/> [URL="<xsl:value-of select="@ref"/>.html",fillcolor="#fffcaf",tooltip="element <xsl:value-of select="@ref"/>"];
         <xsl:value-of select="$name"/> -> <xsl:value-of select="@ref"/>;
      </xsl:for-each>
      <xsl:for-each select="//xs:element[.//xs:element[@ref=$name]]">
         <xsl:value-of select="@name"/> [URL="<xsl:value-of select="@name"/>.html",fillcolor="#fffcaf",tooltip="element <xsl:value-of select="@name"/>"];
         <xsl:value-of select="@name"/> -> <xsl:value-of select="$name"/>;
      </xsl:for-each>
   }
</xsl:template>

<xsl:template name="do-diagram-node">
</xsl:template>

<xsl:template match="xs:simpleType">
   <a name="simple-type-{@name}"/>
   <p><b>Simple type <xsl:value-of select="@name"/></b></p>
   <xsl:apply-templates/>

</xsl:template>


<xsl:template match="xs:attribute" mode="attributes">
  <tr valign="baseline">
    <td align="left" rowspan="2">
      <b><xsl:value-of select="@name"/></b>
    </td>
    <td>
      <xsl:choose>
        <xsl:when test="xs:simpleType">
          <xsl:apply-templates select="xs:simpleType"/>
        </xsl:when>
        <xsl:when test="@type">
          <xsl:call-template name="simpleType">
            <xsl:with-param name="type" select="@type"/>
          </xsl:call-template>
        </xsl:when>
      </xsl:choose>
    </td>
    <td>
      <xsl:choose>
        <xsl:when test="@use">
          <xsl:value-of select="@use"/>
        </xsl:when>
        <xsl:otherwise>
          <xsl:text>optional</xsl:text>
        </xsl:otherwise>
      </xsl:choose>
      <xsl:if test="@default">
        <xsl:text>; default: </xsl:text>
        <code>
          <xsl:value-of select="@default"/>
        </code>
      </xsl:if>
    </td>
  </tr>
  <tr valign="baseline">
    <td colspan="2">
      <xsl:apply-templates select="xs:annotation"/>
    </td>
  </tr>
</xsl:template>

<xsl:template match="xs:attribute"/>
<xsl:template match="xs:attributeGroup[@ref]" mode="attGroup">
    <h3>Attribute Group <a href="{@ref}.html"> <xsl:value-of select="@ref"/>
      </a> </h3>
</xsl:template>
<xsl:template match="xs:attributeGroup[@name]">
    <xsl:call-template name="write-html-page">
      <xsl:with-param name="href" select="concat(@name,'.html')"/>
      <xsl:with-param name="content">
        <h2 id="attribute-group-{@name}">
          <xsl:text>Attribute Group </xsl:text>
          <xsl:value-of select="@name"/>
        </h2>
        <table border="1">
          <xsl:apply-templates select="descendant::xs:attribute"
            mode="attributes"/>
        </table>
      </xsl:with-param>
    </xsl:call-template>
</xsl:template>

<xsl:template match="xs:choice/xs:choice | xs:sequence/xs:choice">
  <li>
    <xsl:call-template name="occurrence"/>
    <xsl:text> choice of:</xsl:text>
    <ul>
      <xsl:apply-templates/>
    </ul>
  </li>
</xsl:template>

<xsl:template match="xs:choice">
  <p>
    <xsl:call-template name="occurrence"/>
    <xsl:text> choice of:</xsl:text>
  </p>
  <ul>
    <xsl:apply-templates/>
  </ul>
</xsl:template>

<xsl:template match="xs:complexType">
  <xsl:choose>
    <xsl:when test="xs:all">
      <p>All</p>
    </xsl:when>
    <xsl:when test="xs:choice | xs:complexContent | xs:group | xs:sequence | xs:simpleContent">
      <xsl:if test="@mixed='true'">
        <p>Mixed content, including:</p>
      </xsl:if>
      <xsl:apply-templates/>
    </xsl:when>
    <xsl:when test="@mixed='true'">
      <p>Character content</p>
    </xsl:when>
    <xsl:otherwise>
      <p>Empty</p>
    </xsl:otherwise>
  </xsl:choose>
</xsl:template>

<xsl:template match="xs:schema/xs:annotation/xs:documentation">
  <xsl:choose>
    <xsl:when test="count(..|../../xs:annotation[xs:documentation][1]) = 1">
      <h1>
        <xsl:copy-of select="*|text()"/>
      </h1>
      <!-- <xsl:call-template name="toc"/> -->
    </xsl:when>
    <xsl:otherwise>
      <p>
        <xsl:copy-of select="*|text()"/>
      </p>
    </xsl:otherwise>
  </xsl:choose>
</xsl:template>

<xsl:template match="xs:documentation">
  <p>
    <xsl:copy-of select="*|text()"/>
  </p>
</xsl:template>

<xsl:template match="xs:element[@name]">
   <xsl:call-template name="write-html-page">
      <xsl:with-param name="href" select="concat(@name,'.html')"/>
      <xsl:with-param name="content">
         <h2 id="element-type-{@name}">
           <xsl:text>Element Type </xsl:text>
           <xsl:value-of select="@name"/>
         </h2>
         <xsl:if test="/xs:schema/@targetNamespace">
           <p>
             <xsl:text>Namespace: </xsl:text>
             <code>
               <xsl:value-of select="/xs:schema/@targetNamespace"/>
             </code>
           </p>
         </xsl:if>
         <xsl:apply-templates select="xs:annotation"/>

         <h3>Usage Diagram</h3>
         <p>The following diagram shows usage relationships between elements.</p>
         <img src="{@name}.gif" ismap="yes" usemap="#usage-diagram"/>
         <map name="usage-diagram">@INSERTFILE(<xsl:value-of select="@name"/>.map)</map>

         <xsl:document href="{$outputdir}/{@name}.dot" method="text">
            <xsl:call-template name="create-element-usage-diagram"/>
         </xsl:document>

         <h3>Content Model</h3>
         <xsl:choose>
           <xsl:when test="xs:complexType">
             <xsl:apply-templates select="xs:complexType"/>
           </xsl:when>
           <xsl:when test="xs:simpleType">
             <xsl:apply-templates select="xs:simpleType"/>
           </xsl:when>
           <xsl:when test="@type">
             <p>
               <xsl:call-template name="simpleType">
                 <xsl:with-param name="type" select="@type"/>
               </xsl:call-template>
             </p>
           </xsl:when>
           <xsl:otherwise>
             <p>Empty</p>
           </xsl:otherwise>
         </xsl:choose>
         <xsl:if test="descendant::xs:attribute">
           <h3>Attributes</h3>
           <table border="1">
             <xsl:apply-templates select="descendant::xs:attribute" mode="attributes"/>
           </table>
         </xsl:if>
          <xsl:if test="descendant::xs:attributeGroup[@ref]">
            <xsl:apply-templates select="descendant::xs:attributeGroup"
              mode="attGroup"/>
          </xsl:if>
      </xsl:with-param>
   </xsl:call-template>
</xsl:template>

<xsl:template match="xs:choice/xs:element[@ref] | xs:sequence/xs:element[@ref]">
  <li>
    <xsl:call-template name="occurrence"/>
    <xsl:text> </xsl:text>
    <a href="{@ref}.html">
      <xsl:value-of select="@ref"/>
    </a>
  </li>
</xsl:template>

<xsl:template match="xs:enumeration">
  <li>
    <code>
      <xsl:value-of select="@value"/>
    </code>
  </li>
</xsl:template>

<xsl:template match="xs:group[@name]">
   <xsl:call-template name="write-html-page">
      <xsl:with-param name="href" select="concat(@name,'.html')"/>
      <xsl:with-param name="content">
         <h2>
           <xsl:text>Content Model Group </xsl:text>
           <xsl:value-of select="@name"/>
         </h2>
         <xsl:apply-templates select="xs:annotation"/>
         <xsl:if test="descendant::xs:attribute">
           <table border="1">
             <caption>Attributes</caption>
             <xsl:apply-templates select="descendant::xs:attribute"
               mode="attributes"/>
           </table>
         </xsl:if>
         <h3>Content Particle</h3>
         <xsl:apply-templates select="xs:choice | xs:complexContent | xs:group | xs:sequence | xs:simpleContent"/>
      </xsl:with-param>
   </xsl:call-template>
</xsl:template>

<xsl:template match="xs:choice/xs:group[@ref] | xs:sequence/xs:group[@ref]" priority="2">
  <li>
    <xsl:call-template name="occurrence"/>
    <xsl:text> </xsl:text>
    <a href="#group-{@ref}">
      <xsl:value-of select="@ref"/>
    </a>
    <xsl:text> group</xsl:text>
  </li>
</xsl:template>

<xsl:template match="xs:group[@ref]" priority="1">
  <ul>
    <li>
      <xsl:call-template name="occurrence"/>
      <xsl:text> </xsl:text>
      <a href="#group-{@ref}">
        <xsl:value-of select="@ref"/>
      </a>
      <xsl:text> group</xsl:text>
    </li>
  </ul>
</xsl:template>

<xsl:template match="xs:restriction">
  <p>
    <xsl:call-template name="simpleType">
      <xsl:with-param name="type" select="@base"/>
    </xsl:call-template>
  </p>
  <xsl:if test="xs:enumeration">
    <p>
      <xsl:text>Enumeration:</xsl:text>
    </p>
    <ul>
      <xsl:apply-templates select="xs:enumeration"/>
    </ul>
  </xsl:if>
</xsl:template>

<xsl:template name="create-usage-diagram">
   digraph xsd {
      node [fontsize=10,fontname=helvetica,shape=box,height=.25,style=filled,fillcolor="#fffcaf"];
      <xsl:for-each select="//xs:element[@name]">
         <xsl:variable name="name" select="@name"/>
         <xsl:value-of select="@name"/> [URL="<xsl:value-of select="concat(@name,'.html')"/>"];
         <xsl:for-each select=".//xs:element[@ref]">
            <xsl:value-of select="$name"/> -> <xsl:value-of select="@ref"/>;
         </xsl:for-each>
      </xsl:for-each>
   }
</xsl:template>

<xsl:template match="xs:choice/xs:sequence | xs:sequence/xs:sequence">
  <li>
    <xsl:call-template name="occurrence"/>
    <xsl:text> sequences of:</xsl:text>
    <ol>
      <xsl:apply-templates/>
    </ol>
  </li>
</xsl:template>

<xsl:template match="xs:sequence">
  <p>
    <xsl:call-template name="occurrence"/>
    <xsl:text> sequences of:</xsl:text>
  </p>
  <ol>
    <xsl:apply-templates/>
  </ol>
</xsl:template>

<xsl:template name="occurrence">
  <xsl:variable name="minOccurs">
    <xsl:choose>
      <xsl:when test="@minOccurs">
        <xsl:value-of select="@minOccurs"/>
      </xsl:when>
      <xsl:otherwise>
        <xsl:value-of select="1"/>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:variable>
  <xsl:variable name="maxOccurs">
    <xsl:choose>
      <xsl:when test="@maxOccurs">
        <xsl:value-of select="@maxOccurs"/>
      </xsl:when>
      <xsl:otherwise>
        <xsl:value-of select="1"/>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:variable>
  <xsl:choose>
    <xsl:when test="$minOccurs = $maxOccurs">
      <xsl:text>Exactly </xsl:text>
      <xsl:value-of select="$minOccurs"/>
    </xsl:when>
    <xsl:when test="$minOccurs = 0 and $maxOccurs = 'unbounded'">
      <xsl:text>Optional repeatable</xsl:text>
    </xsl:when>
    <xsl:when test="$maxOccurs = 'unbounded'">
      <xsl:value-of select="$minOccurs"/>
      <xsl:text> or more</xsl:text>
    </xsl:when>
    <xsl:when test="$minOccurs = 0 and $maxOccurs = 1">
      <xsl:text>An optional</xsl:text>
    </xsl:when>
    <xsl:when test="$minOccurs = 0">
      <xsl:text>Up to </xsl:text>
      <xsl:value-of select="$maxOccurs"/>
    </xsl:when>
    <xsl:otherwise>
      <xsl:text>Between </xsl:text>
      <xsl:value-of select="$minOccurs"/>
      <xsl:text> and </xsl:text>
      <xsl:value-of select="$maxOccurs"/>
    </xsl:otherwise>
  </xsl:choose>
</xsl:template>

<xsl:template name="simpleType">
  <xsl:param name="type"/>
  <xsl:choose>
    <xsl:when test="starts-with($type, 'xs:')">
      <xsl:text>Built-in type </xsl:text>
      <xsl:value-of select="substring-after($type, ':')"/>
    </xsl:when>
    <xsl:otherwise>
      <xsl:text>Simple type </xsl:text>
      <a href="overview.html#simple-type-{$type}">
        <xsl:value-of select="$type"/>
      </a>
    </xsl:otherwise>
  </xsl:choose>
</xsl:template>

</xsl:stylesheet>

