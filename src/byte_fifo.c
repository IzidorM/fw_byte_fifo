/*
 * SPDX-FileCopyrightText: 2024 Izidor Makuc <izidor@makuc.info>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "byte_fifo_internal.h"
#include "byte_fifo.h"

enum rstatus byte_fifo_init_internal(struct byte_fifo *f,
                            uint8_t *buffer,
                            uint16_t size)
{
        if ((NULL == f) || NULL == buffer 
	    || 0 == size 
	    // size needs to be power of 2
	    || (size & (size - 1)) != 0)
	    
        {
                return BF_INVALID_ARGS;
        }
        
        f->fifo_in = 0;
        f->fifo_out = 0;
        f->fifo_size = size;
        f->fifo_buff = buffer;
	f->fifo_size_mask = size - 1;

        return BF_OK;
}

struct byte_fifo *byte_fifo_init(struct byte_fifo_settings *settings)
{
        if ((NULL == settings) 
	    || NULL == settings->my_malloc
	    || NULL == settings->fifo_buff
	    || 0 == settings->fifo_size
	    // size needs to be power of 2
	    || (settings->fifo_size & (settings->fifo_size - 1)) != 0)
        {
                return NULL;
        }

	struct byte_fifo *f = settings->my_malloc(
		sizeof(struct byte_fifo));
	if (NULL == f)
	{
		return NULL;
	}

	// no need to check return value,
	// because we already checked the arguments
	// so it can't fail
	byte_fifo_init_internal(f, settings->fifo_buff,
				settings->fifo_size);

	return f;
}

void byte_fifo_reset(struct byte_fifo *f)
{
       f->fifo_in = f->fifo_out;
}

bool byte_fifo_is_full(struct byte_fifo *f)
{
        return ((f->fifo_out + f->fifo_size) == f->fifo_in);
}

bool byte_fifo_is_empty(struct byte_fifo *f)
{
        return (f->fifo_in == f->fifo_out);
}

void byte_fifo_write(struct byte_fifo *f, uint8_t c)
{
        f->fifo_buff[f->fifo_in & f->fifo_size_mask] = c;
        f->fifo_in += 1;
}

uint8_t byte_fifo_read(struct byte_fifo *f)
{
        uint8_t c = f->fifo_buff[f->fifo_out & f->fifo_size_mask];
        f->fifo_out += 1;
        return c;
}

uint8_t byte_fifo_peak(struct byte_fifo *f, uint32_t index)
{
	return f->fifo_buff[(f->fifo_out + index) & f->fifo_size_mask];
}

uint16_t byte_fifo_get_fill_count(struct byte_fifo *f)
{
        if (f->fifo_out <= f->fifo_in)
        {
                return f->fifo_in - f->fifo_out;
        }
        else
        {
		//case handling fifo_in overflow
                //return f->fifo_in 
		//	+ (0xffff - (uint32_t) f->fifo_out) + 1;
                return (uint16_t) (f->fifo_in + 
			(UINT16_MAX - f->fifo_out) + 1);
			

        }
}

uint16_t byte_fifo_get_free_space(struct byte_fifo *f)
{
	return f->fifo_size - byte_fifo_get_fill_count(f);
}

