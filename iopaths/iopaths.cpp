#include "iopaths.hpp"

typedef clp::Paths::iterator psi;
typedef clp::Path::iterator pi;
typedef DPaths::iterator dsi;
typedef DPath::iterator di;


int getPathsSerializedSize(clp::Paths &paths, PathCloseMode mode) {
    int s = 0;
    for (psi path = paths.begin(); path != paths.end(); ++path) {
        s += (int)path->size();
    }
    s *= 2;
    s += 1 + ((int)paths.size());
    if ((mode == PathLoop) && (paths.size() > 0)) s += 2 * (int)paths.size();
    s *= 8;
    return s;
}

#define  READ_BINARY_INCLASS(start, size, num, f)   {size_t numio = fread  ((start), (size), (num), (f)); if (numio!=(num)) { errs.push_back(IOErr(f, false, MY_FUNCTION)); return false; } }
#define WRITE_BINARY_INCLASS(start, size, num, f)   {size_t numio = fwrite ((start), (size), (num), (f)); if (numio!=(num)) { errs.push_back(IOErr(f, true,  MY_FUNCTION)); return false; } }


bool IOPaths::readDoublePaths(clp::Paths  &paths, double scalingfactor) {
    int64 numpaths, numpoints;

    READ_BINARY_INCLASS(&numpaths, sizeof(int64), 1, f);

    size_t oldsize = paths.size(), newsize = oldsize + numpaths;

    paths.resize(newsize);

    for (psi path = paths.begin() + oldsize; path != paths.end(); ++path) {
        READ_BINARY_INCLASS(&numpoints, sizeof(int64), 1, f);
        path->resize(numpoints);
    }

    double v[2];
    for (psi path = paths.begin() + oldsize; path != paths.end(); ++path) {
        pi pathend = path->end();
        for (pi point = path->begin(); point != pathend; ++point) {
            READ_BINARY_INCLASS(v, sizeof(double), 2, f);
            point->X = (int64)(v[0] * scalingfactor);
            point->Y = (int64)(v[1] * scalingfactor);
        }
    }
    return true;
}

bool   IOPaths::readDoublePaths(DPaths  &paths) {
    int64 numpaths, numpoints;

    READ_BINARY_INCLASS(&numpaths, sizeof(int64), 1, f);

    size_t oldsize = paths.size(), newsize = oldsize + numpaths;

    paths.resize(newsize);

    for (dsi path = paths.begin() + oldsize; path != paths.end(); ++path) {
        READ_BINARY_INCLASS(&numpoints, sizeof(int64), 1, f);
        path->resize(numpoints);
    }

    for (dsi path = paths.begin() + oldsize; path != paths.end(); ++path) {
        READ_BINARY_INCLASS(&path->front(), sizeof(double), path->size() * 2, f);
    }
    return true;
}

bool  IOPaths::writeDoublePaths(DPaths &paths, PathCloseMode mode) {
    int64 numpaths = paths.size(), numpointsdeclared;
    WRITE_BINARY_INCLASS(&numpaths, sizeof(int64), 1, f);

    bool addlast = (mode == PathLoop) && (paths.size() > 0);
    for (dsi path = paths.begin(); path != paths.end(); ++path) {
        numpointsdeclared = path->size();
        if (addlast) ++numpointsdeclared;
        WRITE_BINARY_INCLASS(&numpointsdeclared, sizeof(int64), 1, f);
    }

    for (dsi path = paths.begin(); path != paths.end(); ++path) {
        WRITE_BINARY_INCLASS(&((*path)[0]), sizeof(double), path->size() * 2, f);
        if (addlast) {
            WRITE_BINARY_INCLASS(&((*path)[0]), sizeof(double), 2, f);
        }
    }
    return true;
}

bool  IOPaths::writeDoublePaths(clp::Paths  &paths, double scalingfactor, PathCloseMode mode) {

    int64 numpaths = paths.size(), numpointsdeclared;
    WRITE_BINARY_INCLASS(&numpaths, sizeof(int64), 1, f);

    bool addlast = (mode == PathLoop) && (paths.size() > 0);
    for (psi path = paths.begin(); path != paths.end(); ++path) {
        numpointsdeclared = path->size();
        if (addlast) ++numpointsdeclared;
        WRITE_BINARY_INCLASS(&numpointsdeclared, sizeof(int64), 1, f);
    }

    double v[2];
    for (psi path = paths.begin(); path != paths.end(); ++path) {
        pi pathend = path->end();
        for (pi point = path->begin(); point != pathend; ++point) {
            v[0] = point->X * scalingfactor;
            v[1] = point->Y * scalingfactor;
            WRITE_BINARY_INCLASS(v, sizeof(double), 2, f);
        }
        if (addlast) {
            v[0] = path->front().X * scalingfactor;
            v[1] = path->front().Y * scalingfactor;
            WRITE_BINARY_INCLASS(v, sizeof(double), 2, f);
        }
    }
    return true;
}


bool IOPaths::readClipperPaths(clp::Paths &paths) {
    int64 numpaths, numpoints;

    READ_BINARY_INCLASS(&numpaths, sizeof(int64), 1, f);

    size_t oldsize = paths.size(), newsize = oldsize + numpaths;

    paths.resize(newsize);

    for (psi path = paths.begin() + oldsize; path != paths.end(); ++path) {
        READ_BINARY_INCLASS(&numpoints, sizeof(int64), 1, f);
        path->resize(numpoints);
    }

    for (psi path = paths.begin() + oldsize; path != paths.end(); ++path) {
        READ_BINARY_INCLASS(&((*path)[0]), sizeof(int64), path->size() * 2, f);
    }
    return true;
}

bool IOPaths::writeClipperPaths(clp::Paths &paths, PathCloseMode mode) {
    int64 numpaths = paths.size(), numpointsdeclared;

    WRITE_BINARY_INCLASS(&numpaths, sizeof(int64), 1, f);
    bool addlast = (mode == PathLoop) && (paths.size() > 0);
    for (psi path = paths.begin(); path != paths.end(); ++path) {
        numpointsdeclared = path->size();
        if (addlast) ++numpointsdeclared;
        WRITE_BINARY_INCLASS(&numpointsdeclared, sizeof(int64), 1, f);
    }

    for (psi path = paths.begin(); path != paths.end(); ++path) {
        WRITE_BINARY_INCLASS(&((*path)[0]), sizeof(int64), path->size() * 2, f);
        if (addlast) {
            WRITE_BINARY_INCLASS(&((*path)[0]), sizeof(int64), 2, f);
        }
    }
    return true;
}
