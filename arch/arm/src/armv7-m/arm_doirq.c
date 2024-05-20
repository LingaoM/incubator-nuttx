/****************************************************************************
 * arch/arm/src/armv7-m/arm_doirq.c
 *
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.  The
 * ASF licenses this file to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance with the
 * License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

#include <stdint.h>
#include <assert.h>

#include <nuttx/irq.h>
#include <nuttx/arch.h>
#include <nuttx/board.h>
#include <arch/board/board.h>
#include <sched/sched.h>

#include "arm_internal.h"
#include "exc_return.h"
#include "nvic.h"

/****************************************************************************
 * Public Functions
 ****************************************************************************/

void arm_vector_doirq(void)
{
  struct tcb_s *rtcb = this_task_irq();
  int irq = getipsr();
  int flags;

  flags = up_irq_save();

  if (up_running_task() == NULL)
    {
      up_set_running_task(this_task_irq());
    }

  up_irq_restore(flags);

  arm_ack_irq(irq);
  irq_dispatch(irq, NULL);

  if (rtcb != this_task_irq())
    {
      up_trigger_irq(NVIC_IRQ_PENDSV, 0);
    }
}

uint32_t *arm_doirq(int irq, uint32_t *regs)
{
  board_autoled_on(LED_INIRQ);
#ifdef CONFIG_SUPPRESS_INTERRUPTS
  PANIC();
#else

  /* Acknowledge the interrupt */

  arm_ack_irq(irq);

  if (irq == NVIC_IRQ_PENDSV)
    {
      int flags;

      flags = up_irq_save();

      if (up_running_task() != NULL)
        {
          up_running_task()->xcp.regs = regs;
        }

      regs = this_task_irq()->xcp.regs;
      up_set_running_task(NULL);

      up_irq_restore(flags);
    }
  else
    {
      CURRENT_REGS = regs;

      up_set_running_task(this_task_irq());
      up_running_task()->xcp.regs = regs;

      /* Deliver the IRQ */

      irq_dispatch(irq, regs);

      /* Return to thread mode, restore newest thread regs */

      regs = (uint32_t *)CURRENT_REGS;
      up_set_running_task(NULL);

      CURRENT_REGS = NULL;
    }
#endif

  board_autoled_off(LED_INIRQ);
  return regs;
}
