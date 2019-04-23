

  #include "Memoization_uthash.h"

namespace inet {

namespace icancloud {




    MemoSupport::MemoSupport ( string component )
    {
            this->memo_calls   = nullptr ;
            this->memo_isDirty = 0 ;
            sprintf(this->file_name, "MEMO.%s.bin.gz", component.c_str()) ;
            sprintf(this->component_name, "%s", component.c_str()) ;

            restore_bin() ;
    }

    MemoSupport::~MemoSupport ( )
    {
            dump_bin() ;
    }

    double *MemoSupport::find ( unsigned int bsize, float *b )
    {
       MemoNode *fcall;

       // found it
       HASH_FIND(hh, this->memo_calls, b, bsize, fcall);
       if (nullptr == fcall)
               return nullptr ;

       fcall->hits++ ;
       return &(fcall->result_value) ;
    }

    void MemoSupport::add ( unsigned int bsize, float *b, double result )
    {
       MemoNode *fcall;

       // add it
       fcall     = (MemoNode *)malloc(sizeof(MemoNode) + bsize) ;
       fcall->id =  (float *)((char*)fcall + sizeof(MemoNode)) ;

           memcpy(fcall->id, b, bsize) ;
       fcall->id_size = bsize ;
       fcall->result_value = result ;
       fcall->hits = 1L ;

       this->memo_isDirty = 1 ;
       HASH_ADD_KEYPTR(hh,
               this->memo_calls,
               fcall->id,
               fcall->id_size,
               fcall);
    }

    void MemoSupport::dump_bin ( void )
    {
            gzFile file ;
            MemoNode *s ;

            // if (! this->memo_isDirty) return;

            file = gzopen(file_name, "w");
            if (!file) return;

            for (s = this->memo_calls;
                 s != nullptr;
                 s = (MemoNode *)(s->hh.next))
            {
                if (s->hits > 0)
                {
                    gzwrite(file, &(s->id_size),      sizeof(int)) ;
                    gzwrite(file, &(s->result_value), sizeof(double)) ;
                    gzwrite(file, &(s->hits),         sizeof(long)) ;
                    gzwrite(file,   s->id,            s->id_size) ;
                }
            }

            gzclose(file);
    }

    void MemoSupport::restore_bin ( void )
    {
            gzFile file;
            MemoNode *fcall, *s;
            int ret;

            file = gzopen(file_name, "r");
            if (nullptr==file) return ;

            while (!gzeof(file))
            {
                     fcall = (MemoNode *)malloc(sizeof(MemoNode)) ;
                     if (!fcall) { perror("malloc: "); continue; }

                  ret = gzread(file, &(fcall->id_size),      sizeof(int));
                  ret = gzread(file, &(fcall->result_value), sizeof(double));
                  ret = gzread(file, &(fcall->hits),         sizeof(long));
                  if (ret<0) { free(fcall) ; continue; }

                     fcall = (MemoNode *)realloc(fcall, sizeof(MemoNode) + fcall->id_size) ;
                     if (!fcall) { perror("realloc: "); continue; }

                  fcall->id = (float *)((char *)fcall + sizeof(MemoNode)) ;
                  ret = gzread(file, fcall->id, fcall->id_size);

                     fcall->hits = 1L ;

                  HASH_FIND(hh, this->memo_calls, fcall->id, fcall->id_size, s);
                  if (!s) {
                      HASH_ADD_KEYPTR(hh, this->memo_calls, fcall->id, fcall->id_size, fcall);
                  }
            }

            gzclose(file);
    }

    void MemoSupport::stats ( void )
    {
            gzFile file;
            MemoNode *fcall;
            int ret;

            file = gzopen(file_name, "r");
            if (nullptr==file) return ;

        cout << endl ;

        cout << setw(22) << "input\t"
             << setw(22) << "output\t"
             << setw(22) << "hits\t"
             << endl ;

            while (!gzeof(file))
            {
                     fcall = (MemoNode *)malloc(sizeof(MemoNode)) ;
                     if (!fcall) { perror("malloc: "); continue; }

                  ret = gzread(file, &(fcall->id_size),      sizeof(int));
                  ret = gzread(file, &(fcall->result_value), sizeof(double));
                  ret = gzread(file, &(fcall->hits),         sizeof(long));
                  if (ret<0) { free(fcall); continue; }

                     fcall = (MemoNode *)realloc(fcall, sizeof(MemoNode) + fcall->id_size) ;
                     if (!fcall) { perror("realloc: "); continue; }

                  fcall->id = (float *)((char *)fcall + sizeof(MemoNode)) ;
                  ret = gzread(file, fcall->id, fcall->id_size);

                  int last_i = fcall->id_size/sizeof(float);
                  for (int i=0; i<last_i; i++) {
                       cout << fixed << setw(22) << fcall->id[i] ;
                       if (i!=last_i-1)
                            cout << "," ;
                       else cout << "\t" ;
                  }
                  cout << fixed << setw(22) << fcall->result_value << "\t"
                       << fixed << setw(22) << fcall->hits         << "\t"
                       << endl ;

                     free(fcall) ;
            }

            gzclose(file);
    }

} // namespace icancloud
} // namespace inet
