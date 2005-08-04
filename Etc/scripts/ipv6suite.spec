%{!?cvsdate: %{expand: %%define cvsdate %%(/bin/date +"%Y%m%d")}}

%define myrelease 1

#example rebuild statement from srpm 
#rpmbuild --rebuild --define "cvsdate <cvsdate>" i.e --define "cvsdate 20040302"
Summary: IPv6Suite is an OMNeT++ model suite for accurate simulation of IPv6 protocols and networks.
Name: ipv6suite
Version: 0.92
Release: 3.cvs%{cvsdate}_%{myrelease}
License: GPL
Group: Applications/Engineering
URL: http://ctieware.eng.monash.edu.au/twiki/bin/view/Simulation/IPv6Suite
Source0: %{name}-%{version}_%{cvsdate}.tar.bz2
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-buildroot
Requires: omnetpp
BuildRequires: omnetpp >= 3.0, boost >= 1.30.0 , cmake >= 2.0
%description
IPv6Suite models the functionality of the following RFCs:

    * RFC 2373 IP Version 6 Addressing Architecture
    * RFC 2460 Internet Protocol, Version 6 (IPv6) Specification}
    * RFC 2374 An IPv6 Aggregatable Global Unicast Address Format
    * RFC 2463 Internet Control Message Protocol (ICMPv6) for the Internet 
      Protocol Version 6 (IPv6) Specification
    * RFC 2461 Neighbor Discovery for IP Version 6 (IPv6)
    * RFC 2462 IPv6 Stateless Address Autoconfiguration
    * RFC 2472 IP Version 6 over PPP
    * RFC 2473 Generic Packet Tunneling in IPv6
    * RFC 2464 Transmission of IPv6 Packets over Ethernet Networks
    * Mobility Support in IPv6 (MIPv6) revision 18
    * Hierarchical Mobile IPv6 Mobility Management (HMIPv6) revision 6 
    * Optimistic Duplicate Address Detection revision 4
    * Fast Solicited Router Advertisements revision 4 

Copy files from %{_docdir}/%{name}-%{version}/example to your own directory. 
E.g. cp -r  %{_docdir}/%{name}-%{version}/example .; cd example
"ccmake ." to configure and generate the Makefiles
"make" to build the MIPv6Network example network and run ./ExampleBuild.

Customise CMakeLists.txt for your particular project.

%prep
%setup -q -n %{name}-%{version}_%{cvsdate}

%build
CMAKE_OPTIONS="BUILD_HMIP BUILD_MOBILITY EWU_L2TRIGGER JLAI_FASTRA JLAI_ODAD"
for o in $CMAKE_OPTIONS; 
do
  FINAL_OPTION="-D$o:BOOL=ON $FINAL_OPTION"
done
cmake $FINAL_OPTION
if [ `hostname` = "tangles0.localdomain" ]; then
  make parallelBuild
else
  make
fi

