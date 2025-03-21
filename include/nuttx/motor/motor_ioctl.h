/****************************************************************************
 * include/nuttx/motor/motor_ioctl.h
 * NuttX Motor-Related IOCTLs definitions
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

#ifndef __INCLUDE_NUTTX_MOTOR_MOTOR_IOCTL_H
#define __INCLUDE_NUTTX_MOTOR_MOTOR_IOCTL_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>
#include <nuttx/fs/ioctl.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* All foc-related IOCTL commands must be defined in this header file
 * in order to assure that every IOCTL command is unique and will not be
 * aliased.
 */

#define MTRIOC_START          _MTRIOC(1)
#define MTRIOC_STOP           _MTRIOC(2)
#define MTRIOC_GET_STATE      _MTRIOC(3)
#define MTRIOC_CLEAR_FAULT    _MTRIOC(4)
#define MTRIOC_SET_PARAMS     _MTRIOC(5)
#define MTRIOC_SET_CONFIG     _MTRIOC(6)
#define MTRIOC_GET_INFO       _MTRIOC(7)
#define MTRIOC_SET_MODE       _MTRIOC(8)
#define MTRIOC_SET_LIMITS     _MTRIOC(9)
#define MTRIOC_SET_FAULT      _MTRIOC(10)
#define MTRIOC_GET_FAULT      _MTRIOC(11)
#define MTRIOC_PWM_OFF        _MTRIOC(12)
#define MTRIOC_CALIBRATE      _MTRIOC(13)
#define MTRIOC_SELFTEST       _MTRIOC(14)
#define MTRIOC_SET_CALIBDATA  _MTRIOC(15)

/****************************************************************************
 * Public Types
 ****************************************************************************/

/* Motor driver operation modes */

enum motor_opmode_e
{
  MOTOR_OPMODE_INIT      = 0,   /* Initial mode */
  MOTOR_OPMODE_POSITION  = 1,   /* Position control mode */
  MOTOR_OPMODE_SPEED     = 2,   /* Speed control mode */
  MOTOR_OPMODE_TORQUE    = 3,   /* Torque control mode */
  MOTOR_OPMODE_FORCE     = 4,   /* Force control mode */
  MOTOR_OPMODE_PATTERN   = 5    /* Pattern control mode */
};

/* Motor driver state */

enum motor_state_e
{
  MOTOR_STATE_INIT     = 0,     /* Initial state */
  MOTOR_STATE_IDLE     = 1,     /* IDLE state */
  MOTOR_STATE_RUN      = 2,     /* Run state */
  MOTOR_STATE_FAULT    = 3,     /* Fault state */
  MOTOR_STATE_CRITICAL = 4      /* Critical Fault state */
};

/* Motor driver fault type */

enum motor_fault_e
{
  MOTOR_FAULT_OVERCURRENT  = (1 << 0),  /* Over-current Fault */
  MOTOR_FAULT_OVERVOLTAGE  = (1 << 1),  /* Over-voltage Fault */
  MOTOR_FAULT_OVERPOWER    = (1 << 2),  /* Over-power Fault (electrical) */
  MOTOR_FAULT_OVERTEMP     = (1 << 3),  /* Over-temperature Fault */
  MOTOR_FAULT_OVERLOAD     = (1 << 4),  /* Motor overload Fault (mechanical) */
  MOTOR_FAULT_LOCKED       = (1 << 5),  /* Motor locked Fault */
  MOTOR_FAULT_INVAL_PARAM  = (1 << 6),  /* Invalid parameter Fault */
  MOTOR_FAULT_OTHER        = (1 << 7)   /* Other Fault */
};

/* Motor direction */

enum motor_direction_e
{
  MOTOR_DIR_CCW = -1,
  MOTOR_DIR_CW  = 1
};

/* This structure contains feedback data from motor driver */

