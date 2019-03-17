#ifndef SRC_APP_H_
#define SRC_APP_H_

#include <cstdint>
#include "libpekin.h"
#include "error_handler.h"
#include "display/ili9481_cmd_defs.h"

namespace App {
    inline constexpr uint16_t ui_width = Libp::Ili9481::device_height;
    inline constexpr uint16_t ui_height = Libp::Ili9481::device_width;
}

/// Under the hood, everything is Celsius
enum class TempUnit : uint8_t {
    celsius, fahrenheit
};


#endif /* SRC_APP_H_ */
