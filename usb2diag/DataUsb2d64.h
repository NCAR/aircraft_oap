#ifndef _DataUsb2d64_h_
#define _DataUsb2d64_h_

#define _LARGEFILE64_SOURCE
#include "DataMng.h"


#define _FILE_OFFSET_BITS 64
/* -------------------------------------------------------------------- */
/**
 * Class to mng PMS-2D 64 bit data.
 */
class DataUsb2d64: public DataMng
{

public:
  DataUsb2d64 ( FILE * fp);
  virtual ~DataUsb2d64() {}  

  virtual QPointArray* GetPoints();
  
private:
  virtual void _getRecord(int x, int y, unsigned char* data_p); 
  virtual void _get2drec();
  QString _gettw(unsigned long long t);
  static const unsigned long long _syncMask, _syncWord;

};

#endif
