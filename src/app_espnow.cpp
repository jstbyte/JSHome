#include <app_common.h>
#include "app_espnow.h"

u8_t gatewayLinkType = GATEWAY_LINK_NA;
uint8_t espnow_gateway_mac_addr[6];

espnow_config_t loadEspnowConfig(String path)
{
    espnow_config_t config;
    StaticJsonDocument<128> configDoc;
    File configFile = LittleFS.open(path, "r");
    if (deserializeJson(configDoc, configFile) == DeserializationError::Ok)
    {
        config.channel = configDoc["channel"].as<u8_t>();
        config.timeout = configDoc["timeout"].as<u32_t>();
        config.gateway = configDoc["gateway"].as<String>();

        configFile.close();
        return config;
    }
    DEBUG_LOG_LN("ESPNOW Config DeserializationError");

    reBoot(1);
    return config;
}

void espnowRecvCallback(u8_t *mac, u8_t *payload, u8_t len)
{
    DEBUG_LOG_LN("---------ESPNOW Data Recived---------")
    MsgPacketizer::feed(payload, len);
}

void emmittEspNowEvent()
{
    if (gatewayLinkType == GATEWAY_LINK_STD_CONNECTED)
    {
        pkt_digi_out_data_pack_t dodpack;
        dodpack.chipId = ESP.getChipId();
        dodpack.data = digiOut.reads();
        const auto &packet = MsgPacketizer::encode(PKT_DIGI_OUT_DATA_PACK, dodpack);
        esp_now_send(espnow_gateway_mac_addr, (u8_t *)packet.data.data(), (u8_t)packet.data.size());
        DEBUG_LOG_LN("ESPNOW Event Emitted!")
    }

    if (gatewayLinkType == GATEWAY_LINK_UART_CONNECTED)
    {
        pkt_digi_out_data_pack_t dodpack;
        dodpack.chipId = ESP.getChipId();
        dodpack.data = digiOut.reads();
        MsgPacketizer::send(Serial, PKT_DIGI_OUT_DATA_PACK, dodpack);
        return;
    }
}

void sendRecoveryLinkRequest()
{
    pkt_gateway_link_t gateway;
    gateway.chipId = ESP.getChipId();
    gateway.macAddr = WiFi.macAddress();
    gateway.linkType = GATEWAY_LINK_DATA_REQUEST;
    const auto &packet = MsgPacketizer::encode(PKT_GATEWAY_LINK, gateway);
    esp_now_send(espnow_gateway_mac_addr, (u8_t *)packet.data.data(), packet.data.size());
    DEBUG_LOG_LN("ESPNOW Recovery Data Requested!");
}

void on_pkt_digi_out_data(pkt_digi_out_data_t dod)
{
    digiOut.write(dod.index, dod.state);
    if (dod.reply)
    {
        digiOut.start();
    }
    DEBUG_LOG_LN("ESPNOW PKT_DIGI_OUT_DATA Recived!")
}

void on_pkt_digi_out_data_pack(pkt_digi_out_data_pack_t dodpack)
{
    if (dodpack.chipId == ESP.getChipId())
    {
        digiOut.writes(dodpack.data);
    }

    DEBUG_LOG_LN("ESPNOW PKT_DIGI_OUT_DATA_PACK Recived!")
}

void on_pkt_gateway_link(pkt_gateway_link_t gateway)
{
    DEBUG_LOG("LINK Packet Recived: ( ");
    DEBUG_LOG(gateway.macAddr);
    DEBUG_LOG(" ) : ");
    DEBUG_LOG_LN(gateway.linkType);

    /* Protect From Redundent Execution */
    if (gateway.linkType == gatewayLinkType)
    {
        return;
    }

    /* Only Linked Gateway Can Be Connected To Internet */
    if (gateway.linkType == GATEWAY_LINK_STD_CONNECTED)
    {
        char mac[20];
        mac2str(espnow_gateway_mac_addr, mac);
        if (gateway.macAddr != mac)
        {
            return;
        }
        DEBUG_LOG_LN("ESPNOW Gateway Connected.");
    }

    if (gateway.linkType == GATEWAY_LINK_UART && gatewayLinkType != GATEWAY_LINK_STD_CONNECTED)
    {
        MsgPacketizer::subscribe(Serial, PKT_WIFI_BOOT, &reBoot);
        MsgPacketizer::subscribe(Serial, PKT_DIGI_OUT_DATA, &on_pkt_digi_out_data);
        MsgPacketizer::subscribe(Serial, PKT_DIGI_OUT_DATA_PACK, &on_pkt_digi_out_data_pack);
        MsgPacketizer::subscribe(
            Serial,
            PKT_GATEWAY_PIPE,
            [&](pkt_gateway_pipe_t pipe)
            {
                u8_t mac[6];
                str2mac(pipe.mac.c_str(), mac);
                esp_now_send(mac, pipe.payload.data(), pipe.payload.size());
            });

        MsgPacketizer::subscribe_manual(
            PKT_GATEWAY_PIPE,
            [&](pkt_gateway_pipe_t pipe)
            {
                Serial.write(pipe.payload.data(), pipe.payload.size());
            });
    }

    gatewayLinkType = gateway.linkType;
}

void setupEspNow(String path)
{
    auto config = loadEspnowConfig(path);
    str2mac(config.gateway.c_str(), espnow_gateway_mac_addr);

    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    esp_now_init();
    esp_now_set_self_role(ESP_NOW_ROLE_COMBO);
    esp_now_register_recv_cb(espnowRecvCallback);
    MsgPacketizer::subscribe_manual(PKT_WIFI_BOOT, &reBoot);
    MsgPacketizer::subscribe_manual(PKT_GATEWAY_LINK, &on_pkt_gateway_link);
    MsgPacketizer::subscribe_manual(PKT_DIGI_OUT_DATA, &on_pkt_digi_out_data);
    MsgPacketizer::subscribe_manual(PKT_DIGI_OUT_DATA_PACK, &on_pkt_digi_out_data_pack);
    analogWrite(LED_BUILTIN, 254);
    DEBUG_LOG_LN("ESPNOW Setup Complate")

#ifndef SERIAL_DEBUG_LOG
    if (rtcMemory.getData()->bootCount == 0)
    {
        sendRecoveryLinkRequest();
    }
#else
    sendRecoveryLinkRequest();
#endif

#ifndef SERIAL_DEBUG_LOG
    MsgPacketizer::subscribe(Serial, PKT_GATEWAY_LINK, &on_pkt_gateway_link);
#endif
}
