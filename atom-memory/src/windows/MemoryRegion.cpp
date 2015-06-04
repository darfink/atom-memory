#include <atom-memory/MemoryRegion.hpp>
#include <atom-ex/WindowsException.hpp>

#include "Windows.hpp"

namespace atom {
  void MemoryRegion::SetFlags(int flags) {
    ulong winFlags = ConvertToWinFlags(flags);

    for(MemoryPage& page : mPages) {
      ulong previousFlags;
      ATOM_WINDOWS_ASSERT(VirtualProtect(
        page.base, page.size, winFlags, &previousFlags));

      page.previousFlags = ConvertFromWinFlags(previousFlags);
      page.currentFlags = flags;
    }
  }

  void MemoryRegion::ResetFlags(bool initial) {
    for(MemoryPage& page : mPages) {
      ulong flags = (initial ? page.initialFlags : page.previousFlags);
      ulong previousFlags;

      ATOM_WINDOWS_ASSERT(VirtualProtect(
        page.base, page.size, ConvertToWinFlags(flags), &previousFlags));

      page.previousFlags = ConvertFromWinFlags(previousFlags);
      page.currentFlags = flags;
    }
  }

  void MemoryRegion::UpdateMemoryPages(uintptr_t startPage, size_t pageCount) {
    mPages.clear();

    for(uint i = 0; i < pageCount; i++) {
      MemoryPage page = {};

      page.size = PageSize;
      page.base = reinterpret_cast<void*>(startPage + (PageSize * i));

      MEMORY_BASIC_INFORMATION memoryInformation;
      ATOM_WINDOWS_ASSERT(VirtualQuery(
        page.base, &memoryInformation, sizeof(MEMORY_BASIC_INFORMATION)));

      // We assume that only one page within the region is retrieved
      assert(PageSize == memoryInformation.RegionSize);

      int flags = ConvertFromWinFlags(memoryInformation.Protect);
      page.currentFlags = page.initialFlags = page.previousFlags = flags;

      mPages.push_back(page);
    }
  }
}
