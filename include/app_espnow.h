#pragma once
#include <espnow.h>
#include <ESP8266WiFi.h>
#include <MsgPacketizer.h>

extern uint8_t espnow_gateway_mac_addr[6];

typedef struct
{
    u8_t channel;
    u32_t timeout;
    String gateway;
} espnow_config_t;

enum GATEWAY_LINK_TYPES
{
    GATEWAY_LINK_NA,
    GATEWAY_LINK_STD,
    GATEWAY_LINK_UART,
    GATEWAY_LINK_DATA_REQUEST,
    GATEWAY_LINK_STD_CONNECTED,
    GATEWAY_LINK_UART_CONNECTED
};

// MsgPacketizer Packet Types Index;
enum PACKET_TYPES_INDEX
{
    PKT_NULL,
    PKT_WIFI_BOOT,
    PKT_GATEWAY_LINK,
    PKT_GATEWAY_PIPE,
    PKT_DIGI_OUT_DATA,
    PKT_DIGI_OUT_DATA_PACK,
};

typedef u32_t pkt_wifi_boot_t;
typedef u8_t pkt_gateway_find_t;

typedef struct
{
    u32_t chipId;
    u8_t linkType;
    MsgPack::str_t macAddr;
    MSGPACK_DEFINE(chipId, linkType, macAddr);
} pkt_gateway_link_t;

typedef struct
{
    uint8_t index;
    uint8_t state;
    uint8_t reply;
    MSGPACK_DEFINE(index, state, reply);
} pkt_digi_out_data_t;

typedef struct
{
    uint32_t chipId;
    MsgPack::str_t data;
    MSGPACK_DEFINE(chipId, data);
} pkt_digi_out_data_pack_t;

typedef struct
{
    MsgPack::str_t mac;
    MsgPack::arr_t<u8_t> payload;
    MSGPACK_DEFINE(mac, payload);
} pkt_gateway_pipe_t;

void emmittEspNowEvent();
void setupEspNow(String path);
espnow_config_t loadEspnowConfig(String path);