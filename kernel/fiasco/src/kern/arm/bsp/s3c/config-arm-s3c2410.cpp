INTERFACE [arm && s3c2410]:

#define TARGET_NAME "S3C2410"

EXTENSION class Config
{
public:
  enum
  {
    Scheduling_irq       = 14,
    scheduler_irq_vector = Scheduling_irq,
    Max_num_irqs         = 67,
    Max_num_dirqs        = 65,
    Vkey_irq             = 65,
    Tbuf_irq             = 66,
  };
};

