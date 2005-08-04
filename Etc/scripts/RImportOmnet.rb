#! /usr/bin/env ruby
#
#  Author: Johnny Lai
#  Copyright (c) 2004 Johnny Lai
#
# =DESCRIPTION
# Imports omnetpp.vec files
#
# =REVISION HISTORY
#  INI 2004-09-26 Changes
#

require 'optparse'
require 'pp'

#controls whether R commands are printed on screen. Turn on via -X option
$debug = false
$test  = false

#global is evil but don't know which other way to pass variable between diff
#classes and in diff scopes
$lastCommand = []
class IO
  alias old_puts puts

  # 
  # As we only use pipe to R any other puts is to terminal mainly. puts to R
  # should be done in a put/get combo in order to keep both processes in sync
  # otherwise ruby is much faster than R and may overload input buffer of
  # R. Keeping in sync does not work in practice read inline comments of self.puts
  #
  def puts(str)
    if self.tty?
      self.old_puts str
      return
    end

    $lastCommand.push str

    if $debug
      $defout.old_puts str 
      $defout.flush
    end

    self.old_puts str

    $lastCommand.shift if $lastCommand.size > 2
    if false
      #keeping in sync does not work in practice because we need to read the
      #output of R and we would have swallowed it up here unless we can somehow
      #do gets here and check that its not a newline and reinsert it back in
      #after doing another gets so we get the newline
      self.old_puts %{cat("\\n")}
      self.gets
    end
  end
end

alias old_execute `

#Capture standard error too when executing things via backqoute or %x
def `(cmd)
  old_execute(cmd + " 2>&1")
end

#
# Imports omnetpp.vec files
#
class RImportOmnet
  VERSION       = "$Revision$"
  REVISION_DATE = "$Date$"
  AUTHOR        = "Johnny Lai"

  RSlave = "R --slave --quiet --vanilla --no-readline"
  #Doing **/*.ext would find all files with .ext recursively while with only one
  #* it is like only in one subdirectory deep
  SingleVariantTest = "*/*.vec"

  attr_accessor :rdata, :filter

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

  #
  # Initialse option variables in case the actual option is not parsed in 
  #
  def initialize
    @debug    = false 
    @verbose  = false
    @quit     = false
    @agg      = false
    @collect  = false
    @filter   = nil
    @vectorStarted = nil
    @rdata    = "test2.Rdata"
    @srdata   = "superCombined.Rdata"
    @restrict = nil
    @rsize    = 0 #no restriction on vector length
    @aggprefix = %{a.} #dataframes have this as prefix when aggregating runs

    get_options

  rescue => err
    STDERR.puts usage
    STDERR.puts "\n" if err
    STDERR.puts err  if err
    STDERR.puts err.backtrace if err
    exit 1
  end

  #
  # Returns usage string
  #
  def usage
    ARGV.options
