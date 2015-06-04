#pragma once

namespace atom {
namespace Memory {
  /* Describes the different memory flags */
  enum Flag {
    Execute = (1 << 0),
    Write   = (1 << 1),
    Read    = (1 << 2),
  };

  /* Allocate system memory
   *
   * @size The memory size to allocate in bytes
   * @flags The permissions for the memory
   * @allocated The total size allocated in bytes
   * @return The address of the allocated memory
   */
  void* Allocate(size_t size, int flags, size_t* allocated = nullptr);

  /* Release system memory
   *
   * @address The address of the memory
   * @size The memory size in bytes
   */
  void Free(void* address, size_t size);

  /* Get the operating systems page size
   *
   * @return The page size in bytes
   */
  size_t GetPageSize();

  /* Adjusts an address to an alignment
   *
   * @base The address
   * @alignment The alignment
   * @return The aligned address
   */
  template<typename T>
  inline T AlignTo(T base, T alignment) {
    return (base + (alignment - 1)) & ~(alignment - 1);
  }

  /* A shorthand constant for execute, read and write permissions */
  static const unsigned long ReadWriteExecute = (Read | Write | Execute);
} /* Memory */
} /* atom */