%install
rm -rf $RPM_BUILD_ROOT
%define ipv6suite_data %{_datadir}/%{name}
%define include_dir %{_includedir}/%{name}
%define misc_dir %{ipv6suite_data}/Etc
%define cmake_dir %{misc_dir}/CMake
%define ned_dir %{ipv6suite_data}/ned
make customInstall
mkdir -pv $RPM_BUILD_ROOT%{_libdir}
cp -p lib/*.so $RPM_BUILD_ROOT%{_libdir}

mkdir -pv $RPM_BUILD_ROOT%{include_dir}
cp -p include/* $RPM_BUILD_ROOT%{include_dir}

mkdir -pv $RPM_BUILD_ROOT%{ned_dir}
cp -p ned/*.ned $RPM_BUILD_ROOT%{ned_dir}

mkdir -pv $RPM_BUILD_ROOT%{cmake_dir}

cp -p Etc/default.ini $RPM_BUILD_ROOT%{misc_dir}
perl -i -pwe "s|^.*IPv6routingFile.*$|*.IPv6routingFile=\"%{misc_dir}/empty.xml\"|g" $RPM_BUILD_ROOT%{misc_dir}/default.ini

cp -p Etc/empty.xml $RPM_BUILD_ROOT%{misc_dir}
perl -i -pwe "s|^.*!DOCTYPE.*$|\<!DOCTYPE netconf SYSTEM \"%{misc_dir}/netconf2.dtd\"\>|" $RPM_BUILD_ROOT%{misc_dir}/empty.xml

cp -p Etc/netconf2.dtd $RPM_BUILD_ROOT%{misc_dir}
cp -p Etc/CMake/*.cmake $RPM_BUILD_ROOT%{cmake_dir}

mkdir example
cp -p CMakeCache.txt example
pushd example
perl -i -pwe "s/^.*CMAKE_PATH.*$//g" CMakeCache.txt
perl -i -pwe "s/^.*LIBRARY_TYPE.*$//g" CMakeCache.txt
perl -i -pwe "s/^.*CMAKE_HOME_DIRECTORY.*$//g" CMakeCache.txt
perl -i -pwe "s/^.*CMAKE_INSTALL_PREFIX.$//g" CMakeCache.txt
perl -i -pwe "s/^.*_LIB_DEPENDS.$//g" CMakeCache.txt
perl -i -pwe 's|^.*CACHEFILE.*$||' CMakeCache.txt
popd

cp -p Examples/MIPv6Network/MIPv6Network.xml example
perl -i -pwe 's|^.*!DOCTYPE.*$|<!DOCTYPE netconf SYSTEM "%{misc_dir}/netconf2.dtd">|' example/MIPv6Network.xml

cp -p Examples/MIPv6Network/omnetpp.ini example
perl -i -pwe 's|^include.*default.ini$|include %{misc_dir}/default.ini|g' example/omnetpp.ini

cp -p Examples/MIPv6Network/MIPv6Network.ned example/example.ned

cat <<EOF > example/CMakeLists.txt
PROJECT(Example)

CMAKE_MINIMUM_REQUIRED(VERSION 2.0)

#cmake -C#path to IPv6Suite's CMakeCache.txt
SET(IPV6SUITE_DATA_DIR "%{ipv6suite_data}")
#This line is necessary to distinguish between in source builds and (rpm builds)
SET(IPv6Suite_SOURCE_DIR ${IPV6SUITE_DATA_DIR})

SET(CMAKEFILES_PATH "\${IPV6SUITE_DATA_DIR}/Etc/CMake")
#Settings from these files should not be modified because
#IPv6Suite was built with these settings


SET(CMAKE_INSTALL_PREFIX "\${Example_BINARY_DIR}")

INCLUDE(\${CMAKEFILES_PATH}/Main.cmake)

INCLUDE_DIRECTORIES(\${BOOSTROOT})
INCLUDE_DIRECTORIES("%{include_dir}")

INCLUDE(\${CMAKEFILES_PATH}/LinkLibraries.cmake)

SET(ExampleBuild_ned_includes
  \${IPV6SUITE_DATA_DIR}/ned
)
CREATE_SIMULATION(ExampleBuild ExampleBuild_ned_includes example)
#TK executable built by default in case we build with COMMAND_ENV only
LINK_OPP_LIBRARIES(tkExampleBuild "\${OPP_TKGUILIBRARIES}")
EOF

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root)
%{_libdir}/*.so
%{include_dir}
%{ipv6suite_data}
%doc example 


%changelog
* Wed Nov 24 2004 Johnny Lai <johnny.lai@eng.monash.edu.au> 0.92-3.cvs20040924_1
- Updated BuildRequires to reflect build requirements

* Wed Aug 25 2004 Johnny Lai <johnny.lai@eng.monash.edu.au> 0.92-3.cvsXXXX
- Changed versioning so CVS date is in Release tag (Dag/Dries... convention)


* Tue Mar  2 2004 Johnny Lai <johnny.lai@eng.monash.edu.au> 0.92_20040302-2
- Tested example dir and updated description

* Mon Mar  1 2004 Johnny Lai <johnny.lai@eng.monash.edu.au> 0.92-1
- Initial build.


