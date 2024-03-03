/*
 * SPDX-FileCopyrightText: 2024 Izidor Makuc <izidor@makuc.info>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef BYTE_FIFO_H
#define BYTE_FIFO_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

enum rstatus {
	BF_OK,
	BF_INVALID_ARGS,
	BF_MALLOC_FAIL,
};

struct byte_fifo_settings {
	void *(*my_malloc)(size_t size);
	uint8_t *fifo_buff;
	uint16_t fifo_size;
};

struct byte_fifo;

struct byte_fifo *byte_fifo_init(struct byte_fifo_settings *settings);

bool byte_fifo_is_full(struct byte_fifo *f);
bool byte_fifo_is_empty(struct byte_fifo *f);
void byte_fifo_write(struct byte_fifo *f, uint8_t c);
uint8_t byte_fifo_read(struct byte_fifo *f);
uint8_t byte_fifo_peak(struct byte_fifo *f, uint32_t index);
void byte_fifo_reset(struct byte_fifo *f);

uint16_t byte_fifo_get_free_space(struct byte_fifo *f);
uint16_t byte_fifo_get_fill_count(struct byte_fifo *f);

#endif
