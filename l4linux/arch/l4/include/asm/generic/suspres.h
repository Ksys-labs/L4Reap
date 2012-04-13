#ifndef __ASM_L4__GENERIC__SUSPRES_H__
#define __ASM_L4__GENERIC__SUSPRES_H__

enum l4x_suspend_resume_state {
	L4X_SUSPEND,
	L4X_RESUME,
};

struct l4x_suspend_resume_struct {
	struct list_head list;
	void (*func)(enum l4x_suspend_resume_state);
};

#ifdef CONFIG_PM
void l4x_suspend_resume_register(void (*func)(enum l4x_suspend_resume_state),
                                 struct l4x_suspend_resume_struct *e);
#else
static inline void l4x_suspend_resume_register(void (*func)(enum l4x_suspend_resume_state),
                                               struct l4x_suspend_resume_struct *e)
{}
#endif

#endif /* ! __ASM_L4__GENERIC__SUSPRES_H__ */
