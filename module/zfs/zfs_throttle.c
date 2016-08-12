#include "sys/time.h"
#include "sys/zfs_throttle.h"
#include "sys/zfs_ioctl.h"
#include "sys/zfs_vfsops.h"

uint64_t now(void)
{
    struct timespec ts;
    ts = current_kernel_time();
    return ts.tv_sec * 1000000000LL + ts.tv_nsec;
}

void zfs_do_throttle(zfs_throttle_t *zt, bool is_write, uint64_t size) {
	int64_t _wait;
	uint64_t _now;
	uint64_t data;
	struct semaphore *sem;

	if (zt->is_enabled) {
		if (is_write) {
			if (zt->z_prop_write_bytes > 0) {
				sem = zt->z_sem_real_write;
				down(sem);
				atomic_add(size, &(zt->z_real_write_bytes));
				data = atomic_read(&(zt->z_real_write_bytes));
				//dprintf("------- write bytes throttle 0: %llu\n", data);
				if (data > zt->z_prop_write_bytes) {
					if (zt->z_write_timestamp > 0) {
						_now = now();
						_wait = ((data * 1000000) / zt->z_prop_write_bytes)
								- ((_now - zt->z_write_timestamp) / 1000);
						//dprintf("------- write bytes throttle 1: %llu %llu %llu %llu %lld\n", data, zt->z_prop_write_bytes, _now, zt->z_write_timestamp, _wait);
						if (_wait > 0) {
							usleep_range(_wait, _wait);
						}
					}
					atomic_set(&(zt->z_real_write_bytes), 0);
					zt->z_write_timestamp = now();
				}
				up(sem);
			} else {
				if (zt->z_prop_write_iops > 0) {
					sem = zt->z_sem_real_write;
					down(sem);
					atomic_add(1, &(zt->z_real_write_iops));
					data = atomic_read(&(zt->z_real_write_iops));
					//dprintf("------- write iops throttle 0: %llu\n", data);
					if (data > zt->z_prop_write_iops) {
						if (zt->z_write_timestamp > 0) {
							_now = now();
							_wait = ((data * 1000000) / zt->z_prop_write_iops)
									- ((_now - zt->z_write_timestamp) / 1000);
							//dprintf("------- write iops throttle 1: %llu %llu %llu %llu %lld\n", data, zt->z_prop_write_iops, _now, zt->z_write_timestamp, _wait);
							if (_wait > 0) {
								usleep_range(_wait, _wait);
							}
						}
						atomic_set(&(zt->z_real_write_iops), 0);
						zt->z_write_timestamp = now();
					}
					up(sem);
				}
			}
		} else {
			if (zt->z_prop_read_bytes > 0) {
				sem = zt->z_sem_real_read;
				down(sem);
				atomic_add(size, &(zt->z_real_read_bytes));
				data = atomic_read(&(zt->z_real_read_bytes));
				//dprintf("------- read bytes throttle 0: %llu\n", data);
				if (data > zt->z_prop_read_bytes) {
					if (zt->z_read_timestamp > 0) {
						_now = now();
						_wait = ((data * 1000000) / zt->z_prop_read_bytes)
								- ((_now - zt->z_read_timestamp) / 1000);
						//dprintf("------- read bytes throttle 1: %llu %llu %llu %llu %lld\n", data, zt->z_prop_read_bytes, _now, zt->z_read_timestamp, _wait);
						if (_wait > 0) {
							usleep_range(_wait, _wait);
						}
					}
					atomic_set(&(zt->z_real_read_bytes), 0);
					zt->z_read_timestamp = now();
				}
				up(sem);
			} else {
				if (zt->z_prop_read_iops > 0) {
					sem = zt->z_sem_real_read;
					down(sem);
					atomic_add(1, &(zt->z_real_read_iops));
					data = atomic_read(&(zt->z_real_read_iops));
					//dprintf("------- read iops throttle 0: %llu\n", data);
					if (data > zt->z_prop_read_iops) {
						if (zt->z_read_timestamp > 0) {
							_now = now();
							_wait = ((data * 1000000) / zt->z_prop_read_iops)
									- ((_now - zt->z_read_timestamp) / 1000);
							//dprintf("------- read iops throttle 1: %llu %llu %llu %llu %lld\n", data, zt->z_prop_read_iops, _now, zt->z_read_timestamp, _wait);
							if (_wait > 0) {
								usleep_range(_wait, _wait);
							}
						}
						atomic_set(&(zt->z_real_read_iops), 0);
						zt->z_read_timestamp = now();
					}
					up(sem);
				}
			}
		}
	}
	// update counters
	if (is_write) {
		zt->z_io_serviced->write_bytes.value.ui64 += size;
		zt->z_io_serviced->write_iops.value.ui64++;
	} else {
		zt->z_io_serviced->read_bytes.value.ui64 += size;
		zt->z_io_serviced->read_iops.value.ui64++;
	}
}

