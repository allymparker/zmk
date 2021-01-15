/*
 * Copyright (c) 2020 The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <zephyr.h>
#include <zmk/event_manager.h>
#include <zmk/wpm.h>

struct wpm_state_changed {
    struct zmk_event_header header;
    int state;
};

ZMK_EVENT_DECLARE(wpm_state_changed);

static inline struct wpm_state_changed *create_wpm_state_changed(int state) {
    struct wpm_state_changed *ev = new_wpm_state_changed();
    ev->state = state;

    return ev;
}