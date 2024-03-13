#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>

#include "unity.h"

#include "byte_fifo.h"
#include "byte_fifo_internal.h"

struct byte_fifo f;
struct byte_fifo *fp = &f;

void setUp(void)
{
	memset(&f, 0, sizeof(f));
}

void tearDown(void)
{
        
}

void test_byte_fifo_init_internal_args()
{
	uint8_t mem[8];

	TEST_ASSERT_EQUAL_UINT32(
		BF_INVALID_ARGS, 
		byte_fifo_init_internal(&f, NULL, 0));

	TEST_ASSERT_EQUAL_UINT32(
		BF_INVALID_ARGS, 
		byte_fifo_init_internal(NULL, mem, sizeof(mem)));

	TEST_ASSERT_EQUAL_UINT32(
		BF_INVALID_ARGS, 
		byte_fifo_init_internal(&f, mem, 0));

	TEST_ASSERT_EQUAL_UINT32(
		BF_INVALID_ARGS, 
		byte_fifo_init_internal(&f, mem, sizeof(mem)+1));

	TEST_ASSERT_EQUAL_UINT32(
		BF_OK,
		byte_fifo_init_internal(&f, mem, sizeof(mem)));
}

static void *my_malloc_test(size_t size)
{
	return NULL;
}


void test_byte_fifo_init()
{
	uint8_t mem[8];
	struct byte_fifo_settings settings = {
		.fifo_buff = mem,
		.fifo_size = sizeof(mem),
		.my_malloc = malloc,
	};

	TEST_ASSERT_NULL(
		byte_fifo_init(NULL));

	struct byte_fifo_settings settings_foo_buff = settings;
	settings_foo_buff.fifo_buff = NULL;
	TEST_ASSERT_NULL(
		byte_fifo_init(&settings_foo_buff));

	struct byte_fifo_settings settings_foo_size = settings;
	settings_foo_size.fifo_size = 0;
	TEST_ASSERT_NULL(
		byte_fifo_init(&settings_foo_size));

	settings_foo_size.fifo_size = 3; //test non power of 2 size
	TEST_ASSERT_NULL(
		byte_fifo_init(&settings_foo_size));

	struct byte_fifo_settings settings_foo_malloc = settings;
	settings_foo_malloc.my_malloc = NULL;
	TEST_ASSERT_NULL(
		byte_fifo_init(&settings_foo_malloc));

	settings_foo_malloc.my_malloc = my_malloc_test;
	TEST_ASSERT_NULL(
		byte_fifo_init(&settings_foo_malloc));


	struct byte_fifo *bf = byte_fifo_init(&settings);
	TEST_ASSERT_NOT_NULL(bf);

	free(bf);
}


void test_byte_fifo_init_internal_is_structure_fully_initialized()
{
	uint8_t mem[8];

	// this is size of the structure calculated by hand
	// by adding all the members sizes together
	// if somebody will add a new member to this sturcture
	// this test will fail, unles corrected by hand

	uint32_t byte_fifo_struct_size = sizeof(uint8_t *)
		+ sizeof(uint16_t) * 4;


	TEST_ASSERT_EQUAL_UINT32(
		byte_fifo_struct_size,
		sizeof(f));


	TEST_ASSERT_EQUAL_UINT32(
		BF_OK,
		byte_fifo_init_internal(&f, mem, sizeof(mem)));

	TEST_ASSERT_EQUAL_PTR(mem, f.fifo_buff);

	TEST_ASSERT_EQUAL_UINT16(0, f.fifo_in);
	TEST_ASSERT_EQUAL_UINT16(0, f.fifo_out);
	TEST_ASSERT_EQUAL_UINT16(sizeof(mem), f.fifo_size);
	TEST_ASSERT_EQUAL_UINT16(sizeof(mem)-1, f.fifo_size_mask);
}


