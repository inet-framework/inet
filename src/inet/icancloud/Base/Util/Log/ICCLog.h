/*
 * @class ICCLog ICCLog.h "ICCLog.h"
 *
 * This class generates a log file offering methods to append data, and open/close the file.
 * The main aim of this class is the possibility of compress the logs at runtime.
 *
 * @author Alejandro Calderón
 * @date 2012-08-18
 */

#ifndef ICCLOG_H
#define ICCLOG_H





#include <iostream>
#include <fstream>
#include <cstdarg>
#include <cstring>
#include <stdio.h>
#include <stdlib.h>
#include <zlib.h>

namespace inet {

namespace icancloud {

using namespace std;

  /* 
     Usage:

       ICCLog l1 ;

       l1.Open(logname, true) ;
       l1.Append("@Logger-mode;%d\n", totalNumberNodes) ;
       l1.Close() ; // Explicit
  */

  class ICCLog {

       private:
		  gzFile fz;
		  ofstream f;
		  bool compression;
		  bool isOpened;
		  std::string fname;
		  char   fb[512*1024];

       public:
                   ICCLog ( void ) ;
                  ~ICCLog ( void ) ;

		  void Open   ( const char *fname, bool compression );
		  void Open   ( std::string fname, bool compression );
		  void Append ( const char *format, ... );
		  void Close  ( void );
  } ;

} // namespace icancloud
} // namespace inet

#endif /* ICCLOG_H */

