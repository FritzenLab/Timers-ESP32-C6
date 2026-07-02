#include <driver/gpio.h>
#include <driver/gptimer.h>

#define LED_GPIO GPIO_NUM_1
gptimer_handle_t timer = NULL;

void setupGPIO()
{
    gpio_config_t io_conf = {};
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = (1ULL << LED_GPIO);
    gpio_config(&io_conf);
    gpio_set_level(LED_GPIO, 0);
}
bool IRAM_ATTR timer_callback(
    gptimer_handle_t timer,
    const gptimer_alarm_event_data_t *edata,
    void *user_ctx)
{
    static bool state = false;
    state = !state;
    gpio_set_level(LED_GPIO, state);
    return false;
}
void setupTimer()
{
    gptimer_config_t config = {
        .clk_src = GPTIMER_CLK_SRC_DEFAULT,
        .direction = GPTIMER_COUNT_UP,
        .resolution_hz = 100000,      // 100 kHz
    };

    gptimer_new_timer(&config, &timer);

    gptimer_event_callbacks_t cbs = {
        .on_alarm = timer_callback,
    };

    gptimer_register_event_callbacks(timer, &cbs, NULL);

    gptimer_alarm_config_t alarm = {
        .alarm_count = 5000,         // 50 ms, since resolution_hz above is 100000 Hz, giving 10us per tick. Then 5000 * 10us = 50ms
        .reload_count = 0,
        .flags = {
            .auto_reload_on_alarm = true,
        },
    };

    gptimer_set_alarm_action(timer, &alarm);
    gptimer_enable(timer);
    gptimer_start(timer);
}

void setup() {
  // put your setup code here, to run once:
  setupGPIO();
  setupTimer();
}

void loop() {
  // put your main code here, to run repeatedly:

}
