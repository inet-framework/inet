//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 


  #include "ICCLog.h"

namespace inet {

namespace icancloud {



  ICCLog::ICCLog ( void )
  {
      this->isOpened    = false;
      this->compression = false;
      this->fname       = "" ;
  }

  ICCLog::~ICCLog ( void )
  {
      this->Close();
  }

  void ICCLog::Open ( std::string fname, bool compression )
  {
      return this->Open(fname.c_str(), compression) ;
  }

  void ICCLog::Open ( const char *fname, bool compression )
  {
      if (true == this->isOpened)
      {
          if (this->fname == fname)
               return;
          else this->Close();
      }

      this->isOpened    = true;
      this->compression = compression;
      this->fname       = fname;

      if (this->compression == true)
          this->fz = gzopen(fname, "ab1h");
      else this->f.open(fname, ios::app);
  }

  void ICCLog::Append ( const char *format, ... )
  {
      va_list args;
      va_start(args, format);

      vsprintf(this->fb, format, args);

      if (true == this->isOpened)
      {
          if (this->compression == true)
              gzwrite(fz, fb, strlen(fb));
          else f << this->fb;
      }

      va_end(args);
  }

  void ICCLog::Close ( void )
  {
      if (true == this->isOpened)
      {
          if (true == this->compression)
              gzclose_w(this->fz);
          else f.close();

          this->isOpened = false;
      }
  }


} // namespace icancloud
} // namespace inet
