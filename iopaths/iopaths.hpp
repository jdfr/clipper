#ifndef  IOPATHS_HEADER
#define  IOPATHS_HEADER

#include <stdio.h>

#include "common.hpp"

#if defined(_MSC_VER) //(defined(_WIN32) || defined(_WIN64))
  #define MY_FILENAME __FILE__
  #define MY_FUNCTION __FUNCTION__
#else
  #define MY_FILENAME __FILE__
  #define MY_FUNCTION __func__
#endif

#define STRINGIZE(x) STRINGIZE2(x)
#define STRINGIZE2(x) #x
#define LINE_STRING STRINGIZE(__LINE__)

#define IOERR(f, num, numio) {fprintf(stderr, "IOERROR IN LINE %s OF %s, function %s. ERRNO: %d. INTENDED/REAL IO BYTES: %d/%d\n", LINE_STRING, MY_FILENAME, MY_FUNCTION, ferror((f)), (num), (numio)); exit(-1);}

#define  READ_BINARY(start, size, num, f)   {size_t numio = fread  ((start), (size), (num), (f)); if (numio!=(num)) IOERR((f), (num), numio); }
#define WRITE_BINARY(start, size, num, f)   {size_t numio = fwrite ((start), (size), (num), (f)); if (numio!=(num)) IOERR((f), (num), numio); }

//#define DEBUG_IOPATHS
#ifdef DEBUG_IOPATHS
  void setIOErrOutput(FILE* err);
#endif

typedef std::vector<clp::DoublePoint> DPath;
typedef std::vector<DPath>            DPaths;

typedef std::vector<FILE*> FILES;

inline void   readInt64(FILE * f, int64 &v)  {  READ_BINARY(&v, sizeof(int64),  1, f); }
inline void  writeInt64(FILE * f, int64 &v)  { WRITE_BINARY(&v, sizeof(int64),  1, f); }
inline void  readDouble(FILE * f, double &v) {  READ_BINARY(&v, sizeof(double), 1, f); }
inline void writeDouble(FILE * f, double &v) { WRITE_BINARY(&v, sizeof(double), 1, f); }

void   readClipperPaths (FILE *f, clp::Paths &paths);
void  writeClipperPaths (FILE *f, clp::Paths &paths, PathCloseMode mode);
void  writeClipperSlice (FILE *f, clp::Paths &paths, std::vector<double> &zs, PathCloseMode mode);

void   readDoublePaths  (FILE *f, clp::Paths  &paths, double scalingfactor);
void   readDoublePaths  (FILE *f,     DPaths  &paths);
void  writeDoublePaths  (FILE *f, clp::Paths  &paths, double scalingfactor, PathCloseMode mode);
void  writeDoublePaths  (FILES fs,clp::Paths  &paths, double scalingfactor, PathCloseMode mode);

void  writePrefixedClipperPaths(FILE *f, clp::Paths &paths, PathCloseMode mode);
void   readPrefixedClipperPaths(FILE *f, clp::Paths &paths);

#endif