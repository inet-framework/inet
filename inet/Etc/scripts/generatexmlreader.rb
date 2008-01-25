#! /usr/bin/env ruby
#
#  Author: Johnny Lai
#  Copyright (c) 2004 Johnny Lai
#
# =DESCRIPTION
# Script to generate C++ code to read XML data from document instances of
# netconf.xsd.
#
# =REVISION HISTORY
#  INI 2004-02-20 Changes
#
# {{ Includes

require 'optparse'
require 'pp'
require "rexml/document"
include REXML

#require "rexml/parsers/sax2parser"

# }}

#@@Attributes = "xs:complexType/xs:attribute"

###Todo Fix #ifdef thingies

#
# Generate C++ headers to store XML data from XML Schema
# Generate C++ XML reading code to fill class data members
#
class GenerateXMLReader
  VERSION       = "$Revision$"
  REVISION_DATE = "$Date$"
  AUTHOR        = "Johnny Lai"
 
  #
  # Returns a version string similar to:
  #  <app_name>:  Version: 1.2 Created on: 2002/05/08 by Jim Freeze
  # The version number is maintained by CVS. 
  # The date is the last checkin date of this file.
  # 
  def version
    "Version: #{VERSION.split[1]} Created on: " +
      "#{REVISION_DATE.split[1]} by #{AUTHOR}"
  end

  # Can use YAML to encode the Schema to Cpp mapping and read it into @xmlToCpp
  # here

  def initialize
    @debug    = false
    @verbose  = false
    @quit = false
    @opt_subset = ""

    @opt_schema = false
    @elements = Hash.new
    @types = Hash.new

    
    @xmlToCpp = { "xs:NMTOKEN", "string",
                 "IfaceType", "string",
                 "xs:string", "string",
                 "BooleanType", "bool",
                 "IPv6PrefixType", "IPv6Address",
                 "IPv6AddressType", "IPv6Address",
                 "IPv6AddressStrictType", "ipv6_addr",
                 "xs:decimal", "double"
    }

    @elementToClass = { "node", "RoutingTable6XMLBase",
                       "netconf", "IPv6XMLWrapManagerBase",
                       "interface", "Interface6EntryXMLBase",
                       "WEInfo", "WirelessEtherModuleXMLBase"
    }

    #Stores the generated classes as a { classname => classdefinition }
    @genClasses = Hash.new

    #While it is probably better to introduce an extra fixed attribute into the
    #attributeGroup to declare this mapping in the XML explicitly it is faster
    #to do it this way for now
    @attributeToOption = { "Host", nil,
                          "MIPv6", "USE_MOBILITY",
                          "HMIP", "USE_HMIP"                          
    }

    ### Things we don't want to parse for header generation (array like xml bits)
    @skips = ["netconf", "route", "sourceRoute", "tunnel"]

    #Any attribute in interface without a Host word in the front means it is a
    #host and anything else is a router

    get_options

    if 0 == ARGV.size
      #STDERR.puts usage
      #exit 0
    end

  rescue => err
    STDERR.puts err
    STDERR.puts usage
    exit 1
  end

  #
  # Returns usage string
  #
  def usage
    print ARGV.options
  end

  #
  # Processes command line arguments
  #
  def get_options
    ARGV.options { |opt|
      opt.banner = "Usage: ruby #{__FILE__} [options] "

      opt.on("--help", "What you see right now"){ puts opt; exit 0}

      #Try testing with this 
      #ruby __FILE__ -x -c -s test
      opt.on("-x", "parse arguments and show Usage") {|@quit|}

      opt.on("--doc=DIRECTORY", String, "Output rdoc (Ruby HTML documentation) into directory"){|dir|
        system("rdoc -o #{dir} #{__FILE__}")
      }

      opt.on("--verbose", "-v", "print intermediate steps to STDERR"){|@verbose|}

      opt.on("--schema", "-S", "Use Schema i.e. XSD rather than XML document"){|@opt_schema|}
      
      opt.on_tail("By default splits data according to opt_filter, ",
                  "Subset plots on #{@opt_subset}. Will not create histograms")

      opt.parse!
    } or  exit(1);

    if @quit
      pp self
      (print ARGV.options; exit)
    end

  rescue NameError => err
    STDERR.puts "ERROR: #{err}"
    exit 1
  rescue => err
    STDERR.puts "ERROR: #{err}"
    exit 1
  end

  #
  # Launches the application
  #  GenerateXMLReader.new.run
  #
  def run
    if (@opt_schema) then
      file = File.new("/home/jmll/src/IPv6Suite/Etc/netconf.xsd")
    else
      file = File.new("/home/jmll/src/IPv6Suite/Etc/HMIPv6Network-schema.xml")
    end

