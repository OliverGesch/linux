/*
 * Driver for edm on GPIO lines capable of generating interrupts.
 *
 * Copyright 2015 Wig Cheng <wig.cheng@technexion.com>
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/sysctl.h>
#include <linux/proc_fs.h>
#include <linux/gpio.h>
#include <linux/of_platform.h>
#include <linux/of_gpio.h>
#include <linux/mutex.h>
#include <linux/delay.h>

//#define EDM1_ENABLE
//#define PICO_ENABLE
#define EDM1_SX_ENABLE

#define WB_CHECK				(60) //gpio 2_28
#define WB_WLAN_REG_ON		(26) // gpio 1_26
#define WB_WLAN_WAKE			(30) // gpio 1_30
#define WB_WLAN_HOST_WAKE	(29) // gpio 1_29

#define WB_WLAN_EN_C1			(160) // gpio 6_0
#define WB_WLAN_REF_ON_C1	(159) //gpio 5_31
#define WB_BT_EN_C1			(149) // gpio 5_21
#define WB_BT_WAKE_C1			(158) // gpio 5_30
#define WB_BT_HOST_WAKE_C1	(148) // gpio 5_20

#define WB_WLAN_EN_B1			(130) // gpio 5_2
#define WB_WLAN_REF_ON_B1	(61) //gpio 2_29
#define WB_BT_EN_B1			(77) // gpio 3_13
#define WB_BT_WAKE_B1			(78) // gpio 3_14
#define WB_BT_HOST_WAKE_B1	(79) // gpio 3_15

#define EDM1_WLAN_EN			(160) // gpio 6_0
#define EDM1_WLAN_REF_ON		(159) //gpio 5_31
#define EDM1_WLAN_REG_ON		(26) // gpio 1_26
#define EDM1_WLAN_WAKE		(30) // gpio 1_30
#define EDM1_WLAN_HOST_WAKE	(29) // gpio 1_29

#define EDM1_BT_EN				(149) // gpio 5_21
#define EDM1_BT_WAKE			(158) // gpio 5_30
#define EDM1_BT_HOST_WAKE	(148) // gpio 5_20

#define PICO_WLAN_REG_ON		(7) // gpio 1_7
#define PICO_WLAN_HOST_WAKE	(203) // gpio 7_11
#define PICO_BT_EN				(204) // gpio 7_12
#define PICO_BT_WAKE			(4) // gpio 1_4
#define PICO_BT_HOST_WAKE		(5) // gpio 1_5

struct gpio_edm_data {
		unsigned int gpio;
		unsigned int value;
		const char * direction;
		const char *desc;
};

static unsigned int gpio_num=0;
static unsigned int *gpio_location=NULL;
static DEFINE_MUTEX(gpio_lock);

static int edm1_gpio_init(void)
{
	int val;
	int error;

	mutex_lock(&gpio_lock);
	error = gpio_request(EDM1_WLAN_EN, "EDM1_WLAN_EN");
	gpio_direction_output(EDM1_WLAN_EN,1);
	error = gpio_export(EDM1_WLAN_EN, true);
	msleep(5);
	error = gpio_request(EDM1_WLAN_REF_ON, "EDM1_WLAN_REF_ON");
	gpio_direction_output(EDM1_WLAN_REF_ON,1);
	error = gpio_export(EDM1_WLAN_REF_ON, true);
	msleep(5);
	error = gpio_request(EDM1_WLAN_REG_ON, "EDM1_WLAN_REG_ON");
	gpio_direction_output(EDM1_WLAN_REG_ON,1);
	error = gpio_export(EDM1_WLAN_REG_ON, true);
	msleep(5);
	error = gpio_request(EDM1_WLAN_WAKE, "EDM1_WLAN_WAKE");
	gpio_direction_output(EDM1_WLAN_WAKE,1);
	error = gpio_export(EDM1_WLAN_WAKE, true);
	msleep(5);
	error = gpio_request(EDM1_WLAN_HOST_WAKE, "WB_WLAN_HOST_WAKE");
	gpio_direction_input(EDM1_WLAN_HOST_WAKE);
	error = gpio_export(EDM1_WLAN_HOST_WAKE, true);
	msleep(5);
	error = gpio_request(EDM1_BT_EN, "WB_BT_EN");
	gpio_direction_output(EDM1_BT_EN,1);
	error = gpio_export(EDM1_BT_EN, true);
	msleep(5);
	error = gpio_request(EDM1_BT_WAKE, "WB_BT_WAKE");
	gpio_direction_output(EDM1_BT_WAKE,1);
	error = gpio_export(EDM1_BT_WAKE, true);
	msleep(5);
	error = gpio_request(EDM1_BT_HOST_WAKE, "WB_BT_HOST_WAKE");
	gpio_direction_input(EDM1_BT_HOST_WAKE);
	error = gpio_export(EDM1_BT_HOST_WAKE, true);
	mutex_unlock(&gpio_lock);
}

static int pico_gpio_init(void)
{
	int val;
	int error;

	mutex_lock(&gpio_lock);
	error = gpio_request(PICO_WLAN_REG_ON, "PICO_WLAN_REG_ON");
	gpio_direction_output(PICO_WLAN_REG_ON,1);
	error = gpio_export(PICO_WLAN_REG_ON, true);
	msleep(5);
	error = gpio_request(PICO_WLAN_HOST_WAKE, "PICO_WLAN_HOST_WAKE");
	gpio_direction_input(PICO_WLAN_HOST_WAKE);
	error = gpio_export(PICO_WLAN_HOST_WAKE, true);
	msleep(5);
	error = gpio_request(PICO_BT_EN, "PICO_BT_EN");
	gpio_direction_output(PICO_BT_EN,1);
	error = gpio_export(PICO_BT_EN, true);
	msleep(5);
	error = gpio_request(PICO_BT_WAKE, "PICO_BT_WAKE");
	gpio_direction_output(PICO_BT_WAKE,1);
	error = gpio_export(PICO_BT_WAKE, true);
	msleep(5);
	error = gpio_request(PICO_BT_HOST_WAKE, "PICO_BT_HOST_WAKE");
	gpio_direction_input(PICO_BT_HOST_WAKE);
	error = gpio_export(PICO_BT_HOST_WAKE, true);
	mutex_unlock(&gpio_lock);
}


static int wondboard_gpio_init(void)
{
	int val;
	int error;

	mutex_lock(&gpio_lock);
	val=gpio_get_value(WB_CHECK);
	if(val) { //C1
		error = gpio_request(WB_WLAN_EN_C1, "WB_WLAN_EN");
		gpio_direction_output(WB_WLAN_EN_C1,1);
		error = gpio_export(WB_WLAN_EN_C1, true);
		msleep(5);
		error = gpio_request(WB_WLAN_REF_ON_C1, "WB_WLAN_REF_ON");
		gpio_direction_output(WB_WLAN_REF_ON_C1,1);
		error = gpio_export(WB_WLAN_REF_ON_C1, true);
		msleep(5);
		error = gpio_request(WB_WLAN_REG_ON, "WB_WLAN_REG_ON");
		gpio_direction_output(WB_WLAN_REG_ON,1);
		error = gpio_export(WB_WLAN_REG_ON, true);
		msleep(5);
		error = gpio_request(WB_WLAN_WAKE, "WB_WLAN_WAKE");
		gpio_direction_output(WB_WLAN_WAKE,1);
		error = gpio_export(WB_WLAN_WAKE, true);
		msleep(5);
		error = gpio_request(WB_WLAN_HOST_WAKE, "WB_WLAN_HOST_WAKE");
		gpio_direction_input(WB_WLAN_HOST_WAKE);
		error = gpio_export(WB_WLAN_HOST_WAKE, true);
		msleep(5);
		error = gpio_request(WB_BT_EN_C1, "WB_BT_EN");
		gpio_direction_output(WB_BT_EN_C1,1);
		error = gpio_export(WB_BT_EN_C1, true);
		msleep(5);
		error = gpio_request(WB_BT_WAKE_C1, "WB_BT_WAKE");
		gpio_direction_output(WB_BT_WAKE_C1,1);
		error = gpio_export(WB_BT_WAKE_C1, true);
		msleep(5);
		error = gpio_request(WB_BT_HOST_WAKE_C1, "WB_BT_HOST_WAKE");
		gpio_direction_input(WB_BT_HOST_WAKE_C1);
		error = gpio_export(WB_BT_HOST_WAKE_C1, true);

	}else { // B1
		error = gpio_request(WB_WLAN_EN_B1, "WB_WLAN_EN");
		gpio_direction_output(WB_WLAN_EN_B1,0);
		msleep(11);
		gpio_direction_output(WB_WLAN_EN_B1,1);
		error = gpio_export(WB_WLAN_EN_B1, true);

		msleep(5);
		error = gpio_request(WB_WLAN_REF_ON_B1, "WB_WLAN_REF_ON");
		gpio_direction_output(WB_WLAN_REF_ON_B1,1);
		error = gpio_export(WB_WLAN_REF_ON_B1, true);
		msleep(5);
		error = gpio_request(WB_WLAN_REG_ON, "WB_WLAN_REG_ON");
		gpio_direction_output(WB_WLAN_REG_ON,1);
		error = gpio_export(WB_WLAN_REG_ON, true);
		msleep(5);
		error = gpio_request(WB_WLAN_WAKE, "WB_WLAN_WAKE");
		gpio_direction_output(WB_WLAN_WAKE,1);
		error = gpio_export(WB_WLAN_WAKE, true);
		msleep(5);
		error = gpio_request(WB_WLAN_HOST_WAKE, "WB_WLAN_HOST_WAKE");
		gpio_direction_input(WB_WLAN_HOST_WAKE);
		error = gpio_export(WB_WLAN_HOST_WAKE, true);
		msleep(5);
		error = gpio_request(WB_BT_EN_B1, "WB_BT_EN");
		gpio_direction_output(WB_BT_EN_B1,1);
		error = gpio_export(WB_BT_EN_B1, true);
		msleep(5);
		error = gpio_request(WB_BT_WAKE_B1, "WB_BT_WAKE");
		gpio_direction_output(WB_BT_WAKE_B1,1);
		error = gpio_export(WB_BT_WAKE_B1, true);
		msleep(5);
		error = gpio_request(WB_BT_HOST_WAKE_B1, "WB_BT_HOST_WAKE");
		gpio_direction_input(WB_BT_HOST_WAKE_B1);
		error = gpio_export(WB_BT_HOST_WAKE_B1, true);
	}
	mutex_unlock(&gpio_lock);

	return 0;
}


static int gpio_edm_setup_key(struct platform_device *pdev,
				const struct gpio_edm_data *edm_data)
{
	const char *desc = edm_data->desc ? edm_data->desc : "gpio_edm";
	struct device *dev = &pdev->dev;
	int error;

	if (gpio_is_valid(edm_data->gpio)) {

		mutex_lock(&gpio_lock);
		error = gpio_request(edm_data->gpio, desc);
		if (error < 0) {
			dev_err(dev, "Failed to request GPIO %d, error %d\n",
				edm_data->gpio, error);
			return error;
		}

		if(!strcmp(edm_data->direction,"in"))
			gpio_direction_input(edm_data->gpio);
		else
			gpio_direction_output(edm_data->gpio,edm_data->value);

		error = gpio_export(edm_data->gpio, true);
		if (error < 0) {
			dev_err(dev, "Failed to export GPIO %d, error %d\n",
				edm_data->gpio, error);
			return error;
		}
		mutex_unlock(&gpio_lock);

	}
	else
		goto fail;

	if(!strcmp(desc,"WB_CHECK"))
		wondboard_gpio_init();

#ifdef EDM1_ENABLE
	edm1_gpio_init();
#elif defined PICO_ENABLE
	pico_gpio_init();
#endif

	return 0;

fail:
	if (gpio_is_valid(edm_data->gpio))
		gpio_free(edm_data->gpio);

	return error;
}

#ifdef CONFIG_OF
/*
 * Translate OpenFirmware node properties into platform_data
 */
