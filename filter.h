#ifndef _FILTER_H_INCLUDED_
#define _FILTER_H_INCLUDED_

#include "common.h"

int filter_init(void);
void filter_clear(void);

#define try_filter_tty_operation(tty, execution) do         \
{                                                           \
    int ret = filter_tty_operation(__FUNCTION__, (tty));    \
    return ret ? ret : (execution);                         \
} while (0)

void filter_set_blacklist(const char *exename);
const char * filter_get_blacklist(void);

int filter_tty_operation(const char *op, struct tty_struct *tty);

#endif // _FILTER_H_INCLUDED_
