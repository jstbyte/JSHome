#include <app_common.h>
#include "app_espnow.h"

u8_t gatewayStatus = 0;
uint8_t gatewayMac[6];

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
    if (gatewayStatus == GATEWAY_STATUS_STD_CONNECTED)
    {
        pkt_digiout_events_t digiOutEvent;
        digiOutEvent.id = ESP.getChipId();
        digiOutEvent.data = digiOut.reads();
        const auto &packet = MsgPacketizer::encode(PKT_DIGIOUT_EVENTS, digiOutEvent);
        esp_now_send(gatewayMac, (u8_t *)packet.data.data(), (u8_t)packet.data.size());
        DEBUG_LOG_LN("ESPNOW DigiOut Event Emitted!")
    }

    if (gatewayStatus == GATEWAY_STATUS_UART_CONNECTED)
    {
        pkt_digiout_events_t digiOutEvent;
        digiOutEvent.id = ESP.getChipId();
        digiOutEvent.data = digiOut.reads();
        MsgPacketizer::send(Serial, PKT_DIGIOUT_EVENTS, digiOutEvent);
        return;
    }
}

void regReq()
{
    pkt_device_info_t devInfo;
    devInfo.id = ESP.getChipId();
    devInfo.mac = WiFi.macAddress();
    const auto &packet = MsgPacketizer::encode(PKT_REGISTER_DEVICE, devInfo);
    esp_now_send(gatewayMac, (u8_t *)packet.data.data(), packet.data.size());
    DEBUG_LOG_LN("ESPNOW Send's Device Reg. Req.");
}

void onPktDigioutWrite(pkt_digiout_write_t data)
{
    digiOut.write(data.index, data.state);
    if (data.trigger)
    {
        digiOut.start();
    }
    DEBUG_LOG("ESPNOW DigiOut Write :: INDEX: ")
    DEBUG_LOG(data.index)
    DEBUG_LOG(" STATE:")
    DEBUG_LOG_LN(data.state)
}

void onPktDigioutWrites(pkt_digiout_writes_t data)
{
    digiOut.writes(data.states);
    if (data.trigger)
    {
        digiOut.start();
    }

    DEBUG_LOG("ESPNOW DigiOut Writes :")
    DEBUG_LOG_LN(data.states)
}

void onPktGatewayStatus(pkt_gateway_status_t status)
{
    gatewayStatus = status;
    DEBUG_LOG("ESPNOW Gateway Status Recived: ");
    DEBUG_LOG_LN(status);
}

/* UART Gateway Proxy Handlers */
void onRegReq(pkt_device_info_t devInfo)
{
    MsgPacketizer::send(Serial, PKT_REGISTER_DEVICE, devInfo);

    pkt_gateway_status_t status = GATEWAY_STATUS_STD;
    if (gatewayStatus == GATEWAY_STATUS_UART_CONNECTED)
    {
        status = GATEWAY_STATUS_STD_CONNECTED;
    }

    u8_t mac[6];
    str2mac(devInfo.mac.c_str(), mac);
    const auto &packet = MsgPacketizer::encode(PKT_GATEWAY_STATUS, status);
    esp_now_send(mac, (u8_t *)packet.data.data(), packet.data.size());
}

void onPktDigioutEvents(pkt_digiout_events_t event)
{
    MsgPacketizer::send(Serial, PKT_DIGIOUT_EVENTS, event);
}

void onPktUartGatewayStatus(pkt_gateway_status_t status)
{
    if (status == gatewayStatus)
    {
        return; /* Save Time & Memory */
    }

    /* Broadcast Address */
    u8_t mac[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
    const auto &packet = MsgPacketizer::encode(PKT_GATEWAY_STATUS, status);

    if (gatewayStatus == GATEWAY_STATUS_UART || gatewayStatus == GATEWAY_STATUS_UART_CONNECTED)
    {
        esp_now_send(mac, (u8_t *)packet.data.data(), packet.data.size());
        gatewayStatus = status;
        return;
    }
    gatewayStatus = status;

    MsgPacketizer::subscribe(Serial, PKT_WIFI_TIMEOUT, &reBoot);
    MsgPacketizer::subscribe(Serial, PKT_DIGIOUT_WRITE, &onPktDigioutWrite);
    MsgPacketizer::subscribe(Serial, PKT_DIGIOUT_WRITES, &onPktDigioutWrites);
    MsgPacketizer::subscribe(Serial, PKT_DIGIOUT_WRITES, &onPktDigioutWrites);

    MsgPacketizer::subscribe(
        Serial,
        PKT_GATEWAY_DATA_PIPE,
        [&](MsgPack::str_t mac, pkt_encoded_data_t data)
        {
            u8_t macAddr[6];
            str2mac(mac.c_str(), macAddr);
            esp_now_send(macAddr, data.data(), data.size());
        });

    /* GateWay Spacific Only */
    MsgPacketizer::subscribe_manual(PKT_DIGIOUT_EVENTS, &onPktDigioutEvents);
    MsgPacketizer::subscribe_manual(PKT_REGISTER_DEVICE, &onRegReq);
    MsgPacketizer::subscribe_manual(
        PKT_GATEWAY_DATA_PIPE,
        [&](pkt_encoded_data_t data)
        {
            Serial.write(data.data(), data.size());
        });

    esp_now_send(mac, (u8_t *)packet.data.data(), packet.data.size());
}

void setupEspNow(String path)
{
    auto config = loadEspnowConfig(path);
    str2mac(config.gateway.c_str(), gatewayMac);

    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    esp_now_init();
    esp_now_set_self_role(ESP_NOW_ROLE_COMBO);
    esp_now_register_recv_cb(espnowRecvCallback);
    /* Subscribe To Espnow Packets */
    MsgPacketizer::subscribe_manual(PKT_WIFI_TIMEOUT, &reBoot);
    MsgPacketizer::subscribe_manual(PKT_DIGIOUT_WRITE, &onPktDigioutWrite);
    MsgPacketizer::subscribe_manual(PKT_DIGIOUT_WRITES, &onPktDigioutWrites);
    MsgPacketizer::subscribe_manual(PKT_GATEWAY_STATUS, &onPktGatewayStatus);
    DEBUG_LOG_LN("ESPNOW Setup Complate")

#ifndef SERIAL_DEBUG_LOG
    MsgPacketizer::subscribe(Serial, PKT_GATEWAY_STATUS, &onPktUartGatewayStatus);
    if (rtcMemory.getData()->bootCount == 0)
    {
        regReq();
    }
#else
    regReq();
#endif
}
