#pragma once

#include "esphome/core/component.h"
#include "esphome/components/number/number.h"
#include "../matrix_display.h"

namespace esphome::matrix_display::matrix_display_brightness
{
static const char *TAG = "matrix_display.number";

class MatrixDisplayBrightness : public number::Number, public Component
{
 public:
  void setup()
  {
    publish_state(display_->get_initial_brightness());
  }

  void control(float value);
  void dump_config() override;
  void set_display(MatrixDisplay *display) { this->display_ = display; }

 protected:
  MatrixDisplay *display_;
};

static void set_reference(MatrixDisplayBrightness *brightness,
                          MatrixDisplay *display)
{
  brightness->set_display(display);
}

static void publish_state(MatrixDisplayBrightness *brightness, int value)
{
  brightness->publish_state(value);
}
}  // namespace esphome::matrix_display::matrix_display_brightness

