#pragma once

#include <mutex>

#define SCOPE_LOCK(InLock) std::lock_guard<std::mutex> _(InLock);