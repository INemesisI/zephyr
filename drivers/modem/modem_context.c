/** @file
 * @brief Modem context helper driver
 *
 * A modem context driver allowing application to handle all
 * aspects of received protocol data.
 */

/*
 * Copyright (c) 2019 Foundries.io
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <logging/log.h>
LOG_MODULE_REGISTER(modem_context, CONFIG_MODEM_LOG_LEVEL);

#include <kernel.h>

#include "modem_context.h"

static struct modem_context *contexts[CONFIG_MODEM_CONTEXT_MAX_NUM];

void modem_context_hexdump(const char *desc, const u8_t *packet, size_t length)
{
#if defined(CONFIG_MODEM_CONTEXT_VERBOSE_DEBUG)
	char output[sizeof("xxxxyyyy xxxxyyyy")];
	int n = 0, k = 0;
	u8_t byte;

	printk("\n");
	while (length--) {
		if (n % 16 == 0) {
			printk("%s: %08X ", desc, n);
		}

		byte = *packet++;

		printk("%02X ", byte);

		if (byte < 0x20 || byte > 0x7f) {
			output[k++] = '.';
		} else {
			output[k++] = byte;
		}

		n++;
		if (n % 8 == 0) {
			if (n % 16 == 0) {
				output[k] = '\0';
				printk(" [%s]\n", output);
				k = 0;
			} else {
				printk(" ");
			}
		}
	}

	if (n % 16) {
		int i;

		output[k] = '\0';

		for (i = 0; i < (16 - (n % 16)); i++) {
			printk("   ");
		}

		if ((n % 16) < 8) {
			printk(" "); /* one extra delimiter after 8 chars */
		}

		printk(" [%s]\n", output);
	}
#endif
}

char *modem_context_sprint_ip_addr(const struct sockaddr *addr)
{
	static char buf[NET_IPV6_ADDR_LEN];

	if (addr->sa_family == AF_INET6) {
		return net_addr_ntop(AF_INET6, &net_sin6(addr)->sin6_addr,
				     buf, sizeof(buf));
	}

	if (addr->sa_family == AF_INET) {
		return net_addr_ntop(AF_INET, &net_sin(addr)->sin_addr,
				     buf, sizeof(buf));
	}

	LOG_ERR("Unknown IP address family:%d", addr->sa_family);
	strcpy(buf, "unk");
	return buf;
}

int modem_context_get_addr_port(const struct sockaddr *addr, u16_t *port)
{
	if (!addr || !port) {
		return -EINVAL;
	}

	if (addr->sa_family == AF_INET6) {
		*port = ntohs(net_sin6(addr)->sin6_port);
		return 0;
	} else if (addr->sa_family == AF_INET) {
		*port = ntohs(net_sin(addr)->sin_port);
		return 0;
	}

	return -EPROTONOSUPPORT;
}

/**
 * @brief  Finds modem context which owns the iface device.
 *
 * @param  *dev: device used by the modem iface.
 *
 * @retval Modem context or NULL.
 */
struct modem_context *modem_context_from_iface_dev(struct device *dev)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(contexts); i++) {
		if (contexts[i] && contexts[i]->iface.dev == dev) {
			return contexts[i];
		}
	}

	return NULL;
}

/**
 * @brief  Assign a modem context if there is free space.
 *
 * @note   Amount of stored modem contexts is determined by
 *         CONFIG_MODEM_CONTEXT_MAX_NUM.
 *
 * @param  *ctx: modem context to persist.
 *
 * @retval 0 if ok, < 0 if error.
 */
static int modem_context_get(struct modem_context *ctx)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(contexts); i++) {
		if (!contexts[i]) {
			contexts[i] = ctx;
			return 0;
		}
	}

	return -ENOMEM;
}

struct modem_context *modem_context_from_id(int id)
{
	if (id >= 0 && id < ARRAY_SIZE(contexts)) {
		return contexts[id];
	} else {
		return NULL;
	}
}

int modem_context_register(struct modem_context *ctx)
{
	int ret;

	if (!ctx) {
		return -EINVAL;
	}

	ret = modem_context_get(ctx);
	if (ret < 0) {
		return ret;
	}

	ret = modem_pin_init(ctx);
	if (ret < 0) {
		LOG_ERR("modem pin init error: %d", ret);
		return ret;
	}

	return 0;
}
