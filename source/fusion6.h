/**
 *    ||          ____  _ __  ______
 * +------+      / __ )(_) /_/ ____/_________ _____  ___
 * | 0xBC |     / __  / / __/ /    / ___/ __ `/_  / / _	\
 * +------+    / /_/ / / /_/ /___ / /  / /_/ / / /_/  __/
 *  ||  ||    /_____/_/\__/\____//_/   \__,_/ /___/\___/
 *
 * Crazyflie control firmware
 *
 * Copyright (C) 2011-2012 Bitcraze AB
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, in version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 */

#pragma once
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif


typedef struct  {
  float q0;
  float q1;
  float q2;
  float q3;  // quaternion of sensor frame relative to auxiliary frame
} quaternion_t;

  typedef struct  {
  float r;
  float p;
  float y;
  } euler_rpy_t;


void sensfusion6Init(const euler_rpy_t *euler);
bool sensfusion6Test(void);

void sensfusion6UpdateQ(float gx, float gy, float gz,
			float ax, float ay, float az,
			float dt);
void sensfusion6GetQuaternion(quaternion_t *q);
void sensfusion6GetEulerRPY(float* roll, float* pitch, float* yaw);
float sensfusion6GetAccZWithoutGravity(const float ax, const float ay, const float az);
float sensfusion6GetInvThrustCompensationForTilt(void);


#ifdef __cplusplus
}  // extern "C" 
#endif
