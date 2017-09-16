# Customized ClipperLib #

This is a fork of the C++ version of Angus Johnson's excellent [ClipperLib library](http://www.angusj.com/delphi/clipper.php). 

Fancy template metaprogramming and neat C++11 features are not used, both to avoid unnecessary complexity and to keep the project viable in old compilers.

Please note: as the original ClipperLib, this library is intended to be source-included (rather than linked) in other projects.

## Main differences with the original ##

* It is patched to use native 128 bit integer multiplication if available (with Visual Studio's intrinsics and GCC's `__int128`). It helps a bit, but surprisingly not that much.

* Simple IO functions for `ClipperLib::Paths` and related types are added in a separate file.

* It is patched to optionally use a very simple and potentially very fast custom allocator instead of `std::allocator`, because the original code can allocate *huge* amounts of short-lived small objects, so under some workloads it spends most of its time managing the memory. Our custom bump allocator reserves large swaths of memory and bump-allocates small chunks as requested.

**PLEASE NOTE**: the last feature breaks drop-in compatibility with the original ClipperLib, because the custom allocator is not default-constructible and the underlying infrastructure must be cleaned up after each clipping operation. However, adaptation should require only minimal changes for most applications. Adaptation is a matter of taking into account the following:

 * By default, the old-style allocation is used. Only two main differences with vanilla ClipperLib:
  
   1. `Clipper` and `ClipperOffset` objects take a `CLIPPER_MMANAGER` object on construction:
     
        
**BEFORE:**
```c++
ClipperLib::Clipper clipper;
ClipperLib::ClipperOffset offset;
```
       
**AFTER:**
```c++
CLIPPER_MMANAGER manager;
ClipperLib::Clipper clipper(manager);
ClipperLib::ClipperOffset offset(manager);
```

  2. `PolyTree` objects are now owned by `Clipper` and `ClipperOffset` objects. `Execute()` methods set pointers to these owned `PolyTrees`, and the `PolyTrees` are cleared when executing the method `Clear()` of the owner.
     
**BEFORE:**
```c++
ClipperLib::Clipper clipper;
ClipperLib::PolyTree polytree;

//configure the clipper object and add paths as necessary...

clipper.Execute(cliptype, polytree);
clipper.Clear();

//read polytree's data...
```
            
**AFTER:**
```c++
CLIPPER_MMANAGER manager;
ClipperLib::Clipper clipper(manager);
ClipperLib::PolyTree *polytree;

//configure the clipper object and add paths as necessary...

clipper.Execute(cliptype, polytree);

//read polytree's data...

clipper.Clear();
```     

* To use the (custom) bump allocator, define `CLIPPER_USE_ARENA` before it is used in clipper.hpp (either `#define` it or add its definition to the compiler arguments). Then, you use ClipperLib just as described in the previous point, except you have to add some arguments upon constructing a `CLIPPER_MMANAGER`. Also, you **MUST** use a different `CLIPPER_MMANAGER` for each `Clipper` / `ClipperOffset` object, because nasty interactions will crash your application otherwise. The arguments are simple enough:
            
```c++
CLIPPER_MMANAGER manager("name_for_debug_purposes",
                         bool_arg_true_if_you_want_debug_messages,
                         byte_size_threshold_for_default_allocation,
                         arena_size_in_bytes);
```

