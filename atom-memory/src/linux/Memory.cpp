#pragma once

#include <atom-ex/SystemException.hpp>
#include <atom-memory/Memory.hpp>
#include <sys/types.h>
#include <sys/mman.h>
#include <cassert>

namespace atom {
namespace Memory {
  void* Allocate(size_t size, int flags, size_t* allocated) {
    assert(size > 0);

    size = AlignTo(size, GetPageSize());
    void* address = mmap(nullptr, size, flags, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    ATOM_SYSTEM_ASSERT(address != MAP_FAILED);

    if(allocated != nullptr) {
      allocated = size;
    }

    return address;
  }

  void Free(void* address, size_t size) {
    assert(address != nullptr);
    ATOM_SYSTEM_ASSERT(munmap(address, size) == 0);
  }

  size_t GetPageSize() {
    static size_t pageSize;

    if(pageSize == 0) {
      pageSize = getpagesize();
    }

    return pageSize;
  }
} /* Memory */
} /* atom */
