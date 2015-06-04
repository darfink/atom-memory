#pragma once

namespace atom {
  /* Converts Memory::Flag to windows flags
   *
   * @flags Bitwise combination for memory options
   */
  unsigned long ConvertToWinFlags(int flags);

  /* Converts windows flags to Memory::Flag
   *
   * @flags Window flags for memory options
   */
  int ConvertFromWinFlags(unsigned long flags);
}
