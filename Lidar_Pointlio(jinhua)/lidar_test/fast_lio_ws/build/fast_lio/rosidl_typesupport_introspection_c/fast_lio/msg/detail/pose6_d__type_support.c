// generated from rosidl_typesupport_introspection_c/resource/idl__type_support.c.em
// with input from fast_lio:msg/Pose6D.idl
// generated code does not contain a copyright notice

#include <stddef.h>
#include "fast_lio/msg/detail/pose6_d__rosidl_typesupport_introspection_c.h"
#include "fast_lio/msg/rosidl_typesupport_introspection_c__visibility_control.h"
#include "rosidl_typesupport_introspection_c/field_types.h"
#include "rosidl_typesupport_introspection_c/identifier.h"
#include "rosidl_typesupport_introspection_c/message_introspection.h"
#include "fast_lio/msg/detail/pose6_d__functions.h"
#include "fast_lio/msg/detail/pose6_d__struct.h"


#ifdef __cplusplus
extern "C"
{
#endif

void fast_lio__msg__Pose6D__rosidl_typesupport_introspection_c__Pose6D_init_function(
  void * message_memory, enum rosidl_runtime_c__message_initialization _init)
{
  // TODO(karsten1987): initializers are not yet implemented for typesupport c
  // see https://github.com/ros2/ros2/issues/397
  (void) _init;
  fast_lio__msg__Pose6D__init(message_memory);
}

void fast_lio__msg__Pose6D__rosidl_typesupport_introspection_c__Pose6D_fini_function(void * message_memory)
{
  fast_lio__msg__Pose6D__fini(message_memory);
}

size_t fast_lio__msg__Pose6D__rosidl_typesupport_introspection_c__size_function__Pose6D__acc(
  const void * untyped_member)
{
  (void)untyped_member;
  return 3;
}

const void * fast_lio__msg__Pose6D__rosidl_typesupport_introspection_c__get_const_function__Pose6D__acc(
  const void * untyped_member, size_t index)
{
  const double * member =
    (const double *)(untyped_member);
  return &member[index];
}

void * fast_lio__msg__Pose6D__rosidl_typesupport_introspection_c__get_function__Pose6D__acc(
  void * untyped_member, size_t index)
{
  double * member =
    (double *)(untyped_member);
  return &member[index];
}

void fast_lio__msg__Pose6D__rosidl_typesupport_introspection_c__fetch_function__Pose6D__acc(
  const void * untyped_member, size_t index, void * untyped_value)
{
  const double * item =
    ((const double *)
    fast_lio__msg__Pose6D__rosidl_typesupport_introspection_c__get_const_function__Pose6D__acc(untyped_member, index));
  double * value =
    (double *)(untyped_value);
  *value = *item;
}

void fast_lio__msg__Pose6D__rosidl_typesupport_introspection_c__assign_function__Pose6D__acc(
  void * untyped_member, size_t index, const void * untyped_value)
{
  double * item =
    ((double *)
    fast_lio__msg__Pose6D__rosidl_typesupport_introspection_c__get_function__Pose6D__acc(untyped_member, index));
  const double * value =
    (const double *)(untyped_value);
  *item = *value;
}

size_t fast_lio__msg__Pose6D__rosidl_typesupport_introspection_c__size_function__Pose6D__gyr(
  const void * untyped_member)
{
  (void)untyped_member;
  return 3;
}

const void * fast_lio__msg__Pose6D__rosidl_typesupport_introspection_c__get_const_function__Pose6D__gyr(
  const void * untyped_member, size_t index)
{
  const double * member =
    (const double *)(untyped_member);
  return &member[index];
}

void * fast_lio__msg__Pose6D__rosidl_typesupport_introspection_c__get_function__Pose6D__gyr(
  void * untyped_member, size_t index)
{
  double * member =
    (double *)(untyped_member);
  return &member[index];
}

void fast_lio__msg__Pose6D__rosidl_typesupport_introspection_c__fetch_function__Pose6D__gyr(
  const void * untyped_member, size_t index, void * untyped_value)
{
  const double * item =
    ((const double *)
    fast_lio__msg__Pose6D__rosidl_typesupport_introspection_c__get_const_function__Pose6D__gyr(untyped_member, index));
  double * value =
    (double *)(untyped_value);
  *value = *item;
}

void fast_lio__msg__Pose6D__rosidl_typesupport_introspection_c__assign_function__Pose6D__gyr(
  void * untyped_member, size_t index, const void * untyped_value)
{
  double * item =
    ((double *)
    fast_lio__msg__Pose6D__rosidl_typesupport_introspection_c__get_function__Pose6D__gyr(untyped_member, index));
  const double * value =
    (const double *)(untyped_value);
  *item = *value;
}

size_t fast_lio__msg__Pose6D__rosidl_typesupport_introspection_c__size_function__Pose6D__vel(
  const void * untyped_member)
{
  (void)untyped_member;
  return 3;
}

const void * fast_lio__msg__Pose6D__rosidl_typesupport_introspection_c__get_const_function__Pose6D__vel(
  const void * untyped_member, size_t index)
{
  const double * member =
    (const double *)(untyped_member);
  return &member[index];
}

void * fast_lio__msg__Pose6D__rosidl_typesupport_introspection_c__get_function__Pose6D__vel(
  void * untyped_member, size_t index)
{
  double * member =
    (double *)(untyped_member);
  return &member[index];
}

void fast_lio__msg__Pose6D__rosidl_typesupport_introspection_c__fetch_function__Pose6D__vel(
  const void * untyped_member, size_t index, void * untyped_value)
{
  const double * item =
    ((const double *)
    fast_lio__msg__Pose6D__rosidl_typesupport_introspection_c__get_const_function__Pose6D__vel(untyped_member, index));
  double * value =
    (double *)(untyped_value);
  *value = *item;
}

void fast_lio__msg__Pose6D__rosidl_typesupport_introspection_c__assign_function__Pose6D__vel(
  void * untyped_member, size_t index, const void * untyped_value)
{
  double * item =
    ((double *)
    fast_lio__msg__Pose6D__rosidl_typesupport_introspection_c__get_function__Pose6D__vel(untyped_member, index));
  const double * value =
    (const double *)(untyped_value);
  *item = *value;
}

size_t fast_lio__msg__Pose6D__rosidl_typesupport_introspection_c__size_function__Pose6D__pos(
  const void * untyped_member)
{
  (void)untyped_member;
  return 3;
}

const void * fast_lio__msg__Pose6D__rosidl_typesupport_introspection_c__get_const_function__Pose6D__pos(
  const void * untyped_member, size_t index)
{
  const double * member =
    (const double *)(untyped_member);
  return &member[index];
}

void * fast_lio__msg__Pose6D__rosidl_typesupport_introspection_c__get_function__Pose6D__pos(
  void * untyped_member, size_t index)
{
  double * member =
    (double *)(untyped_member);
  return &member[index];
}

void fast_lio__msg__Pose6D__rosidl_typesupport_introspection_c__fetch_function__Pose6D__pos(
  const void * untyped_member, size_t index, void * untyped_value)
{
  const double * item =
    ((const double *)
    fast_lio__msg__Pose6D__rosidl_typesupport_introspection_c__get_const_function__Pose6D__pos(untyped_member, index));
  double * value =
    (double *)(untyped_value);
  *value = *item;
}

void fast_lio__msg__Pose6D__rosidl_typesupport_introspection_c__assign_function__Pose6D__pos(
  void * untyped_member, size_t index, const void * untyped_value)
{
  double * item =
    ((double *)
    fast_lio__msg__Pose6D__rosidl_typesupport_introspection_c__get_function__Pose6D__pos(untyped_member, index));
  const double * value =
    (const double *)(untyped_value);
  *item = *value;
}

size_t fast_lio__msg__Pose6D__rosidl_typesupport_introspection_c__size_function__Pose6D__rot(
  const void * untyped_member)
{
  (void)untyped_member;
  return 9;
}

const void * fast_lio__msg__Pose6D__rosidl_typesupport_introspection_c__get_const_function__Pose6D__rot(
  const void * untyped_member, size_t index)
{
  const double * member =
    (const double *)(untyped_member);
  return &member[index];
}

void * fast_lio__msg__Pose6D__rosidl_typesupport_introspection_c__get_function__Pose6D__rot(
  void * untyped_member, size_t index)
{
  double * member =
    (double *)(untyped_member);
  return &member[index];
}

void fast_lio__msg__Pose6D__rosidl_typesupport_introspection_c__fetch_function__Pose6D__rot(
  const void * untyped_member, size_t index, void * untyped_value)
{
  const double * item =
    ((const double *)
    fast_lio__msg__Pose6D__rosidl_typesupport_introspection_c__get_const_function__Pose6D__rot(untyped_member, index));
  double * value =
    (double *)(untyped_value);
  *value = *item;
}

void fast_lio__msg__Pose6D__rosidl_typesupport_introspection_c__assign_function__Pose6D__rot(
  void * untyped_member, size_t index, const void * untyped_value)
{
  double * item =
    ((double *)
    fast_lio__msg__Pose6D__rosidl_typesupport_introspection_c__get_function__Pose6D__rot(untyped_member, index));
  const double * value =
    (const double *)(untyped_value);
  *item = *value;
}

static rosidl_typesupport_introspection_c__MessageMember fast_lio__msg__Pose6D__rosidl_typesupport_introspection_c__Pose6D_message_member_array[6] = {
  {
    "offset_time",  // name
    rosidl_typesupport_introspection_c__ROS_TYPE_DOUBLE,  // type
    0,  // upper bound of string
    NULL,  // members of sub message
    false,  // is array
    0,  // array size
    false,  // is upper bound
    offsetof(fast_lio__msg__Pose6D, offset_time),  // bytes offset in struct
    NULL,  // default value
    NULL,  // size() function pointer
    NULL,  // get_const(index) function pointer
    NULL,  // get(index) function pointer
    NULL,  // fetch(index, &value) function pointer
    NULL,  // assign(index, value) function pointer
    NULL  // resize(index) function pointer
  },
  {
    "acc",  // name
    rosidl_typesupport_introspection_c__ROS_TYPE_DOUBLE,  // type
    0,  // upper bound of string
    NULL,  // members of sub message
    true,  // is array
    3,  // array size
    false,  // is upper bound
    offsetof(fast_lio__msg__Pose6D, acc),  // bytes offset in struct
    NULL,  // default value
    fast_lio__msg__Pose6D__rosidl_typesupport_introspection_c__size_function__Pose6D__acc,  // size() function pointer
    fast_lio__msg__Pose6D__rosidl_typesupport_introspection_c__get_const_function__Pose6D__acc,  // get_const(index) function pointer
    fast_lio__msg__Pose6D__rosidl_typesupport_introspection_c__get_function__Pose6D__acc,  // get(index) function pointer
    fast_lio__msg__Pose6D__rosidl_typesupport_introspection_c__fetch_function__Pose6D__acc,  // fetch(index, &value) function pointer
    fast_lio__msg__Pose6D__rosidl_typesupport_introspection_c__assign_function__Pose6D__acc,  // assign(index, value) function pointer
    NULL  // resize(index) function pointer
  },
  {
    "gyr",  // name
    rosidl_typesupport_introspection_c__ROS_TYPE_DOUBLE,  // type
    0,  // upper bound of string
    NULL,  // members of sub message
    true,  // is array
    3,  // array size
    false,  // is upper bound
    offsetof(fast_lio__msg__Pose6D, gyr),  // bytes offset in struct
    NULL,  // default value
    fast_lio__msg__Pose6D__rosidl_typesupport_introspection_c__size_function__Pose6D__gyr,  // size() function pointer
    fast_lio__msg__Pose6D__rosidl_typesupport_introspection_c__get_const_function__Pose6D__gyr,  // get_const(index) function pointer
    fast_lio__msg__Pose6D__rosidl_typesupport_introspection_c__get_function__Pose6D__gyr,  // get(index) function pointer
    fast_lio__msg__Pose6D__rosidl_typesupport_introspection_c__fetch_function__Pose6D__gyr,  // fetch(index, &value) function pointer
    fast_lio__msg__Pose6D__rosidl_typesupport_introspection_c__assign_function__Pose6D__gyr,  // assign(index, value) function pointer
    NULL  // resize(index) function pointer
  },
  {
    "vel",  // name
    rosidl_typesupport_introspection_c__ROS_TYPE_DOUBLE,  // type
    0,  // upper bound of string
    NULL,  // members of sub message
    true,  // is array
    3,  // array size
    false,  // is upper bound
    offsetof(fast_lio__msg__Pose6D, vel),  // bytes offset in struct
    NULL,  // default value
    fast_lio__msg__Pose6D__rosidl_typesupport_introspection_c__size_function__Pose6D__vel,  // size() function pointer
    fast_lio__msg__Pose6D__rosidl_typesupport_introspection_c__get_const_function__Pose6D__vel,  // get_const(index) function pointer
    fast_lio__msg__Pose6D__rosidl_typesupport_introspection_c__get_function__Pose6D__vel,  // get(index) function pointer
    fast_lio__msg__Pose6D__rosidl_typesupport_introspection_c__fetch_function__Pose6D__vel,  // fetch(index, &value) function pointer
    fast_lio__msg__Pose6D__rosidl_typesupport_introspection_c__assign_function__Pose6D__vel,  // assign(index, value) function pointer
    NULL  // resize(index) function pointer
  },
  {
    "pos",  // name
    rosidl_typesupport_introspection_c__ROS_TYPE_DOUBLE,  // type
    0,  // upper bound of string
    NULL,  // members of sub message
    true,  // is array
    3,  // array size
    false,  // is upper bound
    offsetof(fast_lio__msg__Pose6D, pos),  // bytes offset in struct
    NULL,  // default value
    fast_lio__msg__Pose6D__rosidl_typesupport_introspection_c__size_function__Pose6D__pos,  // size() function pointer
    fast_lio__msg__Pose6D__rosidl_typesupport_introspection_c__get_const_function__Pose6D__pos,  // get_const(index) function pointer
    fast_lio__msg__Pose6D__rosidl_typesupport_introspection_c__get_function__Pose6D__pos,  // get(index) function pointer
    fast_lio__msg__Pose6D__rosidl_typesupport_introspection_c__fetch_function__Pose6D__pos,  // fetch(index, &value) function pointer
    fast_lio__msg__Pose6D__rosidl_typesupport_introspection_c__assign_function__Pose6D__pos,  // assign(index, value) function pointer
    NULL  // resize(index) function pointer
  },
  {
    "rot",  // name
    rosidl_typesupport_introspection_c__ROS_TYPE_DOUBLE,  // type
    0,  // upper bound of string
    NULL,  // members of sub message
    true,  // is array
    9,  // array size
    false,  // is upper bound
    offsetof(fast_lio__msg__Pose6D, rot),  // bytes offset in struct
    NULL,  // default value
    fast_lio__msg__Pose6D__rosidl_typesupport_introspection_c__size_function__Pose6D__rot,  // size() function pointer
    fast_lio__msg__Pose6D__rosidl_typesupport_introspection_c__get_const_function__Pose6D__rot,  // get_const(index) function pointer
    fast_lio__msg__Pose6D__rosidl_typesupport_introspection_c__get_function__Pose6D__rot,  // get(index) function pointer
    fast_lio__msg__Pose6D__rosidl_typesupport_introspection_c__fetch_function__Pose6D__rot,  // fetch(index, &value) function pointer
    fast_lio__msg__Pose6D__rosidl_typesupport_introspection_c__assign_function__Pose6D__rot,  // assign(index, value) function pointer
    NULL  // resize(index) function pointer
  }
};

static const rosidl_typesupport_introspection_c__MessageMembers fast_lio__msg__Pose6D__rosidl_typesupport_introspection_c__Pose6D_message_members = {
  "fast_lio__msg",  // message namespace
  "Pose6D",  // message name
  6,  // number of fields
  sizeof(fast_lio__msg__Pose6D),
  fast_lio__msg__Pose6D__rosidl_typesupport_introspection_c__Pose6D_message_member_array,  // message members
  fast_lio__msg__Pose6D__rosidl_typesupport_introspection_c__Pose6D_init_function,  // function to initialize message memory (memory has to be allocated)
  fast_lio__msg__Pose6D__rosidl_typesupport_introspection_c__Pose6D_fini_function  // function to terminate message instance (will not free memory)
};

// this is not const since it must be initialized on first access
// since C does not allow non-integral compile-time constants
static rosidl_message_type_support_t fast_lio__msg__Pose6D__rosidl_typesupport_introspection_c__Pose6D_message_type_support_handle = {
  0,
  &fast_lio__msg__Pose6D__rosidl_typesupport_introspection_c__Pose6D_message_members,
  get_message_typesupport_handle_function,
};

ROSIDL_TYPESUPPORT_INTROSPECTION_C_EXPORT_fast_lio
const rosidl_message_type_support_t *
ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_introspection_c, fast_lio, msg, Pose6D)() {
  if (!fast_lio__msg__Pose6D__rosidl_typesupport_introspection_c__Pose6D_message_type_support_handle.typesupport_identifier) {
    fast_lio__msg__Pose6D__rosidl_typesupport_introspection_c__Pose6D_message_type_support_handle.typesupport_identifier =
      rosidl_typesupport_introspection_c__identifier;
  }
  return &fast_lio__msg__Pose6D__rosidl_typesupport_introspection_c__Pose6D_message_type_support_handle;
}
#ifdef __cplusplus
}
#endif
