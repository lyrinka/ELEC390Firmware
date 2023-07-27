#ifndef CRITREGION_H__
#define CRITREGION_H__

#include <stm32g071xx.h>

#define critEnter() unsigned int __X_IE = __get_PRIMASK(); __disable_irq()
#define critExit() __set_PRIMASK(__X_IE)

#endif
