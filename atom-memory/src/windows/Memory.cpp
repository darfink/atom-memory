#include <atom-ex/WindowsException.hpp>
#include <atom-memory/Memory.hpp>
#include <cassert>

#include "Convert.hpp"

namespace atom {
namespace Memory {
  void* Allocate(size_t size, int flags, size_t* allocated) {
    assert(size > 0);

    size = AlignTo(size, GetPageSize());
    void* address = VirtualAlloc(
      nullptr, size, MEM_COMMIT | MEM_RESERVE, ConvertToWinFlags(flags));

    ATOM_WINDOWS_ASSERT(address != nullptr);

    if(allocated != nullptr) {
      // The allocated memory is at least one page size
      *allocated = size;
    }

    return address;
  }

  void Free(void* address, size_t /* size */) {
    assert(address != nullptr);
    ATOM_WINDOWS_ASSERT(VirtualFree(address, 0, MEM_RELEASE));
  }

  size_t GetPageSize() {
    static size_t pageSize;

    if(pageSize == 0) {
      SYSTEM_INFO systemInfo;
      GetSystemInfo(&systemInfo);

      // Retrieve the size of a memory region page
      pageSize = systemInfo.dwPageSize;
    }

    return pageSize;
  }
} /* Memory */
} /* atom */
