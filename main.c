#include <stdint.h>
#include <stdbool.h>
#include "flash.h"
#include "driverlib/sysctl.h"
#include "HAL/CAN_SLAVE.h"
#define NVIC_VTABLE_R           (*((volatile uint32_t *)0xE000ED08))


#define BANK1_ADDR              0x00019000
#define ACTIVE_BANK_FLAG_ADDR   0x00016800
#define UNINIT_FLASH_VALUE      0xFFFFFFFF
#define APP_HEX_ARRAY_SIZE      1000
#define DELAY_AFTER_RST_RQST_ms 20
#define BANK2_ADDR              0x00032000

void Flashing_code(uint32_t address,uint32_t *code_arr_word, uint32_t arr_size_bytes);
void byteToWordArray(const uint8* arr);


#define CODE_SIZE_IN_BYTES   10*1024

uint32_t g_app_hex_arr[CODE_SIZE_IN_BYTES/4]={0};


uint8_t g_reset_flag = 0;
uint8_t g_flashing_request_flag = 0;
uint32_t g_active_bank_flag =0;
uint8_t g_done_flag = 0;

//uint32_t g_app_hex_arr[APP_HEX_ARRAY_SIZE];


///*BL CAN interrupt handler when recieving data from CAN bus*/
//void CAN_rx_handler_BL(void)
//{
//    if(msg_id == RST_ID)
//    {
//    SysCtlReset(); /*reset peripherals*/
//
//    /*call reset handler of BL*/
//    __asm(" MOV R0,#0x00000000 \n");  /* Address of the start of the IVT */
//    __asm(" LDR R0, [R0, #4]");
//    __asm(" BX R0\n");               /* Branch to the reset handler*/
//    }
//    /*delay between received frames, not delay here*/
//    else if(msg_id == FLASH_REQ_ID)
//    {
//        /*send flashing request --> flashing_request = 1*/
//    }
//    else if(msg_id == DATA_ID)
//    {
//        /*then recieve the data frames in a global array*/
//    }
//    else if(msg_id == DONE_ID)
//    {
//        /* done_flag =1*/
//    }
//}
//
//
///*BL CAN interrupt handler when recieving data from CAN bus*/
//void CAN_handler_app1(void)
//{
//    SysCtlReset(); /*reset peripherals*/
//
//    /*call reset handler of BL*/
//    __asm(" MOV R0,#0x00000000 \n");  /* Address of the start of the IVT */
//    __asm(" LDR R0, [R0, #4]");
//    __asm(" BX R0\n");               /* Branch to the reset handler*/
//}
//
//void CAN_handler_app2(void)
//{
//    SysCtlReset(); /*reset peripherals*/
//
//    /*call reset handler of BL*/
//    __asm(" MOV R0,#0x00000000 \n");  /* Address of the start of the IVT */
//    __asm(" LDR R0, [R0, #4]");
//    __asm(" BX R0\n");               /* Branch to the reset handler*/
//}

