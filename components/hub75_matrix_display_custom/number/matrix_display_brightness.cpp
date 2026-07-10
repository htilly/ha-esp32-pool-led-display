#include "matrix_display_brightness.h"

namespace esphome::matrix_display::matrix_display_brightness
{
void MatrixDisplayBrightness::control(float value)
{
  display_->set_brightness((int) value);
  for (MatrixDisplayBrightness *brightness_value :
       display_->get_brightness_values()) {
    brightness_value->publish_state(value);
  }
}

void MatrixDisplayBrightness::dump_config()
{
  ESP_LOGCONFIG(TAG, "MatrixDisplayBrightness");
}
}  // namespace esphome::matrix_display::matrix_display_brightness