#    doc = Document.new(string)
    @doc = Document.new(file)

    ###Pretty dumb you need to specify exactly which elements you want to listen to.

#   @parser = Parsers::SAX2Parser.new(File.new(file.path))
#   @parser.listen( %w{ xs:element xs:sequence xs:complexType xs:simpleType xs:attributeGroup xs:enumeration xs:attribute } ) do |uri, localname, qname, attrs|
#     next if localname =~ /annotation/
#     puts "ln is #{localname}"
#    #puts "qname is #{qname}"
#    #puts "attrs is #{attrs}"
#  end
#  @parser.parse

    parseSchema if @opt_schema
    parseSchemaInstance if not @opt_schema
  end

  def parseSchemaInstance
    @doc.elements.each("netconf/local"){|elem|
      puts elem.attributes["node"]
    }  
  end

  ###parseSimpleType/Content and ComplexContent prob. similar with respect to
  ###restriction and extension. Refactor.
  def parseSimpleType(elem, anonName=nil)
    valid = false
    if not elem.attributes.has_key?("name") then
      throw "anonName argument is nil" if not anonName
    else
      name = elem.attributes["name"] 
    end

    restrictElem = elem.elements["xs:restriction"]
    throw "No restriction found for simpleType: #{elem}" if not restrictElem
    case restrictElem.attributes["base"]
    when "xs:string"
      #anonymous or not does not really indicate whether it is an enumerated type
      #refactor this too.
      if not elem.attributes.has_key?("name") then      
        enums=Array.new()      
        throw "No enumerations in simpleType #{elem}" if not restrictElem.elements["xs:enumeration"]

        elem.elements.each("xs:restriction/xs:enumeration"){|e|
          throw "Unknown SimpleType:enumeration construct #{e}" if not e.attributes.has_key?("value")
          enums.push(e.attributes["value"])
        }
        if @verbose then
          print "#{anonName} <= "
          enums.each{|a| 
            print a, " "
          } 
          puts
        end
        #enum = "#{anonName}<=" 
        enum = ""
        enums.each{|a|
          if enum == "" then
            enum = "#{a}" 
          else
            enum = enum + ", " + "#{a}"
          end
        }
        throw "anonymous SimpleType #{anonName} already declared before as #{@types[anonName]} and now as #{enum}" if @types.has_key?(anonName)
        @types[anonName]=enum
        return {anonName, anonName}
      else # has name attribute
        print("simpleType of ", elem.attributes["name"], "\n") if @verbose
        throw "SimpleType #{name} already declared before as #{@types[name]} and now as #{name}" if @types.has_key?(name)
        @types[name]=name
        return {name, name}
        #mapXMLToCpp(elem.attributes["name"])
      end
    else
      @types[name]=restrictElem.attributes["base"]
      return {name, @types[name]}
    end
  end

  def parseSchema
    @doc.root.elements.each("xs:simpleType"){|elem|
      parseSimpleType(elem)
    }
    @doc.root.elements.each("xs:attributeGroup"){|elem|
      parseAttributeGroups(elem)
    }
    @doc.root.elements.each("xs:element"){|elem|
      case elem.attributes["name"]
   #     when "netconf"
   #        generateXMLWrapManagerBase elem
      when /.*/
        parseElement(elem)
     #when "global"
     
     #        generateTemplate elem
     #        when "local"
      end

    }

    pp @elements if @debug
    pp @types if @debug

    @@indent = "  "
       #, "misc"
    @elementToClass.keys.each {|elemName|
      case elemName
      when "interface"
      else
      end

      @filename = elemName

      @genClasses[elemName] = genClassHeader(elemName)

      resolveElementBranch(elemName)

      @genClasses[elemName] += "};"

      puts @genClasses[elemName]

    }
  end

  def  resolveElementBranch(elemName, arrayComp = nil)
    if elemName != @filename then
        return if @elementToClass.keys.include?(elemName)
    end

    if not arrayComp then

      throw "element #{elemName} not found in @elements" if not
      @elements.has_key?(elemName)

      throw "Structure of @element tree is wrong as #{elemName} has hash type of \
    #{@elements[elemName].class.name}" if @elements[elemName].class != Array

      @elements[elemName].each{|a|
        resolveElementBranch(elemName, a)
      }
    else

      case arrayComp.class.name
      when "String"
        #Recursively descend by looking up in @elements or @types until we reach
        #a Hash I think
        if @elements.has_key?(arrayComp) then
          resolveElementBranch(arrayComp, nil)
        else

          #throw "type #{arrayComp} for element #{elemName} not found in
          #  @elements and @types" if not @types.has_key?(arrayComp)
          if @types.has_key?(arrayComp)
            puts "#{elemName}: #{arrayComp}" 
            @elemName = elemName            
            resolveTypeBranch(arrayComp)
          else
            if cppType?(arrayComp) then
              @genClasses[@filename] += (@@indent + cppCode(mapXMLToCpp(arrayComp), "_" + elemName) + "\n")
            else

              throw "Unable to resolve further for element #{elemName} of
            #{arrayComp}" 

            end

          end
        end
      when "Hash"
        @genClasses[@filename] += (@@indent + cppCode(mapXMLToCpp(arrayComp.values[0]), "_" + arrayComp.keys[0]) + "\n")
      else
        throw "Unknown @element #{elemName} with type of #{arrayComp.class.name}"
      end

    end

  end

  def cppType?(value)
    begin
      mapXMLToCpp(value)
    rescue
      return false     
    else
      return true 
    end
  end


  ##Types can either be basic types or attributeGroups 
  def resolveTypeBranch(typeName, arrayComp = nil)
    #    puts "typeName is #{typeName}: #{arrayComp}"

    if not arrayComp then
      search = typeName
    else
      search = arrayComp
    end

    if cppType?(search) then
      return @genClasses[@filename] += (@@indent + cppCode(mapXMLToCpp(search), "_" + @elemName) + "\n")
    end
    
    found = false
    if @types.has_key?(search) then
      found = true
      value = @types[search]
      case value.class.name
      when "Array"
        value.each{|a|
          @elemName = a.keys[0]
          ## condition true means it has to be a resolvable simple type
          if a.keys[0] == a.values[0] then
            puts "key=>value match? #{a.keys[0]} and #{a.values[0]}"
            if not cppType?(search) then
              #One more search
              puts "One more search"
              resolveTypeBranch(a.values[0])
            else
              return @genClasses[@filename] += (@@indent + cppCode(mapXMLToCpp(a.values[0]), "_" + @elemName) + "\n")
            end
          else
            resolveTypeBranch(a.keys[0], a.values[0])
          end
        }
      when "String"
        puts "typeName = #{typeName} arrayComp #{arrayComp} @elemName #{@elemName} value #{value}"
        resolveTypeBranch(value)
      else
      end #end case
    end #end types.has_key

    if not found then
      if cppType?(search) then
        return @genClasses[@filename] += (@@indent + cppCode(mapXMLToCpp(search), "_" + @elemName) + "\n")
      else
        throw "Failed to resolve xmlType #{search} for element/attribute #{@elemName}"
      end
    end
  end


  ##Types can either be basic types or attributeGroups 
  def resolveTypeBranch2(typeName, arrayComp = nil)
    puts "typeName is #{typeName}: #{arrayComp}"
    if not arrayComp then      
      if @types.has_key?(typeName) then
        value = @types[typeName]
        case value.class.name
        when "Array"
          value.each{|a|
            puts "in array of type branch #{typeName}|#{a}"
            resolveTypeBranch(typeName, a)
          }
        when "String"
          puts "should return typeName is #{typeName} and value is #{value}"
          if cppType?(value) then
            return @genClasses[@filename] += (@@indent + cppCode(mapXMLToCpp(value), "_" + @elemName) + "\n")
          else

            throw "type #{typeName} has value #{value} not found in @types
      should have resolved to cpp type by now" if not @types.has_key?(value)

            resolveTypeBranch(value, nil)
          end
        else
          
          throw "Structure of @type tree is wrong as #{typeName} has hash type of
    #{@types[typeName].class.name}"

        end #case
      else
        ## @elemName is a real hack
        @genClasses[@filename] += (@@indent + cppCode(mapXMLToCpp(typeName), "_" + @elemName) + "\n")
      end
    else #arrayComp
      throw "type #{typeName} not found in @types" if not @types.has_key?(typeName)
      
      case arrayComp.class.name
      when "Hash"
        puts "#{typeName}: #{arrayComp.values[0]}"
        begin
          @elemName =  arrayComp.keys[0]
          resolveTypeBranch(arrayComp.values[0])
        rescue
          @genClasses[@filename] += (@@indent + cppCode(mapXMLToCpp(arrayComp.values[0]), "_" + arrayComp.keys[0]) + "\n")         
        end

        #Cheating as no type component is a string only hashes but we call this
        #function with the elemName and arrayComp from element and we are really
        #looking to resolve arrayComp and at same time requiring the use of
        #elemName

      #when String #(Instead of this hack we can introduce a current element
      #name local variable for element/attribute name)

      else
        throw "Unknown @type #{typeName} with ruby type of #{arrayComp.class.name}"
      end

    end

  end

  ###Parse top level elements which may define subelements that depend on other things
  def parseElement(elem)
    throw "No name attribute for: #{elem}" if not elem.attributes.has_key?("name")
    name = elem.attributes["name"]
    throw "Element #{name} has already been defined" if @elements.has_key?(name)
    @elements[name]=Array.new
    subElem = elem.elements["xs:complexType"]

    if not subElem then
      throw "Unknown top level element #{name} as no subelem complexType or attribute type" if not elem.attributes.has_key?("type")
      print "#{name} has type ", elem.attributes["type"], "\n" if @verbose
      throw "Other unhandled elements for #{name}: #{elem}" if elem.elements.size > 0
      return @elements[name].push(elem.attributes["type"])      
      #return cppCode(mapXMLToCpp(elem.attributes["type"]), name)
    end
    
    subElem.elements.each(){|e|
      case e.name
      when "attribute"
        @elements[name].push(parseAttribute(e, name))
        #next
      when "sequence"

        #Sequence really requires parsing of contraints and for netconf is just
        #a navigation and encapsulation thing and cannot really generate any c++
        #structure. In fact this is true of most elements except interface
        #perhaps where it will matter.
        next if @skips.include?(name)
        parseSequence(e, name)
      when "attributeGroup"
        parseAttributeGroups(e, name)
      when "choice"
        puts "Todo choice use by unused element #{name}"
      when "simpleContent"
        parseSimpleContent(e, name)
      else
        throw "Unknown complexType structure encountered: #{e.name} for #{name} element"
      end      
    }

  end

  def genClassHeader(classname)
    <<END
