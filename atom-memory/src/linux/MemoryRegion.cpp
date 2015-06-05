#include <atom-memory/MemoryRegion.hpp>
#include <atom-ex/SystemException.hpp>
#include <sys/mman.h>
#include <unistd.h>
#include <string>
#include <fstream>
#include <regex>
#include <map>

namespace atom {
  void MemoryRegion::SetFlags(int flags) {
    for(MemoryPage& page : mPages) {
      page.previousFlags = page.currentFlags;

      ATOM_SYSTEM_ASSERT(mprotect(page.base, page.size, flags));
      page.currentFlags = flags;
    }
  }

  void MemoryRegion::ResetFlags(bool initial) {
    for(MemoryPage& page : mPages) {
      unsigned long flags = (initial ? page.initialFlags : page.previousFlags);
      page.previousFlags = page.currentFlags;

      ATOM_SYSTEM_ASSERT(mprotect(page.base, page.size, flags));
      page.currentFlags = flags;
    }
  }

  void MemoryRegion::UpdateMemoryPages(uintptr_t startPage, size_t pageCount) {
    mPages.clear();

    std::ifstream fmaps("/proc/self/maps");
    std::regex regex("^([0-9a-fA-F]+)-[0-9a-fA-F]+ (\\w|-){4}");
    std::string input;

    // TODO: support pageCount = 0
    assert(pageCount > 0);

    for(size_t i = 0; i < pageCount; i++) {
      MemoryPage page = {};

      page.size = Memory::GetPageSize();
      page.base = reinterpret_cast<void*>(startPage + (page.size * i));
      page.committed = true;
      page.guarded = false;

      while(std::getline(fmaps, input)) {
        // Create our regex iterator so we can check each of our group
        std::sregex_token_iterator it(input.begin(), input.end(), regex, 1);

        std::string address = "0x";
        address += *it;

        // The addresses are in hex, and therefore we must add '0x'
        // otherwise 'stoul' won't recognize the string as hex
        if(std::stoul(address, 0, 16) != reinterpret_cast<unsigned long>(page.base)) {
          continue;
        }

        std::string permissions = *(++it);

        static const std::map<char, unsigned long> Keys = {
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
