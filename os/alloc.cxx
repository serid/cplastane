#include "alloc.hxx"

#if defined(unix) || defined(__unix__) || defined(__unix)
#define CPLASTANE_UNIX
#endif

#if defined(_WIN32) || defined(_WIN64)
#define CPLASTANE_WIN
#endif

#ifdef CPLASTANE_UNIX

#include <stdexcept>
#include <sys/mman.h>

void *alloc_executable(size_t size) {
    void *mem = mmap(nullptr, size, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (mem == MAP_FAILED)
        throw std::runtime_error("allocation failed");
    return mem;
}

void dealloc(void *mem, size_t size) {
    int i = munmap(mem, size);
    if (i == -1)
        throw std::runtime_error("deallocation failed");
}

void flush_instruction_cache(void *mem, size_t size) {
    void* mem1 = mem;
    size_t size1 = size;
}

#endif

#ifdef CPLASTANE_WIN

#include <stdexcept>
#include <Windows.h>
#include <Memoryapi.h>

void *alloc_executable(size_t size) {
    void *mem = VirtualAlloc(NULL, size, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (mem == NULL)
        throw std::runtime_error("allocation failed");
    return mem;
}

void dealloc(void *mem, size_t size) {
    size_t asize = size;
    BOOL b = VirtualFree(mem, 0, MEM_RELEASE);
    if (b == 0)
        throw std::runtime_error("deallocation failed");
}

void flush_instruction_cache(void *mem, size_t size) {
    BOOL b = FlushInstructionCache(GetCurrentProcess(), mem, size);
    if (b == 0)
        throw std::runtime_error("flush failed");
}

#endif
