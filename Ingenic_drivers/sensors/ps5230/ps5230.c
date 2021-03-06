/*
 * ps5230.c
 *
 * Copyright (C) 2012 Ingenic Semiconductor Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#define DEBUG

#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/clk.h>
#include <linux/delay.h>
#include <sensor-common.h>
#include <apical-isp/apical_math.h>
#include <linux/proc_fs.h>
#include <soc/gpio.h>

#define PS5230_CHIP_ID_H	(0x52)
#define PS5230_CHIP_ID_L	(0x30)
#define PS5230_REG_END		0xff
#define PS5230_REG_DELAY	0xfe
#define PS5230_BANK_REG		0xef

#define PS5230_SUPPORT_PCLK (81000000)
#define SENSOR_OUTPUT_MAX_FPS 30
#define SENSOR_OUTPUT_MIN_FPS 5

static int reset_gpio = GPIO_PA(18);
module_param(reset_gpio, int, S_IRUGO);
MODULE_PARM_DESC(reset_gpio, "Reset GPIO NUM");

static int pwdn_gpio = -1;
module_param(pwdn_gpio, int, S_IRUGO);
MODULE_PARM_DESC(pwdn_gpio, "Power down GPIO NUM");

static int sensor_gpio_func = DVP_PA_HIGH_10BIT;
module_param(sensor_gpio_func, int, S_IRUGO);
MODULE_PARM_DESC(sensor_gpio_func, "Sensor GPIO function");

struct regval_list {
	unsigned char reg_num;
	unsigned char value;
};

/*
 * the part of driver maybe modify about different sensor and different board.
 */
struct again_lut {
	unsigned int value;
	unsigned int gain;
};

struct again_lut ps5230_again_lut[] = {
#if 0
	{0x1000, 0},
	{0xf0f, 5731},
	{0xe39, 11136},
	{0xd79, 16247},
	{0xccd, 21097},
	{0xc31, 25710},
	{0xba3, 30108},
	{0xb21, 34311},
#endif
	/* start frome 1.5x */
	{0xaab, 38335},
	{0xa3d, 42195},
	{0x9d9, 45903},
	{0x97b, 49471},
	{0x925, 52910},
	{0x8d4, 56227},
	{0x889, 59433},
	{0x842, 62533},
	{0x800, 65535},
	{0x788, 71266},
	{0x71c, 76671},
	{0x6bd, 81782},
	{0x666, 86632},
	{0x618, 91245},
	{0x5d1, 95643},
	{0x591, 99846},
	{0x555, 103870},
	{0x51f, 107730},
	{0x4ec, 111438},
	{0x4be, 115006},
	{0x492, 118445},
	{0x46a, 121762},
	{0x444, 124968},
	{0x421, 128068},
	{0x400, 131070},
	{0x3c4, 136801},
	{0x38e, 142206},
	{0x35e, 147317},
	{0x333, 152167},
	{0x30c, 156780},
	{0x2e9, 161178},
	{0x2c8, 165381},
	{0x2ab, 169405},
	{0x28f, 173265},
	{0x276, 176973},
	{0x25f, 180541},
	{0x249, 183980},
	{0x235, 187297},
	{0x222, 190503},
	{0x211, 193603},
	{0x200, 196605},
	{0x1e2, 202336},
	{0x1c7, 207741},
	{0x1af, 212852},
	{0x19a, 217702},
	{0x186, 222315},
	{0x174, 226713},
	{0x164, 230916},
	{0x155, 234940},
	{0x148, 238800},
	{0x13b, 242508},
	{0x12f, 246076},
	{0x125, 249515},
	{0x11a, 252832},
	{0x111, 256038},
	{0x108, 259138},
	{0x100, 262140},
	{0xf1, 267849},
	{0xe3, 273507},
	{0xd7, 278642},
	{0xcd, 283145},
	{0xc3, 287873},
	{0xba, 292341},
	{0xb2, 296497},
	{0xab, 300290},
	{0xa4, 304242},
	{0x9e, 307766},
	{0x98, 311427},
	{0x92, 315234},
	{0x8d, 318529},
	{0x89, 321240},
	{0x84, 324765},
	{0x80, 327675},
};

struct tx_isp_sensor_attribute ps5230_attr;

unsigned int ps5230_alloc_again(unsigned int isp_gain, unsigned char shift, unsigned int *sensor_again)
{
	struct again_lut *lut = ps5230_again_lut;
	while(lut->gain <= ps5230_attr.max_again) {
		if(isp_gain <= ps5230_again_lut[0].gain) {
			*sensor_again = lut[0].value;
			return lut[0].gain;
		}
		else if(isp_gain < lut->gain) {
			*sensor_again = (lut - 1)->value;
			return (lut - 1)->gain;
		}
		else{
			if((lut->gain == ps5230_attr.max_again) && (isp_gain >= lut->gain)) {
				*sensor_again = lut->value;
				return lut->gain;
			}
		}

		lut++;
	}

	return isp_gain;
}

unsigned int ps5230_alloc_dgain(unsigned int isp_gain, unsigned char shift, unsigned int *sensor_dgain)
{
	return isp_gain;
}

struct tx_isp_sensor_attribute ps5230_attr={
	.name = "ps5230",
	.chip_id = 0x5230,
	.cbus_type = TX_SENSOR_CONTROL_INTERFACE_I2C,
	.cbus_mask = V4L2_SBUS_MASK_SAMPLE_8BITS | V4L2_SBUS_MASK_ADDR_8BITS,
	.cbus_device = 0x48,
	.dbus_type = TX_SENSOR_DATA_INTERFACE_DVP,
	.dvp = {
		.mode = SENSOR_DVP_HREF_MODE,
		.blanking = {
			.vblanking = 0,
			.hblanking = 0,
		},
	},
	.max_again = 327675,
	.max_dgain = 0,
	.min_integration_time = 2,
	.min_integration_time_native = 2,
	.max_integration_time_native = 1348,
	.integration_time_limit = 1348,
	.total_width = 2400,
	.total_height = 1350,
	.max_integration_time = 1348,
	.one_line_expr_in_us = 30,
	.integration_time_apply_delay = 2,
	.again_apply_delay = 2,
	.dgain_apply_delay = 2,
	.sensor_ctrl.alloc_again = ps5230_alloc_again,
	.sensor_ctrl.alloc_dgain = ps5230_alloc_dgain,
};


