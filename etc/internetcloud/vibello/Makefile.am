
# INCLUDES = -I$(top_srcdir)

bin_PROGRAMS = vibello
vibello_CXXFLAGS = -pthread
vibello_SOURCES = graph.cpp vibello.cpp main.cpp
vibello_LDADD=-lX11 -lboost_thread -lboost_program_options

PINGER_LOOKUP_SOURCE = @PINGER_LOOKUP_SOURCE@
COUNTRY_LOOKUP_SOURCE = @COUNTRY_LOOKUP_SOURCE@
SPEEDTEST_SOURCE = @SPEEDTEST_SOURCE@
GNP_DIMENSIONS = @GNP_DIMENSIONS@

THREADS = `cat /proc/cpuinfo | grep ^processor | wc -l`

all: data/measured_data.xml

state/database_created.stamp:
	bin/create_database.rb
	touch $@

state/data_loaded.stamp: state/database_created.stamp $(SPEEDTEST_SOURCE)
	bin/load_ookla.rb load $(SPEEDTEST_SOURCE)
	touch $@

state/data_normalized.stamp: state/data_loaded.stamp
	bin/load_ookla.rb normalize
	touch $@

state/pinger_loaded.stamp: state/database_created.stamp $(PINGER_LOOKUP_SOURCE)
	bin/load_pinger.rb load_pinger $(PINGER_LOOKUP_SOURCE)
	touch $@

state/countries_loaded.stamp: state/database_created.stamp $(COUNTRY_LOOKUP_SOURCE)
	bin/load_pinger.rb load_countries  $(COUNTRY_LOOKUP_SOURCE)
	touch $@

data/latencies.csv data/hosts.csv: state/data_normalized.stamp
	bin/prepare_vibello_input.rb
	touch $@

data/ehosts.csv data/errors.csv: vibello data/latencies.csv data/hosts.csv
	./vibello --nox --threads=$(THREADS)

state/vibello_results_loaded.stamp: data/ehosts.csv
	bin/load_vibello_results.rb
	touch $@

state/found_max_bw.stamp: state/data_normalized.stamp
	bin/write_xml.rb find_max_bw
	touch $@

state/computed_wire_access_latencies.stamp: state/found_max_bw.stamp
	bin/write_xml.rb compute_wire_access_latencies
	touch $@

data/measured_data.xml: state/vibello_results_loaded.stamp state/pinger_loaded.stamp state/countries_loaded.stamp state/found_max_bw.stamp state/computed_wire_access_latencies.stamp
	bin/write_xml.rb write_xml

