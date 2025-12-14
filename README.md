# Pool LED Display - ESP32 HUB75 Matrix Display

An ESP32-based LED matrix display system for monitoring pool metrics, weather, Spotify playback, and more. Built with ESPHome and designed to integrate seamlessly with Home Assistant.

## Overview

This project creates a dual-panel HUB75 LED matrix display (128x32 pixels total) that shows:
- **Pool metrics**: Temperature, pH, ORP, pump RPM, power consumption, people count
- **Weather information**: Current conditions and temperature
- **Spotify integration**: Currently playing artist and track
- **Custom messages**: User-defined text display
- **Stopwatch**: Timer functionality
- **Debug information**: System status and diagnostics

## Hardware Requirements

### Required Components
- **ESP32 development board** (ESP32-DevKitC or similar)
- **HUB75 LED matrix panels** (2x 64x32 RGB panels)
- **ICN2038S driver** compatible panels
- **BME280 sensor** (for internal temperature, pressure, humidity monitoring)
- **Power supply** (adequate for LED panels - typically 5V, 10A+)
- **Level shifters** (if needed for 3.3V to 5V conversion)

### Bill of Materials (BOM)

- [HUB75 LED Matrix Panel](https://www.aliexpress.com/item/1005007071422965.html) - AliExpress link for LED matrix panels
- [RGB Matrix Panel Drive Interface Board for ESP32 DMA](https://www.electrodragon.com/product/rgb-matrix-panel-drive-interface-board-for-esp32-dma/) - Interface board for connecting ESP32 to HUB75 panels

### Pin Connections

The display uses the following GPIO pins:
- **RGB Data**: R1=25, G1=26, B1=27, R2=14, G2=12, B2=13
- **Address**: A=23, B=19, C=5, D=17
- **Control**: LAT=4, OE=15, CLK=16
- **I2C**: SDA=21, SCL=22 (for BME280)

## Software Requirements

- **ESPHome** (latest version)
- **Home Assistant** (with ESPHome integration)
- **External Component**: [ESPHome-HUB75-MatrixDisplayWrapper](https://github.com/TillFleisch/ESPHome-HUB75-MatrixDisplayWrapper)

## Installation

### 1. ESPHome Setup

1. Install ESPHome (via Home Assistant add-on or standalone)
2. Create a new ESPHome device configuration
3. Copy the contents of `pool-led-display.yaml` to your ESPHome configuration

### 2. Secrets Configuration

Create a `secrets.yaml` file in your ESPHome configuration directory with:

```yaml
wifi_ssid: "Your_WiFi_SSID"
wifi_password: "Your_WiFi_Password"
api_pool_led-display: "Your_API_Encryption_Key"
ota_pool_led-display: "Your_OTA_Password"
```

### 3. Required Files

Ensure you have the following files in your ESPHome configuration directory:

**Fonts** (in `fonts/` directory):
- `spleen-5x8.bdf`
- `Roboto-Regular.ttf`
- `tom-thumb.bdf`
- `6_10.bdf`

**Images** (in `images/weather/` directory):
- `sunny.png`
- `partlycloudy.png`
- `cloudy.png`
- `rain.png`
- `snow.png`
- `tstorms.png`
- `fog.png`

**Includes** (in `includes/common/` directory):
- `wifi_diagnostics.yaml`

### 4. Home Assistant Integration

The display requires the following Home Assistant entities:

#### Sensors
- `sensor.pool_switch_electric_consumption_w` - Pool heater power consumption
- `sensor.current_pool_session_duration` - Active pool session duration
- `sensor.bestway_ph` - Pool pH level
- `sensor.bestway_orp` - Pool ORP level
- `sensor.bestway_temperature` - Pool water temperature
- `sensor.pool_pump_current_pool_pump_rpm` - Pool pump RPM
- `sensor.pooldetect_person_active_count` - Number of people in pool
- `sensor.pool_session_start` - Pool session start time
- `weather.smhi_home` - Weather entity

#### Inputs
- `input_number.pool_session_max_people` - Maximum people in pool session
- `input_text.pool_led_custom_message` - Custom message text
- `input_boolean.pool_led_display_spotify` - Enable/disable Spotify page
- `input_boolean.pool_led_display_custom_message` - Enable/disable custom message page
- `input_boolean.pool_led_display_weather` - Enable/disable weather page
- `input_boolean.pool_led_display_stats` - Enable/disable stats page
- `input_boolean.pool_led_display_stop_timer` - Enable/disable stop timer page
- `input_boolean.pool_led_display_debug` - Enable/disable debug page
- `input_boolean.pool_led_timer_running` - Stop timer running state

#### Other Entities
- `media_player.spotify_htilly` - Spotify media player
- `sensor.pool_heater_hvac_mode` - Pool heater mode

## Display Layout

The display is divided into two sections:

### Left Panel (0-63px) - Always Visible
- **Top Row**: pH/ORP reading with color-coded bar (alternates every 15 seconds)
- **Top Right**: Current time (HH:MM)
- **Middle**: Pool pump RPM bar
- **Center**: Pool temperature (large, color-coded)
- **Bottom**: Power consumption bar (red)
- **Bottom Left**: Number of people in pool
- **Bottom Right**: Heater icon (when active)

### Right Panel (65-127px) - Page-Based Content

The display automatically cycles through enabled pages every 30 seconds:

#### Page 0: Spotify
- Scrolling artist name (top)
- Scrolling track title (bottom, cyan color)
- Automatically scrolls if text is too long

#### Page 1: Custom Message
- User-defined custom text
- Auto-scrolling for long messages
- Centered if short enough

#### Page 2: Weather
- Weather condition icon (sunny, cloudy, rain, etc.)
- Outdoor temperature

#### Page 3: Pool Statistics
- People currently in pool
- Maximum people for session
- Current session duration (minutes)

#### Page 4: Stop Timer
- Running stopwatch display (MM:SS.T)
- Status indicator (Running/Stopped)
- Automatically switches to this page when timer starts

#### Page 5: Debug
- ESP32 internal temperature
- WiFi signal strength (dBm)
- System uptime (minutes)

## Features

### Color-Coded Metrics
- **pH Levels**: 
  - Red: 6.0-6.6 or 8.4-9.0 (unsafe)
  - Orange: 6.6-7.2 or 7.6-8.4 (acceptable)
  - Green: 7.2-7.6 (optimal)
  
- **ORP Levels**:
  - Red: 300-400 or 900-1000 (unsafe)
  - Orange: 400-550 or 650-900 (acceptable)
  - Green: 550-650 (optimal)

- **Temperature**:
  - Blue: < 22°C (cold)
  - Yellow: 22-27°C (comfortable)
  - Red: > 27°C (warm)

### Scrolling Text
Long text automatically scrolls horizontally with a pause at the beginning. Text repeats seamlessly for continuous display.

### Automatic Page Rotation
Pages automatically rotate every 30 seconds, showing only enabled pages. The stop timer page can be manually triggered and takes priority.

## Configuration

### Display Settings
- **Update Interval**: 0.1s (10 FPS)
- **I2S Speed**: 15 MHz
- **Clock Phase**: false
- **Brightness**: Adjustable via Home Assistant number entity

### Time Zone
Currently set to `Europe/Stockholm`. Change in the `time` section if needed.

### BME280 Address
Default I2C address is `0x76`. Change to `0x77` if your sensor uses that address.

## Usage

1. **Enable/Disable Pages**: Use the Home Assistant input booleans to control which pages are displayed
2. **Custom Messages**: Set text in `input_text.pool_led_custom_message`
3. **Stop Timer**: Toggle `input_boolean.pool_led_timer_running` to start/stop the timer
4. **Brightness Control**: Adjust via the "Pool LED Brightness" number entity in Home Assistant
5. **Power Control**: Use the "Pool LED Power" switch to turn display on/off

## Troubleshooting

### Display Not Showing
- Check power supply (LED panels require significant current)
- Verify all pin connections
- Check that the display switch is ON in Home Assistant

### Missing Data
- Ensure all required Home Assistant entities exist
- Check entity IDs match your Home Assistant setup
- Verify sensors are reporting data

### WiFi Issues
- Check WiFi credentials in `secrets.yaml`
- Review WiFi diagnostics (if included)
- Check signal strength in debug page

### I2C Sensor Not Found
- Verify BME280 connections (SDA/SCL)
- Check I2C address (try 0x77 if 0x76 doesn't work)
- Ensure proper power to sensor

## License

This project is for personal use. Please respect the licenses of:
- ESPHome (Apache 2.0)
- External components used
- Fonts and images

## Credits

- **ESPHome**: Framework for ESP32 device management
- **HUB75 Matrix Display Wrapper**: [TillFleisch/ESPHome-HUB75-MatrixDisplayWrapper](https://github.com/TillFleisch/ESPHome-HUB75-MatrixDisplayWrapper)
- Weather icons and fonts from various open-source projects

## Contributing

This is a personal project, but suggestions and improvements are welcome!

## Support

For issues related to:
- **ESPHome**: Check [ESPHome documentation](https://esphome.io/)
- **Hardware**: Verify connections and power supply
- **Home Assistant**: Ensure all entities are properly configured

