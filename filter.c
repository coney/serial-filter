#include "filter.h"

static char blacklist[32] = "bash";

void filter_set_blacklist(const char *exename) {
    if (strlen(exename)) {
        strncpy(blacklist, exename, sizeof(blacklist) - 1);
        loginfo("update blacklist to %s\n", blacklist);
    }
}

const char * filter_get_blacklist(void) {
    return blacklist;
}

int filter_tty_operation(const char *op, struct tty_struct *tty)
{
    loginfo("%s tty:%s from process %s:%d\n", op, tty->name, current_task_exe_name(), current_task_pid());
    if (!strcmp(blacklist, current_task_exe_name())) {
        loginfo("reject %s access to %s from %s\n", op, tty->name, current_task_exe_name());
        return -EPERM;
    }
    return 0;
}


int filter_init(void)
{
    return 0;
}

void filter_clear(void)
{

}