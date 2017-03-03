/* Generated by the protocol buffer compiler.  DO NOT EDIT! */
/* Generated from: timing_packet.proto */

/* Do not generate deprecated warnings for self */
#ifndef PROTOBUF_C__NO_DEPRECATED
#define PROTOBUF_C__NO_DEPRECATED
#endif

#include "timing_packet.pb-c.h"
void   aero_network__ptp1588_timing_packet__init
                     (AeroNetwork__Ptp1588TimingPacket         *message)
{
  static AeroNetwork__Ptp1588TimingPacket init_value = AERO_NETWORK__PTP1588_TIMING_PACKET__INIT;
  *message = init_value;
}
size_t aero_network__ptp1588_timing_packet__get_packed_size
                     (const AeroNetwork__Ptp1588TimingPacket *message)
{
  assert(message->base.descriptor == &aero_network__ptp1588_timing_packet__descriptor);
  return protobuf_c_message_get_packed_size ((const ProtobufCMessage*)(message));
}
size_t aero_network__ptp1588_timing_packet__pack
                     (const AeroNetwork__Ptp1588TimingPacket *message,
                      uint8_t       *out)
{
  assert(message->base.descriptor == &aero_network__ptp1588_timing_packet__descriptor);
  return protobuf_c_message_pack ((const ProtobufCMessage*)message, out);
}
size_t aero_network__ptp1588_timing_packet__pack_to_buffer
                     (const AeroNetwork__Ptp1588TimingPacket *message,
                      ProtobufCBuffer *buffer)
{
  assert(message->base.descriptor == &aero_network__ptp1588_timing_packet__descriptor);
  return protobuf_c_message_pack_to_buffer ((const ProtobufCMessage*)message, buffer);
}
AeroNetwork__Ptp1588TimingPacket *
       aero_network__ptp1588_timing_packet__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data)
{
  return (AeroNetwork__Ptp1588TimingPacket *)
     protobuf_c_message_unpack (&aero_network__ptp1588_timing_packet__descriptor,
                                allocator, len, data);
}
void   aero_network__ptp1588_timing_packet__free_unpacked
                     (AeroNetwork__Ptp1588TimingPacket *message,
                      ProtobufCAllocator *allocator)
{
  assert(message->base.descriptor == &aero_network__ptp1588_timing_packet__descriptor);
  protobuf_c_message_free_unpacked ((ProtobufCMessage*)message, allocator);
}
static const ProtobufCFieldDescriptor aero_network__ptp1588_timing_packet__field_descriptors[18] =
{
  {
    "mac_address",
    1,
    PROTOBUF_C_LABEL_OPTIONAL,
    PROTOBUF_C_TYPE_INT64,
    offsetof(AeroNetwork__Ptp1588TimingPacket, has_mac_address),
    offsetof(AeroNetwork__Ptp1588TimingPacket, mac_address),
    NULL,
    NULL,
    0,             /* flags */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
  {
    "sync_receive_s",
    2,
    PROTOBUF_C_LABEL_OPTIONAL,
    PROTOBUF_C_TYPE_INT32,
    offsetof(AeroNetwork__Ptp1588TimingPacket, has_sync_receive_s),
    offsetof(AeroNetwork__Ptp1588TimingPacket, sync_receive_s),
    NULL,
    NULL,
    0,             /* flags */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
  {
    "sync_receive_ns",
    3,
    PROTOBUF_C_LABEL_OPTIONAL,
    PROTOBUF_C_TYPE_INT32,
    offsetof(AeroNetwork__Ptp1588TimingPacket, has_sync_receive_ns),
    offsetof(AeroNetwork__Ptp1588TimingPacket, sync_receive_ns),
    NULL,
    NULL,
    0,             /* flags */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
  {
    "delay_request_send_s",
    4,
    PROTOBUF_C_LABEL_OPTIONAL,
    PROTOBUF_C_TYPE_INT32,
    offsetof(AeroNetwork__Ptp1588TimingPacket, has_delay_request_send_s),
    offsetof(AeroNetwork__Ptp1588TimingPacket, delay_request_send_s),
    NULL,
    NULL,
    0,             /* flags */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
  {
    "delay_request_send_ns",
    5,
    PROTOBUF_C_LABEL_OPTIONAL,
    PROTOBUF_C_TYPE_INT32,
    offsetof(AeroNetwork__Ptp1588TimingPacket, has_delay_request_send_ns),
    offsetof(AeroNetwork__Ptp1588TimingPacket, delay_request_send_ns),
    NULL,
    NULL,
    0,             /* flags */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
  {
    "delay_request_receive_s",
    6,
    PROTOBUF_C_LABEL_OPTIONAL,
    PROTOBUF_C_TYPE_INT32,
    offsetof(AeroNetwork__Ptp1588TimingPacket, has_delay_request_receive_s),
    offsetof(AeroNetwork__Ptp1588TimingPacket, delay_request_receive_s),
    NULL,
    NULL,
    0,             /* flags */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
  {
    "delay_request_receive_ns",
    7,
    PROTOBUF_C_LABEL_OPTIONAL,
    PROTOBUF_C_TYPE_INT32,
    offsetof(AeroNetwork__Ptp1588TimingPacket, has_delay_request_receive_ns),
    offsetof(AeroNetwork__Ptp1588TimingPacket, delay_request_receive_ns),
    NULL,
    NULL,
    0,             /* flags */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
  {
    "offset_from_master_s",
    8,
    PROTOBUF_C_LABEL_OPTIONAL,
    PROTOBUF_C_TYPE_INT32,
    offsetof(AeroNetwork__Ptp1588TimingPacket, has_offset_from_master_s),
    offsetof(AeroNetwork__Ptp1588TimingPacket, offset_from_master_s),
    NULL,
    NULL,
    0,             /* flags */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
  {
    "offset_from_master_ns",
    9,
    PROTOBUF_C_LABEL_OPTIONAL,
    PROTOBUF_C_TYPE_INT32,
    offsetof(AeroNetwork__Ptp1588TimingPacket, has_offset_from_master_ns),
    offsetof(AeroNetwork__Ptp1588TimingPacket, offset_from_master_ns),
    NULL,
    NULL,
    0,             /* flags */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
  {
    "mean_path_delay_s",
    10,
    PROTOBUF_C_LABEL_OPTIONAL,
    PROTOBUF_C_TYPE_INT32,
    offsetof(AeroNetwork__Ptp1588TimingPacket, has_mean_path_delay_s),
    offsetof(AeroNetwork__Ptp1588TimingPacket, mean_path_delay_s),
    NULL,
    NULL,
    0,             /* flags */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
  {
    "mean_path_delay_ns",
    11,
    PROTOBUF_C_LABEL_OPTIONAL,
    PROTOBUF_C_TYPE_INT32,
    offsetof(AeroNetwork__Ptp1588TimingPacket, has_mean_path_delay_ns),
    offsetof(AeroNetwork__Ptp1588TimingPacket, mean_path_delay_ns),
    NULL,
    NULL,
    0,             /* flags */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
  {
    "master_to_slave_delay_s",
    12,
    PROTOBUF_C_LABEL_OPTIONAL,
    PROTOBUF_C_TYPE_INT32,
    offsetof(AeroNetwork__Ptp1588TimingPacket, has_master_to_slave_delay_s),
    offsetof(AeroNetwork__Ptp1588TimingPacket, master_to_slave_delay_s),
    NULL,
    NULL,
    0,             /* flags */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
  {
    "master_to_slave_delay_ns",
    13,
    PROTOBUF_C_LABEL_OPTIONAL,
    PROTOBUF_C_TYPE_INT32,
    offsetof(AeroNetwork__Ptp1588TimingPacket, has_master_to_slave_delay_ns),
    offsetof(AeroNetwork__Ptp1588TimingPacket, master_to_slave_delay_ns),
    NULL,
    NULL,
    0,             /* flags */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
  {
    "slave_to_master_delay_s",
    14,
    PROTOBUF_C_LABEL_OPTIONAL,
    PROTOBUF_C_TYPE_INT32,
    offsetof(AeroNetwork__Ptp1588TimingPacket, has_slave_to_master_delay_s),
    offsetof(AeroNetwork__Ptp1588TimingPacket, slave_to_master_delay_s),
    NULL,
    NULL,
    0,             /* flags */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
  {
    "slave_to_master_delay_ns",
    15,
    PROTOBUF_C_LABEL_OPTIONAL,
    PROTOBUF_C_TYPE_INT32,
    offsetof(AeroNetwork__Ptp1588TimingPacket, has_slave_to_master_delay_ns),
    offsetof(AeroNetwork__Ptp1588TimingPacket, slave_to_master_delay_ns),
    NULL,
    NULL,
    0,             /* flags */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
  {
    "port_state",
    16,
    PROTOBUF_C_LABEL_OPTIONAL,
    PROTOBUF_C_TYPE_INT32,
    offsetof(AeroNetwork__Ptp1588TimingPacket, has_port_state),
    offsetof(AeroNetwork__Ptp1588TimingPacket, port_state),
    NULL,
    NULL,
    0,             /* flags */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
  {
    "experiment_id",
    17,
    PROTOBUF_C_LABEL_OPTIONAL,
    PROTOBUF_C_TYPE_STRING,
    0,   /* quantifier_offset */
    offsetof(AeroNetwork__Ptp1588TimingPacket, experiment_id),
    NULL,
    NULL,
    0,             /* flags */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
  {
    "time_bucket",
    18,
    PROTOBUF_C_LABEL_OPTIONAL,
    PROTOBUF_C_TYPE_INT64,
    offsetof(AeroNetwork__Ptp1588TimingPacket, has_time_bucket),
    offsetof(AeroNetwork__Ptp1588TimingPacket, time_bucket),
    NULL,
    NULL,
    0,             /* flags */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
};
static const unsigned aero_network__ptp1588_timing_packet__field_indices_by_name[] = {
  6,   /* field[6] = delay_request_receive_ns */
  5,   /* field[5] = delay_request_receive_s */
  4,   /* field[4] = delay_request_send_ns */
  3,   /* field[3] = delay_request_send_s */
  16,   /* field[16] = experiment_id */
  0,   /* field[0] = mac_address */
  12,   /* field[12] = master_to_slave_delay_ns */
  11,   /* field[11] = master_to_slave_delay_s */
  10,   /* field[10] = mean_path_delay_ns */
  9,   /* field[9] = mean_path_delay_s */
  8,   /* field[8] = offset_from_master_ns */
  7,   /* field[7] = offset_from_master_s */
  15,   /* field[15] = port_state */
  14,   /* field[14] = slave_to_master_delay_ns */
  13,   /* field[13] = slave_to_master_delay_s */
  2,   /* field[2] = sync_receive_ns */
  1,   /* field[1] = sync_receive_s */
  17,   /* field[17] = time_bucket */
};
static const ProtobufCIntRange aero_network__ptp1588_timing_packet__number_ranges[1 + 1] =
{
  { 1, 0 },
  { 0, 18 }
};
const ProtobufCMessageDescriptor aero_network__ptp1588_timing_packet__descriptor =
{
  PROTOBUF_C__MESSAGE_DESCRIPTOR_MAGIC,
  "aero_network.Ptp1588TimingPacket",
  "Ptp1588TimingPacket",
  "AeroNetwork__Ptp1588TimingPacket",
  "aero_network",
  sizeof(AeroNetwork__Ptp1588TimingPacket),
  18,
  aero_network__ptp1588_timing_packet__field_descriptors,
  aero_network__ptp1588_timing_packet__field_indices_by_name,
  1,  aero_network__ptp1588_timing_packet__number_ranges,
  (ProtobufCMessageInit) aero_network__ptp1588_timing_packet__init,
  NULL,NULL,NULL    /* reserved[123] */
};
