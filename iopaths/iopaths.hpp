#ifndef  IOPATHS_HEADER
#define  IOPATHS_HEADER

#include <stdio.h>

#include "common.hpp"

typedef struct Slice {
    std::vector<double> zs;
    clp::Paths paths;
    Slice(){}
    Slice(clp::Paths p) : paths(p) {}
} Slice;

typedef std::vector<Slice> Slices;

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

void   readInt64        (FILE * f, int64 &v);
void  writeInt64        (FILE * f, int64 &v);
void   readClipperPaths(FILE *f, clp::Paths &paths);
void  writeClipperPaths(FILE *f, clp::Paths &paths, PathCloseMode mode);
void   readClipperSlice (FILE *f, Slice      &slice);
void  writeClipperSlice (FILE *f, Slice      &slice, PathCloseMode mode);
void  writeClipperSlice (FILE *f, clp::Paths &paths, std::vector<double> &zs, PathCloseMode mode);
void   readClipperSlices(FILE *f, Slices     &slices);
void  writeClipperSlices(FILE *f, Slices     &slices, PathCloseMode mode);
void   readClipperSlicesWithTemplate(FILE *f, Slices &slices, Slice &templt);

void  writePrefixedClipperPaths(FILE *f, clp::Paths &paths, PathCloseMode mode);

#endif