void test_byte_fifo_write()
{
	uint8_t mem[5];
	memset(mem, 0xff, sizeof(mem));

	TEST_ASSERT_EQUAL_UINT32(
		BF_OK,
		byte_fifo_init_internal(fp, mem, 4));


	// fill fifo
	byte_fifo_write(fp, 1);
	TEST_ASSERT_EQUAL_UINT8(1, fp->fifo_buff[0]);
	TEST_ASSERT_EQUAL_UINT8(0xff, fp->fifo_buff[1]);

	byte_fifo_write(fp, 2);
	TEST_ASSERT_EQUAL_UINT8(2, fp->fifo_buff[1]);
	TEST_ASSERT_EQUAL_UINT8(0xff, fp->fifo_buff[2]);

	byte_fifo_write(fp, 3);
	TEST_ASSERT_EQUAL_UINT8(3, fp->fifo_buff[2]);
	TEST_ASSERT_EQUAL_UINT8(0xff, fp->fifo_buff[3]);

	byte_fifo_write(fp, 4);
	TEST_ASSERT_EQUAL_UINT8(4, fp->fifo_buff[3]);
	TEST_ASSERT_EQUAL_UINT8(0xff, fp->fifo_buff[4]);

	//check if fifo head wraps around to begginning of the buffer
	byte_fifo_write(fp, 5);
	TEST_ASSERT_EQUAL_UINT8(0xff, fp->fifo_buff[4]);
	TEST_ASSERT_EQUAL_UINT8(5, fp->fifo_buff[0]);

}

void test_byte_fifo_read()
{
	uint8_t mem[5];
	memset(mem, 0xff, sizeof(mem));

	TEST_ASSERT_EQUAL_UINT32(
		BF_OK,
		byte_fifo_init_internal(fp, mem, 4));


	byte_fifo_write(fp, 1);
	TEST_ASSERT_EQUAL_UINT8(1, byte_fifo_read(fp));

	byte_fifo_write(fp, 2);
	byte_fifo_write(fp, 3);
	TEST_ASSERT_EQUAL_UINT8(2, byte_fifo_read(fp));
	TEST_ASSERT_EQUAL_UINT8(3, byte_fifo_read(fp));

	byte_fifo_write(fp, 4);
	TEST_ASSERT_EQUAL_UINT8(4, byte_fifo_read(fp));

	// does read wrap around to the beginning of the buffer
	byte_fifo_write(fp, 5);
	TEST_ASSERT_EQUAL_UINT8(5, byte_fifo_read(fp));

	// testing if there is a buffer overflow in fifo memory
	// always at the end because it doesnt hurt :D
	TEST_ASSERT_EQUAL_UINT8(0xff, fp->fifo_buff[4]);
}

void test_byte_fifo_is_full()
{
	uint8_t mem[5];
	memset(mem, 0xff, sizeof(mem));

	TEST_ASSERT_EQUAL_UINT32(
		BF_OK,
		byte_fifo_init_internal(fp, mem, 4));

	for (uint32_t i=0; sizeof(mem)-1 > i; i++)
	{
		byte_fifo_write(fp, i & 0xff);
	}

	TEST_ASSERT(byte_fifo_is_full(fp));

	// go over whole fifo memory by removing 
	// and adding one element and check if fifo_is_full
	// works as expected
	for (uint32_t i=0; sizeof(mem)*2 > i; i++)
	{
		byte_fifo_read(fp);
		TEST_ASSERT(!byte_fifo_is_full(fp));

		byte_fifo_write(fp, 0);
		TEST_ASSERT(byte_fifo_is_full(fp));
	}


	// testing if there is a buffer overflow in fifo memory
	// always at the end because it doesnt hurt :D
	TEST_ASSERT_EQUAL_UINT8(0xff, fp->fifo_buff[4]);
}

void test_byte_fifo_is_empty()
{
	uint8_t mem[5];
	memset(mem, 0xff, sizeof(mem));

	TEST_ASSERT_EQUAL_UINT32(
		BF_OK,
		byte_fifo_init_internal(fp, mem, 4));


	TEST_ASSERT(byte_fifo_is_empty(fp));

	// go over whole fifo memory by adding 
	// and removing one element and check if fifo_is_empty
	// works as expected
	for (uint32_t i=0; sizeof(mem)*2 > i; i++)
	{
		byte_fifo_write(fp, 0);
		TEST_ASSERT(!byte_fifo_is_empty(fp));

		byte_fifo_read(fp);
		TEST_ASSERT(byte_fifo_is_empty(fp));
	}


	// testing if there is a buffer overflow in fifo memory
	// always at the end because it doesnt hurt :D
	TEST_ASSERT_EQUAL_UINT8(0xff, fp->fifo_buff[4]);
}

