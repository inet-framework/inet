#! /usr/bin/env ruby
#
#  Author: Johnny Lai
#  Copyright (c) 2004 Johnny Lai
#
# =DESCRIPTION
# Sends UDP packets with the timestamp as a payload at user specified intervals
#
# =REVISION HISTORY
#  INI 2004-09-17 Changes
#

require 'optparse'
require 'pp'

require "socket"
require "timeout"

#
# Send or receive UDP packets
#
class UDPTimestamp
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

  #
  # Intialise and parse command line options
  #
  def initialize
    @debug    = false 
    @verbose  = false
    @quit     = false
    @interval = 0.01
    @payload  = nil
    @port     = 8080
    @use_payload = false
    @receive = false

    get_options

  rescue => err
    STDERR.puts err
    STDERR.puts "\n"
    STDERR.puts usage
    exit 1
  end

  #
  # Returns usage string
  #
  def usage
    ARGV.options
  end

  #
  # Processes command line arguments
  #
  def get_options
    ARGV.options { |opt|
      opt.banner = "Usage: ruby #{__FILE__} [options] ipv6addr port"

      opt.on("--help", "What you see right now"){ puts opt; exit 0}

      opt.on("--doc=DIRECTORY", String, "Output rdoc (Ruby HTML documentation) into directory"){|dir|
        system("rdoc -o #{dir} #{__FILE__}")
      }

      opt.on("--verbose", "-v", "print space separated timestamps to STDOUT"){|@verbose|}

      opt.on("--interval=FLOAT", "-i", Float, "approximate interval to send packets at will sleep for this many seconds before sending. Not very accurate when doing less than 0.01. Will divide interval by 1000 when < 0.01 (inteval of 0.001 effective to reduce send interval to less than 0.01 but suddenly jumps up to 0.00005 and 100% cpu time) Probably need proper scheduling besides sleep to get better accuracy"){|@interval|
        puts "using interval of #{@interval}" if not @verbose
      }

      opt.on("--data=STRING", "-d", String, "payload to send (by default sends timestamp",
             "like this #{Time.now().to_f})"){|@payload|
        @use_payload = true
        puts "using payload of #{@payload}" if not @verbose
      }

      opt.on("-r", "Receive mode "){|@receive|}

      opt.on_tail("default port is #{@port}")
      opt.parse!
    } or  exit(1);

    if @quit
      pp self
      (print ARGV.options; exit) 
    end

    raise "Missing host from the commandline." if ARGV.size < 1

    @host = ARGV.shift
    
    @port = ARGV.shift if not ARGV.empty?

  end

  #
  # Launches the application
  #  UDPTimestamp.new.run
  #
  def run
    if not @receive then
      sendUDPStream
    else
      receiveUDPStream
    end
  end

  def receiveUDPStream
    BasicSocket.do_not_reverse_lookup = true

    server = UDPSocket.open(Socket::AF_INET6)
    server.bind(@host, @port)
    while true
      d = server.recvfrom(64)
      p d[0]
    end
  end

  def sendUDPStream
    u = UDPSocket.open(Socket::AF_INET6)
    sleeps = @interval
    sleeps = @interval/1000 if @interval < 0.01
    seq = 0
    while true
      t = Time.now
      ##{t.sec}.#{t.usec}
      @payload = "#{t.to_f}" if not @use_payload
      @payload = "#{@payload} #{seq}"
      seq=seq+1

      u.send(@payload, 0, @host, @port)
      print @payload, " " if @verbose
      #For more than 10ms accurracy need to sleep less
      sleep(sleeps)
    end
    rescue NameError => err
    STDERR.puts err
    STDERR.puts "\n"
    STDERR.puts "Most likely IPv6 support was not enabled for ruby during compilation"
    exit 1 
  end#run
# 2001:388:608c:fc:20e:cff:fe08:5e74 #supersupreme
# 2001:388:608c:fc:20e:cff:fe08:3d6e #margarita
end#UDPTimestamp

#main
app = UDPTimestamp.new.run


##Unit test for this class/module
require 'test/unit'

class TC_UDPTimestamp < Test::Unit::TestCase
  def test_quoteString
    #assert_equal("\"quotedString\"",
    #             quoteString("quotedString"),
    #             "The quotes should surround the quotedString")
  end
end

if $0 != __FILE__ then
  ##Fix Ruby debugger to allow debugging of test code
  require 'test/unit/ui/console/testrunner'
  Test::Unit::UI::Console::TestRunner.run(TC_UDPTimestamp)
end

