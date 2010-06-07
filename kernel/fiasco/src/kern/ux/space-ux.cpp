IMPLEMENTATION[ux]:

PUBLIC inline
pid_t
Space::pid() const		// returns host pid number
{
  return mem_space()->pid();
}

IMPLEMENT inline
void
Space::switchin_ldt() const
{}
