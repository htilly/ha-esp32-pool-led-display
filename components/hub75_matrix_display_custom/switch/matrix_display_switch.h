#pragma once

#include "esphome/core/component.h"
#include "esphome/components/switch/switch.h"
#include "../matrix_display.h"

namespace esphome::matrix_display::matrix_display_switch
{
static const char *TAG = "matrix_display.switch";

class MatrixDisplaySwitch : public switch_::Switch, public Component
{
 public:
  void write_state(bool state);
  void setup() override;
  void dump_config() override;

  void set_display(MatrixDisplay *display) { this->display_ = display; }

 protected:
  MatrixDisplay *display_;
};

static void set_reference(MatrixDisplaySwitch *switch_, MatrixDisplay *display)
{
  switch_->set_display(display);
}
}  // namespace esphome::matrix_display::matrix_display_switch