void test_byte_fifo_reset()
{
	uint8_t mem[5];
	memset(mem, 0xff, sizeof(mem));

	TEST_ASSERT_EQUAL_UINT32(
		BF_OK,
		byte_fifo_init_internal(fp, mem, 4));

	byte_fifo_write(fp, 0);
	TEST_ASSERT(!byte_fifo_is_empty(fp));

	byte_fifo_reset(fp);
	TEST_ASSERT(byte_fifo_is_empty(fp));
}

void test_byte_fifo_get_fill_count()
{
	uint8_t mem[5];
	memset(mem, 0xff, sizeof(mem));

	TEST_ASSERT_EQUAL_UINT32(
		BF_OK,
		byte_fifo_init_internal(fp, mem, 4));

	TEST_ASSERT_EQUAL_UINT16(0, byte_fifo_get_fill_count(fp));

	byte_fifo_write(fp, 0);
	TEST_ASSERT_EQUAL_UINT16(1, byte_fifo_get_fill_count(fp));

	byte_fifo_write(fp, 0);
	byte_fifo_write(fp, 0);
	byte_fifo_write(fp, 0);
	TEST_ASSERT_EQUAL_UINT16(4, byte_fifo_get_fill_count(fp));

	byte_fifo_read(fp);
	TEST_ASSERT_EQUAL_UINT16(3, byte_fifo_get_fill_count(fp));

	byte_fifo_read(fp);
	byte_fifo_read(fp);
	byte_fifo_read(fp);
	TEST_ASSERT_EQUAL_UINT16(0, byte_fifo_get_fill_count(fp));

	// testing if there is a buffer overflow in fifo memory
	// always at the end because it doesnt hurt :D
	TEST_ASSERT_EQUAL_UINT8(0xff, fp->fifo_buff[4]);
}



void test_byte_fifo_get_free_space()
{
	uint8_t mem[5];
	memset(mem, 0xff, sizeof(mem));

	TEST_ASSERT_EQUAL_UINT32(
		BF_OK,
		byte_fifo_init_internal(fp, mem, 4));

	TEST_ASSERT_EQUAL_UINT16(4, byte_fifo_get_free_space(fp));

	byte_fifo_write(fp, 0);
	TEST_ASSERT_EQUAL_UINT16(3, byte_fifo_get_free_space(fp));

	byte_fifo_write(fp, 0);
	byte_fifo_write(fp, 0);
	byte_fifo_write(fp, 0);
	TEST_ASSERT_EQUAL_UINT16(0, byte_fifo_get_free_space(fp));

	byte_fifo_read(fp);
	TEST_ASSERT_EQUAL_UINT16(1, byte_fifo_get_free_space(fp));

	byte_fifo_write(fp, 0);
	TEST_ASSERT_EQUAL_UINT16(0, byte_fifo_get_free_space(fp));

	byte_fifo_read(fp);
	byte_fifo_read(fp);
	TEST_ASSERT_EQUAL_UINT16(2, byte_fifo_get_free_space(fp));

	byte_fifo_read(fp);
	byte_fifo_read(fp);
	TEST_ASSERT_EQUAL_UINT16(4, byte_fifo_get_free_space(fp));


	byte_fifo_reset(fp);
	TEST_ASSERT_EQUAL_UINT16(4, byte_fifo_get_free_space(fp));
}

void test_byte_fifo_peak_head()
{
	uint8_t mem[5];
	memset(mem, 0xff, sizeof(mem));

	TEST_ASSERT_EQUAL_UINT32(
		BF_OK,
		byte_fifo_init_internal(fp, mem, 4));

	byte_fifo_write(fp, 0xff);
	TEST_ASSERT_EQUAL_UINT8(0xff, byte_fifo_peak(fp, 0));

	byte_fifo_write(fp, 1);
	byte_fifo_write(fp, 2);
	TEST_ASSERT_EQUAL_UINT8(0xff, byte_fifo_peak(fp, 0));

	
	TEST_ASSERT_EQUAL_UINT8(0xff, byte_fifo_read(fp));
	TEST_ASSERT_EQUAL_UINT8(1, byte_fifo_peak(fp, 0));

	byte_fifo_read(fp);

	TEST_ASSERT_EQUAL_UINT8(2, byte_fifo_peak(fp, 0));
}