static struct regval_list ps5230_init_regs_1920_1080_25fps[] = {

	{0xEF, 0x01},
	{0x05, 0x03},
	{0xEF, 0x00},
	{0x06, 0x02},
	{0x0B, 0x00},
	{0x0C, 0xA0},
	{0x10, 0x01},
	{0x12, 0x80},
	{0x13, 0x00},
	{0x14, 0xFF},
	{0x15, 0x03},
	{0x16, 0xFF},
	{0x17, 0xFF},
	{0x18, 0xFF},
	{0x19, 0x64},
	{0x1A, 0x64},
	{0x1B, 0x64},
	{0x1C, 0x64},
	{0x1D, 0x64},
	{0x1E, 0x64},
	{0x1F, 0x64},
	{0x20, 0x64},
	{0x21, 0x00},
	{0x22, 0x00},
	{0x23, 0x00},
	{0x24, 0x00},
	{0x25, 0x00},
	{0x26, 0x00},
	{0x27, 0x00},
	{0x28, 0x00},
	{0x29, 0x64},
	{0x2A, 0x64},
	{0x2B, 0x64},
	{0x2C, 0x64},
	{0x2D, 0x64},
	{0x2E, 0x64},
	{0x2F, 0x64},
	{0x30, 0x64},
	{0x31, 0x0F},
	{0x32, 0x00},
	{0x33, 0x64},
	{0x34, 0x64},
	{0x89, 0x10},
	{0x8B, 0x00},
	{0x8C, 0x00},
	{0x8D, 0x00},
	{0x8E, 0x00},
	{0x8F, 0x00},
	{0x90, 0x02},
	{0x91, 0x00},
	{0x92, 0x11},
	{0x93, 0x10},
	{0x94, 0x00},
	{0x95, 0x00},
	{0x96, 0x00},
	{0x97, 0x00},
	{0x99, 0x00},
	{0x9A, 0x00},
	{0x9B, 0x09},
	{0x9C, 0x00},
	{0x9D, 0x00},
	{0x9E, 0x40},
	{0x9F, 0x00},
	{0xA0, 0x0A},
	{0xA1, 0x00},
	{0xA2, 0x1E},
	{0xA3, 0x07},
	{0xA4, 0xFF},
	{0xA5, 0x03},
	{0xA6, 0xFF},
	{0xA7, 0x00},
	{0xA8, 0x00},
	{0xA9, 0x11},
	{0xAA, 0x23},
	{0xAB, 0x23},
	{0xAD, 0x00},
	{0xAE, 0x00},
	{0xAF, 0x00},
	{0xB0, 0x00},
	{0xB1, 0x00},
	{0xBE, 0x15},/*Hsync ISP*/
	{0xBF, 0x00},
	{0xC0, 0x10},
	{0xC7, 0x10},
	{0xC8, 0x01},
	{0xC9, 0x00},
	{0xCA, 0x55},
	{0xCB, 0x06},
	{0xCC, 0x09},
	{0xCD, 0x00},
	{0xCE, 0xA2},
	{0xCF, 0x00},
	{0xD0, 0x02},
	{0xD1, 0x10},
	{0xD2, 0x1E},
	{0xD3, 0x19},
	{0xD4, 0x04},
	{0xD5, 0x18},
	{0xD6, 0xC8},
	{0xF0, 0x00},
	{0xF1, 0x00},
	{0xF2, 0x00},
	{0xF3, 0x00},
	{0xF4, 0x00},
	{0xF5, 0x40},
	{0xF6, 0x00},
	{0xF7, 0x00},
	{0xF8, 0x00},
	{0xED, 0x01},
	{0xEF, 0x01},
	{0x02, 0xFF},
	{0x03, 0x01},
	{0x04, 0x45},
	{0x05, 0x03},
	{0x06, 0xFF},
	{0x07, 0x00},
	{0x08, 0x00},
	{0x09, 0x00},
	{0x0A, 0x05},
	{0x0B, 0x45},/*Lpf 464 for 30fps*/
	{0x0C, 0x00},
	{0x0D, 0x02},
	{0x0E, 0x00},
	{0x0F, 0x92},
	{0x10, 0x00},
	{0x11, 0x01},
	{0x12, 0x00},
	{0x13, 0x92},
	{0x14, 0x01},
	{0x15, 0x00},
	{0x16, 0x00},
	{0x17, 0x00},
	{0x1A, 0x00},
	{0x1B, 0x07},
	{0x1C, 0x90},
	{0x1D, 0x04},
	{0x1E, 0x4A},
	{0x1F, 0x00},
	{0x20, 0x00},
	{0x21, 0x00},
	{0x22, 0xD4},
	{0x23, 0x10},
	{0x24, 0xA0},
	{0x25, 0x00},
	{0x26, 0x08},
	{0x27, 0x09},
	{0x28, 0x60},
	{0x29, 0x0F},
	{0x2A, 0x09},
	{0x2B, 0x5F},
	{0x2C, 0x10},
	{0x2D, 0x42},
	{0x2E, 0x42},
	{0x2F, 0x10},
	{0x30, 0x41},
	{0x31, 0x44},
	{0x32, 0x10},
	{0x33, 0x41},
	{0x34, 0x00},
	{0x35, 0x01},
	{0x36, 0x00},
	{0x37, 0x2A},
	{0x38, 0x55},
	{0x39, 0x2C},
	{0x3A, 0x4A},
	{0x3B, 0x42},
	{0x3C, 0x02},
	{0x3D, 0x02},
	{0x3E, 0x11},
	{0x3F, 0x5E},
	{0x40, 0x40},
	{0x41, 0x15},
	{0x42, 0x5E},
	{0x43, 0x40},
	{0x44, 0x05},
	{0x45, 0x41},
	{0x46, 0x1c},
	{0x47, 0x05},
	{0x48, 0x50},
	{0x49, 0x0d},
	{0x4A, 0x12},
	{0x4B, 0x40},
	{0x4C, 0x80},
	{0x4D, 0x1E},
	{0x4E, 0x01},
	{0x4F, 0x4A},
	{0x50, 0x05},
	{0x51, 0x05},
	{0x52, 0x56},
	{0x53, 0x05},
	{0x54, 0x00},
	{0x55, 0x00},
	{0x56, 0x78},
	{0x57, 0x00},
	{0x58, 0x92},
	{0x59, 0x00},
	{0x5A, 0xD6},
	{0x5B, 0x00},
	{0x5C, 0x2E},
	{0x5D, 0x01},
	{0x5E, 0x9E},
	{0x5F, 0x00},
	{0x60, 0x3C},
	{0x61, 0x08},
	{0x62, 0x4C},
	{0x63, 0x62},
	{0x64, 0x02},
	{0x65, 0x00},
	{0x66, 0x00},
	{0x67, 0x00},
	{0x68, 0x00},
	{0x69, 0x00},
	{0x6A, 0x00},
	{0x6B, 0x92},
	{0x6C, 0x00},
	{0x6D, 0x64},
	{0x6E, 0x01},
	{0x6F, 0xEE},
	{0x70, 0x00},
	{0x71, 0x64},
	{0x72, 0x08},
	{0x73, 0x98},
	{0x74, 0x00},
	{0x75, 0x00},
	{0x76, 0x00},
	{0x77, 0x00},
	{0x78, 0x0A},
	{0x79, 0xAB},
	{0x7A, 0x50},
	{0x7B, 0x40},
	{0x7C, 0x40},
	{0x7D, 0x00},
	{0x7E, 0x01},
	{0x7F, 0x00},
	{0x80, 0x00},
	{0x87, 0x00},
	{0x88, 0x08},
	{0x89, 0x01},
	{0x8A, 0x04},
	{0x8B, 0x4A},
	{0x8C, 0x00},
	{0x8D, 0x01},
	{0x8E, 0x00},
	{0x90, 0x00},
	{0x91, 0x01},
	{0x92, 0x80},
	{0x93, 0x00},
	{0x94, 0xFF},
	{0x95, 0x00},
	{0x96, 0x00},
	{0x97, 0x01},
	{0x98, 0x02},
	{0x99, 0x09},
	{0x9A, 0x60},
	{0x9B, 0x02},
	{0x9C, 0x60},
	{0x9D, 0x00},
	{0x9E, 0x00},
	{0x9F, 0x00},
	{0xA0, 0x00},
	{0xA1, 0x00},
	{0xA2, 0x00},
	{0xA3, 0x00},
	{0xA4, 0x14},
	{0xA5, 0x04},
	{0xA6, 0x38},
	{0xA7, 0x00},
	{0xA8, 0x08},
	{0xA9, 0x07},
	{0xAA, 0x80},
	{0xAB, 0x01},
	{0xAD, 0x00},
	{0xAE, 0x00},
	{0xAF, 0x00},
	{0xB0, 0x50},
	{0xB1, 0x00},
	{0xB2, 0x00},
	{0xB3, 0x00},
	{0xB4, 0x50},
	{0xB5, 0x07},
	{0xB6, 0x80},
	{0xB7, 0x82},
	{0xB8, 0x0A},
	{0xCD, 0x95},
	{0xCE, 0xF6},
	{0xCF, 0x01},
	{0xD0, 0x42},
	{0xD1, 0x30},
	{0xD2, 0x10},
	{0xD3, 0x1C},
	{0xD4, 0x00},
	{0xD5, 0x01},
	{0xD6, 0x00},
	{0xD7, 0x07},
	{0xD8, 0x84},
	{0xD9, 0x54},
	{0xDA, 0x70},
	{0xDB, 0x70},
	{0xDC, 0x10},
	{0xDD, 0x60},
	{0xDE, 0x50},
	{0xDF, 0x43},
	{0xE0, 0x70},
	{0xE1, 0x01},
	{0xE2, 0x35},
	{0xE3, 0x21},
	{0xE4, 0x66},
	{0xE6, 0x00},
	{0xE7, 0x00},
	{0xF5, 0x02},
	{0xF6, 0xA8},
	{0xF7, 0x03},
	{0xF0, 0x00},
	{0xF4, 0x02},
	{0xF2, 0x19},
	{0xF1, 0x0E},
	{0xF5, 0x12},
	{0x09, 0x01},
	{0xEF, 0x02},
	{0x00, 0x4A},
	{0x01, 0xA0},
	{0x02, 0x03},
	{0x03, 0x00},
	{0x04, 0x00},
	{0x05, 0x30},
	{0x06, 0x04},
	{0x07, 0x01},
	{0x08, 0x03},
	{0x09, 0x00},
	{0x0A, 0x30},
	{0x0B, 0x00},
	{0x0C, 0x00},
	{0x0D, 0x08},
	{0x0E, 0x20},
	{0x0F, 0x08},
	{0x10, 0x42},
	{0x11, 0x00},
	{0x12, 0xC0},
	{0x13, 0x10},
	{0x14, 0x00},
	{0x15, 0x10},
	{0x16, 0x00},
	{0x17, 0x00},
	{0x18, 0x33},
	{0x19, 0x33},
	{0x1A, 0x30},
	{0x30, 0xFF},
	{0x31, 0x06},
	{0x32, 0x07},
	{0x33, 0x85},
	{0x34, 0x00},
	{0x35, 0x00},
	{0x36, 0x00},
	{0x37, 0x01},
	{0x38, 0x00},
	{0x39, 0x00},
	{0x3A, 0xCE},
	{0x3B, 0x17},
	{0x3C, 0x64},
	{0x3D, 0x04},
	{0x3E, 0x00},
	{0x3F, 0x0A},
	{0x40, 0x04},
	{0x41, 0x05},
	{0x42, 0x04},
	{0x43, 0x05},
	{0x45, 0x00},
	{0x46, 0x00},
	{0x47, 0x00},
	{0x48, 0x00},
	{0x49, 0x00},
	{0x4A, 0x00},
	{0x4B, 0x00},
	{0x4C, 0x00},
	{0x4D, 0x00},
	{0x88, 0x07},
	{0x89, 0x22},
	{0x8A, 0x00},
	{0x8B, 0x14},
	{0x8C, 0x00},
	{0x8D, 0x00},
	{0x8E, 0x52},
	{0x8F, 0x60},
	{0x90, 0x20},
	{0x91, 0x00},
	{0x92, 0x01},
	{0x9E, 0x00},
	{0x9F, 0x40},
	{0xA0, 0x30},
	{0xA1, 0x00},
	{0xA2, 0x00},
	{0xA3, 0x03},
	{0xA4, 0xC0},
	{0xA5, 0x00},
	{0xA6, 0x00},
	{0xA7, 0x00},
	{0xA8, 0x00},
	{0xA9, 0x00},
	{0xAA, 0x00},
	{0xAB, 0x00},
	{0xAC, 0x00},
	{0xB7, 0x00},
	{0xB8, 0x00},
	{0xB9, 0x00},
	{0xBA, 0x00},
	{0xBB, 0x00},
	{0xBC, 0x00},
	{0xBD, 0x00},
	{0xBE, 0x00},
	{0xBF, 0x00},
	{0xC0, 0x00},
	{0xC1, 0x00},
	{0xC2, 0x00},
	{0xC3, 0x00},
	{0xC4, 0x00},
	{0xC5, 0x00},
	{0xC6, 0x00},
	{0xC7, 0x00},
	{0xC8, 0x00},
	{0xC9, 0x00},
	{0xCA, 0x00},
	{0xED, 0x01},
	{0xEF, 0x05},
	{0x03, 0x10},
	{0x04, 0xE0},
	{0x05, 0x01},
	{0x06, 0x00},
	{0x07, 0x80},
	{0x08, 0x02},
	{0x09, 0x09},
	{0x0A, 0x04},
	{0x0B, 0x06},
	{0x0C, 0x0C},
	{0x0D, 0xA1},
	{0x0E, 0x00},
	{0x0F, 0x00},
	{0x10, 0x01},
	{0x11, 0x00},
	{0x12, 0x00},
	{0x13, 0x00},
	{0x14, 0xB8},
	{0x15, 0x07},
	{0x16, 0x06},
	{0x17, 0x06},
	{0x18, 0x03},
	{0x19, 0x04},
	{0x1A, 0x06},
	{0x1B, 0x06},
	{0x1C, 0x07},
	{0x1D, 0x08},
	{0x1E, 0x1A},
	{0x1F, 0x00},
	{0x20, 0x00},
	{0x21, 0x1E},
	{0x22, 0x1E},
	{0x23, 0x01},
	{0x24, 0x04},
	{0x27, 0x00},
	{0x28, 0x00},
	{0x2A, 0x08},
	{0x2B, 0x02},
	{0x2C, 0xA4},
	{0x2D, 0x06},
	{0x2E, 0x00},
	{0x2F, 0x05},
	{0x30, 0xE0},
	{0x31, 0x01},
	{0x32, 0x00},
	{0x33, 0x00},
	{0x34, 0x00},
	{0x35, 0x00},
	{0x36, 0x00},
	{0x37, 0x00},
	{0x38, 0x0E},
	{0x39, 0x01},
	{0x3A, 0x02},
	{0x3B, 0x01},
	{0x3C, 0x00},
	{0x3D, 0x00},
	{0x3E, 0x00},
	{0x3F, 0x00},
	{0x40, 0x16},
	{0x41, 0x26},
	{0x42, 0x00},
	{0x47, 0x05},
	{0x48, 0x07},
	{0x49, 0x01},
	{0x4D, 0x02},
	{0x4F, 0x00},
	{0x54, 0x05},
	{0x55, 0x01},
	{0x56, 0x05},
	{0x57, 0x01},
	{0x58, 0x02},
	{0x59, 0x01},
	{0x5B, 0x00},
	{0x5C, 0x03},
	{0x5D, 0x00},
	{0x5E, 0x07},
	{0x5F, 0x08},
	{0x60, 0x00},
	{0x61, 0x00},
	{0x62, 0x00},
	{0x63, 0x28},
	{0x64, 0x30},
	{0x65, 0x9E},
	{0x66, 0xB9},
	{0x67, 0x52},
	{0x68, 0x70},
	{0x69, 0x4E},
	{0x70, 0x00},
	{0x71, 0x00},
	{0x72, 0x00},
	{0x90, 0x04},
	{0x91, 0x01},
	{0x92, 0x00},
	{0x93, 0x00},
	{0x94, 0x03},
	{0x96, 0x00},
	{0x97, 0x01},
	{0x98, 0x01},
	{0xA0, 0x00},
	{0xA1, 0x01},
	{0xA2, 0x00},
	{0xA3, 0x01},
	{0xA4, 0x00},
	{0xA5, 0x01},
	{0xA6, 0x00},
	{0xA7, 0x00},
	{0xAA, 0x00},
	{0xAB, 0x0F},
	{0xAC, 0x08},
	{0xAD, 0x09},
	{0xAE, 0x0A},
	{0xAF, 0x0B},
	{0xB0, 0x00},
	{0xB1, 0x00},
	{0xB2, 0x01},
	{0xB3, 0x00},
	{0xB4, 0x00},
	{0xB5, 0x0A},
	{0xB6, 0x0A},
	{0xB7, 0x0A},
	{0xB8, 0x0A},
	{0xB9, 0x00},
	{0xBA, 0x00},
	{0xBB, 0x00},
	{0xBC, 0x00},
	{0xBD, 0x00},
	{0xBE, 0x00},
	{0xBF, 0x00},
	{0xC0, 0x00},
	{0xC1, 0x00},
	{0xC2, 0x00},
	{0xC3, 0x00},
	{0xC4, 0x00},
	{0xC5, 0x00},
	{0xC6, 0x00},
	{0xC7, 0x00},
	{0xC8, 0x00},
	{0xD3, 0x80},
	{0xD4, 0x00},
	{0xD5, 0x00},
	{0xD6, 0x03},
	{0xD7, 0x77},
	{0xD8, 0x00},
	{0xED, 0x01},
	{0xEF, 0x00},
	{0x11, 0x00},
	{0xEF, 0x01},
	{0x02, 0xFB},
	{0x05, 0x31},
	{0x09, 0x01},

