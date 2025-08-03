#include "emg_adc.hpp"
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(ble_svc, LOG_LEVEL_INF);

/* 128-bit base UUID: 3F5Bxxxx-D946-4844-B1CE-B29134DDEAF5 */
#define BT_UUID_EMG_SERVICE  BT_UUID_128_ENCODE(0x3F5B0001,0xD946,0x4844,0xB1CE,0xB29134DDEAF5)
#define BT_UUID_EMG_DATA_CH  BT_UUID_128_ENCODE(0x3F5B0002,0xD946,0x4844,0xB1CE,0xB29134DDEAF5)

static struct bt_conn *current_conn;
static uint8_t notify_enabled;

static ssize_t ccc_cfg(const bt_gatt_attr*, uint16_t, uint16_t flags, const void *value, uint16_t len)
{
    notify_enabled = bt_gatt_is_subscribed(current_conn, BT_GATT_CCC(value), BT_GATT_CCC_NOTIFY);
    return len;
}

BT_GATT_SERVICE_DEFINE(emg_svc,
    BT_GATT_PRIMARY_SERVICE(BT_UUID_EMG_SERVICE),
    BT_GATT_CHARACTERISTIC(BT_UUID_EMG_DATA_CH,
        BT_GATT_CHRC_NOTIFY,
        BT_GATT_PERM_NONE, NULL, NULL, NULL),
    BT_GATT_CCC(ccc_cfg, BT_GATT_PERM_READ | BT_GATT_PERM_WRITE),
);

static void connected(struct bt_conn *conn, uint8_t err)
{
    if (!err) current_conn = bt_conn_ref(conn);
}
static void disconnected(struct bt_conn *conn, uint8_t) { bt_conn_unref(current_conn); }

BT_CONN_CB_DEFINE(conn_cb) = {
    .connected = connected,
    .disconnected = disconnected,
};

void ble_service_init()
{
    bt_enable(nullptr);
    bt_le_adv_start(BT_LE_ADV_CONN_NAME, NULL, 0, NULL, 0);
    LOG_INF("BLE advertising");
}

/* Worker thread: pop frames and notify */
void ble_tx_thread()
{
    emg_frame_t *f;
    while (true) {
        f = (emg_frame_t*)k_fifo_get(&adc_fifo, K_FOREVER);
        if (notify_enabled && current_conn) {
            bt_gatt_notify(current_conn, &emg_svc.attrs[1],
                           f->sample, sizeof(f->sample));
        }
        k_free(f);
    }
}

K_THREAD_DEFINE(ble_tx_id, 1024, ble_tx_thread, NULL, NULL, NULL,
                5, 0, 0);