static struct gpio_edm_data *
gpio_edm_get_devtree_pdata(struct device *dev)
{
	struct device_node *node, *pp;
	struct gpio_edm_data *pdata;

	int error;
	int ngpios;
	int i;

	node = dev->of_node;
	if (!node) {
		error = -ENODEV;
		goto err_out;
	}

	ngpios = of_get_child_count(node);
	if (ngpios == 0) {
		error = -ENODEV;
		goto err_out;
	}

	pdata = kzalloc(sizeof(struct gpio_edm_data) * ngpios,GFP_KERNEL);

	if (!pdata) {
		error = -ENOMEM;
		goto err_out;
	}

	gpio_num= ngpios;

	i = 0;
	for_each_child_of_node(node, pp) {
		int gpio;
		enum of_gpio_flags flags;

		gpio = of_get_gpio_flags(pp, 0, &flags);
		if (gpio < 0) {
			error = gpio;
			if (error != -EPROBE_DEFER)
				dev_err(dev,
					"Failed to get gpio flags, error: %d\n",
					error);
			goto err_free_pdata;
		}

		pdata[i].gpio=gpio;
		pdata[i].desc = of_get_property(pp, "label", NULL);
		pdata[i].direction= of_get_property(pp, "dir", NULL);

		if (of_property_read_u32(pp, "value", &pdata[i].value))
			pdata[i].value = (unsigned int)flags;

		i+=1;
	}

	if (gpio_num == 0) {
		error = -EINVAL;
		goto err_free_pdata;
	}

	return pdata;

err_free_pdata:
	kfree(pdata);
err_out:
	return ERR_PTR(error);
}

