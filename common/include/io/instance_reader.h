#pragma once

#include "instance.h"

#include <string>

namespace InstanceReader {

Instance loadFromCsv(const std::string& file_path);

}  // namespace InstanceReader