#    <<-USAGE
#Usage: #{File.basename $0} [-v] file
#  -v|--verbose      print intermediate steps to STDERR
#    USAGE
  end

  #
  # Processes command line arguments
  #
  def get_options
    ARGV.options { |opt|
      opt.banner = version
      opt.separator "Usage: #{File.basename __FILE__} [options] [omnetpp.vec] "
      opt.separator ""
      opt.separator "Specific options:"

      
      #Try testing with this 
      #ruby __FILE__ -x -c -s test
      opt.on("-x", "parse arguments and show Usage") {|@quit|
        pp self
        (print ARGV.options; exit) 
      }

      opt.on("-a", "Aggregate mode i.e. read from many vec files dir layout generated via graph-xx.sh"){|@agg|
      }

      opt.on("--[no-]combine=[FLAG]", "-c", "load all results from subdirectories into",
             "one big RData file in topdir"){ |@opt_combine|}

      opt.on("--output=filename","-o", String, "Output R data to filename"){|@rdata|}
      opt.on("--output=filename","-O", String, "Output super R data to filename"){|@srdata|}
      # List of arguments.
      opt.on("-f", "--filter x,y,z", Array, "list of vector indices to grab, other indexes are ignored") {|@filter|
        self.filter.uniq!
      }
      opt.on("-c", "Collect all Rdata files in different subdirs into one with directory names as scheme. ",
             "If -a given will aggregate runs first before collecting. Run from the top dir"){|@collect|}

      opt.on("-r", "--restrict x,y,z", Array, "Restrict these vectors to at most size rows"){|@restrict|}
      opt.on("-s", "--size x", Integer, "restrict vectors specified to --restrict to size rows "){|@rsize|}
      opt.separator ""
      opt.separator "Common options:"

      opt.on("--doc=DIRECTORY", String, "Output rdoc (Ruby HTML documentation) into directory"){|dir|
        system("rdoc -a --webcvs=\"http://localhost/cgi-bin/viewcvs.cgi/\" --tab-width=2 --diagram --inline-source -N -o #{dir} #{__FILE__}")
	exit
      }

      opt.on("--verbose", "-v", "print intermediate steps to STDOUT"){|@verbose|}

      opt.on("--debug", "-d", "print debugging info to STDOUT"){|@debug| 
        $debug = @debug
      }

      opt.on("--test", "run unit tests"){|$test|}

      opt.on_tail("-h", "--help", "What you see right now") do
        puts opt
        exit
      end

      opt.on_tail("--version", "Show version") do
        #puts OptionParser::Version.join('.')
        puts version
        exit
      end

      opt.parse!
    } or  exit(1);

  end
  
