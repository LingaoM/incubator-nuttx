/****************************************************************************
 * arch/risc-v/src/common/riscv_doirq.c
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
#include <nuttx/addrenv.h>
#include <nuttx/arch.h>
#include <nuttx/board.h>
#include <arch/barriers.h>
#include <arch/board/board.h>
#include <sched/sched.h>

#include "riscv_internal.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* ISR preemption per ARCH_HIPRI_INTERRUPT */
#ifdef CONFIG_ARCH_HIPRI_INTERRUPT

static inline_function void irq_nested_dipatch(int irq, uintptr_t *regs)
{
  /* exception should not be interrupt by any other interrupt.
   * for example, ecall or panic triggered.
   */

  bool preemptable = irq > RISCV_MAX_EXCEPTION;
  if (preemptable)
    {
      up_irq_enable();
    }

  irq_dispatch(irq, regs);

  if (preemptable)
    {
      up_irq_disable();
    }
}

#  define IRQ_DISPATCH(irq, regs) irq_nested_dipatch(irq, regs)
#else
#  define IRQ_DISPATCH(irq, regs) irq_dispatch(irq, regs)
#endif

/****************************************************************************
 * Public Data
 ****************************************************************************/

/****************************************************************************
 * Private Data
 ****************************************************************************/

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static uintptr_t *riscv_doirq_top(int irq, uintptr_t *regs)
{
  board_autoled_on(LED_INIRQ);
#ifdef CONFIG_SUPPRESS_INTERRUPTS
  PANIC();
#else

  /* NOTE: In case of ecall, we need to adjust mepc in the context */

  if (irq >= RISCV_IRQ_ECALLU && irq <= RISCV_IRQ_ECALLM)
    {
      regs[REG_EPC] += 4;
    }

  /* Current regs non-zero indicates that we are processing an interrupt;
   * CURRENT_REGS is also used to manage interrupt level context switches.
   */

  DEBUGASSERT(CURRENT_REGS == NULL);
  CURRENT_REGS = regs;

  /* As irq nested supported, we have to makesure CURRENT_REG modified
   * before allow new interrupt to enter.
   */

  __DMB();

  /* Deliver the IRQ */

  IRQ_DISPATCH(irq, regs);

  /* As irq nested supported, have to ensure interrupt disabled before
   * get the lastest regs.
   */

  __DMB();

  /* Check for a context switch.  If a context switch occurred, then
   * CURRENT_REGS will have a different value than it did on entry.  If an
   * interrupt level context switch has occurred, then restore the floating
   * point state and the establish the correct address environment before
   * returning from the interrupt.
   */

  if (regs != CURRENT_REGS)
    {
#ifdef CONFIG_ARCH_ADDRENV
      /* Make sure that the address environment for the previously
       * running task is closed down gracefully (data caches dump,
       * MMU flushed) and set up the address environment for the new
       * thread at the head of the ready-to-run list.
       */

      addrenv_switch(NULL);
#endif

      /* Record the new "running" task when context switch occurred.
       * g_running_tasks[] is only used by assertion logic for reporting
       * crashes.
       */

      g_running_tasks[this_cpu()] = this_task();

      /* Restore the cpu lock */

      restore_critical_section();

      /* If a context switch occurred while processing the interrupt then
       * CURRENT_REGS may have change value.  If we return any value
       * different from the input regs, then the lower level will know
       * that a context switch occurred during interrupt processing.
       */

      regs = (uintptr_t *)CURRENT_REGS;
    }

  /* Set CURRENT_REGS to NULL to indicate that we are no longer in an
   * interrupt handler.
   */

  CURRENT_REGS = NULL;

#endif
  board_autoled_off(LED_INIRQ);
  return regs;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

uintptr_t *riscv_doirq(int irq, uintptr_t *regs)
{
  if (up_interrupt_context())
    {
      IRQ_DISPATCH(irq, regs);
      return regs;
    }

  return riscv_doirq_top(irq, regs);
}
