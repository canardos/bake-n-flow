#ifndef SRC_ERROR_HANDLER_H_
#define SRC_ERROR_HANDLER_H_

#include <error.h>

/**
 * Initialize the error USART output.
 */
void initErrHndlr();

/**
 * Return a reference to the global error handling object.
 *
 * @p initErrHndlr must be called prior to using the returned object.
 *
 * @return
 */
Libp::Error& getErrHndlr();


#ifdef  USE_FULL_ASSERT
/// called by STM32 assert_param macro
inline
void assert_failed(uint8_t* file, uint32_t line) {
    getErrHndlr().halt(Libp::ErrCode::assert_fail, "Assert failed in '%s' line %d", (const char*)file, (int)line);
}
#endif /* USE_FULL_ASSERT */


#endif /* SRC_ERROR_HANDLER_H_ */
