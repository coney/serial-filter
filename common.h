#ifndef _COMMON_H_INCLUDED_
#define _COMMON_H_INCLUDED_

#include <linux/module.h>
#include <linux/serial_core.h>
#include "klog.h"
#include "config.h"

static inline const char * current_task_exe_name(void)
{
    // TODO - get from mm
    return current->comm;

}

static inline int current_task_pid(void)
{
    return task_pid_nr(current);
}


#endif // _COMMON_H_INCLUDED_
