#include "ServiceWorker.h"
#include <WiFi.h>
#include "NetworkManager.h"
#include "WeatherService.h"
#include "CryptoService.h"
#include "YouTubeService.h"
#include "AirspaceService.h"

static TaskHandle_t s_workerTaskHandle = NULL;
static bool s_isBusy = false;
static bool s_workerStarted = false;

static void serviceWorkerTask(void* pvParameters) {
    Serial.printf("[Worker] Background FreeRTOS Service Worker active on Core %d\n", xPortGetCoreID());

    for (;;) {
        if (Network_IsConnected()) {
            s_isBusy = true;

            // Stagger network queries across Core 0 background loop
            Weather_Update();
            vTaskDelay(pdMS_TO_TICKS(150));

            Crypto_Update();
            vTaskDelay(pdMS_TO_TICKS(150));

            YouTube_Update();
            vTaskDelay(pdMS_TO_TICKS(150));

            Airspace_Fetch();
            vTaskDelay(pdMS_TO_TICKS(150));

            s_isBusy = false;
        }

        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

void ServiceWorker_Init() {
    if (s_workerStarted) return;

    BaseType_t res = xTaskCreatePinnedToCore(
        serviceWorkerTask,
        "svc_worker",
        8192,
        NULL,
        1, // Priority 1 (idle/worker, lower than Core 0 WiFi driver tasks)
        &s_workerTaskHandle,
        0  // Pinned to Core 0 (PRO_CPU)
    );

    if (res == pdPASS) {
        s_workerStarted = true;
        Serial.println("[Worker] Successfully spawned Core 0 background service task");
    } else {
        Serial.printf("[Worker] Failed to create worker task: %d\n", (int)res);
    }
}

void ServiceWorker_TriggerAll() {
    Weather_ForceFetch();
    Crypto_ForceFetch();
    YouTube_ForceFetch();
    Airspace_ForceFetch();
}

bool ServiceWorker_IsBusy() {
    return s_isBusy;
}

