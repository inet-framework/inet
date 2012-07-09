#!/usr/bin/ruby

require 'bin/database'
require "rexml/document.rb"
require "rexml/streamlistener.rb"

class PingerListener 
  include REXML::StreamListener

  def initialize
    @sth = $dbh.prepare("INSERT INTO pinger_lookup VALUES(?, ?, ?, ?, ?, ?)")
  end

  def tag_start(name, atr)
    if name == 'SummaryReport'
      #$countries[attrs['code']] = attrs['regionPingEr']
      #$dbh.do("INSERT INTO pinger_lookup VALUES('#{atr['from']}', '#{atr['to']}', #{atr['minimumRtt']}, #{atr['averageRtt']}, #{atr['delayVariation']}, #{atr['packetLoss']})")
      @sth.execute(atr['from'], atr['to'], atr['minimumRtt'], atr['averageRtt'], atr['delayVariation'], atr['packetLoss'])
    end
  end
end


def load_pinger(source)


$db.do("DROP TABLE IF EXISTS pinger_lookup")
$db.do("CREATE TABLE pinger_lookup (
    from_region VARCHAR(20) NOT NULL,
    to_region VARCHAR(30) NOT NULL,
    min_rtt FLOAT NOT NULL,
    avg_rtt FLOAT NOT NULL,
    delay_variation FLOAT NOT NULL,
    packet_loss FLOAT NOT NULL,
    PRIMARY KEY(from_region, to_region))")


list = PingerListener.new 
REXML::Document.parse_stream(source, list)

end


class CountriesListener 
  include REXML::StreamListener
  attr_accessor :countries

  def initialize
    @countries = {}
  end

  def tag_start(name, attrs)
#    if name == 'Host'
#      @countries[attrs['countryCode']] = attrs['continentalArea']
#    end
    if name == 'CountryKey'
      countries[attrs['code']] = attrs['regionPingEr']
    end
  end
end


def load_countries(source)

$db.do("DROP TABLE IF EXISTS country_lookup")
$db.do("CREATE TABLE country_lookup (
    country_code VARCHAR(2) PRIMARY KEY,
    pinger_region VARCHAR(20) NOT NULL)")

list = CountriesListener.new 
REXML::Document.parse_stream(source, list)

list.countries.each_pair {|cc,pr|
      puts "#{cc} -> #{pr}"
      $dbh.do("INSERT INTO country_lookup VALUES('#{cc}', '#{pr}')")
}

# TODO: add missing entries to country lookup 
$dbh.do("INSERT INTO country_lookup VALUES('RU', 'Russia')")
end

source = File.new(ARGV[1])

case ARGV[0]
 when 'load_pinger'
  load_pinger(source)
 when 'load_countries'
  load_countries(source)
 else
  puts "Unknown command"
  exit 1
end
