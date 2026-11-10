/* Experimental opt-in Zork status extension, not an upstream allocation.
 * SPDX-License-Identifier: Apache-2.0
 */
#include <FreeRTOS.h>
#include <task.h>
#include <string.h>
#include "companion_status.h"
#ifdef CONFIG_COMPANION_STATUS
static TaskHandle_t reporter;
static uint8_t role,stage,flags=4; /* transport initialized; role unknown */
static uint16_t diagnostic;
static uint32_t keyboards,sequence;
static void changed(void) { if(reporter)xTaskNotifyGive(reporter); }
void companion_status_controller_result(int result) {
  taskENTER_CRITICAL();
  role=result==0?1:0;stage=result==0?1:2;
  flags=result==0?7:4;diagnostic=result==0?0:1;
  keyboards=0;
  taskEXIT_CRITICAL();
  changed();
}
void companion_status_stopping(void) {
  taskENTER_CRITICAL();stage=3;flags&=5;keyboards=0;taskEXIT_CRITICAL();
  changed();
}
void companion_status_keyboard(unsigned index, bool present) {
  if(index>=32)return;
  taskENTER_CRITICAL();
  if(present&&stage==1)keyboards|=1UL<<index;
  else keyboards&=~(1UL<<index);
  taskEXIT_CRITICAL();
  changed();
}
static uint16_t crc16(const uint8_t *data,unsigned length) {
  uint16_t crc=0xffff;
  while(length--) {
    crc^=(uint16_t)*data++<<8;
    for(unsigned bit=0;bit<8;bit++)crc=(crc&0x8000)?(uint16_t)((crc<<1)^0x1021):(uint16_t)(crc<<1);
  }
  return crc;
}
static void le32(uint8_t *data,uint32_t value) {
  for(unsigned i=0;i<4;i++)data[i]=(uint8_t)(value>>(i*8));
}
static void report_task(void *argument) {
  (void)argument;
  for(;;) {
    uint8_t frame[28]={0x7e,1,'Z','K','C','P',1,28,1};
    uint32_t keys;
    taskENTER_CRITICAL();
    frame[9]=role;frame[10]=stage;frame[11]=flags;
    frame[22]=(uint8_t)diagnostic;frame[23]=(uint8_t)(diagnostic>>8);
    keys=keyboards;
    taskEXIT_CRITICAL();
    unsigned count=0;
    while(keys){count+=keys&1;keys>>=1;}
    frame[20]=(uint8_t)count;frame[21]=(uint8_t)(count>>8);
    if(count)frame[11]|=8;
    le32(frame+12,sequence);
    le32(frame+16,(uint32_t)((uint64_t)xTaskGetTickCount()*1000/configTICK_RATE_HZ));
    uint16_t crc=crc16(frame,26);frame[26]=(uint8_t)(crc>>8);frame[27]=(uint8_t)crc;
    if(mcu_hw_status_send(frame,sizeof(frame)))sequence++;
    /* Periodic even with no FPGA reply or HID device. State notifications
     * wake early; the lock attempt is non-blocking, not a hard health timer. */
    ulTaskNotifyTake(pdTRUE,pdMS_TO_TICKS(250)?pdMS_TO_TICKS(250):1);
  }
}
void companion_status_start(void) {
  if(xTaskCreate(report_task,"core status",1024,NULL,configMAX_PRIORITIES-2,&reporter)!=pdPASS)
    reporter=NULL; /* No false readiness if allocation fails: core sees silence. */
}
#endif
