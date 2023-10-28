/*
 * flash.h
 *
 *  Created on: Sep 24, 2023
 *      Author: Lenovo
 */

#ifndef FLASH_H_
#define FLASH_H_

#include <stdint.h>
#include <stdbool.h>
#include "driverlib/flash.h"

void Flashing_code(uint32_t address,uint32_t *code_arr_word, uint32_t arr_size_bytes);

#endif /* FLASH_H_ */
