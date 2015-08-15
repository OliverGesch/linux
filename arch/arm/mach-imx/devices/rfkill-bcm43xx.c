/*
 *
 * Copyright (C) 2013 Vladimir Ermakov <vooon341@gmail.com>
 * Copyright (C) 2014 Richard Hu <linuxfae@technexion.com>
 *
 * based on arch/arm/mach-imx/devices/wand-rfkill.c
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/of_device.h>
#include <linux/pinctrl/consumer.h>
#include <linux/platform_device.h>
#include <linux/rfkill.h>
#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>


struct bcm43xx_rfkill_data {
	struct rfkill *rfkill_dev;
	int shutdown_gpio;
	const char *shutdown_name;
};

static int bcm43xx_rfkill_set_block(void *data, bool blocked)
{
	struct bcm43xx_rfkill_data *rfkill = data;

	pr_debug("bcm43xx-rfkill: set block %d\n", blocked);

	if (blocked) {
		if (gpio_is_valid(rfkill->shutdown_gpio))
			gpio_direction_output(rfkill->shutdown_gpio, 0);
	} else {
		if (gpio_is_valid(rfkill->shutdown_gpio))
			gpio_direction_output(rfkill->shutdown_gpio, 1);
	}

	return 0;
}

static const struct rfkill_ops bcm43xx_rfkill_ops = {
	.set_block = bcm43xx_rfkill_set_block,
};

static int bcm43xx_rfkill_wifi_probe(struct device *dev,
		struct device_node *np,
		struct bcm43xx_rfkill_data *rfkill
		)
{
	int ret;
	int wl_ref_on, wl_rst_n, wl_reg_on, wl_host_wake;

	wl_ref_on = of_get_named_gpio(np, "wifi-ref-on", 0);
	wl_rst_n = of_get_named_gpio(np, "wifi-rst-n", 0);
	wl_reg_on = of_get_named_gpio(np, "wifi-reg-on", 0);
	wl_host_wake = of_get_named_gpio(np, "wifi-host-wake", 0);

	dev_info(dev, "initialize wifi chip\n");

	if (gpio_is_valid(wl_rst_n)) {
		gpio_request(wl_rst_n, "wl_rst_n");
		gpio_direction_output(wl_rst_n, 0);
		gpio_export(wl_rst_n,true);
		msleep(11);
		gpio_set_value_cansleep(wl_rst_n, 1);
	}

	if (gpio_is_valid(wl_ref_on)) {
		gpio_request(wl_ref_on, "wl_ref_on");
		gpio_direction_output(wl_ref_on, 1);
		gpio_export(wl_ref_on,true);
	}

	if (gpio_is_valid(wl_reg_on)) {
		gpio_request(wl_reg_on, "wl_reg_on");
		gpio_direction_output(wl_reg_on, 1);
		gpio_export(wl_reg_on,true);
	}

	if (gpio_is_valid(wl_host_wake)) {
		gpio_request(wl_host_wake, "wl_host_wake");
		gpio_direction_input(wl_host_wake);
		gpio_export(wl_host_wake,true);
	}

	rfkill->shutdown_name = "wifi_shutdown";
	rfkill->shutdown_gpio = wl_reg_on;

	rfkill->rfkill_dev = rfkill_alloc("wifi-rfkill", dev, RFKILL_TYPE_WLAN,
			&bcm43xx_rfkill_ops, rfkill);
	if (!rfkill->rfkill_dev) {
		ret = -ENOMEM;
		goto wifi_fail_free_gpio;
	}

	ret = rfkill_register(rfkill->rfkill_dev);
	if (ret < 0)
		goto wifi_fail_unregister;

	dev_info(dev, "wifi-rfkill registered.\n");

	return 0;

wifi_fail_unregister:
	rfkill_destroy(rfkill->rfkill_dev);
wifi_fail_free_gpio:
	if (gpio_is_valid(wl_ref_on))     gpio_free(wl_ref_on);
	if (gpio_is_valid(wl_rst_n))    gpio_free(wl_rst_n);
	if (gpio_is_valid(wl_reg_on))    gpio_free(wl_reg_on);
	if (gpio_is_valid(wl_host_wake))      gpio_free(wl_host_wake);

	return ret;
}

static int bcm43xx_rfkill_bt_probe(struct device *dev,
		struct device_node *np,
		struct bcm43xx_rfkill_data *rfkill
		)
{
	int ret;
	int bt_rst_n, bt_reg_on, bt_wake, bt_host_wake;

	bt_rst_n = of_get_named_gpio(np, "bluetooth-on", 0);
	bt_reg_on = of_get_named_gpio(np, "bluetooth-reg-on", 0);
	bt_wake = of_get_named_gpio(np, "bluetooth-wake", 0);
	bt_host_wake = of_get_named_gpio(np, "bluetooth-host-wake", 0);

	dev_info(dev, "initialize bluetooth chip\n");

	if (gpio_is_valid(bt_rst_n)) {
		gpio_request(bt_rst_n, "bt_rst_n");
		gpio_direction_output(bt_rst_n, 0);
		gpio_export(bt_rst_n,true);
		msleep(11);
		gpio_set_value_cansleep(bt_rst_n, 1);
	}

	if (gpio_is_valid(bt_reg_on)) {
		gpio_request(bt_reg_on, "bt_reg_on");
		gpio_direction_output(bt_reg_on, 1);
		gpio_export(bt_reg_on,true);
	}

	if (gpio_is_valid(bt_wake)) {
		gpio_request(bt_wake, "bt_wake");
		gpio_direction_output(bt_wake, 1);
		gpio_export(bt_wake,true);
	}

	if (gpio_is_valid(bt_host_wake)) {
		gpio_request(bt_host_wake, "bt_host_wake");
		gpio_direction_input(bt_host_wake);
		gpio_export(bt_host_wake,true);
	}

	rfkill->shutdown_name = "bluetooth_shutdown";
	rfkill->shutdown_gpio = bt_reg_on;

	rfkill->rfkill_dev = rfkill_alloc("bluetooth-rfkill", dev, RFKILL_TYPE_BLUETOOTH,
			&bcm43xx_rfkill_ops, rfkill);
	if (!rfkill->rfkill_dev) {
		ret = -ENOMEM;
		goto bt_fail_free_gpio;
	}

	ret = rfkill_register(rfkill->rfkill_dev);
	if (ret < 0)
		goto bt_fail_unregister;

	dev_info(dev, "bluetooth-rfkill registered.\n");

	return 0;

bt_fail_unregister:
	rfkill_destroy(rfkill->rfkill_dev);
bt_fail_free_gpio:
	if (gpio_is_valid(bt_rst_n))        gpio_free(bt_rst_n);
	if (gpio_is_valid(bt_reg_on))        gpio_free(bt_reg_on);
	if (gpio_is_valid(bt_wake))      gpio_free(bt_wake);
	if (gpio_is_valid(bt_host_wake)) gpio_free(bt_host_wake);

	return ret;
}

static int bcm43xx_rfkill_probe(struct platform_device *pdev)
{
	struct bcm43xx_rfkill_data *rfkill;
	struct pinctrl *pinctrl;
	int ret;

	dev_info(&pdev->dev, "bcm43xx rfkill initialization\n");

	if (!pdev->dev.of_node) {
		dev_err(&pdev->dev, "no device tree node\n");
		return -ENODEV;
	}

	rfkill = kzalloc(sizeof(*rfkill) * 2, GFP_KERNEL);
	if (!rfkill)
		return -ENOMEM;

	pinctrl = devm_pinctrl_get_select_default(&pdev->dev);
	if (IS_ERR(pinctrl)) {
		int ret = PTR_ERR(pinctrl);
		dev_err(&pdev->dev, "failed to get default pinctrl: %d\n", ret);
		return ret;
	}

	/* setup WiFi */
	ret = bcm43xx_rfkill_wifi_probe(&pdev->dev, pdev->dev.of_node, &rfkill[0]);
	if (ret < 0)
		goto fail_free_rfkill;

	/* setup bluetooth */
	ret = bcm43xx_rfkill_bt_probe(&pdev->dev, pdev->dev.of_node, &rfkill[1]);
	if (ret < 0)
		goto fail_unregister_wifi;

	platform_set_drvdata(pdev, rfkill);

	return 0;

