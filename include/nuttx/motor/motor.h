/****************************************************************************
 * include/nuttx/motor/motor.h
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

#ifndef __INCLUDE_NUTTX_MOTOR_MOTOR_H
#define __INCLUDE_NUTTX_MOTOR_MOTOR_H

/* The motor driver is split into two parts:
 *
 * 1) An "upper half", generic driver that provides the common motor
 *    interface to application level code, and
 * 2) A "lower half", platform-specific driver that implements the low-level
 *    functionality eg.:
 *      - timer controls to implement the PWM signals,
 *      - analog peripherals configuration such as ADC, DAC and comparators,
 *      - control algorithm for motor driver (eg. FOC control for BLDC)
 *
 * This 'upper-half' driver has been designed with flexibility in mind
 * to support all kinds of electric motors and their applications.
 */

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>
#include <nuttx/compiler.h>

#include <nuttx/motor/motor_ioctl.h>

#ifdef CONFIG_MOTOR_UPPER

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/****************************************************************************
 * Public Types
 ****************************************************************************/

/* Motor operations used to call from the upper-half, generic motor driver
 * into lower-half, platform-specific logic.
 */

struct motor_lowerhalf_s;
struct motor_ops_s
{
  /* Configure motor */

  CODE int (*setup)(FAR struct motor_lowerhalf_s *dev);

  /* Disable motor */

  CODE int (*shutdown)(FAR struct motor_lowerhalf_s *dev);

  /* Stop motor */

  CODE int (*stop)(FAR struct motor_lowerhalf_s *dev);

  /* Start motor */

  CODE int (*start)(FAR struct motor_lowerhalf_s *dev);

  /* Set motor parameters */

  CODE int (*params_set)(FAR struct motor_lowerhalf_s *dev,
                         FAR struct motor_params_s *param);

  /* Set motor operation mode */

  CODE int (*mode_set)(FAR struct motor_lowerhalf_s *dev, uint8_t mode);

  /* Set motor limits */

  CODE int (*limits_set)(FAR struct motor_lowerhalf_s *dev,
                         FAR struct motor_limits_s *limits);

  /* Set motor fault */

  CODE int (*fault_set)(FAR struct motor_lowerhalf_s *dev, uint8_t fault);

  /* Get motor state  */

  CODE int (*state_get)(FAR struct motor_lowerhalf_s *dev,
                        FAR struct motor_state_s *state);

  /* Get current fault state */

  CODE int (*fault_get)(FAR struct motor_lowerhalf_s *dev,
                        FAR uint8_t *fault);

  /* Clear fault state */

  CODE int (*fault_clear)(FAR struct motor_lowerhalf_s *dev, uint8_t fault);

  /* Lower-half logic may support platform-specific ioctl commands */

  CODE int (*ioctl)(FAR struct motor_lowerhalf_s *dev, int cmd,
                    unsigned long arg);
};

/* This structure is the generic form of state structure used by lower half
 * motor driver.
 */

struct motor_lowerhalf_s
{
  FAR const struct motor_ops_s *ops;    /* Arch-specific operations */
  uint8_t                      opmode;  /* Motor operation mode */
  uint8_t                      opflags; /* Motor operation flags */
  struct motor_limits_s        limits;  /* Motor absolute limits */
  struct motor_params_s        param;   /* Motor settings */
  struct motor_state_s         state;   /* Motor state */
  FAR void                     *priv;   /* Private data */
};

/****************************************************************************
 * Public Data
 ****************************************************************************/

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

#ifdef __cplusplus
#define EXTERN extern "C"
extern "C"
{
#else
#define EXTERN extern
#endif

/****************************************************************************
 * Name: motor_register
 *
 * Description:
 *   This function binds an instance of a "lower half" motor driver with the
 *   "upper half" motor device and registers that device so that can be used
 *   by application code.
 *
 *   We will register the chararter device with specified path.
 *
 * Input Parameters:
 *   path  - The user specifies path name.
 *   lower - A pointer to an instance of lower half motor driver. This
 *           instance is bound to the motor driver and must persists as long
 *           as the driver persists.
 *
 * Returned Value:
 *   OK if the driver was successfully register; A negated errno value is
 *   returned on any failure.
 *
 ****************************************************************************/

int motor_register(FAR const char *path,
                   FAR struct motor_lowerhalf_s *lower);

#undef EXTERN
#ifdef __cplusplus
}
#endif

#endif /* CONFIG_MOTOR_UPPER */
#endif /* __INCLUDE_NUTTX_DRIVERS_MOTOR_MOTOR_H */
