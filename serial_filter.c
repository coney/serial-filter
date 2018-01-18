#include <linux/module.h>
#include <linux/tty_driver.h>
#include <linux/serial_core.h>
#include <linux/kallsyms.h>

#include "klog.h"
#include "debug_proc.h"

MODULE_AUTHOR("Coney Wu <kunwu@thoughtworks.com>");
MODULE_LICENSE("GPL");
static struct list_head *tty_drivers_ptr = NULL;
static struct mutex *tty_mutex_ptr = NULL;

struct tty_driver *original_driver = NULL;

static const struct tty_operations *original_ops = NULL;
static struct tty_operations hacked_uart_ops = {
};

static int uart_open(struct tty_struct *tty, struct file *filp)
{
    loginfo("uart_open tty:%s from process %s:%d\n", tty->name, current->comm, task_pid_nr(current));
    if (!strcmp("bash", current->comm)) {
        loginfo("reject access to %s from %s\n", tty->name, current->comm);
        return -EPERM;
    }
    return original_ops->open(tty, filp);
}

static void uart_start(struct tty_struct *tty)
{
    loginfo("uart_start %p\n", tty);
    return original_ops->start(tty);
}


static int uart_write(struct tty_struct *tty,
    const unsigned char *buf, int count)
{
    loginfo("uart_write %p\n", tty);
    return original_ops->write(tty, buf, count);
}

void access_tty_drivers(struct list_head * tty_drivers_ptr)
{
    struct tty_driver *p;

    mutex_lock(tty_mutex_ptr);
    list_for_each_entry(p, tty_drivers_ptr, tty_drivers) {
        if (p->type == TTY_DRIVER_TYPE_SERIAL)
        {
            loginfo("tty %p subtype:%04x driver:%s name:%s ops:%p %d:%d\n",
                p, p->subtype, p->driver_name, p->name, p->ops,
                p->major, p->minor_start);
            if (!strcmp("ttyS", p->name))
            {
                loginfo("assign %p to original_driver\n", p);
                original_driver = p;
                original_ops = original_driver->ops;
                memcpy(&hacked_uart_ops, original_driver->ops, sizeof(hacked_uart_ops));
            }
        }
    }
    mutex_unlock(tty_mutex_ptr);
}

int __init serial_filter_init(void)
{
    int ret = 0;
    loginfo("load serial filter driver!\n");

    tty_drivers_ptr = (struct list_head *)kallsyms_lookup_name("tty_drivers");
    tty_mutex_ptr = (struct mutex  *)kallsyms_lookup_name("tty_mutex");

    access_tty_drivers(tty_drivers_ptr);

    hacked_uart_ops.open = uart_open;
    hacked_uart_ops.start = uart_start;
    hacked_uart_ops.write = uart_write;

    loginfo("open %p:%p %p:%p\n",
        hacked_uart_ops.open, original_driver->ops->open,
        hacked_uart_ops.hangup, original_driver->ops->hangup);

    loginfo("hack ops %p from %p to %p\n", original_driver, original_driver->ops, &hacked_uart_ops);

    original_driver->ops = &hacked_uart_ops;

    if ((ret = debug_proc_init()) != 0) {
        logerror("failed to create proc entry!\n");
        goto error;
    }

    loginfo("serial filter driver loaded!\n");
    return 0;

error:
    debug_proc_clear();

    return ret;
}

void __exit serial_filter_exit(void)
{
    loginfo("unload serial filter driver!\n");

    loginfo("recover open from %p to %p\n", original_driver->ops, original_ops);

    original_driver->ops = original_ops;

    debug_proc_clear();

    return;
}



module_init(serial_filter_init);
module_exit(serial_filter_exit);
