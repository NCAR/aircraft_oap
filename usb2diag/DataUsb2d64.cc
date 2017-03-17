

#include "DataUsb2d64.h"


const unsigned long long DataUsb2d64::_syncWord = 0xAAAAAA0000000000LL;
const unsigned long long DataUsb2d64::_syncMask = 0xFFFFFF0000000000LL;

/* -------------------------------------------------------------------- */
DataUsb2d64::DataUsb2d64 (FILE * fp) :DataMng(fp, 64)
{
  SetBt(64);
  Setfp(fp);
  Init();
}

/* -------------------------------------------------------------------- */
QPointArray* DataUsb2d64::GetPoints()
{ 
  if (!_chkInit()) {return NULL;}
  _pts= new QPointArray(TBYTE*8*_row_n*_rcdpr_n); 
  _ptstx= new QPointArray(_row_n*_rcdpr_n); 
  _ptsln= new QPointArray(_row_n); 
  _text ="";
  _get2drec(); 

  if (_hdr==NULL && _twod_rec==NULL) {_pts->resize(0); return _pts;}
  size_t       xleft =5, ytop =8, rheight=27;
  size_t       x, y = ytop;
  _cnt = 0;
  for (int i = 0; i < _row_n; ++i)
  {
    x = xleft;
    for (int j = 0; j< _rcdpr_n; ++j)
    {
       _getRecord(x, y, (unsigned char*)_twod_rec[i*_rcdpr_n+j].data);
       x += _slide_n;
       _text =_text +"Time:" + _gettw(_hdr[i*_rcdpr_n+j].timetag) 
       +"   Tas:" +QString().setNum(_twod_rec[i*_rcdpr_n+j].tas) + "~";
       _ptstx->setPoint(i*_rcdpr_n+j, xleft+j*_slide_n, y+_bit_n+14);
    }
    _ptsln->setPoint(i, xleft, y-1);
    y += _bit_n+rheight;
  }
   
  _pts->resize(_cnt );
  return _pts;
}

/* -------------------------------------------------------------------- */
void DataUsb2d64::_getRecord(int start_x, int start_y, unsigned char * data_p)
{
  for (int i = 0; i < _slide_n; ++i)
  {
    //get  and flip slide
    unsigned char s1[_byte_n];//_byte_n=8
    for (int n=0; n<_byte_n; ++n) {
      s1[7-n]=*data_p; data_p++;
    }

    unsigned long long slide = *(unsigned long long*)s1;
    //if ((slide & _syncMask) == _syncWord)
    //  printf("\ntime_wd:%llu\n", slide & ~_syncMask);	// Print timing word.
      
    slide = ~slide;
    //check every bit
    unsigned long long  mask =1;
    for (int b=0; b<_bit_n; ++b) { //_bit_n=64
      if (mask & slide) {
        _pts->setPoint(_cnt, start_x + i, start_y + _bit_n-b);
        ++_cnt;
      }
      mask*=2;
    }//b
  }//i
}


/* -------------------------------------------------------------------- */
void DataUsb2d64::_get2drec() {
   int trcd = _row_n*_rcdpr_n;
   _hdr = new nidas_hdr[trcd];
   _twod_rec = new usb2d_rec[trcd];
 
   fsetpos(_fp, &_pp);
   fpos_t tmp=_pp;
   int c = 0, tc=0;
   //get the last few records
   while (!feof(_fp)){
     nidas_hdr	h;
     usb2d_rec d;

     fread(&h, sizeof(nidas_hdr), 1, _fp);
     if (h.length==(unsigned short)_byte_usb2d_rcd)
     {
       fread(&d, h.length, 1, _fp);
       _hdr[c]=h;
       _twod_rec[c]=d;
       //printf("-----%llu %lu %lu %lx %lx\n", h.timetag, h.length,
       // 	h.sample_id, d.tas, d.id);
       c++; tc++;
       if (c>=trcd) { 
         c=0;
         _pp=tmp;
         fgetpos(_fp, &tmp);
       }//if

     } else {
       //fseek (_fp, h.length, SEEK_CUR );
       unsigned char dd[h.length];
       fread(&dd, h.length, 1, _fp);
     }
   } //while
  
   if (tc==0) {_hdr=NULL; _twod_rec=NULL; return;}
   //put cycled buff back to the order
   nidas_hdr	hdr[trcd];
   usb2d_rec	twod_rec[trcd];
  
   if ((c> 0)&& (_hdr[c].length>0)) {
     for (int i =0; i<trcd; i++) {
       hdr[i]= _hdr[c];
       twod_rec[i]= _twod_rec[c];
       c++;
       if (c>=trcd) {c=0;}
     }
     //back to _hdr and _twod_rec
     for (int i =0; i<trcd; i++) {
       _hdr[i]= hdr[i];
       _twod_rec[i]= twod_rec[i];
     }
   } 

   //print out data
   for (int i=0; i<trcd; i++) {
     printf("%llu %lu %lu %lx %lx\n", _hdr[i].timetag, _hdr[i].length,
		_hdr[i].sample_id, _twod_rec[i].tas, _twod_rec[i].id);
   }
   printf("one-run\n\n");
}



QString DataUsb2d64::_gettw(unsigned long long t) {
  
   time_t ut = t / USECS_PER_SEC;
   struct tm *tm= gmtime( &ut);
   char timeStamp[32];
   strftime(timeStamp, sizeof(timeStamp), "%Y-%m-%d %H:%M:%S", tm);
   QString tstr(timeStamp);
   tstr.append(".");
   
   QString n;
   n.setNum((uint)(t%USECS_PER_SEC)/1000);
   tstr.append(n);
   return tstr;
}
