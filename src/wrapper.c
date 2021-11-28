#include "wrapper.h"
#include "C2.h"
#include "C2_ISR.h"
#include "C3.h"
#include "msg.h"

void initWrapper(config_t *config)
{
    C2_init(config);
    ISR_init(config);
    C3_init(config);
    queue_init(config);
}
