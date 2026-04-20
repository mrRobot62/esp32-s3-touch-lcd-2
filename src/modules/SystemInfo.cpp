#include "modules/SystemInfo.hpp"

namespace modules {

/**
 * @brief Return the firmware identifier shown in logs and demo screens.
 *
 * @return Constant string literal that identifies the current firmware image.
 */
const char* SystemInfo::firmwareName() { return "esp32-s3-touch-lcd-2-demo"; }

} // namespace modules
