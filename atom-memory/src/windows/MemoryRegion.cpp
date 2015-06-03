#include <atom-memory/MemoryRegion.hpp>
#include <atom-ex/WindowsException.hpp>

namespace atom {
  size_t MemoryRegion::PageSize;

  const ulong MemoryRegion::ReadWriteExecute =
    MemoryRegion::Execute | MemoryRegion::Write | MemoryRegion::Read;

  static ulong ConvertToWinFlags(int flags) {
    ulong result = 0;

    if(flags == MemoryRegion::Read) {
      result = PAGE_READONLY;
    } else if(flags == (MemoryRegion::Read | MemoryRegion::Write)) {
      result = PAGE_READWRITE;
    } else if(flags == (MemoryRegion::Read | MemoryRegion::Execute)) {
      result = PAGE_EXECUTE_READ;
    } else if(flags == 0) {
      result = PAGE_NOACCESS;
    } else {
      result = PAGE_EXECUTE_READWRITE;
    }

    return result;
  }

  static int ConvertFromWinFlags(ulong flags) {
    int result = 0;

    // NOTE: We take advantage of fall-trough cases
    switch(flags) {
    case PAGE_NOACCESS:
      result = 0;
      break;
    case PAGE_EXECUTE_READ:
      result |= MemoryRegion::Read;
    case PAGE_EXECUTE:
      result |= MemoryRegion::Execute;
      break;
    case PAGE_EXECUTE_READWRITE:
    case PAGE_EXECUTE_WRITECOPY:
      result = MemoryRegion::ReadWriteExecute;
      break;
    case PAGE_READWRITE:
    case PAGE_WRITECOPY:
      result |= MemoryRegion::Write;
    case PAGE_READONLY:
      result |= MemoryRegion::Read;
      break;
    default: assert(false);
    }

    return result;
  }

  void MemoryRegion::SetFlags(int flags) {
    ulong winFlags = ConvertToWinFlags(flags);

    for(MemoryPage& page : mPages) {
      ulong previousFlags;
      ATOM_WINSERT(VirtualProtect(page.base, page.size, winFlags, &previousFlags));

      page.previousFlags = ConvertFromWinFlags(previousFlags);
      page.currentFlags = flags;
    }
  }

  void MemoryRegion::ResetFlags(bool initial) {
    for(MemoryPage& page : mPages) {
      ulong flags = (initial ? page.initialFlags : page.previousFlags);
      ulong previousFlags;

      ATOM_WINSERT(VirtualProtect(page.base, page.size, ConvertToWinFlags(flags), &previousFlags));

      page.previousFlags = ConvertFromWinFlags(previousFlags);
      page.currentFlags = flags;
    }
  }

  size_t MemoryRegion::GetPageSize() {
    SYSTEM_INFO systemInfo;
    GetSystemInfo(&systemInfo);

    // Retrieve the size of a memory region page
    return systemInfo.dwPageSize;
  }

  void MemoryRegion::UpdateMemoryPages(uintptr_t startPage, size_t pageCount) {
    mPages.clear();

    for(uint i = 0; i < pageCount; i++) {
      MemoryPage page = {};

      page.size = PageSize;
      page.base = reinterpret_cast<void*>(startPage + (PageSize * i));

      MEMORY_BASIC_INFORMATION memoryInformation;
      ATOM_WINSERT(VirtualQuery(page.base, &memoryInformation, sizeof(MEMORY_BASIC_INFORMATION)));

      // We assume that only one page within the region is retrieved
      assert(PageSize == memoryInformation.RegionSize);

      int flags = ConvertFromWinFlags(memoryInformation.Protect);
      page.currentFlags = page.initialFlags = page.previousFlags = flags;

      mPages.push_back(page);
    }
  }
}
