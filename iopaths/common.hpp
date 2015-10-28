#ifndef  COMMON_HEADER
#define  COMMON_HEADER

//some definitions and shortcuts

#include "clipper.hpp"

namespace clp = ClipperLib;

typedef clp::cInt int64;

//this enum has equivalent meaning to the "closed" flag in ClipperBase.AddPaths
enum PathCloseMode {PathOpen, PathLoop};

#endif