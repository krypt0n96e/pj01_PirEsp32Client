#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "protocol_examples_common.h"
#include "esp_http_client.h"
#include "esp_adc/adc_oneshot.h"

#define EXAMPLE_ADC1_CHAN0 ADC_CHANNEL_6
#define DEVICE_ID 1
#define HOST "http://192.168.104.79:8888"
#define MAX_HTTP_OUTPUT_BUFFER 256
#define MAX_POST_SIZE 4096
#define TEMP_SIZE 10
#define VALUE_PER_POST 200
#define TASK0_DELAY 1000
#define TASK1_DELAY 1000
#define TASK2_DELAY 20

static const char *TAG_HTTP_POST = "HTTP_POST";
static const char *TAG_HTTP_GET = "HTTP_GET";
static const char *TAG_ADC = "ONE_SHOT_ADC";

TaskHandle_t task_adc_oneshot_write = NULL;
TaskHandle_t task_http_get_data = NULL;
TaskHandle_t task_http_post_data = NULL;

static uint8_t logs = 0;
static uint8_t writeStage = 0;
static uint8_t tempPostIndex;
static char temp[TEMP_SIZE][MAX_POST_SIZE];

// Function prototypes
static esp_err_t _http_event_handler(esp_http_client_event_t *evt);
int http_post_json_handle(char *post_data);
void http_get_handle();
void oneshot_adc_read(int *value);
void http_post_data(void *pvParameters);
void http_get_data(void *pvParameters);
void adc_oneshot_write(void *pvParameters);

void cleanup_and_exit(esp_http_client_handle_t client, char *url, char *output_buffer)
{
    if (client)
    {
        esp_http_client_cleanup(client);
    }
    if (url)
    {
        vPortFree(url);
    }
    if (output_buffer)
    {
        vPortFree(output_buffer);
    }
}

static esp_err_t _http_event_handler(esp_http_client_event_t *evt)
{
    switch (evt->event_id)
    {
    case HTTP_EVENT_ERROR:
        ESP_LOGE(TAG_HTTP_POST, "HTTP_EVENT_ERROR");
        break;
    case HTTP_EVENT_ON_CONNECTED:
        ESP_LOGI(TAG_HTTP_POST, "HTTP_EVENT_ON_CONNECTED");
        break;
    case HTTP_EVENT_HEADER_SENT:
        ESP_LOGI(TAG_HTTP_POST, "HTTP_EVENT_HEADER_SENT");
        break;
    case HTTP_EVENT_ON_HEADER:
        ESP_LOGI(TAG_HTTP_POST, "HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key, evt->header_value);
        break;
    case HTTP_EVENT_ON_DATA:
        ESP_LOGI(TAG_HTTP_POST, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
        break;
    case HTTP_EVENT_ON_FINISH:
        ESP_LOGI(TAG_HTTP_POST, "HTTP_EVENT_ON_FINISH");
        break;
    case HTTP_EVENT_DISCONNECTED:
        ESP_LOGI(TAG_HTTP_POST, "HTTP_EVENT_DISCONNECTED");
        break;
    case HTTP_EVENT_REDIRECT:
        ESP_LOGI(TAG_HTTP_POST, "HTTP_EVENT_REDIRECT");
        break;
    }
    return ESP_OK;
}

int http_post_json_handle(char *post_data)
{
    esp_http_client_handle_t client = esp_http_client_init(&(esp_http_client_config_t){
        .url = "http://192.168.104.79:8888/esp",
        .event_handler = _http_event_handler,
        .method = HTTP_METHOD_POST,
    });

    if (!client)
    {
        ESP_LOGE(TAG_HTTP_POST, "Failed to initialize HTTP client");
        return 1;
    }

    esp_http_client_set_header(client, "Content-Type", "application/json");
    esp_http_client_set_post_field(client, post_data, strlen(post_data));

    esp_err_t err = esp_http_client_perform(client);
    if (err == ESP_OK)
    {
        ESP_LOGI(TAG_HTTP_POST, "HTTP POST Status = %d, content_length = %lld",
                 esp_http_client_get_status_code(client),
                 esp_http_client_get_content_length(client));
    }
    else
    {
        ESP_LOGE(TAG_HTTP_POST, "HTTP POST request failed: %s", esp_err_to_name(err));
    }

    cleanup_and_exit(client, NULL, NULL);
    return (err == ESP_OK) ? 0 : 1;
}

void http_get_handle()
{
    char *url = (char *)pvPortMalloc(40);
    if (!url)
    {
        ESP_LOGE(TAG_HTTP_GET, "Failed to allocate memory for URL");
        return;
    }

    char *output_buffer = (char *)pvPortMalloc(MAX_HTTP_OUTPUT_BUFFER + 1);
    if (!output_buffer)
    {
        ESP_LOGE(TAG_HTTP_GET, "Failed to allocate memory for output buffer");
        vPortFree(url);
        return;
    }

    snprintf(url, 40, "%s/device?id=%d", HOST, DEVICE_ID);

    esp_http_client_config_t config = {
        .url = url,
        .method = HTTP_METHOD_GET,
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);

    if (!client)
    {
        ESP_LOGE(TAG_HTTP_GET, "Failed to initialize HTTP client");
        cleanup_and_exit(NULL, url, output_buffer);
        return;
    }

    esp_err_t err = esp_http_client_open(client, 0);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG_HTTP_GET, "Failed to open HTTP connection: %s", esp_err_to_name(err));
    }
    else
    {
        int content_length = esp_http_client_fetch_headers(client);
        if (content_length < 0)
        {
            ESP_LOGE(TAG_HTTP_GET, "HTTP client fetch headers failed");
        }
        else
        {
            int data_read = esp_http_client_read_response(client, output_buffer, MAX_HTTP_OUTPUT_BUFFER);
            if (data_read >= 0)
            {
                ESP_LOGI(TAG_HTTP_GET, "HTTP GET Status = %d, content_length = %" PRId64,
                         esp_http_client_get_status_code(client),
                         esp_http_client_get_content_length(client));

                logs = output_buffer[23] - '0';
            }
            else
            {
                ESP_LOGE(TAG_HTTP_GET, "Failed to read response");
            }
        }
    }

    cleanup_and_exit(client, url, output_buffer);
}

