#ifndef ALLOCATION_SCHEMES_HEADER
#define ALLOCATION_SCHEMES_HEADER

#include <vector>
#include <stdexcept>
#include <sstream>
#include <cstring>

/*this class is intended to have a common interface with ArenaMemoryManager,
to avoid heavy use of template metaprogramming (which would require
extensive changes to the code using ClipperLib)*/
class SimpleMemoryManager {
public:
    static const bool isArena                         = false;
    static const bool usePlacementNew                 = false;
    static const bool useDelete                       = true;
    static const bool useReset                        = false;
    static const bool allocator_default_constructible = true;
    void  *allocate(size_t n)                              { return new char[n]; }
    void deallocate(void *p, size_t n)                     { delete[](char*)(p); }
    template<typename T>     T *allocate(size_t n)         { return   (T*)(allocate(sizeof(T)*n)); }
    template<typename T> void deallocate(T *p, size_t n)   { delete[](char*)(p); }
    template<typename T> std::allocator<T> get_allocator() { return std::allocator<T>(); }
    void free() {}
    void reset() {}
protected:
};

template<typename T> class ArenaAllocator;

/*this is a dumb memory arena. It allocates memory as needeed, but never really frees
it until it is reset, so depending on the workload it may require *a lot* of memory.
But it is real fast, because allocation usually just increments a pointer, and
deallocation is a NOP. Allocates new memory chunks as needed*/
class ArenaMemoryManager {
public:
    static const bool isArena                         = true;
    static const bool usePlacementNew                 = true;
    static const bool useDelete                       = false;
    static const bool useReset                        = true;
    static const bool allocator_default_constructible = false;
    ArenaMemoryManager(size_t _chunkSize, size_t initNumChunks = 1) : chunks(initNumChunks), chunkSize(_chunkSize) {
        if (initNumChunks == 0) initNumChunks = 1; //must have at least one initial chunk!
        for (std::vector<char *>::iterator chunk = chunks.begin(); chunk != chunks.end(); ++chunk) {
            *chunk = new char[chunkSize];
        }
        reset();
    }
    ~ArenaMemoryManager() { free(); delete[] chunks.front(); }
    //PLEASE NOTE: argument n MUST be aligned to whatever the maximum alignment used in the arena!!!!!
    //we could align it on each call, but I wonder how likely is the compiler to optimize it away when not required...
    void *allocate(size_t n) {
        if ((n % sizeof(double)) != 0) {
            //C++ should never call this method with n such that this branch gets activated.
            //But, just in case, we assume that aligning to sizeof(double)==8 should be enough
            n = ((n / sizeof(double)) + 1) * sizeof(double);
        }
        char *result = currentPointer;
        currentPointer += n;
        if (currentPointer > endPointer) {
            if (n > chunkSize) {
                std::ostringstream fmt;
                fmt << "In ArenaMemoryManager: chunkSize " << chunkSize << " is smaller than requested allocation size " << n << ". Please increase chunkSize.";
                throw std::runtime_error(fmt.str());
            }
            ++currentChunk;
            if (currentChunk >= chunks.size()) {
                chunks.push_back(new char[chunkSize]);
            }
            result = chunks[currentChunk];
            endPointer = result + chunkSize;
            currentPointer = result + n;
        }
        return result;
    }
    void deallocate(void *p, size_t n)                   {}
    template<typename T> T     *allocate(size_t n)       { return (T*)(allocate(sizeof(T)*n)); }
    template<typename T> void deallocate(T *p, size_t n) {}
    void reset() {
        currentChunk = 0;
        currentPointer = chunks[currentChunk];
        endPointer = currentPointer + chunkSize;
    }
    template<typename T> ArenaAllocator<T> get_allocator() { return ArenaAllocator<T>(*this); }
    //will free all chunks EXCEPT the first one.
    //Rationale: sucessful calls to ArenaAllocator::allocate(0) must return non-NULL pointers.
    //Always having at least one chunk is the easiest way to guarantee it without handling the empty case
    void free() {
        for (std::vector<char *>::iterator chunk = chunks.begin() + 1; chunk != chunks.end(); ++chunk) {
            delete[] * chunk;
        }
        chunks.resize(1, chunks.front());
        reset();
    }
protected:
    std::vector<char *> chunks;
    size_t chunkSize;
    size_t currentChunk;
    char *currentPointer;
    char *endPointer;
    template<typename T> friend class ArenaAllocator;
};

typedef std::vector<int>::const_pointer xxx;

//PLEASE NOTE: the lifetime of any ArenaAllocator should NEVER surpass the lifetime of its ArenaMemoryManager
template<typename T> class ArenaAllocator {
public:
    typedef T* pointer;
    typedef const T* const_pointer;
    typedef T& reference;
    typedef const T& const_reference;
    typedef size_t size_type;
    typedef ptrdiff_t difference_type;
    typedef T value_type;
    template <typename U> ArenaAllocator(const ArenaAllocator<U>& other) : arena(other.arena) {}
    //template <typename U> ArenaAllocator<T>& operator = (const ArenaAllocator<U>& other) { arena = other.arena; return *this; };
    template <typename U> struct rebind  { typedef ArenaAllocator<U> other; };
    T*     allocate(std::size_t n)       { return arena->allocate<T>(n); }
    void deallocate(T* p, std::size_t n) {}
    void construct(pointer p, const_reference val) { new ((void*)p) T(val); }
    void destroy(  pointer p) { p->~T(); }
    pointer       address(      reference x) const { return &x; }
    const_pointer address(const_reference x) const { return &x; }
    size_type max_size() const { return LLONG_MAX; }
#ifdef CLIPPER_ALLOCATOR_CXX11 //the code must be compilable in C++03 (for Perl and Python)
    typedef std::true_type propagate_on_container_copy_assignment;
    typedef std::true_type propagate_on_container_move_assignment;
    typedef std::true_type propagate_on_container_swap;
    typedef std::true_type is_always_equal;
#endif
protected:
    ArenaAllocator(ArenaMemoryManager &_arena) : arena(&_arena) {};
    ArenaMemoryManager *arena;
    friend class ArenaMemoryManager;
    template <typename> friend class ArenaAllocator;
    template <class V, class W> friend bool operator==(const ArenaAllocator<V>&, const ArenaAllocator<W>&);
    template <class V, class W> friend bool operator!=(const ArenaAllocator<V>&, const ArenaAllocator<W>&);
};

template <class V, class W> bool operator==(const ArenaAllocator<V> &a, const ArenaAllocator<W> &b) { return a.arena == b.arena; }
template <class V, class W> bool operator!=(const ArenaAllocator<V> &a, const ArenaAllocator<W> &b) { return a.arena != b.arena;; }

#endif