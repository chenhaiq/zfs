#ifndef    _SYS_ZFS_THROTTLE_H
#define    _SYS_ZFS_THROTTLE_H

#if defined(_KERNEL)

#include <linux/delay.h>
#include <linux/semaphore.h>
#include <sys/systm.h>

#endif /* _KERNEL */

struct zfs_throttle;               /* defined in zfs_ioctl.h */

extern void zfs_do_throttle(struct zfs_throttle *zt, bool is_write, uint64_t size);

#endif /* _SYS_ZFS_THROTTLE_H */
