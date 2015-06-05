#include <atom-memory/MemoryRegion.hpp>
#include <atom-ex/WindowsException.hpp>
#include <windows.h>

#include "Convert.hpp"

namespace atom {
  void MemoryRegion::SetFlags(int flags) {
    unsigned long winFlags = ConvertToWinFlags(flags);

    for(MemoryPage& page : mPages) {
      unsigned long previousFlags;
      ATOM_WINDOWS_ASSERT(VirtualProtect(
        page.base, page.size, winFlags, &previousFlags));

      page.previousFlags = ConvertFromWinFlags(previousFlags);
      page.currentFlags = flags;
    }
  }

  void MemoryRegion::ResetFlags(bool initial) {
    for(MemoryPage& page : mPages) {
      unsigned long flags = (initial ? page.initialFlags : page.previousFlags);
      unsigned long previousFlags;

      ATOM_WINDOWS_ASSERT(VirtualProtect(
        page.base, page.size, ConvertToWinFlags(flags), &previousFlags));

      page.previousFlags = ConvertFromWinFlags(previousFlags);
      page.currentFlags = flags;
    }
  }

  void MemoryRegion::UpdateMemoryPages(uintptr_t startPage, size_t pageCount) {
    mPages.clear();

    const size_t PageSize = Memory::GetPageSize();

    do {
      void* base = reinterpret_cast<void*>(startPage + (PageSize * mPages.size()));

      MEMORY_BASIC_INFORMATION memoryInformation;
      ATOM_WINDOWS_ASSERT(VirtualQuery(
        base, &memoryInformation, sizeof(MEMORY_BASIC_INFORMATION)));

      size_t pagesInRegion = memoryInformation.RegionSize / PageSize;
      int flags = ConvertFromWinFlags(memoryInformation.Protect);

      // When a memory region is queried on Windows, the size is equal to all
      // memory pages that lie consecutively in memory with the same flags.
      // To avoid unnecessary API calls, one might be enough to retrieve the
      // memory information relative to the supplied page count. In case the
      // page count is zero, we retrieve all pages within the region.
      for(size_t i = 0; i < pagesInRegion && (pageCount == 0 || mPages.size() < pageCount); i++) {
        MemoryPage page = {};

        page.size = PageSize;
        page.base = reinterpret_cast<void*>(startPage + (PageSize * mPages.size()));
        page.currentFlags = page.initialFlags = page.previousFlags = flags;
        page.guarded = !!(memoryInformation.Protect & PAGE_GUARD);
        page.committed = !!(memoryInformation.State & MEM_COMMIT);

        mPages.push_back(page);
      }
    } while(mPages.size() < pageCount);
  }
}
