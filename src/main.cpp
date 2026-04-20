#include "app/Application.hpp"

namespace {

/** @brief Single application instance used by the Arduino runtime callbacks. */
app::Application application;

} // namespace

/**
 * @brief Arduino setup hook that delegates startup to the application object.
 */
void setup() { application.setup(); }

/**
 * @brief Arduino loop hook that delegates recurring work to the application object.
 */
void loop() { application.loop(); }
