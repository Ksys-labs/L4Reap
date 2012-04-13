/*
 * Generic task code. Used by some API interfaces.
 */

#include <asm/l4lxapi/generic/task_gen.h>

#include <asm/bitops.h>
#include <asm/l4linux/rmgr.h>

#include <linux/spinlock.h>

unsigned int l4lx_task_used[L4LX_TASK_VEC_SIZE] = {0, };
static DEFINE_SPINLOCK(l4lx_task_lock);

/*
 * l4lx_task_number_allocate_vector and l4lx_task_number_free_vector
 * use a simple array (used as a bit vector) to know whether a task number
 * is free or used.
 */

int l4lx_task_number_allocate_vector(void)
{
	int res = -1;
	int n;

	spin_lock(&l4lx_task_lock);

	n = find_first_zero_bit(l4lx_task_used, L4LX_TASK_BITVEC_SIZE);

	if (n >= L4LX_TASK_BITVEC_SIZE)
		goto out;

	set_bit(n, l4lx_task_used);

	res =  TASK_NO_MIN + n;

out:
	spin_unlock(&l4lx_task_lock);
	return res;
}

int l4lx_task_number_free_vector(int task_no)
{
	int res = -1;

	spin_lock(&l4lx_task_lock);

	task_no -= TASK_NO_MIN;
	if (task_no < 0 || task_no >= L4LX_TASK_BITVEC_SIZE)
		goto out;

	if (! test_and_clear_bit(task_no, l4lx_task_used))
		goto out;

	res = 0;
out:
	spin_unlock(&l4lx_task_lock);
	return res;
}

/*
 * l4lx_task_number_allocate_rmgr and l4lx_task_number_free_rmgr
 * ask the RMGR directly without managing a bit vector.
 */

/* VX2 doesn't know anything about RMGR, so hide this for it */
#if !defined(CONFIG_L4_VX2)
static int last_task_no = TASK_NO_MIN;

int l4lx_task_number_allocate_rmgr(void)
{
	int i;

	spin_lock(&l4lx_task_lock);

	i = last_task_no + 1;
	while (i != last_task_no) {
		if (i == TASK_NO_MAX)
			i = TASK_NO_MIN;
		
		if (!rmgr_get_task(i)) {
			last_task_no = i;
			goto out;
		}
		i++;
	}

	i = -1; /* failed, no free task no available */

out:
	spin_unlock(&l4lx_task_lock);
	return i;
}

int l4lx_task_number_free_rmgr(int task_number)
{
	int res;

	spin_lock(&l4lx_task_lock);
	res = rmgr_free_task(task_number);
	spin_unlock(&l4lx_task_lock);

	return res;
}
#endif
