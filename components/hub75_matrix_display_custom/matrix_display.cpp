#include "matrix_display.h"
#include "ESP32-HUB75-VirtualMatrixPanel_T.hpp"

#ifndef MATRIX_DISPLAY_VIRTUAL_CHAIN_TYPE
#define MATRIX_DISPLAY_VIRTUAL_CHAIN_TYPE CHAIN_TOP_LEFT_DOWN
#endif

namespace esphome
{
namespace matrix_display
{

static const char *const TAG = "matrix_display";

class VirtualDisplayAdapter
{
 public:
  VirtualDisplayAdapter(int virtual_rows, int virtual_cols, int panel_width,
                        int panel_height)
  {
    this->display_ = new VirtualMatrixPanel_T<MATRIX_DISPLAY_VIRTUAL_CHAIN_TYPE>(
        virtual_rows, virtual_cols, panel_width, panel_height);
  }

  void set_display(MatrixPanel_I2S_DMA &dma_display)
  {
    this->display_->setDisplay(dma_display);
  }

  void draw_pixel(int x, int y, Color color)
  {
    this->display_->drawPixelRGB888(x, y, color.r, color.g, color.b);
  }

  void fill(Color color)
  {
    this->display_->fillScreenRGB888(color.r, color.g, color.b);
  }

 protected:
  VirtualMatrixPanel_T<MATRIX_DISPLAY_VIRTUAL_CHAIN_TYPE> *display_;
};

void MatrixDisplay::setup()
{
  ESP_LOGCONFIG(TAG, "Setting up MatrixDisplay...");

  this->mxconfig_.min_refresh_rate = 1000 / update_interval_;
  this->mxconfig_.double_buff = true;

  this->dma_display_ = new MatrixPanel_I2S_DMA(this->mxconfig_);
  this->dma_display_->begin();
  this->set_brightness(this->initial_brightness_);
  this->dma_display_->clearScreen();

  if (this->is_virtual_enabled_()) {
    this->virtual_display_ = new VirtualDisplayAdapter(
        this->virtual_rows_, this->virtual_cols_, this->mxconfig_.mx_width,
        this->mxconfig_.mx_height);
    this->virtual_display_->set_display(*this->dma_display_);
  }

  set_state(!this->power_switches_.size());
}

void MatrixDisplay::update()
{
  if (this->enabled_) {
    this->do_update_();
  } else {
    this->dma_display_->clearScreen();
  }
  this->dma_display_->flipDMABuffer();
}

void MatrixDisplay::dump_config()
{
  ESP_LOGCONFIG(TAG, "MatrixDisplay:");

  HUB75_I2S_CFG cfg = this->dma_display_->getCfg();

  ESP_LOGCONFIG(TAG, "  Pins: R1:%i, G1:%i, B1:%i, R2:%i, G2:%i, B2:%i",
                cfg.gpio.r1, cfg.gpio.g1, cfg.gpio.b1, cfg.gpio.r2,
                cfg.gpio.g2, cfg.gpio.b2);
  ESP_LOGCONFIG(TAG, "  Pins: A:%i, B:%i, C:%i, D:%i, E:%i", cfg.gpio.a,
                cfg.gpio.b, cfg.gpio.c, cfg.gpio.d, cfg.gpio.e);
  ESP_LOGCONFIG(TAG, "  Pins: LAT:%i, OE:%i, CLK:%i", cfg.gpio.lat,
                cfg.gpio.oe, cfg.gpio.clk);

  ESP_LOGCONFIG(TAG, "  Virtual Rows: %i", this->virtual_rows_);
  ESP_LOGCONFIG(TAG, "  Virtual Cols: %i", this->virtual_cols_);
  ESP_LOGCONFIG(TAG, "  Virtual Enabled: %s",
                TRUEFALSE(this->is_virtual_enabled_()));

  switch (cfg.driver) {
    case HUB75_I2S_CFG::shift_driver::SHIFTREG:
      ESP_LOGCONFIG(TAG, "  Driver: SHIFTREG");
      break;
    case HUB75_I2S_CFG::shift_driver::FM6124:
      ESP_LOGCONFIG(TAG, "  Driver: FM6124");
      break;
    case HUB75_I2S_CFG::shift_driver::FM6126A:
      ESP_LOGCONFIG(TAG, "  Driver: FM6126A");
      break;
    case HUB75_I2S_CFG::shift_driver::ICN2038S:
      ESP_LOGCONFIG(TAG, "  Driver: ICN2038S");
      break;
    case HUB75_I2S_CFG::shift_driver::MBI5124:
      ESP_LOGCONFIG(TAG, "  Driver: MBI5124");
      break;
    case HUB75_I2S_CFG::shift_driver::DP3246:
      ESP_LOGCONFIG(TAG, "  Driver: DP3246");
      break;
  }

  switch (cfg.line_decoder) {
    case HUB75_I2S_CFG::line_driver::TYPE138:
      ESP_LOGCONFIG(TAG, "  Driver: TYPE138");
      break;
    case HUB75_I2S_CFG::line_driver::TYPE595:
      ESP_LOGCONFIG(TAG, "  Driver: TYPE595 | SM5368");
      break;
    case HUB75_I2S_CFG::line_driver::TYPE_DIRECT:
      ESP_LOGCONFIG(TAG, "  Driver: TYPE_DIRECT");
      break;
    case HUB75_I2S_CFG::line_driver::SM5266P:
      ESP_LOGCONFIG(TAG, "  Driver: SM5266P");
      break;
  }

  ESP_LOGCONFIG(TAG, "  I2S Speed: %u MHz", (uint32_t) cfg.i2sspeed / 1000000);
  ESP_LOGCONFIG(TAG, "  Latch Blanking: %i", cfg.latch_blanking);
  ESP_LOGCONFIG(TAG, "  Clock Phase: %s", TRUEFALSE(cfg.clkphase));
  ESP_LOGCONFIG(TAG, "  Min Refresh Rate: %i", cfg.min_refresh_rate);
}

void MatrixDisplay::set_brightness(int brightness)
{
  this->dma_display_->setBrightness8(brightness);
}

void HOT MatrixDisplay::draw_absolute_pixel_internal(int x, int y, Color color)
{
  if (x >= this->get_width_internal() || x < 0 || y >= this->get_height_internal() ||
      y < 0)
    return;

  if (this->virtual_display_ != nullptr) {
    this->virtual_display_->draw_pixel(x, y, color);
    return;
  }

  this->dma_display_->drawPixelRGB888(x, y, color.r, color.g, color.b);
}

void MatrixDisplay::fill(Color color)
{
  if (this->virtual_display_ != nullptr) {
    this->virtual_display_->fill(color);
    return;
  }

  this->dma_display_->fillScreenRGB888(color.r, color.g, color.b);
}

void MatrixDisplay::filled_rectangle(int x1, int y1, int width, int height,
                                     Color color)
{
  if (this->virtual_display_ != nullptr) {
    for (int y = y1; y < y1 + height; y++) {
      for (int x = x1; x < x1 + width; x++) {
        this->draw_absolute_pixel_internal(x, y, color);
      }
    }
    return;
  }

  this->dma_display_->fillRect(x1, y1, width, height, color.r, color.g, color.b);
}

}  // namespace matrix_display
}  // namespace esphome
