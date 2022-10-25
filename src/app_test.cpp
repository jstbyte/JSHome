#ifdef ESP_RCTRL_TEST
#include <DigiOut.h>
#include <app_espnow.h>
int step = 1;
bool testResults[] = {false, false};
DigiOut dout;
pkt_digi_out_data_pack_t dodpack;

void espnowCallback(u8_t *mac, u8_t *payload, u8_t len)
{
    Serial.println("ESPNOW Data Received!");
    MsgPacketizer::feed(payload, len);
}

void eventRecived(pkt_digi_out_data_pack_t swStates)
{
    Serial.print(step);
    Serial.print(" =>  ");
    Serial.print(dout.reads());
    Serial.print(" : ");
    Serial.println(swStates.data);

    testResults[step - 1] = dout.reads() == swStates.data;
    dodpack.chipId = swStates.chipId;
}

void booEventRecived(pkt_boot_signal_t bootSignal)
{
    Serial.print("Remote Device Boot Count : ");
    Serial.println(bootSignal.bootCount);
}

void setup()
{
    Serial.begin(9600);
    WiFi.mode(WIFI_STA);
    if (esp_now_init() == 0)
    {
        esp_now_set_self_role(ESP_NOW_ROLE_COMBO);
        esp_now_register_recv_cb(&espnowCallback);
        MsgPacketizer::subscribe_manual(PKT_DIGI_OUT_DATA_PACK, &eventRecived);
        MsgPacketizer::subscribe_manual(PKT_BOOT_SIGNAL, &booEventRecived);
        DEBUG_LOG("ESPNOW Setup Complate!");
    }

    u8_t initialPins[] = {5, 4, 14, 12};
    dout.load(initialPins, 4);
    dout.writes(HIGH);

    Serial.print("Initial States : ");
    Serial.println(dout.reads());

    {
        step = 1;
        dout.write(2, 0);
        pkt_digi_out_data_t dod;
        dod.index = 2;
        dod.state = 0;
        dod.reply = true;
        const auto &packet = MsgPacketizer::encode(PKT_DIGI_OUT_DATA, dod);
        Serial.println("Sending Data Step 1");
        esp_now_send(espnow_gateway_mac_addr, (u8_t *)packet.data.data(), packet.data.size());
        delay(1000);
    }

    {
        step = 2;
        dout.writes(1);
        dout.write(1, 0);
        dout.write(3, 0);
        dodpack.data = dout.reads();
        const auto &packet = MsgPacketizer::encode(PKT_DIGI_OUT_DATA_PACK, dodpack);
        esp_now_send(espnow_gateway_mac_addr, (u8_t *)packet.data.data(), packet.data.size());

        pkt_digi_out_data_t dod;
        dod.index = 24;
        dod.state = 0;
        dod.reply = true;
        const auto &packet_ = MsgPacketizer::encode(PKT_DIGI_OUT_DATA, dod);
        Serial.println("Sending Data Step 2");
        esp_now_send(espnow_gateway_mac_addr, (u8_t *)packet_.data.data(), packet_.data.size());
        delay(1000);
    }

    {
        const auto &packet = MsgPacketizer::encode(PKT_WIFI_BOOT, 30000);
        Serial.println("Sending Data for WIFI_BOOT");
        esp_now_send(espnow_gateway_mac_addr, (u8_t *)packet.data.data(), packet.data.size());
    }
    for (size_t i = 0; i < 2; i++)
    {
        Serial.print("Test ");
        Serial.print(i + 1);
        if (testResults[i])
        {
            Serial.println(" : PASSED");
        }
        else
        {
            Serial.println(" : FAILED");
        }
    }
}

void loop(){};

#endif