///Generated with #{__FILE__} on #{`date`}.
///Contains #{classname} attributes
class #{mapElementToClass(classname)}
{
END
  end

  def parseSimpleContent(elem, topElementName)
    throw "Parse error #{elem} is not a simpleContent element" if elem.name !~ /simpleContent/
    subElem = elem.elements["xs:extension"]
    throw "Unrecognised simpleContent structure for #{topElementName}: #{elem}" if not subElem
    throw "No extension base for simpleContent of #{topElementName}: #{elem}" if not subElem.attributes.has_key?("base")
    puts "SimpleContent of #{topElementName}<=#{subElem.attributes["base"]}" if @verbose
    @elements[topElementName].push(subElem.attributes["base"])
    parseAttributes(subElem, topElementName)
  end

  def parseSequence(elem, topElementName)
    if not elem.elements.each(){|e|
        throw "no ref for #{topElementName}: #{e}" if not e.attributes.has_key?("ref") 
        puts "ref of #{e.attributes["ref"]} for sequence in #{topElementName}" if @verbose
        @elements[topElementName].push(e.attributes["ref"])
        #mapXMLToCpp(elem.attributes["ref"])
      }
      throw "No subelements for sequence #{topElementName}: #{elem}"
    end
  end

  def parseAttributeGroups(elem, parentName=nil)
    throw "Why are we here?" if elem.name !~ /attributeGroup/