void test_byte_fifo_peak_index()
{
	uint8_t mem[5];
	memset(mem, 0xff, sizeof(mem));

	TEST_ASSERT_EQUAL_UINT32(
		BF_OK,
		byte_fifo_init_internal(fp, mem, 4));

	byte_fifo_write(fp, 0xff);
	byte_fifo_write(fp, 1);
	byte_fifo_write(fp, 2);
	byte_fifo_write(fp, 3);
	TEST_ASSERT_EQUAL_UINT8(0xff, byte_fifo_peak(fp, 0));
	TEST_ASSERT_EQUAL_UINT8(1, byte_fifo_peak(fp, 1));
	TEST_ASSERT_EQUAL_UINT8(2, byte_fifo_peak(fp, 2));
	TEST_ASSERT_EQUAL_UINT8(3, byte_fifo_peak(fp, 3));

	
	TEST_ASSERT_EQUAL_UINT8(0xff, byte_fifo_read(fp));
	TEST_ASSERT_EQUAL_UINT8(1, byte_fifo_read(fp));
	byte_fifo_write(fp, 4);
	byte_fifo_write(fp, 5);

	TEST_ASSERT_EQUAL_UINT8(2, byte_fifo_peak(fp, 0));
	TEST_ASSERT_EQUAL_UINT8(3, byte_fifo_peak(fp, 1));
	TEST_ASSERT_EQUAL_UINT8(4, byte_fifo_peak(fp, 2));
	TEST_ASSERT_EQUAL_UINT8(5, byte_fifo_peak(fp, 3));

	// check if index is wrapped around
	TEST_ASSERT_EQUAL_UINT8(3, byte_fifo_peak(fp, 5));
}

void test_byte_fifo_test_head_tail_pointer_overrun()
{
	uint8_t mem[5];
	memset(mem, 0xff, sizeof(mem));

	TEST_ASSERT_EQUAL_UINT32(
		BF_OK,
		byte_fifo_init_internal(fp, mem, 4));

	for (uint32_t i = 0; i < ((1 << (sizeof(fp->fifo_in)*8))-2) ; i++)
	{
		uint8_t t = i & 0xff;
		byte_fifo_write(fp, t);
		TEST_ASSERT_EQUAL_UINT8(t, byte_fifo_read(fp));
	}


	TEST_ASSERT_EQUAL_UINT16(0xfffe, fp->fifo_in);
	TEST_ASSERT_EQUAL_UINT16(0xfffe, fp->fifo_out);

	TEST_ASSERT(byte_fifo_is_empty(fp));

	byte_fifo_write(fp, 5);

	TEST_ASSERT_EQUAL_UINT8(5, byte_fifo_peak(fp, 0));
	TEST_ASSERT_EQUAL_UINT8(3, byte_fifo_get_free_space(fp));

	byte_fifo_write(fp, 6);

	TEST_ASSERT_EQUAL_UINT8(5, byte_fifo_peak(fp, 0));
	TEST_ASSERT_EQUAL_UINT8(6, byte_fifo_peak(fp, 1));
	TEST_ASSERT_EQUAL_UINT16(2, byte_fifo_get_free_space(fp));

	byte_fifo_write(fp, 7);
	TEST_ASSERT_EQUAL_UINT8(7, byte_fifo_peak(fp, 2));
	TEST_ASSERT_EQUAL_UINT16(1, byte_fifo_get_free_space(fp));

	byte_fifo_write(fp, 8);
	TEST_ASSERT_EQUAL_UINT8(8, byte_fifo_peak(fp, 3));
	TEST_ASSERT_EQUAL_UINT16(0, byte_fifo_get_free_space(fp));

	TEST_ASSERT(!byte_fifo_is_full(fp));
}
