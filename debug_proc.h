/**
  * @file   debug_proc.h
  * @brief  debug procfs header
  * @author ConeyWu
  * @date   10/01/2014
  */
#ifndef __DEBUG_PROC_H__
#define __DEBUG_PROC_H__

#define DEBUG_PROC_NAME            "serial_filter"
#define DEBUG_PROC_CONFIG          "config"
#define DEBUG_PROC_TEST            "test"
#define DEBUG_PROC_BUFSIZE         2048

int debug_proc_init(void);
void debug_proc_clear(void);

#endif // __DEBUG_PROC_H__