    #TODO handle case of when something needs to get ref/attribute from previous
    #or just use the doc root to find the base stuff first.
    if elem.attributes.has_key?("name") then
      name = elem.attributes["name"]
      throw "#{name} already defined when parsing attributeGroup" if @types.has_key?(name)
      @types[name] = Array.new
      elem.elements.each(){|e|
        @types[name].push(parseAttribute(e, name))
      }
    else
      throw "Parse error at attributeGroup: #{elem}" if not elem.attributes.has_key?("ref")
      throw "Invalid parent name" if not parentName
      @elements[parentName].push(elem.attributes["ref"])
    end
  end

  def parseAttribute(elem, name)
    throw "#{elem} in #{name} is not an attribute" if elem.name != "attribute"
    attName = elem.attributes["name"]
    if not elem.attributes.has_key?("type") then
      puts "no type for attr #{attName} of #{name}" if @verbose
      
      subElem = elem.elements["xs:simpleType"]
      if not subElem then
        subElem = elem.elements["xs:restriction/xs:pattern"]

        throw "Unknown XML element found #{name}: #{elem}" if not subElem

        #xs:restriction/xs:pattern don't need to do anything for this as
        #I have custom class defined at earlier stage via type or ref.
        throw "Todo no handlers for  restriction/pattern here"
      else
        parseSimpleType(subElem, attName)
      end
    else 
      #read type
      puts cppCode(mapXMLToCpp(elem.attributes["type"]), elem.attributes["name"]) if @verbose
      #cppCode(mapXMLToCpp(elem.attributes["type"]), attName)
      {attName, elem.attributes["type"]}
    end
  end

