/**
  * @file   klog.h
  * @brief  A simple log implementation
  * @author ConeyWu
  * @date   10/01/2014
  */
#ifndef __K_LOG_H__
#define __K_LOG_H__


#define __KLOG_TOSTRING(x)        #x
#define _KLOG_TOSTRING(x)         __KLOG_TOSTRING(x)

#define writelog(level, fmt...)        \
    printk(level __FILE__ ":" _KLOG_TOSTRING(__LINE__)" - " fmt)

#define logdebug(fmt...)        writelog(KERN_DEBUG, fmt)
#define loginfo(fmt...)         writelog(KERN_INFO, fmt)
#define logerror(fmt...)        writelog(KERN_ERR, fmt)
#define logwarn(fmt...)         writelog(KERN_WARNING, fmt)

#endif // __K_LOG_H__