struct motor_feedback_s
{
#ifdef CONFIG_MOTOR_UPPER_HAVE_POSITION
  float position;               /* Current motor position */
#endif
#ifdef CONFIG_MOTOR_UPPER_HAVE_SPEED
  float speed;                  /* Current motor speed */
#endif
#ifdef CONFIG_MOTOR_UPPER_HAVE_TORQUE
  float torque;                 /* Current motor torque (rotary motor) */
#endif
#ifdef CONFIG_MOTOR_UPPER_HAVE_FORCE
  float force;                  /* Current motor force (linear motor) */
#endif
#ifdef CONFIG_MOTOR_UPPER_HAVE_INPUT_VOLTAGE
  float v_in;                   /* Current input voltage */
#endif
#ifdef CONFIG_MOTOR_UPPER_HAVE_INPUT_CURRENT
  float i_in;                   /* Current input current */
#endif
#ifdef CONFIG_MOTOR_UPPER_HAVE_INPUT_POWER
  float p_in;                   /* Current input power */
#endif
};

/* This structure describes motor driver state */

struct motor_state_s
{
  uint8_t                 state;     /* Motor state  */
  uint8_t                 fault;     /* Motor faults state */
  struct motor_feedback_s fb;        /* Feedback from motor */
};

/* Motor absolute limits. Exceeding this limits should cause critical error
 * This structure must be configured before motor params_set call.
 * When limit is set to 0 then it is ignored.
 */

struct motor_limits_s
{
  bool  lock;                        /* This bit must be set after
                                      * limits configuration.
                                      */
#ifdef CONFIG_MOTOR_UPPER_HAVE_POSITION
  float position;                    /* Maximum motor position */
#endif
#ifdef CONFIG_MOTOR_UPPER_HAVE_SPEED
  float speed;                       /* Maximum motor speed */
#endif
#ifdef CONFIG_MOTOR_UPPER_HAVE_TORQUE
  float torque;                      /* Maximum motor torque (rotary motor) */
#endif
#ifdef CONFIG_MOTOR_UPPER_HAVE_FORCE
  float force;                       /* Maximum motor force (linear motor) */
#endif
#ifdef CONFIG_MOTOR_UPPER_HAVE_ACCELERATION
  float acceleration;                /* Maximum motor acceleration */
#endif
#ifdef CONFIG_MOTOR_UPPER_HAVE_DECELERATION
  float deceleration;                /* Maximum motor decelaration */
#endif
#ifdef CONFIG_MOTOR_UPPER_HAVE_INPUT_VOLTAGE
  float v_in;                        /* Maximum input voltage */
#endif
#ifdef CONFIG_MOTOR_UPPER_HAVE_INPUT_CURRENT
  float i_in;                        /* Maximum input current */
#endif
#ifdef CONFIG_MOTOR_UPPER_HAVE_INPUT_POWER
  float p_in;                        /* Maximum input power */
#endif
};

/* Motor parameters.
 * NOTE: All parameters require not negative value.
 */

struct motor_params_s
{
  bool  lock;                        /* Lock this structure. Set this bit
                                      * if there is no need to change motor
                                      * parameter during run-time.
                                      */
#ifdef CONFIG_MOTOR_UPPER_HAVE_DIRECTION
  int8_t  direction;                 /* Motor movement direction. We do not
                                      * support negative values for parameters,
                                      * so this flag can be used to allow movement
                                      * in the positive and negative direction in
                                      * a given coordinate system.
                                      */
#endif
#ifdef CONFIG_MOTOR_UPPER_HAVE_POSITION
  float position;                    /* Motor position */
#endif
#ifdef CONFIG_MOTOR_UPPER_HAVE_SPEED
  float speed;                       /* Motor speed */
#endif
#ifdef CONFIG_MOTOR_UPPER_HAVE_TORQUE
  float torque;                      /* Motor torque (rotary motor) */
#endif
#ifdef CONFIG_MOTOR_UPPER_HAVE_FORCE
  float force;                       /* Motor force (linear motor) */
#endif
#ifdef CONFIG_MOTOR_UPPER_HAVE_ACCELERATION
  float acceleration;                /* Motor acceleration */
#endif
#ifdef CONFIG_MOTOR_UPPER_HAVE_DECELERATION
  float deceleration;                /* Motor deceleration */
#endif
  FAR void *privdata;                /* out of band data */
};

#endif /* __INCLUDE_NUTTX_MOTOR_MOTOR_IOCTL_H */
