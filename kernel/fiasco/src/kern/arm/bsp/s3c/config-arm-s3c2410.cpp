INTERFACE [arm && s3c2410]:

#define TARGET_NAME "S3C2410"

EXTENSION class Config
{
public:
  enum
  {
    Scheduling_irq       = 14,
    Max_num_dirqs        = 65,
  };
};

