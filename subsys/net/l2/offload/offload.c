/*
 * Copyright (c) 2016 Intel Corporation.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <logging/log.h>
LOG_MODULE_REGISTER(net_l2_offload, LOG_LEVEL_NONE);

#include <net/net_core.h>
#include <net/net_l2.h>
#include <net/net_if.h>
#include <net/net_pkt.h>

#include <net/socket_offload.h>

static inline int enable_offload(struct net_if *iface, bool state)
{
	socket_ops->enable(state);
	return 0;
}


NET_L2_INIT(OFFLOAD_L2, NULL, NULL, enable_offload, NULL);
