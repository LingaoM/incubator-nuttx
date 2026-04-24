/****************************************************************************
 * arch/sim/src/sim/posix/sim_hosttime.c
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

#include <errno.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#include "sim_internal.h"

/****************************************************************************
 * Private Data
 ****************************************************************************/

static uint64_t g_start;

/* Ratio of simulated time to real time in percent.  100 means real-time
 * (default).  Values > 100 speed up simulated time; values < 100 slow it
 * down.  Overridable at runtime via --sim-rt-ratio=<percent>.
 */

static int g_time_ratio = CONFIG_SIM_WALLTIME_RATIO;

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: host_gettime
 ****************************************************************************/

uint64_t host_gettime(bool rtc)
{
  struct timespec tp;
  uint64_t current;

  clock_gettime(rtc ? CLOCK_REALTIME : CLOCK_MONOTONIC, &tp);
  current = 1000000000ull * tp.tv_sec + tp.tv_nsec;

  if (rtc)
    {
      return current;
    }

  if (g_start == 0)
    {
      g_start = current;
    }

  /* Apply time ratio: simulated_time = real_elapsed * ratio / 100 */

  return ((current - g_start) * g_time_ratio) / 100;
}

/****************************************************************************
 * Name: host_sleep
 ****************************************************************************/

void host_sleep(uint64_t nsec)
{
  usleep((nsec + 999) / 1000);
}

/****************************************************************************
 * Name: host_sleepuntil
 ****************************************************************************/

void host_sleepuntil(uint64_t nsec)
{
  uint64_t now;

  now = host_gettime(false);
  if (nsec > now + 1000)
    {
      /* nsec is in simulated time; convert back to real duration to sleep */

      usleep((((nsec - now) * 100) / g_time_ratio) / 1000);
    }
}

/****************************************************************************
 * Name: host_set_timeratio
 *
 * Description:
 *   Set the ratio of simulated time to real time in percent.  100 (default)
 *   means simulated time advances at the same rate as real time.  Values
 *   greater than 100 speed up simulated time; values less than 100 slow it
 *   down.
 *
 * Input Parameters:
 *   ratio - The new time ratio in percent (must be > 0)
 *
 ****************************************************************************/

void host_set_timeratio(int ratio)
{
  if (ratio > 0)
    {
      g_time_ratio = ratio;
    }
}

/****************************************************************************
 * Name: host_settimer
 *
 * Description:
 *   Set up a timer to send periodic signals.
 *
 * Input Parameters:
 *   nsec - timer expire time
 *
 * Returned Value:
 *   On success, (0) zero value is returned, otherwise a negative value.
 *
 ****************************************************************************/

int host_settimer(uint64_t nsec)
{
  struct itimerval it;
  uint64_t usec;

  /* nsec is in simulated time; convert back to real duration. */

  nsec = (nsec * 100) / g_time_ratio;
  usec = (nsec + 999) / 1000;

  it.it_interval.tv_sec  = 0;
  it.it_interval.tv_usec = 0;
  it.it_value.tv_sec     = usec / 1000000;
  it.it_value.tv_usec    = usec % 1000000;

  return setitimer(ITIMER_REAL, &it, NULL);
}

/****************************************************************************
 * Name: host_timerirq
 *
 * Description:
 *   Get timer irq
 *
 * Input Parameters:
 *   None
 *
 * Returned Value:
 *   On success, irq num returned, otherwise a negative value.
 *
 ****************************************************************************/

int host_timerirq(void)
{
  return SIGALRM;
}
