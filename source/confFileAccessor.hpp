#include <ch.h>
#include <hal.h>
#include "confFile.hpp"
#include "threadAndEventDecl.hpp"
#include "frozenDictionary.hpp"


int32_t getDiffPressPeriod(void) {
  return CH_CFG_ST_FREQUENCY /
    std::get<int>(ConfigurationFile_AT(confFile, "thread.frequency.diff_press"));
}
