#include <AtomMemoryRegion/MemoryRegion.hpp>
#include <sys/mman.h>
#include <unistd.h>
#include <string>
#include <fstream>
#include <regex>
#include <map>

namespace atom {
  size_t MemoryRegion::PageSize;

  const ulong MemoryRegion::ReadWriteExecute =
    MemoryRegion::Execute | MemoryRegion::Write | MemoryRegion::Read;

  void MemoryRegion::SetFlags(int flags) {
    for(MemoryPage& page : mPages) {
      page.previousFlags = page.currentFlags;

      if(!mprotect(page.base, page.size, flags)) {
        throw Exception(ATOM_EXCEPTION_INFO, "Couldn't update memory protection flags");
      }

      page.currentFlags = flags;
    }
  }

  void MemoryRegion::ResetFlags(bool initial) {
    for(MemoryPage& page : mPages) {
      ulong flags = (initial ? page.initialFlags : page.previousFlags);
      page.previousFlags = page.currentFlags;

      if(!mprotect(page.base, page.size, flags)) {
        throw Exception(ATOM_EXCEPTION_INFO, "Couldn't update memory protection flags");
      }

      page.currentFlags = flags;
    }
  }

  size_t MemoryRegion::GetPageSize() {
    int pageSize = sysconf(_SC_PAGESIZE);

    if(pageSize == -1) {
      throw Exception(ATOM_EXCEPTION_INFO, "Couldn't retrieve OS page size");
    }

    return static_cast<size_t>(pageSize);
  }

  void MemoryRegion::UpdateMemoryPages(uintptr_t startPage, size_t pageCount) {
    mPages.clear();

    std::ifstream fmaps("/proc/self/maps");
    std::regex regex("^([0-9a-fA-F]+)-[0-9a-fA-F]+ (\\w|-){4}");
    std::string input;

    for(uint i = 0; i < pageCount; i++) {
      MemoryPage page = {};

      page.size = PageSize;
      page.base = reinterpret_cast<void*>(startPage + (PageSize * i));

      while(std::getline(fmaps, input)) {
        // Create our regex iterator so we can check each of our group
        std::sregex_token_iterator it(input.begin(), input.end(), regex, 1);

        std::string address = "0x";
        address += *it;

        // The addresses are in hex, and therefore we must add '0x'
        // otherwise 'stoul' won't recognize the string as hex
        if(std::stoul(address, 0, 16) != reinterpret_cast<ulong>(page.base)) {
          continue;
        }

        std::string permissions = *(++it);

        static const std::map<char, ulong> Keys = {
          { 'r', Read },
          { 'w', Write },
          { 'x', Execute },
        };

        int index = 0;

        for(auto& pair : Keys) {
          if(permissions[index++] == pair.first) {
            page.initialFlags |= pair.second;
          }
        }
      }

      // We haven't changed any flags yet
      page.currentFlags = page.previousFlags = page.initialFlags;
      mPages.push_back(page);
    }
  }
}
