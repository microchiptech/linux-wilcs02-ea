// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2012 - 2023 Microchip Technology Inc., and its subsidiaries.
 * All rights reserved.
 */

#include <linux/delay.h>
#include <linux/of.h>
#include <linux/version.h>
#include <linux/of_gpio.h>
#include <linux/gpio.h>

#include "netdev.h"

/**
 * wilc_of_parse_power_pins() - parse power sequence pins; to keep backward
 *		compatibility with old device trees that doesn't provide
 *		power sequence pins we check for default pins on proper boards
 *
 * @wilc:	wilc data structure
 *
 * Returns:	 0 on success, negative error number on failures.
 */
int wilc_of_parse_power_pins(struct wilc *wilc)
{
	struct wilc_power *power = &wilc->power;

	/* get chip_en pin and deassert it (if it is defined): */
	power->gpios.chip_en = devm_gpiod_get_optional(wilc->dev,
						       "chip_en", GPIOD_OUT_LOW);
	/* get RESET pin and assert it (if it is defined): */
	if (power->gpios.chip_en) {
		pr_info("%s got chip_en gpio", __func__);
		/* if enable pin exists, reset must exist as well */
		power->gpios.reset = devm_gpiod_get(wilc->dt_dev,
						    "reset", GPIOD_OUT_HIGH);
		if (IS_ERR(power->gpios.reset)) {
			pr_err("missing reset gpio.\n");
			return PTR_ERR(power->gpios.reset);
		}
	} else {
		power->gpios.reset = devm_gpiod_get_optional(wilc->dt_dev,
							     "reset", GPIOD_OUT_HIGH);
		if (power->gpios.reset)
			pr_info("%s got reset gpio", __func__);
	}
	return 0;
}

/**
 * wilc_wlan_power() - handle power on/off commands
 *
 * @wilc:	wilc data structure
 * @on:		requested power status
 *
 * Returns:	none
 */
void wilc_wlan_power(struct wilc *wilc, bool on)
{
	if (!wilc->power.gpios.chip_en || !wilc->power.gpios.reset)
		return;

	if (on) {
		gpiod_set_value(wilc->power.gpios.chip_en, 1);
		mdelay(5);
		gpiod_set_value(wilc->power.gpios.reset, 1);
	} else {
		gpiod_set_value(wilc->power.gpios.chip_en, 0);
		gpiod_set_value(wilc->power.gpios.reset, 0);
	}
}

void wilc_wlan_power_deinit(struct wilc *wilc)
{
	if (wilc->power.gpios.chip_en)
		devm_gpiod_put(wilc->dt_dev, wilc->power.gpios.chip_en);

	if (wilc->power.gpios.reset)
		devm_gpiod_put(wilc->dt_dev, wilc->power.gpios.reset);
}
