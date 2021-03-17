#pragma once

#include <cstddef>

void *alloc_executable(size_t size);

void dealloc(void *mem, size_t size);

// Applications should call FlushInstructionCache if they generate or modify code in memory.
// The CPU cannot detect the change, and may execute the old code it cached.
void flush_instruction_cache(void *mem, size_t size);