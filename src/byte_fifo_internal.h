/*
 * SPDX-FileCopyrightText: 2024 Izidor Makuc <izidor@makuc.info>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef BYTE_FIFO_INTERNAL_H
#define BYTE_FIFO_INTERNAL_H

#include "byte_fifo.h"

struct byte_fifo {
        uint8_t *fifo_buff;
        uint16_t fifo_in;
        uint16_t fifo_out;
        uint16_t fifo_size;
        uint16_t fifo_size_mask;
};

enum rstatus byte_fifo_init_internal(struct byte_fifo *f,
                            uint8_t *buffer,
                            uint32_t size);


#endif
