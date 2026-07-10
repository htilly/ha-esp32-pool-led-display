#pragma once

#include <utility>

#include "esphome/core/hal.h"
#include "esphome/core/log.h"
#include "esphome/core/component.h"
#include "esphome/components/display/display_buffer.h"

#include "ESP32-HUB75-MatrixPanel-I2S-DMA.h"

namespace esphome
{
namespace matrix_display
{
class MatrixDisplay;
class VirtualDisplayAdapter;

namespace matrix_display_switch
{
class MatrixDisplaySwitch;
static void set_reference(MatrixDisplaySwitch *switch_, MatrixDisplay *display);
}  // namespace matrix_display_switch

namespace matrix_display_brightness
{
class MatrixDisplayBrightness;
static void set_reference(MatrixDisplayBrightness *brightness, MatrixDisplay *display);
}  // namespace matrix_display_brightness

class MatrixDisplay : public display::DisplayBuffer
{
 public:
  void setup() override;
  void dump_config() override;
  void update() override;

  void register_power_switch(matrix_display_switch::MatrixDisplaySwitch *power_switch)
  {
    this->power_switches_.push_back(power_switch);
    set_reference(power_switch, this);
  };

  void register_brightness(matrix_display_brightness::MatrixDisplayBrightness *brightness)
  {
    this->brightness_values_.push_back(brightness);
    set_reference(brightness, this);
  };

  void set_panel_height(int panel_height) { this->mxconfig_.mx_height = panel_height; }
  void set_panel_width(int panel_width) { this->mxconfig_.mx_width = panel_width; }
  void set_chain_length(int chain_length) { this->mxconfig_.chain_length = chain_length; }
  void set_initial_brightness(int brightness) { this->initial_brightness_ = brightness; };
  int get_initial_brightness() { return this->initial_brightness_; }
  void set_virtual_rows(int virtual_rows) { this->virtual_rows_ = virtual_rows; }
  void set_virtual_cols(int virtual_cols) { this->virtual_cols_ = virtual_cols; }

  void set_pins(InternalGPIOPin *R1_pin, InternalGPIOPin *G1_pin,
                InternalGPIOPin *B1_pin, InternalGPIOPin *R2_pin,
                InternalGPIOPin *G2_pin, InternalGPIOPin *B2_pin,
                InternalGPIOPin *A_pin, InternalGPIOPin *B_pin,
                InternalGPIOPin *C_pin, InternalGPIOPin *D_pin,
                InternalGPIOPin *E_pin, InternalGPIOPin *LAT_pin,
                InternalGPIOPin *OE_pin, InternalGPIOPin *CLK_pin)
  {
    this->mxconfig_.gpio = {
        static_cast<int8_t>(R1_pin->get_pin()),
        static_cast<int8_t>(G1_pin->get_pin()),
        static_cast<int8_t>(B1_pin->get_pin()),
        static_cast<int8_t>(R2_pin->get_pin()),
        static_cast<int8_t>(G2_pin->get_pin()),
        static_cast<int8_t>(B2_pin->get_pin()),
        static_cast<int8_t>(A_pin->get_pin()),
        static_cast<int8_t>(B_pin->get_pin()),
        static_cast<int8_t>(C_pin->get_pin()),
        static_cast<int8_t>(D_pin->get_pin()),
        static_cast<int8_t>(E_pin != nullptr ? E_pin->get_pin() : -1),
        static_cast<int8_t>(LAT_pin->get_pin()),
        static_cast<int8_t>(OE_pin->get_pin()),
        static_cast<int8_t>(CLK_pin->get_pin())};
  }

  void set_shift_driver(HUB75_I2S_CFG::shift_driver shift_driver)
  {
    this->mxconfig_.driver = shift_driver;
  };

  void set_line_decoder(HUB75_I2S_CFG::line_driver line_decoder)
  {
    this->mxconfig_.line_decoder = line_decoder;
  };

  void set_i2sspeed(HUB75_I2S_CFG::clk_speed speed) { this->mxconfig_.i2sspeed = speed; };
  void set_latch_blanking(int latch_blanking)
  {
    this->mxconfig_.latch_blanking = latch_blanking;
  };
  void set_clock_phase(bool clock_phase) { this->mxconfig_.clkphase = clock_phase; }

  display::DisplayType get_display_type() override
  {
    return display::DisplayType::DISPLAY_TYPE_COLOR;
  }

  void fill(Color color) override;
  void filled_rectangle(int x1, int y1, int width, int height,
                        Color color = display::COLOR_ON);
  void set_state(bool state) { this->enabled_ = state; }
  void set_brightness(int brightness);

  std::vector<matrix_display_switch::MatrixDisplaySwitch *> get_power_switches()
  {
    return this->power_switches_;
  }

  std::vector<matrix_display_brightness::MatrixDisplayBrightness *>
  get_brightness_values()
  {
    return this->brightness_values_;
  }

 protected:
  MatrixPanel_I2S_DMA *dma_display_ = nullptr;
  VirtualDisplayAdapter *virtual_display_ = nullptr;
  HUB75_I2S_CFG mxconfig_;
  int initial_brightness_ = 128;
  bool enabled_ = false;
  int virtual_rows_ = 1;
  int virtual_cols_ = 1;
  std::vector<matrix_display_switch::MatrixDisplaySwitch *> power_switches_;
  std::vector<matrix_display_brightness::MatrixDisplayBrightness *> brightness_values_;

  bool is_virtual_enabled_() const
  {
    return this->virtual_rows_ > 1 || this->virtual_cols_ > 1;
  }

  int get_width_internal() override
  {
    if (this->is_virtual_enabled_()) {
      return this->mxconfig_.mx_width * this->virtual_cols_;
    }
    return this->mxconfig_.mx_width * this->mxconfig_.chain_length;
  };

  int get_height_internal() override
  {
    if (this->is_virtual_enabled_()) {
      return this->mxconfig_.mx_height * this->virtual_rows_;
    }
    return this->mxconfig_.mx_height;
  };

  void draw_absolute_pixel_internal(int x, int y, Color color) override;
};

}  // namespace matrix_display
}  // namespace esphome
