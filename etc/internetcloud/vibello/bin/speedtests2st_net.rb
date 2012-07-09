#!/usr/bin/ruby

# Converts a downloaded speedtest.csv file (from www.speedtest.net) to GeoIP enhanced st_net.csv format (used by Ookla)
# Usage: $0 speedtest.csv st_net.csv
# You need GeoIP from http://www.maxmind.com/app/perl
# The resulting file still needs to be edited. This is just for testing in case you do not have access to any real Ookla dataset.

require File.expand_path(File.dirname(__FILE__) + '/my_csv.rb')
require 'ipaddr'
#require 'my_csv'

infile = ARGV[0]
outfile = ARGV[1]

def lookup(ip)
    values = `geoip-lookup-city #{ip}`.split("\n")
    keys = [ :country_code, :country_code3, :country_name, :region, :city, :postal_code, :latitude, :longitude, :area_code, :metro_code ]
    res = {}
    values.each_with_index {|v,i|
        res[keys[i]]=v
    }
res
end

File.open(infile, 'rb') do |io_in|
File.open(outfile, 'wb') do |io_out|
#MyCSV.generate(outfile) do |writer|
    #writer.forced_quote_fields = [1]
    #writer << %w{test_id test_date client_ip_id download_kbps upload_kbps latency server_name server_country server_country_code server_latitude server_longitude user_agent isp_name client_net_speed is_isp client_country client_country_code client_region_name client_region_code client_city client_latitude client_longitude miles_between}
    io_out << %w{test_id test_date client_ip_id download_kbps upload_kbps latency server_name server_country server_country_code server_latitude server_longitude user_agent isp_name client_net_speed is_isp client_country client_country_code client_region_name client_region_code client_city client_latitude client_longitude miles_between}.join(',') << "\n"

test_id = 0
CSV::Reader.parse(io_in) do |row|
    test_id += 1    
    next if test_id == 1
    p row
    ip_address,test_date,time_zone,download_kilobits,upload_kilobits,latency_ms,server_name,distance_miles = row 
     
    geo = lookup(ip_address) 
    client_ip_id = IPAddr.new(ip_address).to_i 

#    writer << [test_id, test_date, client_ip_id, download_kilobits, upload_kilobits, latency_ms, server_name, nil, nil, nil, nil, nil, nil, nil, nil, geo[:country_name], geo[:country_code], geo[:region], geo[:region_code], geo[:city], geo[:latitude], geo[:longitude], distance_miles]
    io_out << %{#{test_id}, "#{test_date}", #{client_ip_id}, #{download_kilobits}, #{upload_kilobits}, #{latency_ms}, "#{server_name}", NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, "#{geo[:country_name]}", "#{geo[:country_code]}", "#{geo[:region]}", "#{geo[:region_code]}", "#{geo[:city]}", #{geo[:latitude]}, #{geo[:longitude]}, #{distance_miles}} << "\n" # TODO: Do proper quoting!
  end
#  end
end
end