fail_unregister_wifi:
	if (rfkill[1].rfkill_dev) {
		rfkill_unregister(rfkill[1].rfkill_dev);
		rfkill_destroy(rfkill[1].rfkill_dev);
	}

	/* TODO free gpio */

fail_free_rfkill:
	kfree(rfkill);

	return ret;
}

static int bcm43xx_rfkill_remove(struct platform_device *pdev)
{
	struct bcm43xx_rfkill_data *rfkill = platform_get_drvdata(pdev);

	dev_info(&pdev->dev, "Module unloading\n");

	if (!rfkill)
		return 0;

	/* WiFi */
	if (gpio_is_valid(rfkill[0].shutdown_gpio))
		gpio_free(rfkill[0].shutdown_gpio);

	rfkill_unregister(rfkill[0].rfkill_dev);
	rfkill_destroy(rfkill[0].rfkill_dev);

	/* Bt */
	if (gpio_is_valid(rfkill[1].shutdown_gpio))
		gpio_free(rfkill[1].shutdown_gpio);

	rfkill_unregister(rfkill[1].rfkill_dev);
	rfkill_destroy(rfkill[1].rfkill_dev);

	kfree(rfkill);

	return 0;
}

static struct of_device_id bcm43xx_rfkill_match[] = {
	{ .compatible = "bcm,bcm43xx_rfkill", },
	{}
};

static struct platform_driver bcm43xx_rfkill_driver = {
	.driver = {
		.name = "bcm43xx-rfkill",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(bcm43xx_rfkill_match),
	},
	.probe = bcm43xx_rfkill_probe,
	.remove = bcm43xx_rfkill_remove
};

module_platform_driver(bcm43xx_rfkill_driver);

MODULE_AUTHOR("Richard Hu <linuxfae@technexion.com>");
MODULE_DESCRIPTION("bcm43xx rfkill driver");
MODULE_LICENSE("GPL v2");
