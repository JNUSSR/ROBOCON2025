// generated from rosidl_generator_c/resource/idl__struct.h.em
// with input from fast_lio:msg/Pose6D.idl
// generated code does not contain a copyright notice

#ifndef FAST_LIO__MSG__DETAIL__POSE6_D__STRUCT_H_
#define FAST_LIO__MSG__DETAIL__POSE6_D__STRUCT_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>


// Constants defined in the message

/// Struct defined in msg/Pose6D in the package fast_lio.
/**
  * the preintegrated Lidar states at the time of IMU measurements in a frame
 */
typedef struct fast_lio__msg__Pose6D
{
  /// the offset time of IMU measurement w.r.t the first lidar point
  double offset_time;
  /// the preintegrated total acceleration (global frame) at the Lidar origin
  double acc[3];
  /// the unbiased angular velocity (body frame) at the Lidar origin
  double gyr[3];
  /// the preintegrated velocity (global frame) at the Lidar origin
  double vel[3];
  /// the preintegrated position (global frame) at the Lidar origin
  double pos[3];
  /// the preintegrated rotation (global frame) at the Lidar origin
  double rot[9];
} fast_lio__msg__Pose6D;

// Struct for a sequence of fast_lio__msg__Pose6D.
typedef struct fast_lio__msg__Pose6D__Sequence
{
  fast_lio__msg__Pose6D * data;
  /// The number of valid items in data
  size_t size;
  /// The number of allocated items in data
  size_t capacity;
} fast_lio__msg__Pose6D__Sequence;

#ifdef __cplusplus
}
#endif

#endif  // FAST_LIO__MSG__DETAIL__POSE6_D__STRUCT_H_
