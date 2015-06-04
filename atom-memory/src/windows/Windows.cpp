#include "Windows.hpp"

namespace atom {
  ulong ConvertToWinFlags(int flags) {
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

  int ConvertFromWinFlags(ulong flags) {
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
}
