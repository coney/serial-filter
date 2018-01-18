#include "common.h"
#include "debug_proc.h"
#include "filter.h"
#include "tty_interceptor.h"

MODULE_AUTHOR("Coney Wu <kunwu@thoughtworks.com>");
MODULE_LICENSE("GPL");


int __init serial_filter_init(void)
{
    int ret = 0;
    loginfo("load serial filter driver!\n");

    if ((ret = filter_init()) != 0) {
        logerror("failed to init filter!\n");
        goto filter_error;
    }

    if ((ret = tty_interceptor_init()) != 0) {
        logerror("failed to init tty interceptor!\n");
        goto tty_error;
    }

    if ((ret = debug_proc_init()) != 0) {
        logerror("failed to create proc entry!\n");
        goto proc_error;
    }

    loginfo("serial filter driver loaded!\n");
    return 0;

proc_error:
    debug_proc_clear();

tty_error:
    tty_intercedtor_clear();

filter_error:
    filter_clear();

    loginfo("serial filter driver failed to load!\n");

    return ret;
}

void __exit serial_filter_exit(void)
{
    loginfo("unload serial filter driver!\n");
    
    debug_proc_clear();
    tty_intercedtor_clear();
    filter_clear();
    return;
}



module_init(serial_filter_init);
module_exit(serial_filter_exit);
