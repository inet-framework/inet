#! /usr/bin/env ruby
#
#  Author: Johnny Lai
#  Copyright (c) 2004 Johnny Lai
#
# =DESCRIPTION
# Part of the cl2html target added for viewcvs delta hyperlink from the 
# cvs changelog
#
# =REVISION HISTORY
#  INI 2004-08-26 Changes
#

require 'optparse'
require 'pp'
require "rexml/document"
include REXML

$test = false

#
# Searches in cvs2cl XML output for cvs revision number and adds a prior
# revision element. Simple and inaccurate method since it only handles the main
# trunk not the branch revision numbers.
#
class PriorCVSRevision
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

  def initialize
    @debug    = false 
    @verbose  = false
    @quit     = false
    @outfilename = false
    get_options

    if 0 == ARGV.size and not $test
      STDERR.puts usage
      exit 0
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
    print ARGV.options;
  end

  #
  # Processes command line arguments
  #
  def get_options
    ARGV.options { |opt|
      opt.banner = "Usage: ruby #{__FILE__} [options] xmlfile"

      opt.on("--help", "What you see right now"){ puts opt; exit 0}

      opt.on("-x", "parse arguments and show Usage") {|@quit|}

      opt.on("--doc=DIRECTORY", String, "Output rdoc (Ruby HTML documentation) into directory"){|dir|
        system("rdoc -o #{dir} #{__FILE__}")
      }

      opt.on("--verbose", "-v", "print intermediate steps to STDERR"){|@verbose|}                                                     
      opt.on("--output=FILE", "-o", String, "Output to file FILE. By default changes xmlfile"){|@outfilename|
        
      }
      opt.on("--test", "unit test"){|$test|}

      opt.on_tail("Add priorRevision elements to xmlfile. xmlfile is from output of cvs2cl.pl")
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
  #  PriorCVSRevision.new.run
  #
  def run
    return if $test

    raise "Missing file from the commandline." if 1 != ARGV.size

    file = File.new(ARGV[0])
    @doc = Document.new(file)
    file.close
    @doc.elements.each("changelog/entry/file"){|elem|
      elem.elements.each("revision"){|rev|
        #Skip initial revision
        next if rev.text =~ /\s*1.1\s*$/
        elem.elements<<priorRev=Element.new('priorRevision')
        priorRev.add_text previousRevision(rev.text)
      }
    }
    @outfilename = ARGV[0] if not @outfilename
    @doc.write  file = File.new(@outfilename,"w")
    file.close
  end#run
  
  #Returns the revision before currentRevision
  def previousRevision(currentRevision)
    currentRevision.to_s.scan(/[.](.+)$/)
    prior = $` + "." + ($1.to_i-1).to_s
    puts "#{prior} <- #{currentRevision}"  if @verbose
    #Too bad there is no opposite to succ/next like previoius (also "1.9".next becomes 2.0 not 1.10)
#    raise "prior rev #{prior.succ} is not same as current rev #{currentRevision} " if prior.succ.to_s != currentRevision.to_s
    return prior
  end

end#PriorCVSRevision

app = PriorCVSRevision.new.run

exit unless $test

##Unit test for this class/module
require 'test/unit'

class TC_PriorCVSRevision < Test::Unit::TestCase
  def test_previousRevision
    prev = PriorCVSRevision.new.previousRevision("1.52")
    assert_equal("1.51",
                 prev,
                 "Should be the previous CVS revision number assuming only trunk development")

    prev = PriorCVSRevision.new.previousRevision(1.9)
    assert_equal("1.8",
                 prev,
                 "Should be the previous CVS revision number assuming only trunk development")

    prev = PriorCVSRevision.new.previousRevision(1.2)
    assert_equal("1.1",
                 prev,
                 "Should be the previous CVS revision number assuming only trunk development")

    prev = PriorCVSRevision.new.previousRevision("1.30")
    assert_equal("1.29",
                 prev,
                 "Should be the previous CVS revision number assuming only trunk development")
  end
end

if $0 != __FILE__ then
  ##Fix Ruby debugger to allow debugging of test code
  require 'test/unit/ui/console/testrunner'
  Test::Unit::UI::Console::TestRunner.run(TC_PriorCVSRevision)
end