int main(void)
{
    uint32 flashing_rqst_timeOut=0;
    SlaveCANInit();
    //    SlaveCANMessage();

    /*restoring active_bank_flag from flash address 0d92160 (90 kilo byte)*/
    g_active_bank_flag = *((uint32_t*)ACTIVE_BANK_FLAG_ADDR);

    if (g_active_bank_flag == UNINIT_FLASH_VALUE) /*first time ever to enter the bootloader*/
    {
        g_active_bank_flag = 0;
        /*store in flash*/
        FlashErase(ACTIVE_BANK_FLAG_ADDR);
        FlashProgram(&g_active_bank_flag, ACTIVE_BANK_FLAG_ADDR, sizeof(g_active_bank_flag));
    }
    else
    {
        /*for MISRA*/
    }

    if(g_active_bank_flag == 0)
    {

//        while(g_reset_flag == 0){}//wait for flashing req that comes from an interrupt of the BL CAN
//        SlaveCANMessage();
//        g_reset_flag = 0;

        while(g_flashing_request_flag == 0){}//wait for flashing req that comes from an interrupt of the BL CAN

        while(g_done_flag==0){
            SlaveCANMessage();
        }
        g_flashing_request_flag =0;
        //when flash request comes, increment the active_bank_flag and store it in flash --> active_bank_flag =1
        g_active_bank_flag++;
        FlashErase(ACTIVE_BANK_FLAG_ADDR);
        FlashProgram(&g_active_bank_flag, ACTIVE_BANK_FLAG_ADDR, sizeof(g_active_bank_flag));
        //flash at address 100kb (bank1)
        byteToWordArray(rx_msg_data);
        Flashing_code(BANK1_ADDR, g_app_hex_arr, sizeof(g_app_hex_arr));
        //and run app1
        /*call reset handler of BL*/

        NVIC_VTABLE_R |= 0x00019000;
        //        HWREG |= 0x00019000;//////////////////////////////

        __asm(" MOV R0,#0x00019000\n");  /* Address of the start of app1 */
        __asm(" LDR R0, [R0, #4]");
        __asm(" BX R0\n");               /* Branch to the reset handler*/
    }

    if(g_active_bank_flag ==1)
    {
        /*delay here, to account for delay between reset request frames and flash request frame*/
//        SysCtlDelay(DELAY_AFTER_RST_RQST_ms * 16000 / 3);

      while( flashing_rqst_timeOut<=60000000)
      {
        if(g_flashing_request_flag == 1 )
        {
            //flash at address 150kb (bank2)
            while(g_done_flag==0){
                SlaveCANMessage();
            }
            //increment the active_bank_flag and store it in flash --> active_bank_flag =2
            g_active_bank_flag++;
            FlashErase(ACTIVE_BANK_FLAG_ADDR);
            FlashProgram(&g_active_bank_flag, ACTIVE_BANK_FLAG_ADDR, sizeof(g_active_bank_flag));


            //flash at address 100kb (bank1)
            byteToWordArray(rx_msg_data);
            Flashing_code(BANK2_ADDR, g_app_hex_arr, sizeof(g_app_hex_arr));
            //and run app2
            /*call reset handler of APP 2*/

            NVIC_VTABLE_R |= 0x00032000;

            __asm(" MOV R0,#0x00032000\n");  /* Address of the start of app2 */
            __asm(" LDR R0, [R0, #4]");
            __asm(" BX R0\n");               /* Branch to the reset handler*/
        }
        flashing_rqst_timeOut++;
      }
      flashing_rqst_timeOut=0; //e7tyaty
            /*run app1*/
            /*call reset handler of app1*/
            __asm(" MOV R0,#0x00019000\n");  /* Address of the start of app1 */
            __asm(" LDR R0, [R0, #4]");
            __asm(" BX R0\n");

    }


    if(g_active_bank_flag ==2)
    {
        while( flashing_rqst_timeOut<=60000000)
        {
            if(g_flashing_request_flag == 1)
            {
                while(g_done_flag==0){
                    SlaveCANMessage();
                }
                //increment the active_bank_flag and store it in flash --> active_bank_flag =1
                g_active_bank_flag=1;
                FlashErase(ACTIVE_BANK_FLAG_ADDR);
                FlashProgram(&g_active_bank_flag, ACTIVE_BANK_FLAG_ADDR, sizeof(g_active_bank_flag));

                byteToWordArray(rx_msg_data);
                //flash at address 100kb (bank1)
                Flashing_code(BANK1_ADDR, g_app_hex_arr, sizeof(g_app_hex_arr));
                //and run app1
                NVIC_VTABLE_R |= 0x00019000;
                /*call reset handler of APP 1*/
                __asm(" MOV R0,#0x00019000\n");  /* Address of the start of app1 */
                __asm(" LDR R0, [R0, #4]");
                __asm(" BX R0\n");               /* Branch to the reset handler*/

            }
            flashing_rqst_timeOut++;
        }
        flashing_rqst_timeOut=0;
            //run app2
            /*call reset handler of app2*/
            __asm(" MOV R0,#0x00032000\n");  /* Address of the start of app1 */
            __asm(" LDR R0, [R0, #4]");
            __asm(" BX R0\n");

    }

}

void byteToWordArray(const uint8* arr)
{
    uint32 i=0;uint8 j=0;
    uint32 index_word_array=0;
    for (i=0;i<CODE_SIZE_IN_BYTES;i+=4)
    {
        for (j=0;j<4;j++)
        {
            if(i+j>=CODE_SIZE_IN_BYTES)
            {
                //                hex_array_word[index_word_array]&=~(1<<(j*8));
                break;
            }
            else
            {
                g_app_hex_arr[index_word_array]|=(arr[i+j]<<(j*8));
            }

        }
        index_word_array++;
        if(i+j>CODE_SIZE_IN_BYTES)
        {
            break;
        }
    }
}
