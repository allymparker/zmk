/*
 * Copyright (c) 2020 The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 */

#include <device.h>
#include <init.h>
#include <kernel.h>

#include <logging/log.h>

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#include <zmk/event_manager.h>
#include <zmk/events/wpm_state_changed.h>
#include <zmk/events/keycode_state_changed.h>

#include <zmk/wpm.h>

#define WPM_INTERVAL_SECONDS 5

// See https://en.wikipedia.org/wiki/Words_per_minute
// "Since the length or duration of words is clearly variable, for the purpose of measurement of
// text entry, the definition of each "word" is often standardized to be five characters or
// keystrokes long in English"
#define CHARS_PER_WORD 5.0

static uint8_t wpm_state;
static uint32_t key_pressed_count;

int zmk_wpm_get_state() { return wpm_state; }

int wpm_event_listener(const struct zmk_event_header *eh) {

    LOG_DBG("Got an event");
    if (is_keycode_state_changed(eh)) {
        struct keycode_state_changed *ev = cast_keycode_state_changed(eh);
        LOG_DBG("Its a keycode: %d %d", ev->keycode, ev->state);
        // count only key up events
        if (!ev->state) {
            key_pressed_count++;
            LOG_DBG("It's a key up - new count %d", key_pressed_count);
        }
    }
    return 0;
}

void wpm_work_handler(struct k_work *work) {
    wpm_state = (key_pressed_count / CHARS_PER_WORD) / (WPM_INTERVAL_SECONDS / 60.0);
    key_pressed_count = 0;
    LOG_DBG("Work handler %d", wpm_state);
    ZMK_EVENT_RAISE(create_wpm_state_changed(wpm_state));
}

K_WORK_DEFINE(wpm_work, wpm_work_handler);

void wpm_expiry_function() { k_work_submit(&wpm_work); }

K_TIMER_DEFINE(wpm_timer, wpm_expiry_function, NULL);

int wpm_init() {
    LOG_DBG("Init Wpm");
    wpm_state = 0;

    k_timer_start(&wpm_timer, K_SECONDS(WPM_INTERVAL_SECONDS), K_SECONDS(WPM_INTERVAL_SECONDS));
    return 0;
}

ZMK_LISTENER(wpm, wpm_event_listener);
ZMK_SUBSCRIPTION(wpm, keycode_state_changed);

SYS_INIT(wpm_init, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);
