#include "health_task.h"

#include "common/configs.h"

TaskHandle_t health_task_handler;

typedef struct {
    uint16_t length;
    const uint16_t* sections;
} buzzer_tune_t;

typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} color_t;

typedef struct {
    uint16_t length;
    const color_t* colors;
    const uint16_t* wait_time;
} rgb_section_t;

typedef enum {
    NOTHING,
    DOUBLE_BEEP,
    TRIPLE_BEEP,
    LONG_BEEP,
    NUM_TUNES           // <- Must be always last
} BUZZER_TUNE_TYPE;

typedef enum{
    OFF,
    ALL_NOMINAL,
    ALERT,
    CRITICAL_ALERT
} RGB_ALERT_TYPE;

void create_buzzer_tune(buzzer_tune_t* tune_ptr, const uint16_t* sections, const uint16_t length){
    tune_ptr->sections = sections;
    tune_ptr->length = length;
}

void health_task(void* param){

    buzzer_tune_t buzzer_tunes[NUM_TUNES];

    // Set up buzzer output
    pinMode(BUZZER_PIN, OUTPUT);
    digitalWrite(BUZZER_PIN, LOW);
    
    buzzer_tune_t tune_low_battery;

    bool isBuzzerPlaying = false;
    uint16_t buzzerTuneId = 0;
    uint16_t actual_section = 0;
    uint16_t buzzer_timer = 0;
    uint64_t last_time = 0;

    // Registering tunes
    create_buzzer_tune(&buzzer_tunes[NOTHING], (const uint16_t[]){0}, 1);
    create_buzzer_tune(&buzzer_tunes[DOUBLE_BEEP], (const uint16_t[]){0, 100, 100, 100}, 4);
    create_buzzer_tune(&buzzer_tunes[TRIPLE_BEEP], (const uint16_t[]){0, 50, 100, 50, 100, 100, 3000}, 7);
    create_buzzer_tune(&buzzer_tunes[LONG_BEEP], (const uint16_t[]){0, 500, 500}, 3);

    neopixelWrite(RGB_LED_PIN, 0, 255, 0);
    vTaskDelay(100);
    neopixelWrite(RGB_LED_PIN, 0, 0, 0);

    while(true){
        bool doPlayAlert = false;
             
        if(doPlayAlert || isBuzzerPlaying){
            buzzer_timer +=  millis() - last_time;

            // Buzz
            if(buzzer_timer > buzzer_tunes[buzzerTuneId].sections[actual_section]){
                digitalWrite(BUZZER_PIN, !digitalRead(BUZZER_PIN));     // Toggle output
                buzzer_timer = 0;
                actual_section++;
            }

            // Stop Buzzing
            if(actual_section >= buzzer_tunes[buzzerTuneId].length){
                isBuzzerPlaying = false;
                buzzer_timer = 0;
                actual_section = 0;
                digitalWrite(BUZZER_PIN, LOW);
            }
            last_time = millis();
        }

        vTaskDelay(HEALTH_DELAY);
    }
}