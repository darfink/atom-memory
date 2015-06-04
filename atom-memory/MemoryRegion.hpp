#pragma once

#include <atom-ex/Exception.hpp>
#include <atom-memory/Memory.hpp>
#include <functional>
#include <cassert>
#include <vector>

namespace {
  typedef unsigned long ulong;
  typedef unsigned int uint;
}

namespace atom {
  /* Describes a page within the region */
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
    std::vector<MemoryPage> mPages;

  public:
    /* The exception this class throws */
    ATOM_DEFINE_EXCEPTION(Exception);

    /* Constructs a memory region from an address and size
     *
     * @address The address in memory
     * @size The size of the memory
     */
    MemoryRegion(void* address, size_t size);

    /* Executes a function while temporarily changing memory region flags
     *
     * @flags The target permissions for the region
     * @function The callback function to execute
     */
    void ExecuteFunction(int flags, std::function<void()> function);

    /* Sets the protection flags for all pages within the region
     *
     * @flags The target permission for the region
     */
    void SetFlags(int flags);

    /* Resets the memory page flags to their previous condition
     *
     * @initial Reset to the initial memory flags
     */
    void ResetFlags(bool initial);

    /* Gets the number of system pages within the region
     *
     * @return The number of pages
     */
    size_t GetPageCount() const;

    /* Returns a memory page at a specific index
     *
     * @index The index of the memory page
     * @return The memory page
     */
    const MemoryPage& operator[](size_t index) const;

    /* Returns an iterator to the beginning of the array
     *
     * @return The memory page iterator (begin)
     */
    std::vector<MemoryPage>::const_iterator begin() const;

    /* Returns an iterator at the end of the array
     *
     * @return The memory page iterator (end)
     */
    std::vector<MemoryPage>::const_iterator end() const;

  private:
    /* Updates all memory pages to their current permission flags
     *
     * This is used in case an outside factor changed the flags
     * during operations (e.g another code, process or the operating
     * system might have altered the permissions.
     *
     * @startPage The address of the first page
     * @pageCount The number of pages
     */
    void UpdateMemoryPages(uintptr_t startPage, size_t pageCount);
  };

  inline MemoryRegion::MemoryRegion(void* address, size_t size) {
    assert(address != NULL);
    assert(size > 0);

    if(PageSize == 0) {
      PageSize = Memory::GetPageSize();
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
