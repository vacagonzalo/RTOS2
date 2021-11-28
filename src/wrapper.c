#include "wrapper.h"
#include "C2.h"
#include "C2_ISR.h"

void initWrapper(config_t *config)
{
    C2_init(config);
    ISR_init(config);
}