	{PS5230_REG_END, 0x00},	/* END MARKER */
};

/*
 * the order of the ps5230_win_sizes is [full_resolution, preview_resolution].
 */
static struct tx_isp_sensor_win_setting ps5230_win_sizes[] = {
	/* 1920*1080 */
	{
		.width		= 1920,
		.height		= 1080,
		.fps		= 25 << 16 | 1,
		.mbus_code	= V4L2_MBUS_FMT_SBGGR10_1X10,
		.colorspace	= V4L2_COLORSPACE_SRGB,
		.regs 		= ps5230_init_regs_1920_1080_25fps,
	}
};

/*
 * the part of driver was fixed.
 */

static struct regval_list ps5230_stream_on[] = {
	{0xEF, 0x01},
	{0x05, 0x31},	/*sw pwdn off*/
	{0x02, 0xf3},	/*sw reset*/
	{0x09, 0x01},
	{0xEF, 0x01},	/* delay > 1ms */
	{PS5230_REG_DELAY, 0x02},
	{0xEF, 0x00},
	{0x11, 0x00},	/*clk not gated*/
	{PS5230_REG_END, 0x00},	/* END MARKER */
};

static struct regval_list ps5230_stream_off[] = {
	{0xEF, 0x00},
	{0x11, 0x80},	/*clk gated*/
	{0xEF, 0x01},
	{0x05, 0x35},	/*sw pwdn*/
	{0x09, 0x01},
	{PS5230_REG_END, 0x00},	/* END MARKER */
};