static struct of_device_id gpio_edm_of_match[] = {
	{ .compatible = "gpio-edm", },
	{ },
};
MODULE_DEVICE_TABLE(of, gpio_edm_of_match);

#else

static inline struct gpio_edm_data *
gpio_edm_get_devtree_pdata(struct device *dev)
{
	return ERR_PTR(-ENODEV);
}

#endif

static void gpio_remove_gpio(unsigned int gpio_data)
{
	if (gpio_is_valid(gpio_data))
		gpio_free(gpio_data);
}

static int gpio_edm_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct class *edm_class = class_create(THIS_MODULE, "edm");
	const struct gpio_edm_data *pdata = dev_get_platdata(dev);
	int i, error;

	if (!pdata) {
		pdata = gpio_edm_get_devtree_pdata(dev);
		if (IS_ERR(pdata))
			return PTR_ERR(pdata);
	}

	gpio_location=kzalloc(sizeof(unsigned int)*gpio_num,GFP_KERNEL);
	if (!gpio_location) {
		error = -ENOMEM;
		goto fail;
	}

	if (IS_ERR(edm_class)) {
		pr_err("Failed to create sysfs class edm\n");
		return -ENOMEM;
	}
	dev = device_create(edm_class, NULL, 0, NULL, "gpio");

	for (i = 0; i < gpio_num; i++) {
		gpio_location[i]=pdata[i].gpio;
		error = gpio_edm_setup_key(pdev, &pdata[i]);
		if (error)
			goto fail;

		gpio_export_link(dev, pdata[i].desc, pdata[i].gpio);
	}

	return 0;

 fail:
	/* If we have no platform data, we allocated pdata dynamically. */
	if (!dev_get_platdata(&pdev->dev))
		kfree(pdata);

	kfree(gpio_location);

	return error;
}

static int gpio_edm_remove(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	const struct gpio_edm_data *pdata = dev_get_platdata(dev);
	int i;

	for (i = 0; i < gpio_num; i++){
		if(gpio_location)
			gpio_remove_gpio(gpio_location[i]);
	}

	gpio_num=0;

	if(gpio_location)
		kfree(gpio_location);

	kfree(pdata);

	return 0;
}


static struct platform_driver gpio_edm_device_driver = {
	.probe		= gpio_edm_probe,
	.remove		= gpio_edm_remove,
	.driver		= {
		.name	= "gpio-edm",
		.owner	= THIS_MODULE,
		.of_match_table = of_match_ptr(gpio_edm_of_match),
	}
};

static int __init gpio_edm_init(void)
{
	return platform_driver_register(&gpio_edm_device_driver);
}

static void __exit gpio_edm_exit(void)
{
	platform_driver_unregister(&gpio_edm_device_driver);
}

fs_initcall(gpio_edm_init);
//late_initcall(gpio_edm_init);
module_exit(gpio_edm_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Wig Cheng <wig.cheng@technexion.com>");
MODULE_DESCRIPTION("EDM driver for GPIOs");
MODULE_ALIAS("platform:gpio-edm");
