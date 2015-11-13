#include "iopaths.hpp"

#include <stdlib.h>

typedef clp::Paths::iterator psi;
typedef clp::Path::iterator pi;
typedef DPaths::iterator dsi;
typedef DPath::iterator di;

//we assume that sizeof(clp::cInt)==sizeof(double)

/*if SAFEWAY is not defined, we assume that:
     * vectors are contiguous
     * sizeof(clp::IntPoint)==(2*sizeof(clp::cInt))
     * IntPoint.X comes before IntPoint.Y
*/
//#define SAFEWAY



#ifdef DEBUG_IOPATHS
  FILE *STDERR=stderr;
  void setIOErrOutput(FILE* err) {
    STDERR = err;
  }
#endif

void   readDoublePaths(FILE *f, clp::Paths  &paths, double scalingfactor) {
    int64 numpaths, numpoints;

    READ_BINARY(&numpaths, sizeof(int64), 1, f);

    size_t oldsize = paths.size(), newsize = oldsize + numpaths;

    paths.resize(newsize);
    double v[2];
    for (psi path = paths.begin() + oldsize; path != paths.end(); ++path) {
        READ_BINARY(&numpoints, sizeof(int64), 1, f);
        path->resize(numpoints);
        pi pathend = path->end();
        for (pi point = path->begin(); point != pathend; ++point) {
            READ_BINARY(v, sizeof(double), 2, f);
            point->X = (int64)(v[0] * scalingfactor);
            point->Y = (int64)(v[1] * scalingfactor);
        }
    }
}

void  writeDoublePaths(FILE *f, clp::Paths  &paths, double scalingfactor, PathCloseMode mode) {
    writeDoublePaths(FILES(1, f), paths, scalingfactor, mode);
}

void  writeDoublePaths(FILES fs, clp::Paths  &paths, double scalingfactor, PathCloseMode mode) {
    int64 numpaths = paths.size(), numpoints, numpointsdeclared;
    for (FILES::iterator f = fs.begin(); f != fs.end(); ++f) {
        WRITE_BINARY(&numpaths, sizeof(int64), 1, *f);
    }

    bool addlast = (mode == PathLoop) && (paths.size() > 0);
    double v[2];
    for (psi path = paths.begin(); path != paths.end(); ++path) {
        numpoints = path->size();
        numpointsdeclared = addlast ? (numpoints + 1) : numpoints;
        for (FILES::iterator f = fs.begin(); f != fs.end(); ++f) {
            WRITE_BINARY(&numpointsdeclared, sizeof(int64), 1, *f);
        }
        pi pathend = path->end();
        for (pi point = path->begin(); point != pathend; ++point) {
            v[0] = point->X * scalingfactor;
            v[1] = point->Y * scalingfactor;
            for (FILES::iterator f = fs.begin(); f != fs.end(); ++f) {
                WRITE_BINARY(v, sizeof(double), 2, *f);
            }
        }
        if (addlast) {
            v[0] = path->front().X * scalingfactor;
            v[1] = path->front().Y * scalingfactor;
            for (FILES::iterator f = fs.begin(); f != fs.end(); ++f) {
                WRITE_BINARY(v, sizeof(double), 2, *f);
            }
        }
    }
}


void readClipperPaths(FILE *f, clp::Paths &paths) {
    int64 numpaths, numpoints;
    
    READ_BINARY(&numpaths, sizeof(int64), 1, f);
    
    size_t oldsize = paths.size(), newsize = oldsize+numpaths;
    
    paths.resize(newsize);
    for (psi path = paths.begin()+oldsize; path!=paths.end(); ++path) {
        READ_BINARY(&numpoints, sizeof(int64), 1, f);
        path->resize(numpoints);
        #ifdef SAFEWAY
            pi pathend = path->end();
            for (pi point=path->begin(); point!=pathend; ++point) {
                READ_BINARY(&point->X, sizeof(int64), 1, f);
                READ_BINARY(&point->Y, sizeof(int64), 1, f);
            }
        #else
            size_t num = 2*numpoints;
            READ_BINARY(&((*path)[0]), sizeof(int64), num, f);
        #endif
    }
}

