#include "flash.h"


void Flashing_code(uint32_t address,uint32_t *code_arr_word, uint32_t arr_size_bytes)
 {
     uint32_t temp_address = address;
     int32_t temp_arr_size_bytes =arr_size_bytes;
     do
     {
         FlashErase(temp_address);
         temp_arr_size_bytes -= 1024;
         temp_address += 1024;
     } while(temp_arr_size_bytes > 1024);

     FlashProgram(code_arr_word, address, arr_size_bytes);
 }
