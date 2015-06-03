#pragma once

#include <atom-ex/Exception.hpp>
#include <functional>
#include <cassert>
#include <vector>

namespace {
  typedef unsigned long ulong;
  typedef unsigned int uint;
}

namespace atom {
  /// <summary>
  /// Describes a page within the region
  /// </summary>
  struct MemoryPage {
    size_t size;
    void* base;
    int currentFlags;
    int previousFlags;
    int initialFlags;
  };

  class MemoryRegion {
  private:
    // Private members
    static size_t PageSize;
    std::vector<MemoryPage> mPages;

  public:
    /// <summary>
    /// Describes the different memory flags
    /// </summary>
    enum Flag {
      Execute = (1 << 0),
      Write   = (1 << 1),
      Read    = (1 << 2),
    };

    // A shorthand constant for execute, read and write permissions
    static const ulong ReadWriteExecute;

  public:
    /// <summary>
    /// The exception this class throws
    /// </summary>
    ATOM_DEFINE_EXCEPTION(Exception);

    /// <summary>
    /// Constructs a memory region from an address and size
    /// </summary>
    MemoryRegion(void* address, size_t size);

    /// <summary>
    /// Executes a function while temporarily changing memory region flags
    /// </summary>
    void ExecuteFunction(int flags, std::function<void()> function);

    /// <summary>
    /// Sets the protection flags for all pages within the region
    /// </summary>
    void SetFlags(int flags);

    /// <summary>
    /// Resets the memory page flags to their initial condition
    /// </summary>
    void ResetFlags(bool initial);

    /// <summary>
    /// Gets the number of pages within the region
    /// </summary>
    size_t GetPageCount() const;

    /// <summary>
    /// Returns a memory page at a specific index
    /// </summary>
    const MemoryPage& operator[](size_t index) const;

    /// <summary>
    /// Returns an iterator to the beginning of the array
    /// </summary>
    std::vector<MemoryPage>::const_iterator begin() const;

    /// <summary>
    /// Returns an iterator at the end of the array
    /// </summary>
    std::vector<MemoryPage>::const_iterator end() const;

  private:
    /// <summary>
    /// Gets the page size of the operating system
    /// </summary>
    static size_t GetPageSize();

    /// <summary>
    /// Updates all memory pages to their current permission flags
    /// </summary>
    void UpdateMemoryPages(uintptr_t startPage, size_t pageCount);
  };

  inline MemoryRegion::MemoryRegion(void* address, size_t size) {
    assert(address != NULL);
    assert(size > 0);

    if(PageSize == 0) {
      PageSize = this->GetPageSize();
    }

    uintptr_t startPage = (reinterpret_cast<uintptr_t>(address) & ~(PageSize - 1));
    uintptr_t lastPage = ((reinterpret_cast<uintptr_t>(address) + size) & ~(PageSize - 1));

    size_t pageCount = ((lastPage - startPage) / PageSize) + 1;
    mPages.reserve(pageCount);

    // The rest is OS specific, so we leave it here
    this->UpdateMemoryPages(startPage, pageCount);
  }

  inline void MemoryRegion::ExecuteFunction(int flags, std::function<void()> function) {
    this->SetFlags(flags);
    function();
    this->ResetFlags(false);
  }

  inline size_t MemoryRegion::GetPageCount() const {
    return mPages.size();
  }

  inline const MemoryPage& MemoryRegion::operator[](size_t index) const {
    return mPages[index];
  }

  inline std::vector<MemoryPage>::const_iterator MemoryRegion::begin() const {
    return mPages.cbegin();
  }

  inline std::vector<MemoryPage>::const_iterator MemoryRegion::end() const {
    return mPages.cend();
  }
}
