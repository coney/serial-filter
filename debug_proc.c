#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <asm/uaccess.h>
#include <linux/export.h>

#include "klog.h"
#include "debug_proc.h"
#include "filter.h"

static char debug_proc_buffer[DEBUG_PROC_BUFSIZE];

static int debug_proc_config_show(struct seq_file *m, void *v) {
    logdebug("proc read (/proc/%s/%s)\n", DEBUG_PROC_NAME, DEBUG_PROC_CONFIG_NAME);
    seq_printf(m, filter_get_blacklist());
    return 0;
}

static int debug_proc_config_open(struct inode *inode, struct file *file) {
    return single_open(file, debug_proc_config_show, NULL);
}

static ssize_t debug_proc_config_write(struct file *file, const char *buffer, size_t count, loff_t *offset) {
    //logdebug("write %zu bytes, offset %p\n", count, offset);
    if (count >= DEBUG_PROC_BUFSIZE) {
        count = DEBUG_PROC_BUFSIZE - 1;
    }

    if (copy_from_user(debug_proc_buffer, buffer, count)) {
        return -EFAULT;
    }

    debug_proc_buffer[count] = 0;

    //logdebug("receive proc command %s\n", debug_proc_buffer);
    filter_set_blacklist(debug_proc_buffer);
    return count;
}

static const struct file_operations debug_proc_config_fops = {
        .owner = THIS_MODULE,
        .open = debug_proc_config_open,
        .read = seq_read,
        .write  = debug_proc_config_write,
        .llseek = seq_lseek,
        .release = single_release,
};



static struct proc_dir_entry *debug_proc_root = NULL;

int debug_proc_init(void) {

    if ((debug_proc_root = proc_mkdir_mode(DEBUG_PROC_NAME, S_IALLUGO, NULL)) == NULL
        || !proc_create(DEBUG_PROC_CONFIG_NAME, S_IALLUGO, debug_proc_root, &debug_proc_config_fops)) {
            debug_proc_clear();
            return -ENOMEM;
    }

    return 0;
}

void debug_proc_clear(void) {
    if (debug_proc_root) {
        remove_proc_entry(DEBUG_PROC_CONFIG_NAME, debug_proc_root);
        remove_proc_entry(DEBUG_PROC_NAME, NULL);
        debug_proc_root = 0;
    }
}
