#pragma once

#include_next <mcuconf.h>

/* Enable I2C1 peripheral for RP2040 */
#undef RP_I2C_USE_I2C1
#define RP_I2C_USE_I2C1 TRUE
