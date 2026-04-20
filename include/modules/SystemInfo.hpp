#pragma once

namespace modules {

/**
 * @brief Small helper for firmware metadata exposed to higher-level modules.
 *
 * Keeping static identity strings behind a dedicated module avoids scattering
 * literals through the codebase and makes later build-time customization easier.
 */
class SystemInfo {
  public:
    /**
     * @brief Return the human-readable firmware name.
     *
     * @return Constant string literal used for logs and on-screen status messages.
     */
    static const char* firmwareName();
};

} // namespace modules
