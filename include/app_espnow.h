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

// MsgPacketizer Packet Types Index;
enum PACKET_TYPES_INDEX
{
    PKT_NULL,
    PKT_WIFI_BOOT,
    PKT_BOOT_SIGNAL,
    PKT_DIGI_OUT_DATA,
    PKT_DIGI_OUT_DATA_PACK
};

typedef u32_t pkt_wifi_boot_t;

typedef struct
{
    u32_t chipId;
    u8_t bootCount;
    MsgPack::str_t macAddr;
    MSGPACK_DEFINE(chipId, bootCount, macAddr);
} pkt_boot_signal_t;

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

void emmittEspNowEvent();
void setupEspNow(String path);
espnow_config_t loadEspnowConfig(String path);