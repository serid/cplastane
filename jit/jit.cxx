#include "jit.hxx"

#include <stdexcept>
#include <cstring>

#include "../os/alloc.hxx"

namespace jit {
    // The function executes the code at pointer mc as if it was a `jit_func_t` function.
    auto eval_mc(const u8 *mc, size_t len) -> i64 {
        // This function places a `ud2` trap after the executable code to catch
        // malicous programs which overflow the buffer and continue executing.
        // Extend buffer to accomodate `ud2` trap.
        size_t buf_len = len + 2;
        void *exec_mc = alloc_executable(len);
        if (exec_mc == nullptr) {
            throw std::logic_error("mmap failed");
        }

        std::memcpy(exec_mc, mc, len);

        // Write the `ud2` trap.
        reinterpret_cast<char *>(exec_mc)[buf_len - 2] = 0x0F;
        reinterpret_cast<char *>(exec_mc)[buf_len - 1] = 0x0B;

        flush_instruction_cache(exec_mc, buf_len);

        auto func = (jit_func_t) exec_mc;

        i64 execution_result = func();

        dealloc(exec_mc, buf_len);

        return execution_result;
    }
}