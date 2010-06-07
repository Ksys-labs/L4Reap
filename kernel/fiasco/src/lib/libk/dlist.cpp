INTERFACE:

class D_list_item
{
public:
  D_list_item() : _n(this), _p(this) {}

private:
  D_list_item *_n;
  D_list_item *_p;
};


IMPLEMENTATION:

PUBLIC inline
void
D_list_item::enqueue_next(D_list_item *i)
{
  i->_p = this;
  i->_n = _n;
  _n->_p = i;
  _n = i;
}

PUBLIC inline
void
D_list_item::enqueue(D_list_item *i)
{
  i->_n = this;
  i->_p = _p;
  _p->_n = i;
  _p = i;
}

PUBLIC inline
void
D_list_item::dequeue()
{
  _p->_n = _n;
  _n->_p = _p;
  _n = _p = this;
}

PUBLIC inline
D_list_item *
D_list_item::prev() const
{ return _p; }

PUBLIC inline
D_list_item *
D_list_item::next() const
{ return _n; }
