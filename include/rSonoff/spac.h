#pragma once
#include <MsgPacketizer.h>

#define IR_POWER 0x1FE48B7
#define IR_MODE 0x1FE58A7
#define IR_MUTE 0x1FE7887
#define IR_EQ 0x1FE20DF
#define IR_1 0x1FE50AF
#define IR_2 0x1FED827
#define IR_3 0x1FEF807
#define IR_4 0x1FE30CF
#define IR_5 0x1FEB04F
#define IR_6 0x1FE708F
#define IR_7 0x1FE00FF
#define IR_8 0x1FEF00F
#define IR_9 0x1FE9867

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
    PKT_DIGIOUT_WRITE,   // pkt_digiout_write_t
    PKT_DIGIOUT_WRITES,  // pkt_digiout_writes_t
    PKT_DIGIOUT_EVENTS,  // pkt_digiout_events_t
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
} pkt_digiout_write_t;

typedef struct
{
    bool trigger;
    MsgPack::str_t states;
    MSGPACK_DEFINE(states, trigger);
} pkt_digiout_writes_t;

typedef struct
{
    pkt_device_id_t id;
    MsgPack::str_t data;
    MSGPACK_DEFINE(id, data);
} pkt_digiout_events_t;