void oneshot_adc_read(int *value)
{
    adc_oneshot_unit_handle_t adc1_handle;
    adc_oneshot_unit_init_cfg_t init_config1 = {
        .unit_id = ADC_UNIT_1,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config1, &adc1_handle));

    adc_oneshot_chan_cfg_t config = {
        .atten = ADC_ATTEN_DB_11,
        .bitwidth = ADC_BITWIDTH_12,
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, EXAMPLE_ADC1_CHAN0, &config));
    ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, EXAMPLE_ADC1_CHAN0, value));
    ESP_LOGI(TAG_ADC, "ADC%d Channel[%d] Raw Data: %d", ADC_UNIT_1 + 1, EXAMPLE_ADC1_CHAN0, *value);

    ESP_ERROR_CHECK(adc_oneshot_del_unit(adc1_handle));
}

void http_post_data(void *pvParameters)
{
    char *post_data;

    while (1)
    {
        // vTaskSuspend(task_http_get_data);

        if (writeStage)
        {
            ESP_LOGI(TAG_HTTP_POST, "Sending HTTP POST request...");
            post_data = (char *)pvPortMalloc(MAX_POST_SIZE + 41);
            if (!post_data)
            {
                ESP_LOGE(TAG_HTTP_POST, "Failed to allocate memory for post_data");
                vTaskDelete(NULL);
            }
            snprintf(post_data, MAX_POST_SIZE + 40, "{\"device_id\":\"%d\",\"data\":\"%s\"}", DEVICE_ID, temp[tempPostIndex]);
            if (http_post_json_handle(post_data) == 0)
            {
                writeStage = 0;
            }
            vPortFree(post_data);
            ESP_LOGI(TAG_HTTP_POST, "HTTP POST request completed");
        }

        // vTaskResume(task_http_get_data);
        vTaskDelay(TASK1_DELAY / portTICK_PERIOD_MS);
    }
}

void http_get_data(void *pvParameters)
{
    TickType_t xLastWakeTime;
    const TickType_t xFrequency = pdMS_TO_TICKS(TASK0_DELAY);

    while (1)
    {
        http_get_handle();

        if (logs == 1)
        {
            vTaskResume(task_http_post_data);
            vTaskResume(task_adc_oneshot_write);
            ESP_LOGI(TAG_HTTP_GET, "LOG START");
        }
        else if((eTaskGetState(task_adc_oneshot_write)==eBlocked)&&(eTaskGetState(task_http_post_data)==eBlocked))
        {
            vTaskSuspend(task_http_post_data);
            vTaskSuspend(task_adc_oneshot_write);
            ESP_LOGI(TAG_HTTP_GET, "LOG STOP");
        }

        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}

void adc_oneshot_write(void *pvParameters)
{
    int count = 0;
    uint8_t tempWriteIndex = 0;
    TickType_t xLastWakeTime;
    const TickType_t xFrequency = pdMS_TO_TICKS(TASK2_DELAY);

    xLastWakeTime = xTaskGetTickCount();

    while (1)
    {
        int value;
        oneshot_adc_read(&value);

        uint32_t current_time_milliseconds = esp_log_timestamp();
        sprintf(temp[tempWriteIndex] + strlen(temp[tempWriteIndex]), "?%ld&%d", current_time_milliseconds, value);

        count++;
        if (count == VALUE_PER_POST)
        {
            count = 0;
            writeStage = 1;
            tempWriteIndex++;
            tempPostIndex = tempWriteIndex - 1;

            if (tempWriteIndex == TEMP_SIZE)
            {
                tempWriteIndex = 0;
            }

            *temp[tempWriteIndex] = '\0';
        }

        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}

void app_main(void)
{
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    ESP_ERROR_CHECK(example_connect());

    xTaskCreatePinnedToCore(http_get_data, "Task0", 8192, NULL, 0, &task_http_get_data, 1);
    xTaskCreatePinnedToCore(http_post_data, "Task1", 8192, NULL, 1, &task_http_post_data, 1);
    xTaskCreatePinnedToCore(adc_oneshot_write, "Task2", 8192, NULL, 1, &task_adc_oneshot_write, 0);
    vTaskSuspend(task_http_post_data);
    vTaskSuspend(task_adc_oneshot_write);
}
