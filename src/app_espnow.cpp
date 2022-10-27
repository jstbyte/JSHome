#include <app_common.h>
#include "app_espnow.h"

uint8_t espnow_gateway_mac_addr[6];

void espnowRecvCallback(u8_t *mac, u8_t *payload, u8_t len)
{
    DEBUG_LOG_LN("ESPNOW Data Recived!")
    MsgPacketizer::feed(payload, len);
}

void emmittEspNowEvent()
{
    pkt_digi_out_data_pack_t dodpack;
    dodpack.chipId = ESP.getChipId();
    dodpack.data = digiOut.reads();
    const auto &packet = MsgPacketizer::encode(PKT_DIGI_OUT_DATA_PACK, dodpack);
    esp_now_send(espnow_gateway_mac_addr, (u8_t *)packet.data.data(), (u8_t)packet.data.size());
    DEBUG_LOG_LN("ESPNOW Event Emitted!")
    return;
}

void emmittBootSignal(u8_t bootCount)
{
    pkt_boot_signal_t bootSignal = {.bootCount = bootCount};
    bootSignal.chipId = ESP.getChipId();
    bootSignal.macAddr = WiFi.macAddress();
    const auto &packet = MsgPacketizer::encode(PKT_BOOT_SIGNAL, bootSignal);
    DEBUG_LOG_LN("ESPNOW BOOT Signal Emitted!");
    esp_now_send(espnow_gateway_mac_addr, (u8_t *)packet.data.data(), packet.data.size());
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
    MsgPacketizer::subscribe_manual(PKT_DIGI_OUT_DATA, &on_pkt_digi_out_data);
    MsgPacketizer::subscribe_manual(PKT_DIGI_OUT_DATA_PACK, &on_pkt_digi_out_data_pack);
    analogWrite(LED_BUILTIN, 254);
    DEBUG_LOG_LN("ESPNOW Setup Complate")

    if (rtcMemory.getData()->bootCount == 0)
    {
        emmittBootSignal(0);
    }
}
