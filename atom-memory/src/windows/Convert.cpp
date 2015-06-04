#include <atom-memory/Memory.hpp>
#include <windows.h>
#include <cassert>

#include "Convert.hpp"

namespace atom {
  unsigned long ConvertToWinFlags(int flags) {
    unsigned long result = 0;

    if(flags == Memory::Read) {
      result = PAGE_READONLY;
    } else if(flags == (Memory::Read | Memory::Write)) {
      result = PAGE_READWRITE;
    } else if(flags == (Memory::Read | Memory::Execute)) {
      result = PAGE_EXECUTE_READ;
    } else if(flags == 0) {
      result = PAGE_NOACCESS;
    } else {
      result = PAGE_EXECUTE_READWRITE;
    }

    return result;
  }

  int ConvertFromWinFlags(unsigned long flags) {
    int result = 0;

    // NOTE: We take advantage of fall-trough cases
    switch(flags) {
    case PAGE_NOACCESS:
      result = 0;
      break;
    case PAGE_EXECUTE_READ:
      result |= Memory::Read;
    case PAGE_EXECUTE:
      result |= Memory::Execute;
      break;
    case PAGE_EXECUTE_READWRITE:
    case PAGE_EXECUTE_WRITECOPY:
      result = Memory::ReadWriteExecute;
      break;
    case PAGE_READWRITE:
    case PAGE_WRITECOPY:
      result |= Memory::Write;
    case PAGE_READONLY:
      result |= Memory::Read;
      break;
    default: assert(false);
    }

    return result;
  }
}
