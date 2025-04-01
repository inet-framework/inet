from inet.simulation.project import *
from inet.common.util import *

inet_project = define_simulation_project("inet", version=None,
                                         folder_environment_variable="INET_ROOT",
                                         bin_folder="bin",
                                         library_folder="src",
                                         executables=["INET"],
                                         dynamic_libraries=["INET"],
                                         static_libraries=["INET"],
                                         ned_folders=["src", "examples", "showcases", "tutorials", "tests/networks", "tests/validation"],
                                         ned_exclusions=None,
                                         ini_file_folders=["examples", "showcases", "tutorials", "tests/fingerprint", "tests/validation"],
                                         image_folders=["images"],
                                         include_folders=["src", "src/inet/transportlayer/tcp_lwip/lwip/include", "src/inet/transportlayer/tcp_lwip/lwip/include/ipv4", "src/inet/transportlayer/tcp_lwip/lwip/include/ipv6"],
                                         cpp_folders=["src"],
                                         cpp_defines=["HAVE_FFMPEG", "HAVE_FFMPEG_SWRESAMPLE"],
                                         msg_folders=["src"],
                                         media_folder="doc/media",
                                         statistics_folder="statistics",
                                         fingerprint_store="tests/fingerprint/store.json",
                                         speed_store="tests/speed/store.json",
                                         external_libraries=["avcodec", "avformat", "avutil", "swresample", "osg", "osgText", "osgDB", "osgGA", "osgViewer", "osgUtil", "OpenThreads", "z3", "omp"],
                                         external_include_folders=["/usr/include/x86_64-linux-gnu"])

inet_baseline_project = define_simulation_project("inet-baseline",
                                                  folder_environment_variable="INET_ROOT",
                                                  folder="../inet-baseline",
                                                  library_folder="src",
                                                  dynamic_libraries=["INET"],
                                                  ned_folders=["src", "examples", "showcases", "tutorials", "tests/networks"]) if os.path.exists(get_workspace_path("inet-baseline")) else None
