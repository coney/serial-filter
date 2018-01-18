#include "tty_interceptor.h"
#include <linux/kallsyms.h>

#include "filter.h"

static struct list_head *tty_drivers_ptr = NULL;
static struct mutex *tty_mutex_ptr = NULL;

struct tty_driver *original_driver = NULL;

typedef struct tty_ops_hook {
    struct tty_driver *driver;
    const struct tty_operations *raw_ops;
    struct tty_operations hooked_ops;
} tty_ops_hook_t ____cacheline_aligned;

// replace with hashtable if necessary
static tty_ops_hook_t tty_ops_hooks[MAX_TTY_OPS_HOOKS_COUNT] = {};

const struct tty_operations * find_raw_ops(struct tty_struct * tty)
{
    for (size_t i = 0; i < ARRAY_SIZE(tty_ops_hooks); i++)
    {
        const tty_ops_hook_t *hook = tty_ops_hooks + i;
        if (hook->driver == tty->driver) {
            return hook->raw_ops;
        }
    }
    BUG();
}

static int open(struct tty_struct *tty, struct file *filp)
{
    try_filter_tty_operation(tty, find_raw_ops(tty)->open(tty, filp));
}

static void close(struct tty_struct *tty, struct file *filp)
{
    loginfo("close tty:%s from process %s:%d\n", tty->name, current_task_exe_name(), current_task_pid());
    find_raw_ops(tty)->close(tty, filp);
}

static void start(struct tty_struct *tty)
{
    try_filter_tty_operation(tty, find_raw_ops(tty)->start(tty));
}

static int write(struct tty_struct *tty, const unsigned char *buf, int count)
{
    try_filter_tty_operation(tty, find_raw_ops(tty)->write(tty, buf, count));
}

static void init_hooked_ops(struct tty_operations *hooked_ops, const struct tty_operations *ops)
{
    memcpy(hooked_ops, ops, sizeof(struct tty_operations));
    hooked_ops->open = open;
    hooked_ops->start = start;
    hooked_ops->write = write;
    hooked_ops->close = close;
}

static void hook_tty_ops(struct tty_driver * drv)
{
    for (size_t i = 0; i < ARRAY_SIZE(tty_ops_hooks); i++)
    {
        tty_ops_hook_t *hook = tty_ops_hooks + i;
        if (hook->driver == drv) {
            // already hooked
            loginfo("already hooked tty driver %s:%p\n", drv->name, drv);
            return;
        }
        else if (!hook->driver) {
            loginfo("hooking tty driver %s:%p\n", drv->name, drv);
            hook->driver = drv;
            hook->raw_ops = drv->ops;
            init_hooked_ops(&hook->hooked_ops, drv->ops);
            barrier();
            drv->ops = &hook->hooked_ops;
            return;
        }
    }
    logwarn("tty ops hook table is full\n");
}

void hook_tty_drivers(struct list_head * tty_drivers_ptr)
{
    struct tty_driver *drv;

    mutex_lock(tty_mutex_ptr);
    list_for_each_entry(drv, tty_drivers_ptr, tty_drivers) {
        if (drv->type == TTY_DRIVER_TYPE_SERIAL)
        {
            loginfo("tty %p subtype:%04x driver:%s name:%s %d:%d\n",
                drv, drv->subtype, drv->driver_name, drv->name, drv->major, drv->minor_start);
            if (!strcmp("ttyS", drv->name) /*|| !strcmp("ttyUSB", p->name)*/)
            {
                hook_tty_ops(drv);
            }
        }
    }
    mutex_unlock(tty_mutex_ptr);
}


static void recover_hooked_drivers(void)
{
    // TODO - hook function may be still running, add sleep or refcount to ensure graceful clear
    for (size_t i = 0; i < ARRAY_SIZE(tty_ops_hooks); i++)
    {
        const tty_ops_hook_t *hook = tty_ops_hooks + i;
        if (hook->driver) {
            loginfo("recover tty driver %s:%p ops from %p to %p\n", 
                hook->driver->name, hook->driver,
                hook->driver->ops, hook->raw_ops);
            hook->driver->ops = hook->raw_ops;
        }
        else {
            break;
        }
    }
    synchronize_rcu();
   
    memset(tty_ops_hooks, 0, sizeof(tty_ops_hooks));
}

int tty_interceptor_init(void)
{

    tty_drivers_ptr = (struct list_head *)kallsyms_lookup_name("tty_drivers");
    tty_mutex_ptr = (struct mutex  *)kallsyms_lookup_name("tty_mutex");

    hook_tty_drivers(tty_drivers_ptr);
    return 0;
}

void tty_intercedtor_clear(void)
{
    recover_hooked_drivers();
}
