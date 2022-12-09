#pragma once
#include <MsgPacketizer.h>

enum GATEWAY_STATUS
{
    GATEWAY_STATUS_NIL,
    GATEWAY_STATUS_STD,
    GATEWAY_STATUS_UART,
    GATEWAY_STATUS_STD_CONNECTED,
    GATEWAY_STATUS_UART_CONNECTED
};

enum PACKET_INDEX_TYPES
{
    PKT_NIL,             // Not Sure!;
    PKT_WIFI_TIMEOUT,    // pkt_wifi_timeout_t
    PKT_SONOFF_WRITE,    // pkt_sonoff_write_t
    PKT_SONOFF_WRITES,   // pkt_sonoff_writes_t
    PKT_SONOFF_EVENTS,   // pkt_sonoff_events_t
    PKT_GATEWAY_STATUS,  // pkt_gateway_status_t
    PKT_REGISTER_DEVICE, // pkt_device_info_t
    /* Extra Packets */
    PKT_GATEWAY_DATA_PIPE, // pkt_encoded_data_t
};

typedef u32_t pkt_device_id_t;
typedef u8_t pkt_wifi_timeout_t;
typedef u8_t pkt_gateway_status_t;
/* MsgPacketizer::encoded >  Data */;
typedef MsgPack::arr_t<u8_t> pkt_encoded_data_t;

typedef struct
{
    pkt_device_id_t id;
    MsgPack::str_t mac;
    MSGPACK_DEFINE(id, mac);
} pkt_device_info_t;

typedef struct
{
    u8_t index;
    u8_t state;
    MSGPACK_DEFINE(index, state);
} pkt_sonoff_write_t;

typedef struct
{
    bool trigger;
    MsgPack::str_t states;
    MSGPACK_DEFINE(states, trigger);
} pkt_sonoff_writes_t;

typedef struct
{
    pkt_device_id_t id;
    MsgPack::str_t data;
    MSGPACK_DEFINE(id, data);
} pkt_sonoff_events_t;
