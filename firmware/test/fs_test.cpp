#include "Wire.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs.h"
#include "nvs_flash.h"
#include <inttypes.h>
#include <logging.h>
#include <stdio.h>

using namespace mirra;

void setup(void)
{
    Serial.begin(115200);
    Serial.println("Serial initialised.");
    // Serial.flush();
    // Wire.begin(SDA_PIN, SCL_PIN); // i2c
    // Initialize NVS
    fs::NVS::init();

    {
        Log::File file{};
        file.push("hellooo... hello hiiiii");
    }

    printf("AFTER COMMIT:\n");
    {
        fs::NVS nvs{"logs"};

        printf("tail: %u, size: %u, head: %u\n", size_t(nvs.getValue<size_t>("tail", 0)),
               size_t(nvs.getValue<size_t>("size", 0)), size_t(nvs.getValue<size_t>("head", 0)));
    }

    // Restart module
    for (int i = 5; i >= 0; i--)
    {
        printf("Restarting in %d seconds...\n", i);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    printf("Restarting now.\n");
    fflush(stdout);
    esp_restart();
}

void loop() {}