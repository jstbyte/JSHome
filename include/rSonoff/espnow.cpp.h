#include "shared.h"
#include <espnow.h>

u8_t gatewayStatus = 0;
uint8_t gatewayMac[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

void espnowRecvCallback(u8_t *mac, u8_t *payload, u8_t len)
{
    DEBUG_LOG_LN("---------ESPNOW Data Recived---------")
    MsgPacketizer::feed(payload, len);
}

void emmittEspNowEvent()
{
    if (gatewayStatus == GATEWAY_STATUS_STD_CONNECTED)
    {
        pkt_sonoff_events_t sonoffEvent;
        sonoffEvent.id = ESP.getChipId();
        sonoffEvent.data = Sonoffe::reads();
        const auto &packet = MsgPacketizer::encode(PKT_SONOFF_EVENTS, sonoffEvent);
        esp_now_send(gatewayMac, (u8_t *)packet.data.data(), (u8_t)packet.data.size());
        DEBUG_LOG_LN("ESPNOW: Sonoff event emitted.")
    }

    if (gatewayStatus == GATEWAY_STATUS_UART_CONNECTED)
    {
        pkt_sonoff_events_t sonoffEvent;
        sonoffEvent.id = ESP.getChipId();
        sonoffEvent.data = Sonoffe::reads();
        MsgPacketizer::send(Serial, PKT_SONOFF_EVENTS, sonoffEvent);
        return;
    }
}

void regReq()
{
    if (ConnMan::data()->bootCount > 0)
        return;

    pkt_device_info_t devInfo;
    devInfo.id = ESP.getChipId();
    devInfo.mac = WiFi.macAddress();
    const auto &packet = MsgPacketizer::encode(PKT_REGISTER_DEVICE, devInfo);
    esp_now_send(gatewayMac, (u8_t *)packet.data.data(), packet.data.size());
    DEBUG_LOG_LN("ESPNOW: Device reg. req. sent.");
}

void onPktSonoffWrite(pkt_sonoff_write_t data)
{
    DEBUG_LOG("Sonoff write :: INDEX: ")
    DEBUG_LOG(data.index)
    DEBUG_LOG(" STATE:")
    DEBUG_LOG_LN(data.state)
    Sonoffe::writer(data.index, data.state);
}

void onPktSonoffWrites(pkt_sonoff_writes_t data)
{
    Sonoffe::writes(data.states);
    if (data.trigger)
    {
        Sonoffe::event.start();
    }

    DEBUG_LOG("Sonoff writes :")
    DEBUG_LOG_LN(data.states)
}

void onPktGatewayStatus(pkt_gateway_status_t status)
{
    gatewayStatus = status;
    DEBUG_LOG("ESPNOW: Gateway status recived: ");
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

void onPktSonoffEvents(pkt_sonoff_events_t event)
{
    MsgPacketizer::send(Serial, PKT_SONOFF_EVENTS, event);
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

    MsgPacketizer::subscribe(Serial, PKT_WIFI_TIMEOUT, [](u32_t timeout)
                             { ConnMan::reboot(timeout, Sonoffe::pins(), Sonoffe::count()); });
    MsgPacketizer::subscribe(Serial, PKT_SONOFF_WRITE, &onPktSonoffWrite);
    MsgPacketizer::subscribe(Serial, PKT_SONOFF_WRITES, &onPktSonoffWrites);
    MsgPacketizer::subscribe(Serial, PKT_SONOFF_WRITES, &onPktSonoffWrites);

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
    MsgPacketizer::subscribe_manual(PKT_SONOFF_EVENTS, &onPktSonoffEvents);
    MsgPacketizer::subscribe_manual(PKT_REGISTER_DEVICE, &onRegReq);
    MsgPacketizer::subscribe_manual(
        PKT_GATEWAY_DATA_PIPE,
        [&](pkt_encoded_data_t data)
        {
            Serial.write(data.data(), data.size());
        });

    esp_now_send(mac, (u8_t *)packet.data.data(), packet.data.size());
}

void setupEspNow()
{
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    esp_now_init();
    esp_now_set_self_role(ESP_NOW_ROLE_COMBO);
    esp_now_register_recv_cb(espnowRecvCallback);
    /* Subscribe To Espnow Packets */
    MsgPacketizer::subscribe_manual(PKT_WIFI_TIMEOUT, [](u32_t timeout)
                                    { ConnMan::reboot(timeout, Sonoffe::pins(), Sonoffe::count()); });
    MsgPacketizer::subscribe_manual(PKT_SONOFF_WRITE, &onPktSonoffWrite);
    MsgPacketizer::subscribe_manual(PKT_SONOFF_WRITES, &onPktSonoffWrites);
    MsgPacketizer::subscribe_manual(PKT_GATEWAY_STATUS, &onPktGatewayStatus);
    DEBUG_LOG_LN("ESPNOW: Initialized.")
    regReq(); /* Send Gateway Registration Request. */

#ifndef SERIAL_DEBUG_LOG
    MsgPacketizer::subscribe(Serial, PKT_GATEWAY_STATUS, &onPktUartGatewayStatus);
#endif
}