  ###Assuming every subelement is an attribute and element is not an
  ###attributeGroup i.e. a simpleContent only as @elements is used not @types as
  ###for attributeGroup. Perhaps should merge with simpleContent

  def parseAttributes(element, topElementName)
    element.elements.each(){|elem| 
      @elements[topElementName].push(parseAttribute(elem, topElementName))
    }
  end

  def generateXMLWrapManagerBase(netconfElem)
    #get attributes
    #parseAttributes(netconfElem.elements["xs:complexType"], )
##   netconfElem.elements.each(@@Attributes){|elem|      
##     puts cppCode(mapXMLToCpp(elem.attributes["type"]), elem.attributes["name"])
##   }

    #Generate header of c++ class including #includes
    #place attributes as data members
    #Generate implementation and also member functions to access values
    
  end

  def mapElementToClass(elemName)
    throw "Element #{elemName} does not map to any class" if not @elementToClass.has_key?(elemName)
    @elementToClass[elemName]
  end

  def mapXMLToCpp(schemaType)
    if schemaType =~ /(.*[ ]){1}.*/ then
      return "enum #{@elemName} { #{schemaType} }; #{@elemName}" 
    else
      throw "Unhandled Schema type #{schemaType}" if not @xmlToCpp.has_key?(schemaType)
      return @xmlToCpp[schemaType]
    end
  end

  def cppCode(cppType, varName)
    case cppType
    when "string"
      return "std::#{cppType} #{varName};"
    when /bool|IPv6Address|ipv6_addr|double|enum/
      return "#{cppType} #{varName};"
    else
      throw "Unhandled C++ type #{cppType};"
    end
  end
end


app = GenerateXMLReader.new.run

##Unit test for this class/module
require 'test/unit'

class TC_GenerateXMLReader < Test::Unit::TestCase
  def test_quoteString
    #assert_equal("\"quotedString\"",
    #             quoteString("quotedString"),
    #             "The quotes should surround the quotedString")
  end
end

if $0 != __FILE__ then
  ##Fix Ruby debugger to allow debugging of test code
  require 'test/unit/ui/console/testrunner'
  Test::Unit::UI::Console::TestRunner.run(TC_GenerateXMLReader)
end