void writeClipperPaths(FILE *f, clp::Paths &paths, PathCloseMode mode) {
    int64 numpaths = paths.size(), numpoints, numpointsdeclared;
#ifdef DEBUG_IOPATHS
    fprintf(STDERR, "In writeClipperPaths, writing numpaths=%lld\n", numpaths); fflush(STDERR);
#endif
    WRITE_BINARY(&numpaths, sizeof(int64), 1, f);
    
#ifdef DEBUG_IOPATHS
    size_t k=0;
#endif
    bool addlast = (mode == PathLoop) && (paths.size() > 0);
    for (psi path = paths.begin(); path!=paths.end(); ++path) {
        numpoints = path->size();
        numpointsdeclared = addlast ? (numpoints+1) : numpoints;
#ifdef DEBUG_IOPATHS
        fprintf(STDERR, "In writeClipperPaths (path %llu/%llu), writing numpoints=%lld\n", k++, paths.size(), numpoints); fflush(STDERR);
#endif
        WRITE_BINARY(&numpointsdeclared, sizeof(int64), 1, f);
#        ifdef SAFEWAY
            pi pathend = path->end();
#         ifdef DEBUG_IOPATHS
            fprintf(STDERR, "In writeClipperPaths, writing %lld points\n", path->size());
            size_t kk=0;
#         endif
            for (pi point=path->begin(); point!=pathend; ++point) {
#         ifdef DEBUG_IOPATHS
                fprintf(STDERR, "    point %llu: %lld, %lld\n", kk++, point->X, point->Y); fflush(STDERR);
#         endif
                WRITE_BINARY(&point->X, sizeof(int64), 1, f);
                WRITE_BINARY(&point->Y, sizeof(int64), 1, f);
            }
#        else
            size_t num = 2*numpoints;
#         ifdef DEBUG_IOPATHS
            fprintf(STDERR, "In writeClipperPaths, writing %lld longints (%lld points)\n", num, numpoints);
            clp::IntPoint *d1 = &((*path)[0]);
            int64 *d2 = (int64*)(&((*path)[0]));
            for (int64 kk=0; kk<numpoints; ++kk) {
                fprintf(STDERR, "    point %lld (style A): %lld, %lld\n", kk, d1->X, d1->Y); d1++;
                int64 a = *(d2++);
                int64 b = *(d2++);
                fprintf(STDERR, "    point %lld (style B): %lld, %lld\n", kk, a, b);
            }
              fflush(STDERR);
#         endif
            WRITE_BINARY(&((*path)[0]), sizeof(int64), num, f);
#        endif
            if (addlast) {
                WRITE_BINARY(&((*path)[0]), sizeof(int64), 2, f);
            }
    }


}


void  writeClipperSlice (FILE *f, clp::Paths &paths, std::vector<double> &zs, PathCloseMode mode) {
    int64 numzs = zs.size();
    
#ifdef DEBUG_IOPATHS
    fprintf(STDERR, "In writeClipperSlice, writing numzs=%lld\n", numzs); fflush(STDERR);
#endif
    WRITE_BINARY(&numzs, sizeof(int64), 1, f);

    #ifdef SAFEWAY
        for (di z = zs.begin(); z!=zs.end(); ++z) {
            WRITE_BINARY(&*z, sizeof(double), 1, f);
        }
    #else
    if (numzs > 0) {
#ifdef DEBUG_IOPATHS
        fprintf(STDERR, "In writeClipperSlice, writing %lld doubles\n", numzs); fflush(STDERR);
#endif
        WRITE_BINARY(&(zs[0]), sizeof(double), numzs, f);
    }
    #endif
    writeClipperPaths(f, paths, mode);//PathLoop);
}

void  writePrefixedClipperPaths(FILE *f, clp::Paths &paths, PathCloseMode mode) {
    int64 numpaths = paths.size(), numpoints, numpointsdeclared;
#ifdef DEBUG_IOPATHS
    fprintf(STDERR, "In writePrefixedClipperPaths, writing numpaths=%lld\n", numpaths); fflush(STDERR);
    int k=0;
#endif

    WRITE_BINARY(&numpaths, sizeof(int64), 1, f);
    bool addlast = (mode == PathLoop) && (paths.size() > 0);
    for (psi path = paths.begin(); path != paths.end(); ++path) {
        numpoints = path->size();
        numpointsdeclared = addlast ? (numpoints + 1) : numpoints;
#ifdef DEBUG_IOPATHS
    fprintf(STDERR, "In writePrefixedClipperPaths, writing numpointsdeclared=%lld for path %d\n", numpointsdeclared, k); fflush(STDERR);
    k++;
#endif
        WRITE_BINARY(&numpointsdeclared, sizeof(int64), 1, f);
    }

#ifdef DEBUG_IOPATHS
    k=0;
#endif
    for (psi path = paths.begin(); path != paths.end(); ++path) {
        numpoints = path->size();
        size_t num = 2 * numpoints;
#ifdef DEBUG_IOPATHS
    fprintf(STDERR, "In writePrefixedClipperPaths, writing path %d with num=%lld\n", k, num); fflush(STDERR);
    k++;
#endif
        WRITE_BINARY(&((*path)[0]), sizeof(int64), num, f);
        if (addlast) {
            WRITE_BINARY(&((*path)[0]), sizeof(int64), 2, f);
        }
    }
}

void readPrefixedClipperPaths(FILE *f, clp::Paths &paths) {
    int64 numpaths, numpoints;

    READ_BINARY(&numpaths, sizeof(int64), 1, f);

    size_t oldsize = paths.size(), newsize = oldsize + numpaths;

    paths.resize(newsize);

    for (psi path = paths.begin() + oldsize; path != paths.end(); ++path) {
        READ_BINARY(&numpoints, sizeof(int64), 1, f);
        path->resize(numpoints);
    }

    for (psi path = paths.begin() + oldsize; path != paths.end(); ++path) {
#ifdef SAFEWAY
        pi pathend = path->end();
        for (pi point = path->begin(); point != pathend; ++point) {
            READ_BINARY(&point->X, sizeof(int64), 1, f);
            READ_BINARY(&point->Y, sizeof(int64), 1, f);
        }
#else
        size_t num = 2 * path->size();
        READ_BINARY(&((*path)[0]), sizeof(int64), num, f);
#endif
    }
}
