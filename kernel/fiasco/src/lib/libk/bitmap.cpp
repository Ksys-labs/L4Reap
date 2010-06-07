INTERFACE:

class Bitmap_base
{
private:
  unsigned long _bits[0];
};

template<int BITS>
class Bitmap : public Bitmap_base
{
private:
  char _bits[(BITS+7)/8];
};


IMPLEMENTATION:


PUBLIC inline
void
Bitmap_base::bit(unsigned long bit, bool on)
{
  unsigned long idx = bit / (sizeof(unsigned long) * 8);
  unsigned long b   = bit % (sizeof(unsigned long) * 8);
  _bits[idx] = (_bits[idx] & ~(1UL << b)) | ((unsigned long)on << b);
}

PUBLIC inline
void
Bitmap_base::clear_bit(unsigned long bit)
{
  unsigned long idx = bit / (sizeof(unsigned long) * 8);
  unsigned long b   = bit % (sizeof(unsigned long) * 8);
  _bits[idx] &= ~(1UL << b);
}

PUBLIC inline
void
Bitmap_base::set_bit(unsigned long bit)
{
  unsigned long idx = bit / (sizeof(unsigned long) * 8);
  unsigned long b   = bit % (sizeof(unsigned long) * 8);
  _bits[idx] |= (1UL << b);
}

PUBLIC inline
unsigned long
Bitmap_base::operator [] (unsigned long bit) const
{
  unsigned long idx = bit / (sizeof(unsigned long) * 8);
  unsigned long b   = bit % (sizeof(unsigned long) * 8);
  return _bits[idx] & (1UL << b);
}

