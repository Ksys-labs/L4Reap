INTERFACE:

class Jdb_prompt_ext
{
public:
  Jdb_prompt_ext();
  virtual void ext() = 0;
  virtual void update();
  virtual ~Jdb_prompt_ext();

  static void do_all();
  static void update_all();
  
private:
  Jdb_prompt_ext *_next;
  Jdb_prompt_ext *_prev;
  static Jdb_prompt_ext *_first;
};

IMPLEMENTATION:

Jdb_prompt_ext *Jdb_prompt_ext::_first;

IMPLEMENT
Jdb_prompt_ext::Jdb_prompt_ext()
  : _next(0), _prev(0)
{
  _next = _first;
  _first = this;
}

IMPLEMENT
Jdb_prompt_ext::~Jdb_prompt_ext()
{
  if (_prev)
    _prev->_next = _next;
  else
    _first = _next;
  
  if (_next)
    _next->_prev = _prev;
}

IMPLEMENT
void Jdb_prompt_ext::update()
{}

IMPLEMENT
void Jdb_prompt_ext::do_all()
{
  Jdb_prompt_ext *e = _first;
  while (e)
    {
      e->ext();
      e = e->_next;
    }
}

IMPLEMENT
void Jdb_prompt_ext::update_all()
{
  Jdb_prompt_ext *e = _first;
  while (e)
    {
      e->update();
      e = e->_next;
    }
}