int ps5230_read(struct v4l2_subdev *sd, unsigned char reg, unsigned char *value)
{
	int ret;
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct i2c_msg msg[2] = {
		[0] = {
			.addr	= client->addr,
			.flags	= 0,
			.len	= 1,
			.buf	= &reg,
		},
		[1] = {
			.addr	= client->addr,
			.flags	= I2C_M_RD,
			.len	= 1,
			.buf	= value,
		}
	};

	ret = i2c_transfer(client->adapter, msg, 2);
	if (ret > 0)
		ret = 0;
	return ret;
}

int ps5230_write(struct v4l2_subdev *sd, unsigned char reg, unsigned char value)
{
	int ret;
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	unsigned char buf[2] = {reg, value};
	struct i2c_msg msg = {
		.addr	= client->addr,
		.flags	= 0,
		.len	= 2,
		.buf	= buf,
	};

	ret = i2c_transfer(client->adapter, &msg, 1);
	if (ret > 0)
		ret = 0;

	return ret;
}

static int ps5230_read_array(struct v4l2_subdev *sd, struct regval_list *vals)
{
	int ret;
	unsigned char val;
	while (vals->reg_num != PS5230_REG_END) {
		if (vals->reg_num == PS5230_REG_DELAY) {
				msleep(vals->value);
		} else {
			ret = ps5230_read(sd, vals->reg_num, &val);
			if (ret < 0)
				return ret;
			if (vals->reg_num == PS5230_BANK_REG){
				val &= 0xe0;
				val |= (vals->value & 0x1f);
				ret = ps5230_write(sd, vals->reg_num, val);
				ret = ps5230_read(sd, vals->reg_num, &val);
			}
		pr_debug("ps5230_read_array ->> vals->reg_num:0x%02x, vals->reg_value:0x%02x\n",vals->reg_num, val);
		}
		vals++;
	}
	return 0;
}