#  private
  #
  # Unused except in unittest.  Retrieve hash of vector index -> label from vec
  # file filename (index is still a numerical string) where label is the vector
  # name in file.
  #
  def retrieveLabels(filename)
    vectors = Hash.new
    
    #foreach less verbose than opening file and reading it
    IO::foreach(filename) {|l|
      case l
      when /^[^\d]/
        #Retrieve vector number
        i = l.split(/\s+/,4)[1]
        #Retrieve vector name
        s = l.split(/\s+/,4)[3]
        #Remove "" and last number
        s = s.split(/["]/,3)[1]
        if not vectors.value?(s)
          vectors[i]=s
        else
          #Retrieve last component of mod path to create unique R column names
          #when identical vector names
          modpath = l.split(/\s+/,4)[2]
          #remove the quote
          mod = modpath.split(".").last.chop!
          vectors[i]="#{mod}.#{s}"
        end
      end
    }
    p vectors if @debug
    return vectors
  end

  #Retrieve hash of vector index -> label from vec file filename (index is still
  #a numerical string) where label is the vector name in file. All labels have
  ##.#{index} appended
  def retrieveLabelsVectorSuffix(filename)
    vectors = Hash.new
    
    #foreach less verbose than opening file and reading it
    IO::foreach(filename) {|l|
      case l
      when /^[^\d]/
        #Retrieve vector number
        i = l.split(/\s+/,4)[1]
        #Retrieve vector name
        s = l.split(/\s+/,4)[3]
        #Remove "" and last number
        s = s.split(/["]/,3)[1]
        vectors[i]=s + "." + i
      end
    }
    p vectors if @debug
    return vectors
  end

  #
  # Retrieve array of object names that match the R regular expression in pattern
  #
  def retrieveRObjects(p, pattern)
    p.puts %{cat(ls(pat="#{pattern}"),"\\n")}
    arrayCode =  %{%w[#{p.gets.chomp!}]}
    eval(arrayCode)
  end

  #
  # Form safe column names from omnetpp vector names
  # p is pipe to R 
  # vectors is hash of index -> vector name produced from retrieveLabels
  def safeColumnNames(p, vectors)
    p.puts %{l<-c("#{vectors.values.join("\",\"")}")}
    p.puts %{l<-make.names(l)}
    #newline is needed otherwise gets stalls. Will get an extra empty element
    #but does not matter as we don't use it
#    p.old_puts %{cat(l,sep="\\",\\"","\\n")}
#    arrayCode =  %{["#{p.gets.chomp!}"]}
    p.puts %{cat(l,sep=" ","\\n")}
    arrayCode =  %{%w[#{p.gets.chomp!}]}
    p.puts %{rm(l)}
    #Eval only only does expressions and not statements?
    eval(arrayCode)
  end

  #pause ruby so R can finish its operations
  def waitForR(rpipe)
    rpipe.puts %{cat("\\n")}
    rpipe.gets
  end

  # Read data from OMNeT++ vecFile and convert to dataframe using vector name as
  # frame name prefix and n as suffix. n is the run number.  Adds to previous
  # run if @agg is in effect and removes .n suffix in frame name. Vector names
  # may be duplicated so the last component of module path is used too if this
  # happens. I guess the best way is to append the vector number to frame name
  # (assuming users don't run half the runs and add new vectors and then run
  # rest of variants). To totally prevent any of this stuff happening we should
  # freeze vector numbers and make sure vector names are unique across whole sim
  # which may be unrealistic?
  def singleRun(p, vecFile, n)
    vectors = retrieveLabelsVectorSuffix(vecFile)
    @vectorStarted = Array.new if not @vectorStarted

    #not caching the vectors' safe column names. Too much hassle and makes code
    #look complex. Loaded vectors can differ a lot anyway.

    a = safeColumnNames(p, vectors)
    p a if @debug
    i = 0 
    vectors.each_pair { |k,v|
      # does not update value in hash only iterator v
      #v = a[i] 
      vectors[k] = a[i]
      i+=1
      raise "different sized containers when assigning safe column names" if vectors.keys.size != a.size
    }

    unless self.filter.nil?
      newIndices = vectors.keys & self.filter
      vectors.delete_if{|k,v|
        not newIndices.include? k
      }
    end

    vectors.each_pair { |k,v|
      raise "logical error in determining common elements" if vectors[k].nil?
      nlines = !@restrict.nil? && @restrict.include?(k)?@rsize:0
      #We'd like to have runnumber.vectorname.vectornumber i.e. #{n}.#{v} but as runnumber at the
      #start appears to be a float it is an R syntax error
      onerunframe = %{#{v}.#{n}}
      columnname = v.sub(%r{[.]\d+$},"")
      p.puts %{tempscan <- scan(p<-pipe("grep ^#{k} #{vecFile}"), list(trashIndex=0,time=0,#{columnname}=0), nlines = #{nlines})}
      p.puts %{close(p)}
      p.puts %{attach(tempscan)}
      p.puts %{#{onerunframe} <- data.frame(run=#{n}, time=time, #{columnname}=#{columnname})}
      p.puts %{detach(tempscan)}
      if @agg
        aggframe = %{#{@aggprefix}#{v}}
        if @vectorStarted.include? k
          # @configName.v cannot be used as dataframe name as it may contain -
          # which is illegal in R so use a instead
          p.puts %{#{aggframe} <- rbind(#{aggframe} , #{onerunframe})}
        else
          p.puts %{#{aggframe} <- #{onerunframe}}
          @vectorStarted.push(k)
        end
        p.puts %{rm(#{onerunframe})}
      end
      waitForR p
    }
    p.puts %{rm(tempscan, p)}
  end

  #Collects stats for a singleVariant. Works with vector files in directory
  #layout generated by graph-simple.sh.
  def aggregateRuns(p, dir)
    #Initialise list of vectors read from vector file for joining dataframes of multiple runs
    @vectorStarted = nil
    pwd = Dir.pwd
    Dir.chdir(dir)
    if not singleVariant?
      $stderr.puts "Cannot aggregate runs when no vec files exist"
      exit 1
    end

    if singleVariant? > 1
      throw "Cannot aggregate runs when vector files are not one dir level below "
    end

    p.puts %{setwd("#{dir}")}
    @configName = File.basename Dir.pwd
    print "config name is ", @configName, "\n" if @verbose
    Dir[SingleVariantTest].each{|f|
      next if f =~ /bad/
      n = File.dirname f 
      puts "#{f} is in dir #{n}" if @verbose

      #Save every runs own data frame too for later analysis

      singleRun(p, f, n)
      if not @agg 
        #forget about resume function for now runs out of diskspace fairly
        #quickly for large datasets (unless we know how to save just v.n)
        output = File.join(File.dirname(f), @rdata)
        p.puts %{save.image("#{output}")} 
      end

    }
    p.puts %{save.image("#{@rdata}")} if @agg
    p.puts %{rm(list=ls())}
  ensure
    Dir.chdir(pwd)
    p.puts %{setwd("#{pwd}")}
  end

  #Test whether dir or current dir if dir is nil is a single variant
  #directory. i.e. subdirectoris with run number as name, each subdir only
  #differing in random seed with all other params constant.
  def singleVariant?(dir=nil)
    pwd = Dir.pwd

    Dir.chdir(dir) if not dir.nil?
    singleVariantTestRec = "**/*.vec"
    dirs = Dir[singleVariantTestRec]
    return nil if dirs.nil? or dirs[0].nil?
    dirs[0].count(File::Separator)
  ensure
    Dir.chdir(pwd)
  end

  # collect the different variants where each variant is a specific build or xml
  # configuration for a similar network topology. Each variant will have many
  # runs, with a different random seed for each run. The runs are collected by
  # aggregateRuns. Each variant's data frame will have a scheme column added to
  # it.  Besides scheme there could be other factors like macro/micro link
  # delay. (This is not catered for yet).  Assumes vector files in directory
  # layout generated by graph-simple.sh
  def collectVariants(p, dir=nil)
    if singleVariant?(dir).nil? and @agg
      $stderr.puts "Cannot aggregate runs when no vec files exist"
      exit 1
    end

    if singleVariant?(dir) > 1
      pwdir=dir.nil? ? Dir.new(Dir.pwd) : Dir.new(dir)
      Dir.chdir(dir) if dir
      @configs = Array.new
      pwdir.each{|d|
        begin
          next if d =~ /^[.]+$/
          next unless File.directory?(d)
          @configs.push d if singleVariant?(d) == 1
        rescue SystemCallError
          $stderr.print "cd to #{d} failed" + $! 
          next
        end
      }

      if @verbose
        print %{configs found are }
        p @configs
      end

      if @agg
        @configs.each{|config|
          aggregateRuns(p, File.join(pwdir.path,config))
        }
      end
#      Dir.entries(dir).reject{|f| f =~/\A\./ or not File.directory?(f) }.each{|d| puts "in #{d}"; aggregateRuns(p, File.join(pwd,d)) }
    end

    #find all Rdatas (matching certain name as specified in output option)
    done = Array.new
    result = Dir["*/#{rdata}"].each{|rd|
      p.puts  %{load("#{rd}")}
      aggprefixre = @aggprefix.sub(".","[.]")
      frames = retrieveRObjects(p, "^#{aggprefixre}.+")
      #Can filter on this too if we specify a list of unwanted columns/framenames
      frames.each{|df|
        config = File.dirname rd
        unless @configs.nil?
          $stderr.puts "Unknown config dir #{config} not in @configs" unless @configs.empty? or not @agg or @configs.include? config
        end
        p.puts %{#{df} <- transform(#{df}, scheme="#{config}")}
        #search for aggframe prefix and remove
        frameName = df.sub(/^#{aggprefixre}/,"")
        #search for other frames with same number of rows and matching time columns
        #cbind the other frame's data column omitting repeated columns i.e. 
        #transform(#{f}, #{oframe.split(.,2[1])}=#{oframe}$#{oframe.split(.,2[1])})

        p.puts %{#{df} <- rbind(s.#{frameName}, #{df})} if done.include? df
        p.puts %{s.#{frameName} <- #{df}}
        done.push df unless done.include? df
        p.puts %{rm(#{df})}
        p.puts %{save.image("#{@srdata}")}
        waitForR p
      }
    }
    raise "Not even one file by the name of #{rdata} was found \nwhen doing collection of Rdata runs. \nPlease specify proper name with -o " if result.empty?
  end
  public

  #
  # Launches the application
  #  RImportOmnet.new.run
  #
  def run
    return if $test
    #Run number
    n = 0    

    IO.popen(RSlave, "w+") { |p|
      if @collect
        collectVariants(p)
      elsif @agg
        @topdir = Dir.pwd
        aggregateRuns(p, @topdir)     
      else
        raise ArgumentError, "No vector file specified", caller[0] if not @agg and ARGV.size < 1

        vecFile = ARGV.shift
        singleRun(p, vecFile, n)
      end

      p.puts %{q("no")}

    }

  rescue Errno::EPIPE => err
    $defout.old_puts err
    $defout.old_puts err.backtrace
    $defout.old_puts "last R command: #{$lastCommand.shift}"
  end#run
  
end#RImportOmnet

app = RImportOmnet.new.run 

exit unless $test

##Unit test for this class/module
require 'test/unit'

class TC_RImportOmnet < Test::Unit::TestCase
  def test_safeColumnNames
    a=@imp.safeColumnNames(@p, Hash[1,"bad names",5,"compliant.column-name"])   
    assert_equal(a, %w{compliant.column.name bad.names}, "Column names should not differ!")
  end

  def test_retrieveRObjects
    @p.puts %{s.me <- data.frame}
    @p.puts %{s.you <- data.frame}
    @p.puts %{s.us.youtoo <- data.frame}
    @p.puts %{u.us.youtoo <- data.frame}
    @p.puts %{t.us.youtoo <- data.frame}
    @p.puts %{z.us.youtoo <- data.frame}
    @p.puts %{super.us.youtoo <- data.frame}
    a = @imp.retrieveRObjects(@p, "^s[.].+")
    assert_equal(a, %w{s.me s.us.youtoo s.you}, "list of matching R objects differ!")
  end

  TestFileName = "test.vec"
  #Test read of sample vector file and also filtering
  def test_singleRun
    #truncate w+
    File.open(TestFileName, "w+"){|f|
      (var = <<TARGET).gsub!(/^\s+/, '')
vector 3  "saitEHCalNet.mn.networkLayer.proc.ICMP.nd"  "Movement Detection Latency"  1
3 143.61245 143.61245
vector 6  "saitEHCalNet.mn.linkLayers[0].networkInterface"  "IEEE 802.11 HO Latency"  1
6 143.627909  0.253460132
vector 5  "saitEHCalNet.mn.networkLayer.proc.mobility"  "handoverLatency"  1
5 143.714402  0.339952951
vector 0  "saitEHCalNet.mn.networkLayer.proc.ICMP.icmpv6Core"  "pingEED"  1
0 155.000993  0.0109926666
0 155.010813  0.010812523
0 155.020673  0.010672523
0 155.030873  0.010872523
0 155.040773  0.010772523
0 155.050853  0.010852523
0 155.060413  0.010412523
0 155.070613  0.010612523
TARGET
       f.print var
    }

     #vanilla singleRun
    @imp.rdata = TestFileName.gsub(/\.vec$/, ".Rdata")
    @imp.singleRun(@p, TestFileName, 3)
    @p.puts  %{save.image("#{@imp.rdata}")}
    # Race conditions exist so sync to R first otherwise may raise missing file exception
    @imp.waitForR(@p)
    raise "expected Rdata file #{@imp.rdata} not found in #{Dir.pwd}" if not (File.file? @imp.rdata)

    @p.puts %{rm(list=ls())}
    @p.puts %{load("#{@imp.rdata}")}   
    e = %w{IEEE.802.11.HO.Latency.6.3 Movement.Detection.Latency.3.3 handoverLatency.5.3 pingEED.0.3}
    r = @imp.retrieveRObjects(@p, "*")
    assert_equal(e, r, "singleRun test: list of matching R objects differ!")
    File.delete(@imp.rdata)

     #Filter test
    @p.puts %{rm(list=ls())}
    @imp.filter = %w{3 6 5}
    @imp.singleRun(@p, TestFileName, 3)
    @p.puts  %{save.image("#{@imp.rdata}")}
    @imp.waitForR(@p)
    raise "expected Rdata file #{@imp.rdata} not found in #{Dir.pwd}" if not (File.file? @imp.rdata)

    @p.puts %{rm(list=ls())}
    @p.puts %{load("#{@imp.rdata}")}
    e = %w{IEEE.802.11.HO.Latency.6.3 Movement.Detection.Latency.3.3 handoverLatency.5.3 }
    r = @imp.retrieveRObjects(@p, "*")
    assert_equal(e, r, "filter test: list of matching R objects differ!")
    File.delete(@imp.rdata, TestFileName)
    
  end

  def test_duplicateVectorNames

    File.open(TestFileName, "w+"){|f|
      (var = <<TARGET).gsub!(/^\s+/, '')
vector 3  "saitEHCalNet.mn.networkLayer.proc.ICMP.nd"  "Movement Detection Latency"  1
3 143.61245 143.61245
vector 6  "saitEHCalNet.mn.linkLayers[0].networkInterface"  "IEEE 802.11 HO Latency"  1
6 143.627909  0.253460132
vector 5  "saitEHCalNet.mn.networkLayer.proc.mobility"  "handoverLatency"  1
5 143.714402  0.339952951
vector 0  "saitEHCalNet.mn.networkLayer.proc.ICMP.icmpv6Core"  "pingEED"  1
0 155.000993  0.0109926666
vector 2  "saitEHCalNet.mn.networkLayer.proc.ICMP.icmpv6Core"  "handoverLatency"  1
2 265.010862  1.19022885
vector 1  "saitEHCalNet.mn.networkLayer.proc.ICMP.duddup"  "handoverLatency"  1
TARGET
      f.print var
    }
     e = Hash["3", "Movement Detection Latency", "6", "IEEE 802.11 HO Latency", "5", "handoverLatency", "0", "pingEED", "2", "icmpv6Core.handoverLatency", "1", "duddup.handoverLatency"]     
     h = @imp.retrieveLabels(TestFileName)
     #becomes an array of arrays
     el = e.sort
     hl = h.sort
     assert_equal(el, hl, "dupVectorNames retrieveLabels:  hash differs")
     e = %w{Movement.Detection.Latency IEEE.802.11.HO.Latency handoverLatency pingEED icmpv6Core.handoverLatency duddup.handoverLatency}
     r=@imp.safeColumnNames(@p, h)
     e = e.sort
     r = r.sort
     assert_equal(e, r, "dupVectorNames safeColumns: list of matching R objects differ!")

     e = Hash["3", "Movement Detection Latency.3", "6", "IEEE 802.11 HO Latency.6", "5", "handoverLatency.5", "0", "pingEED.0", "2", "handoverLatency.2", "1", "handoverLatency.1"]
     r=@imp.retrieveLabelsVectorSuffix(TestFileName)
     assert_equal(e.sort, r.sort, "dupVectorNames retrieveLabelsVectorSuffix failed")
  end

  def test_collect    
    Dir.chdir(File.expand_path(%{~/src/phantasia/master/output/saiteh})){
      `ruby #{__FILE__} -o testcoll.Rdata -a -c -f 3,6,5 -O supertest.Rdata`
      assert_equal(0, $?.exitstatus, "collect with  aggregate mode failed")
      `mv supertest.Rdata baselineCollWithAgg.Rdata`
      assert_equal( 0, $?.exitstatus, "mv not successful")
      `ruby #{__FILE__} -c -f 3,6,5 -O supertest.Rdata -o testcoll.Rdata`
      assert_equal(0, $?.exitstatus, "collect mode failed")
      `diff supertest.Rdata baselineCollWithAgg.Rdata`
      assert_equal(0, $?.exitstatus, "files differ when doing collect with/out aggregate mode")
    } if ENV["USER"] == "jmll"
  end

  if false
  def test_restrict
  end
  end

  def setup
    @p = IO.popen( RImportOmnet::RSlave, "w+")
    @imp = RImportOmnet.new
  end
  
  def teardown
    @p.close
  end

end

if $0 != __FILE__ then
  ##Fix Ruby debugger to allow debugging of test code
  require 'test/unit/ui/console/testrunner'
  Test::Unit::UI::Console::TestRunner.run(TC_RImportOmnet)
end
