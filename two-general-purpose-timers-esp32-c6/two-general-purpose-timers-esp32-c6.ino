#include <driver/gpio.h>
#include <driver/gptimer.h>
#include <esp_task_wdt.h>

#define WDT_TIMEOUT 2 // time in seconds for the watchdog to activate
#define LED_GPIO1 GPIO_NUM_1 // D1 of Xiao ESP32-C6
#define LED_GPIO2 GPIO_NUM_2 // D2 of Xiao ESP32-C6
#define BUTTON_GPIO GPIO_NUM_18 // D10 of Xiao ESP32-C6
gptimer_handle_t led1 = NULL;
gptimer_handle_t led2 = NULL;
volatile bool buttonPressed1 = false;
volatile bool buttonPressed2 = false;

void setupGPIO()
{
    gpio_config_t io_conf = {};

    // Configure LED1 as output (pin 1 (D1), without pull up or pull down)
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = (1ULL << LED_GPIO1);
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    gpio_config(&io_conf);

    gpio_set_level(LED_GPIO1, 0);

    // Configure LED2 as output (pin 2 (D2), without pull up or pull down)
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = (1ULL << LED_GPIO2);
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    gpio_config(&io_conf);

    gpio_set_level(LED_GPIO2, 0);

    // Configure button as input (pin 18 (D10) with pull up only)
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = (1ULL << BUTTON_GPIO);
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    gpio_config(&io_conf);
}

esp_err_t createTimer(
    gptimer_handle_t *timer,
    uint32_t resolution_hz,
    uint64_t alarm_count,
    gptimer_alarm_cb_t callback)
{
    gptimer_config_t config = {
        .clk_src = GPTIMER_CLK_SRC_DEFAULT,
        .direction = GPTIMER_COUNT_UP,
        .resolution_hz = resolution_hz,
    };

    ESP_ERROR_CHECK(gptimer_new_timer(&config, timer));

    gptimer_event_callbacks_t cbs = {
        .on_alarm = callback,
    };

    ESP_ERROR_CHECK(gptimer_register_event_callbacks(*timer, &cbs, NULL));

    gptimer_alarm_config_t alarm = {
        .alarm_count = alarm_count,
        .reload_count = 0,
        .flags = {
            .auto_reload_on_alarm = true,
        },
    };

    ESP_ERROR_CHECK(gptimer_set_alarm_action(*timer, &alarm));
    ESP_ERROR_CHECK(gptimer_enable(*timer));
    ESP_ERROR_CHECK(gptimer_start(*timer));

    return ESP_OK;
}

bool IRAM_ATTR led1_callback(
    gptimer_handle_t led1,
    const gptimer_alarm_event_data_t *edata,
    void *user_ctx)
{
    static bool state = false;
    if(buttonPressed1 == false){
        state = !state;
        gpio_set_level(LED_GPIO1, state);        
    }else{
        state = true;
        gpio_set_level(LED_GPIO1, state);        
    }
    
    return false;
}
bool IRAM_ATTR led2_callback(
    gptimer_handle_t led2,
    const gptimer_alarm_event_data_t *edata,
    void *user_ctx)
{
    static bool state = false;
    if(buttonPressed2 == false){
        state = !state;
        gpio_set_level(LED_GPIO2, state);        
    }else{
        state = true;
        gpio_set_level(LED_GPIO2, state);        
    }
    
    return false;
}
void setup() {
    setupGPIO();
    esp_task_wdt_config_t wdt_config = {
    .timeout_ms = WDT_TIMEOUT * 1000,
    .idle_core_mask = 0,
    .trigger_panic = true,
};
    //Serial.begin(115200);
    esp_err_t err = esp_task_wdt_reconfigure(&wdt_config);
    //Serial.println(esp_err_to_name(err));
    ESP_ERROR_CHECK(err);
    ESP_ERROR_CHECK(esp_task_wdt_add(NULL));
    createTimer(
    &led1,
    100000,
    20000,
    led1_callback);
  
    createTimer(
    &led2,
    100000,
    50000,
    led2_callback);
}

void loop() {
    // when push button on pin 18 (D10 of Xiao ESP32-C6) is pressed
    if (gpio_get_level(BUTTON_GPIO) == 0)
    {
        buttonPressed1= true; // this makes the LED stop blinking and stay solid
        buttonPressed2= true; // this makes the LED stop blinking and stay solid
    }
    // buttonPressed goes to true when the push button is pressed, effectively
    // preventing the watchdog from being reset periodically. After some time 
    // the watchdog is triggered, resetting the microcontroller    
    if(buttonPressed1 == false && buttonPressed2 == false){ 
        esp_task_wdt_reset(); // Reset watchdog timer
    }
}