static int ps5230_write_array(struct v4l2_subdev *sd, struct regval_list *vals)
{
	int ret;
	while (vals->reg_num != PS5230_REG_END) {
		if (vals->reg_num == PS5230_REG_DELAY) {
				msleep(vals->value);
		} else {
			ret = ps5230_write(sd, vals->reg_num, vals->value);
			if (ret < 0){
				printk("ps5230_write error  %d\n" ,__LINE__);
				return ret;
			}
		}
		vals++;
	}
	return 0;
}

static int ps5230_reset(struct v4l2_subdev *sd, u32 val)
{
	return 0;
}

static int ps5230_detect(struct v4l2_subdev *sd, unsigned int *ident)
{
	int ret;
	unsigned char v;
	ret = ps5230_read(sd, 0x00, &v);
	pr_debug("-----%s: %d ret = %d, v = 0x%02x\n", __func__, __LINE__, ret,v);
	if (ret < 0){
		printk("err: ps5230 write error, ret= %d \n",ret);
		return ret;
	}
	if (v != PS5230_CHIP_ID_H)
		return -ENODEV;
	*ident = v;

	ret = ps5230_read(sd, 0x01, &v);
	pr_debug("-----%s: %d ret = %d, v = 0x%02x\n", __func__, __LINE__, ret,v);
	if (ret < 0)
		return ret;
	if (v != PS5230_CHIP_ID_L)
		return -ENODEV;
	*ident = (*ident << 8) | v;
	return 0;
}

static int ps5230_set_integration_time(struct v4l2_subdev *sd, int value)
{
	int ret = 0;
	unsigned int Cmd_OffNy = 0;
	Cmd_OffNy = ps5230_attr.total_height - value;
	ret = ps5230_write(sd, 0xef, 0x01);
	/*Exp Line Control not set*/
	/*ret += ps5230_write(sd, 0x0e, 0x00);*/
	/*ret += ps5230_write(sd, 0x0f, 0x00);*/
	ret += ps5230_write(sd, 0x0d, (unsigned char)(Cmd_OffNy & 0xff));
	ret += ps5230_write(sd, 0x0c, (unsigned char)((Cmd_OffNy & 0xff00) >> 8));
	ret += ps5230_write(sd, 0x09, 0x01);
	if (ret < 0)
		return ret;
	return 0;
}

