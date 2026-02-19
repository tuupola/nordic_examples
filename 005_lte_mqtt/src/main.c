#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <modem/lte_lc.h>
#include <modem/nrf_modem_lib.h>
#include <net/mqtt_helper.h>

LOG_MODULE_REGISTER(main, LOG_LEVEL_INF);

#define MQTT_BROKER_HOSTNAME "broker.emqx.io"
#define MQTT_CLIENT_ID "thingy91x"
#define MQTT_PUB_TOPIC "thingy91x/hello"
#define MQTT_SUB_TOPIC "thingy91x/command"

static K_SEM_DEFINE(mqtt_connected_sem, 0, 1);

static void on_connack(enum mqtt_conn_return_code return_code, bool session_present)
{
    if (return_code != MQTT_CONNECTION_ACCEPTED) {
        LOG_ERR("MQTT connection rejected: %d", return_code);
        return;
    }
    LOG_INF("MQTT connected.");
    /* Allow main() to continue by releaseing the semaphore. */
    k_sem_give(&mqtt_connected_sem);
}

static void on_disconnect(int result)
{
    LOG_INF("MQTT disconnected: %d", result);
}

static void on_publish(struct mqtt_helper_buf topic, struct mqtt_helper_buf payload)
{
    LOG_INF("Received: %.*s on topic: %.*s",
        payload.size, payload.ptr,
        topic.size, topic.ptr
    );
}

static void on_suback(uint16_t message_id, int result)
{
    if (result == 0) {
        LOG_INF("Subscribed (msg id %d)", message_id);
    } else {
        LOG_ERR("Subscribe failed: %d", result);
    }
}

int main(void)
{
    int rc;

    LOG_INF("Initializing the modem firmware.");
    rc = nrf_modem_lib_init();
    if (rc < 0) {
        LOG_ERR("NRF model lib init failed: %d", rc);
        return rc;
    }

    /* This is blocking and can take up to 30 seconds. */
    LOG_INF("Connecting to LTE network.");
    rc = lte_lc_connect();
    if (rc < 0) {
        LOG_ERR("LTE lc connect failed: %d", rc);
        return rc;
    }
    LOG_INF("Connected to LTE network.");

    /* Initialize MQTT helper with callbacks. */
    struct mqtt_helper_cfg cfg = {
        .cb = {
            .on_connack = on_connack,
            .on_disconnect = on_disconnect,
            .on_publish = on_publish,
            .on_suback = on_suback,
        },
    };

    rc = mqtt_helper_init(&cfg);
    if (rc < 0) {
        LOG_ERR("MQTT helper init failed: %d", rc);
        return rc;
    }

    /* Connect to the MQTT broker. */
    struct mqtt_helper_conn_params conn_params = {
        .hostname.ptr = MQTT_BROKER_HOSTNAME,
        .hostname.size = strlen(MQTT_BROKER_HOSTNAME),
        .device_id.ptr = MQTT_CLIENT_ID,
        .device_id.size = strlen(MQTT_CLIENT_ID),
    };

    rc = mqtt_helper_connect(&conn_params);
    if (rc < 0) {
        LOG_ERR("MQTT helper connect failed: %d", rc);
        return rc;
    }

    /* Wait for CONNACK from broker. */
    rc = k_sem_take(&mqtt_connected_sem, K_SECONDS(30));
    if (rc < 0) {
        LOG_ERR("MQTT connect timed out");
        return rc;
    }

    /* Subscribe to a topic. */
    struct mqtt_topic topics[] = {
        {
            .topic.utf8 = MQTT_SUB_TOPIC,
            .topic.size = strlen(MQTT_SUB_TOPIC),
        },
    };
    struct mqtt_subscription_list sub_list = {
        .list = topics,
        .list_count = ARRAY_SIZE(topics),
        .message_id = mqtt_helper_msg_id_get(),
    };

    rc = mqtt_helper_subscribe(&sub_list);
    if (rc < 0) {
        LOG_ERR("MQTT helper subscribe failed: %d", rc);
    }

    /* Publish a message every 10 seconds. */
    char payload[64];
    int count = 0;

    while (1) {
        snprintk(payload, sizeof(payload), "Hello #%d", count++);

        struct mqtt_publish_param param = {
            .message.payload.data = payload,
            .message.payload.len = strlen(payload),
            .message.topic.qos = MQTT_QOS_1_AT_LEAST_ONCE,
            .message_id = mqtt_helper_msg_id_get(),
            .message.topic.topic.utf8 = MQTT_PUB_TOPIC,
            .message.topic.topic.size = strlen(MQTT_PUB_TOPIC),
        };

        rc = mqtt_helper_publish(&param);
        if (rc < 0) {
            LOG_ERR("MQTT helper publish failed: %d", rc);
        } else {
            LOG_INF("Published: %s", payload);
        }

        k_sleep(K_SECONDS(10));
    }
    return 0;
}
