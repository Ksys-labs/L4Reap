INTERFACE [ppc32 && qemu]:

#define TARGET_NAME "QEMU"

EXTENSION class Config
{
public:
  enum
  {
    Max_num_dirqs = 3,
  };
};