static int ps5230_set_analog_gain(struct v4l2_subdev *sd, int value)
{
	int ret = 0;
	unsigned int GDAC = value;
	unsigned int Temp_reg;
	unsigned char tmp;
	unsigned char tmp_e1;
	unsigned char tmp_e2;

	ret += ps5230_write(sd, 0xef, 0x01);
	ret += ps5230_write(sd, 0x79, (unsigned char)(GDAC & 0xff));
	ret += ps5230_write(sd, 0x78, (unsigned char)((GDAC & 0x1f00) >> 8));
	ret += ps5230_write(sd, 0x09, 0x01);
	if (ret < 0)
		return ret;

#if 1
	/*high temperature control*/
	ret = ps5230_write(sd, 0xef, 0x02);
	ret += ps5230_read(sd, 0x1c, &tmp);
	Temp_reg = tmp;
	ret += ps5230_read(sd, 0x1b, &tmp);
	if (ret < 0)
		return ret;
	Temp_reg = (Temp_reg | ((tmp & 0x07) << 8));
	ret = ps5230_write(sd, 0xef, 0x01);
	if(ret < 0)
		return ret;
	if ((Temp_reg > 0x001c) || (GDAC < 0x400)) {
		ps5230_read(sd, 0xe1, &tmp_e1);
		ps5230_read(sd, 0xe2, &tmp_e2);
		if ((tmp_e1 & 0x10) != 0x10)
			ps5230_write(sd, 0xe2, ((tmp_e2 & 0xf0) | 0x09));
		else{
			ps5230_write(sd, 0xe2, ((tmp_e2 & 0xf0) | 0x0d));
		}
		ps5230_write(sd, 0xe1, (tmp_e1 | 0x10));
	} else if ((Temp_reg < 0x0016) && (GDAC > 0x0555)) {
		ps5230_read(sd, 0xe1, &tmp_e1);
		ps5230_read(sd, 0xe2, &tmp_e2);
		if ((tmp_e2 & 0x05) == 0x05) {
			ps5230_write(sd, 0xe2, ((tmp_e2 & 0xf0) | 0x09));
			ps5230_write(sd, 0xe1, tmp_e1 | 0x10);
		}else{
			ps5230_write(sd, 0xe2, tmp_e2 & 0xf7);
			ps5230_write(sd, 0xe1, tmp_e1 & 0xef);
		}
	}
	ps5230_write(sd, 0x09, 0x01);
#endif
	return 0;
}

static int ps5230_set_digital_gain(struct v4l2_subdev *sd, int value)
{
	return 0;
}

static int ps5230_get_black_pedestal(struct v4l2_subdev *sd, int value)
{
	return 0;
}

static int ps5230_init(struct v4l2_subdev *sd, u32 enable)
{
	struct tx_isp_sensor *sensor = (container_of(sd, struct tx_isp_sensor, sd));
	struct tx_isp_notify_argument arg;
	struct tx_isp_sensor_win_setting *wsize = &ps5230_win_sizes[0];
	int ret = 0;

	if(!enable)
		return ISP_SUCCESS;
	sensor->video.mbus.width = wsize->width;
	sensor->video.mbus.height = wsize->height;
	sensor->video.mbus.code = wsize->mbus_code;
	sensor->video.mbus.field = V4L2_FIELD_NONE;
	sensor->video.mbus.colorspace = wsize->colorspace;
	sensor->video.fps = wsize->fps;

	ret = ps5230_write_array(sd, wsize->regs);
	if (ret)
		return ret;
	arg.value = (int)&sensor->video;
	sd->v4l2_dev->notify(sd, TX_ISP_NOTIFY_SYNC_VIDEO_IN, &arg);
	sensor->priv = wsize;
	return 0;
}

static int ps5230_s_stream(struct v4l2_subdev *sd, int enable)
{
	int ret = 0;

	if (enable) {
		ret = ps5230_write_array(sd, ps5230_stream_on);
		pr_debug("ps5230 stream on\n");
	}
	else {
		ret = ps5230_write_array(sd, ps5230_stream_off);
		pr_debug("ps5230 stream off\n");
	}
	return ret;
}

static int ps5230_g_parm(struct v4l2_subdev *sd, struct v4l2_streamparm *parms)
{
	return 0;
}

static int ps5230_s_parm(struct v4l2_subdev *sd, struct v4l2_streamparm *parms)
{
	return 0;
}

