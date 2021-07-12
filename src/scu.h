/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (C) 2021 IBM Corp. */

#ifndef _SCU_H
#define _SCU_H

#include "bits.h"
#include "rev.h"
#include "soc.h"
#include "stdint.h"

#include <errno.h>

#define SCU_G4_STRAP1   	0x70
#define   SCU_G4_STRAP1_SIO_DEC	BIT(20)
#define SCU_G4_STRAP2   	0xd0

#define SCU_G5_STRAP    	0x70
#define   SCU_G5_STRAP_SIO_DEC	BIT(20)
#define SCU_G5_REV      	0x7c

#define SCU_G6_STRAP1		0x500
#define SCU_G6_STRAP2		0x510
#define SCU_G6_STRAP3		0x51C

struct scu_ops;

struct scu {
	struct soc *soc;
	struct soc_region iomem;
	const struct scu_ops *ops;
	const void *match_data;
};

int scu_init(struct scu *ctx, struct soc *soc,
	     const struct soc_device_id *table);
void scu_destroy(struct scu *ctx);

int scu_update_strapping(struct scu *ctx, uint32_t offset, uint32_t mask,
			 uint32_t val);

static inline const void *scu_match_data(struct scu *ctx)
{
	return ctx->match_data;
}

static inline enum ast_generation scu_generation(struct scu *ctx)
{
	return soc_generation(ctx->soc);
}

static inline int scu_readl(struct scu *ctx, uint32_t offset, uint32_t *val)
{
	if (offset > ctx->iomem.length - 4)
		return -EINVAL;

	return soc_readl(ctx->soc, ctx->iomem.start + offset, val);
}

static inline int scu_writel(struct scu *ctx, uint32_t offset, uint32_t val)
{
	if (offset > ctx->iomem.length - 4)
		return -EINVAL;

	return soc_writel(ctx->soc, ctx->iomem.start + offset, val);
}

#endif
