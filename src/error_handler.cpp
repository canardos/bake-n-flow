#include "lp_clock_stm32f1xx.h"
#include "lp_libpekin_stm32_hal.h"
#include "lp_pins_stm32f1xx.h"
#include "serial/lp_uart_stm32f1xx.h"
#include "lp_error.h"
#include "serial/lp_serial_writer.h"
#include "devices/peripherals.h"

static constexpr uint32_t baud_rate = 115200;

using namespace libp;
using namespace libp_stm32;

static uart::UartIo<USART1_BASE> uart_out;
static Error::SetLedFunc my_set_led_func = [](bool on) {
    pins::pcb_led.set(on);
};

static Error error(my_set_led_func, uart_out);

void initErrHndlr()
{
    clk::enable<clk::Apb2::usart1, clk::Apb2::afio>();
    uart_out.start(baud_rate);
}

Error& getErrHndlr()
{
    return error;
}

extern "C"
void hal_error(unsigned char code)
{
    getErrHndlr().halt(code);
}