static int ps5230_set_fps(struct tx_isp_sensor *sensor, int fps)
{
	struct v4l2_subdev *sd = &sensor->sd;
	struct tx_isp_notify_argument arg;
	unsigned int pclk = PS5230_SUPPORT_PCLK;
	unsigned int hts = 0;
	unsigned int vts = 0;
	unsigned int Cmd_Lpf = 0;
	unsigned int Cur_OffNy = 0;
	unsigned int Cur_ExpLine = 0;
	unsigned char tmp;
	unsigned int newformat = 0; //the format is 24.8
	int ret = 0;

	/* the format of fps is 16/16. for example 25 << 16 | 2, the value is 25/2 fps. */
	newformat = (((fps >> 16) / (fps & 0xffff)) << 8) + ((((fps >> 16) % (fps & 0xffff)) << 8) / (fps & 0xffff));
	if(newformat > (SENSOR_OUTPUT_MAX_FPS << 8) || newformat < (SENSOR_OUTPUT_MIN_FPS << 8)){
		printk("warn: fps(%d) no in range\n", fps);
		return -1;
	}
	ret = ps5230_write(sd, 0xef, 0x01);
	if(ret < 0)
		return -1;
	ret = ps5230_read(sd, 0x27, &tmp);
	hts = tmp;
	ret += ps5230_read(sd, 0x28, &tmp);
	if(ret < 0)
		return -1;
	hts = (((hts & 0x1f) << 8) | tmp);

	vts = (pclk * (fps & 0xffff) / hts / ((fps & 0xffff0000) >> 16));
	Cmd_Lpf = vts -1;
	ret = ps5230_write(sd, 0xef, 0x01);
	ret += ps5230_write(sd, 0x0b, (unsigned char)(Cmd_Lpf & 0xff));
	ret += ps5230_write(sd, 0x0a, (unsigned char)(Cmd_Lpf >> 8));
	ret += ps5230_write(sd, 0x09, 0x01);
	if(ret < 0){
		printk("err: ps5230_write err\n");
		return ret;
	}
	ret = ps5230_read(sd, 0x0c, &tmp);
	Cur_OffNy = tmp;
	ret += ps5230_read(sd, 0x0d, &tmp);
	if(ret < 0)
		return -1;
	Cur_OffNy = (((Cur_OffNy & 0xff) << 8) | tmp);
	Cur_ExpLine = ps5230_attr.total_height - Cur_OffNy;

	sensor->video.fps = fps;
	sensor->video.attr->max_integration_time_native = vts - 2;
	sensor->video.attr->integration_time_limit = vts - 2;
	sensor->video.attr->total_height = vts;
	sensor->video.attr->max_integration_time = vts - 2;
	arg.value = (int)&sensor->video;
	sd->v4l2_dev->notify(sd, TX_ISP_NOTIFY_SYNC_VIDEO_IN, &arg);

	ret = ps5230_set_integration_time(sd, Cur_ExpLine);
	if(ret < 0)
		return -1;
	return ret;
}

static int ps5230_set_mode(struct tx_isp_sensor *sensor, int value)
{
	struct tx_isp_notify_argument arg;
	struct v4l2_subdev *sd = &sensor->sd;
	struct tx_isp_sensor_win_setting *wsize = NULL;
	int ret = ISP_SUCCESS;

	if(value == TX_ISP_SENSOR_FULL_RES_MAX_FPS){
		wsize = &ps5230_win_sizes[0];
	}else if(value == TX_ISP_SENSOR_PREVIEW_RES_MAX_FPS){
		wsize = &ps5230_win_sizes[0];
	}

	if(wsize){
		sensor->video.mbus.width = wsize->width;
		sensor->video.mbus.height = wsize->height;
		sensor->video.mbus.code = wsize->mbus_code;
		sensor->video.mbus.field = V4L2_FIELD_NONE;
		sensor->video.mbus.colorspace = wsize->colorspace;
		sensor->video.fps = wsize->fps;
		arg.value = (int)&sensor->video;
		sd->v4l2_dev->notify(sd, TX_ISP_NOTIFY_SYNC_VIDEO_IN, &arg);
	}
	return ret;
}

static int ps5230_g_chip_ident(struct v4l2_subdev *sd,
		struct v4l2_dbg_chip_ident *chip)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	unsigned int ident = 0;
	int ret = ISP_SUCCESS;
	/*if (pwdn_gpio != -1) {
		ret = gpio_request(pwdn_gpio, "ps5230_pwdn");
		if (!ret) {
			gpio_direction_output(pwdn_gpio, 1);
			msleep(50);
			gpio_direction_output(pwdn_gpio, 0);
		} else {
			printk("gpio requrest fail %d\n", pwdn_gpio);
		}
	}*/
	if(reset_gpio != -1){
		ret = gpio_request(reset_gpio,"ps5230_reset");
		if(!ret){
			gpio_direction_output(reset_gpio, 1);
			msleep(5);
			gpio_direction_output(reset_gpio, 0);
			msleep(10);
			gpio_direction_output(reset_gpio, 1);
			msleep(20);
		}else{
			printk("gpio requrest fail %d\n",reset_gpio);
		}
	}

	ret = ps5230_detect(sd, &ident);
	if (ret) {
		v4l_err(client,
			"chip found @ 0x%x (%s) is not an ps5230 chip.\n",
			client->addr, client->adapter->name);
		return ret;
	}
	v4l_info(client, "ps5230 chip found @ 0x%02x (%s)\n",
		client->addr, client->adapter->name);
	return v4l2_chip_ident_i2c_client(client, chip, ident, 0);
}

static int ps5230_s_power(struct v4l2_subdev *sd, int on)
{
	return 0;
}

static long ps5230_ops_private_ioctl(struct tx_isp_sensor *sensor, struct isp_private_ioctl *ctrl)
{
	struct v4l2_subdev *sd = &sensor->sd;
	long ret = 0;
	switch(ctrl->cmd){
		case TX_ISP_PRIVATE_IOCTL_SENSOR_INT_TIME:
			ret = ps5230_set_integration_time(sd, ctrl->value);
			break;
		case TX_ISP_PRIVATE_IOCTL_SENSOR_AGAIN:
			ret = ps5230_set_analog_gain(sd, ctrl->value);
			break;
		case TX_ISP_PRIVATE_IOCTL_SENSOR_DGAIN:
			ret = ps5230_set_digital_gain(sd, ctrl->value);
			break;
		case TX_ISP_PRIVATE_IOCTL_SENSOR_BLACK_LEVEL:
			ret = ps5230_get_black_pedestal(sd, ctrl->value);
			break;
		case TX_ISP_PRIVATE_IOCTL_SENSOR_RESIZE:
			ret = ps5230_set_mode(sensor,ctrl->value);
			break;
		case TX_ISP_PRIVATE_IOCTL_SUBDEV_PREPARE_CHANGE:
			ret = ps5230_write_array(sd, ps5230_stream_off);
			break;
		case TX_ISP_PRIVATE_IOCTL_SUBDEV_FINISH_CHANGE:
			ret = ps5230_write_array(sd, ps5230_stream_on);
			break;
		case TX_ISP_PRIVATE_IOCTL_SENSOR_FPS:
			ret = ps5230_set_fps(sensor, ctrl->value);
			break;
		default:
			break;
	}
	return 0;
}

