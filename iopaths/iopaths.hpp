#ifndef  IOPATHS_HEADER
#define  IOPATHS_HEADER

#include <stdio.h>

#include "common.hpp"

/*we assume that:
* sizeof(clp::cInt)==sizeof(double)
* vectors are contiguous
* sizeof(clp::IntPoint)==(2*sizeof(clp::cInt))
* IntPoint.X comes before IntPoint.Y
*/

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

#define  READ_BINARY_NAIVE(start, size, num, f)   {size_t numio = fread  ((start), (size), (num), (f)); if (numio!=(num)) IOERR((f), (num), numio); }
#define WRITE_BINARY_NAIVE(start, size, num, f)   {size_t numio = fwrite ((start), (size), (num), (f)); if (numio!=(num)) IOERR((f), (num), numio); }

typedef std::vector<clp::DoublePoint> DPath;
typedef std::vector<DPath>            DPaths;

typedef union T64 {
    clp::cInt i;
    double d;
    T64() {}
    T64(clp::cInt _i) : i(_i) {}
    T64(double    _d) : d(_d) {}
} T64;

class IOErr {
public:
    const char * message;
    const char * function;
    FILE * f;
    int errn;
    bool writing;
    IOErr()                                             { clear(); }
    IOErr        (FILE * _f, bool w, const char * func) { setError(_f, w, func); }
    void clear()                                        { f = NULL; errn = 0;     message = NULL;           function = NULL;}
    void setError(FILE * _f, bool w, const char * func) { f = _f;   errn = errno; message = strerror(errn); function = func; }
};

//keep track or errors
class IOPaths {
public:
    std::vector<IOErr> errs;
    FILE *f;
    IOPaths()          : f(NULL) {}
    IOPaths(FILE * _f) : f(_f) {}
    inline void clear()    { errs.clear(); }
    inline bool hasError() { return !errs.empty(); }
    inline bool     readInt64(int64 &v)            { size_t numio = fread (&v, sizeof(v),      1, f); if (numio != 1) { errs.push_back(IOErr(f, false, "readInt64"   )); return false; }; return true; }
    inline bool    writeInt64(int64 v)             { size_t numio = fwrite(&v, sizeof(v),      1, f); if (numio != 1) { errs.push_back(IOErr(f, true,  "writeInt64"  )); return false; }; return true; }
    inline bool    readDouble(double &v)           { size_t numio = fread (&v, sizeof(v),      1, f); if (numio != 1) { errs.push_back(IOErr(f, false, "readDouble"  )); return false; }; return true; }
    inline bool   writeDouble(double v)            { size_t numio = fwrite(&v, sizeof(v),      1, f); if (numio != 1) { errs.push_back(IOErr(f, true,  "writeDouble" )); return false; }; return true; }
    inline bool      readT64P(T64 *v, size_t n)    { size_t numio = fread (v,  sizeof(T64),    n, f); if (numio != n) { errs.push_back(IOErr(f, false, "readT64"     )); return false; }; return true; }
    inline bool     writeT64P(T64 *v, size_t n)    { size_t numio = fwrite(v,  sizeof(T64),    n, f); if (numio != n) { errs.push_back(IOErr(f, true,  "writeT64"    )); return false; }; return true; }
    inline bool    readInt64P(int64  *v, size_t n) { size_t numio = fread (v,  sizeof(int64),  n, f); if (numio != n) { errs.push_back(IOErr(f, false, "readInt64*"  )); return false; }; return true; }
    inline bool   writeInt64P(int64  *v, size_t n) { size_t numio = fwrite(v,  sizeof(int64),  n, f); if (numio != n) { errs.push_back(IOErr(f, true,  "writeInt64*" )); return false; }; return true; }
    inline bool   readDoubleP(double *v, size_t n) { size_t numio =  fread(v,  sizeof(double), n, f); if (numio != n) { errs.push_back(IOErr(f, false, "readDouble*" )); return false; }; return true; }
    inline bool  writeDoubleP(double *v, size_t n) { size_t numio = fwrite(v,  sizeof(double), n, f); if (numio != n) { errs.push_back(IOErr(f, true,  "writeDouble*")); return false; }; return true; }

    bool   readClipperPaths(clp::Paths &paths);
    bool  writeClipperPaths(clp::Paths &paths, PathCloseMode mode);

    bool   readDoublePaths(clp::Paths  &paths, double scalingfactor);
    bool   readDoublePaths(DPaths  &paths);
    bool  writeDoublePaths(DPaths  &paths, PathCloseMode mode);
    bool  writeDoublePaths(clp::Paths  &paths, double scalingfactor, PathCloseMode mode);

    bool  writePrefixedClipperPaths(clp::Paths &paths, PathCloseMode mode);
    bool   readPrefixedClipperPaths(clp::Paths &paths);
};

//this assumes that sizeof(clp::cInt)==sizeof(double)==sizeof(T64)==8
int getPathsSerializedSize(clp::Paths &paths, PathCloseMode mode);

#endif