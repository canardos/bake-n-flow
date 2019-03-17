#include "error.h"
#include "serial/uart_stm32f1xx.h"
#include "serial/serial_writer.h"
#include "clock_stm32f1xx.h"
#include "pins_stm32f1xx.h"
#include "devices/peripherals.h"
#include "libpekin_stm32_hal.h"

static constexpr uint32_t baud_rate = 115200;

using namespace Libp;
using namespace LibpStm32;

static Uart::UartIo<USART1_BASE> uart_out;
static Error::SetLedFunc my_set_led_func = [](bool on) {
    Pins::pcb_led.set(on);
};

static Error error(my_set_led_func, uart_out);

void initErrHndlr()
{
    Clk::enable<Clk::Apb2::usart1, Clk::Apb2::afio>();
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