static long ps5230_ops_ioctl(struct v4l2_subdev *sd, unsigned int cmd, void *arg)
{
	struct tx_isp_sensor *sensor =container_of(sd, struct tx_isp_sensor, sd);
	int ret;
	switch(cmd){
		case VIDIOC_ISP_PRIVATE_IOCTL:
			ret = ps5230_ops_private_ioctl(sensor, arg);
			break;
		default:
			return -1;
			break;
	}
	return 0;
}

#ifdef CONFIG_VIDEO_ADV_DEBUG
static int ps5230_g_register(struct v4l2_subdev *sd, struct v4l2_dbg_register *reg)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	unsigned char val = 0;
	int ret;

	if (!v4l2_chip_match_i2c_client(client, &reg->match))
		return -EINVAL;
	if (!capable(CAP_SYS_ADMIN))
		return -EPERM;
	ret = ps5230_read(sd, reg->reg & 0xffff, &val);
	reg->val = val;
	reg->size = 2;
	return ret;
}

static int ps5230_s_register(struct v4l2_subdev *sd, const struct v4l2_dbg_register *reg)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);

	if (!v4l2_chip_match_i2c_client(client, &reg->match))
		return -EINVAL;
	if (!capable(CAP_SYS_ADMIN))
		return -EPERM;
	ps5230_write(sd, reg->reg & 0xffff, reg->val & 0xff);
	return 0;
}
#endif

static const struct v4l2_subdev_core_ops ps5230_core_ops = {
	.g_chip_ident = ps5230_g_chip_ident,
	.reset = ps5230_reset,
	.init = ps5230_init,
	.s_power = ps5230_s_power,
	.ioctl = ps5230_ops_ioctl,
#ifdef CONFIG_VIDEO_ADV_DEBUG
	.g_register = ps5230_g_register,
	.s_register = ps5230_s_register,
#endif
};

static const struct v4l2_subdev_video_ops ps5230_video_ops = {
	.s_stream = ps5230_s_stream,
	.s_parm = ps5230_s_parm,
	.g_parm = ps5230_g_parm,
};

static const struct v4l2_subdev_ops ps5230_ops = {
	.core = &ps5230_core_ops,
	.video = &ps5230_video_ops,
};

static int ps5230_probe(struct i2c_client *client,
		const struct i2c_device_id *id)
{
	struct v4l2_subdev *sd;
	struct tx_isp_video_in *video;
	struct tx_isp_sensor *sensor;
	struct tx_isp_sensor_win_setting *wsize = &ps5230_win_sizes[0];
	int ret;

	sensor = (struct tx_isp_sensor *)kzalloc(sizeof(*sensor), GFP_KERNEL);
	if(!sensor){
		printk("Failed to allocate sensor subdev.\n");
		return -ENOMEM;
	}
	memset(sensor, 0 ,sizeof(*sensor));
	/* request mclk of sensor */
	sensor->mclk = clk_get(NULL, "cgu_cim");
	if (IS_ERR(sensor->mclk)) {
		printk("Cannot get sensor input clock cgu_cim\n");
		goto err_get_mclk;
	}
	clk_set_rate(sensor->mclk, 24000000);
	clk_enable(sensor->mclk);

	ret = set_sensor_gpio_function(sensor_gpio_func);
	if (ret < 0)
		goto err_set_sensor_gpio;
#if 0
	ps5230_attr.dvp.gpio = sensor_gpio_func;

	switch(sensor_gpio_func){
		case DVP_PA_LOW_10BIT:
		case DVP_PA_HIGH_10BIT:
			mbus = ps5230_mbus_code[0];
			break;
		case DVP_PA_12BIT:
			mbus = ps5230_mbus_code[1];
			break;
		default:
			goto err_set_sensor_gpio;
	}

	for(i = 0; i < ARRAY_SIZE(ps5230_win_sizes); i++)
		ps5230_win_sizes[i].mbus_code = mbus;

#endif
	 /*
		convert sensor-gain into isp-gain,
	 */
	ps5230_attr.max_again = 327675;
	ps5230_attr.max_dgain = 0; //ps5230_attr.max_dgain;
	sd = &sensor->sd;
	video = &sensor->video;
	sensor->video.attr = &ps5230_attr;
	sensor->video.vi_max_width = wsize->width;
	sensor->video.vi_max_height = wsize->height;
	v4l2_i2c_subdev_init(sd, client, &ps5230_ops);
	v4l2_set_subdev_hostdata(sd, sensor);

	pr_debug("@@@@@@@probe ok ------->ps5230\n");
	return 0;
err_set_sensor_gpio:
	clk_disable(sensor->mclk);
	clk_put(sensor->mclk);
err_get_mclk:
	kfree(sensor);

	return -1;
}

static int ps5230_remove(struct i2c_client *client)
{
	struct v4l2_subdev *sd = i2c_get_clientdata(client);
	struct tx_isp_sensor *sensor = v4l2_get_subdev_hostdata(sd);

	if(reset_gpio != -1)
		gpio_free(reset_gpio);
	if(pwdn_gpio != -1)
		gpio_free(pwdn_gpio);

	clk_disable(sensor->mclk);
	clk_put(sensor->mclk);

	v4l2_device_unregister_subdev(sd);
	kfree(sensor);
	return 0;
}

static const struct i2c_device_id ps5230_id[] = {
	{ "ps5230", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, ps5230_id);

static struct i2c_driver ps5230_driver = {
	.driver = {
		.owner	= THIS_MODULE,
		.name	= "ps5230",
	},
	.probe		= ps5230_probe,
	.remove		= ps5230_remove,
	.id_table	= ps5230_id,
};

static __init int init_ps5230(void)
{
	return i2c_add_driver(&ps5230_driver);
}

static __exit void exit_ps5230(void)
{
	i2c_del_driver(&ps5230_driver);
}

module_init(init_ps5230);
module_exit(exit_ps5230);

MODULE_DESCRIPTION("A low-level driver for Primesensor ps5230 sensors");
MODULE_LICENSE("GPL");
