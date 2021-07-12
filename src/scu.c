// SPDX-License-Identifier: Apache-2.0
// Copyright (C) 2021 IBM Corp.

#include "log.h"
#include "scu.h"

#include <errno.h>
#include <inttypes.h>
#include <limits.h>

struct scu_ops {
    int (*update_strapping)(struct scu *ctx, uint32_t offset, uint32_t mask,
                            uint32_t val);
};

int ast2400_scu_update_strapping(struct scu *ctx, uint32_t offset,
                                 uint32_t mask, uint32_t val)
{
    uint32_t reg;
    int rc;

    if (!(offset == SCU_G4_STRAP1 || offset == SCU_G4_STRAP2))
        return -EINVAL;

    if ((rc = scu_readl(ctx, offset, &reg)) < 0)
        return rc;

    reg &= ~mask;
    reg |= val & mask;

    if ((rc = scu_writel(ctx, offset, reg)) < 0)
        return rc;

    return 0;
}

static const struct scu_ops ast2400_scu_ops = {
    .update_strapping = ast2400_scu_update_strapping,
};

int ast2500_scu_update_strapping(struct scu *ctx, uint32_t offset, uint32_t mask,
                                 uint32_t val)
{
    int rc;

    if (offset != SCU_G5_STRAP)
        return -EINVAL;

    if ((rc = scu_writel(ctx, SCU_G5_STRAP, (val & mask))) < 0)
        return rc;

    if ((rc = scu_writel(ctx, SCU_G5_REV, (~val & mask))) < 0)
        return rc;

    return 0;
}

static const struct scu_ops ast2500_scu_ops = {
    .update_strapping = ast2500_scu_update_strapping,
};

int ast2600_scu_update_strapping(struct scu *ctx, uint32_t offset,
                                 uint32_t mask, uint32_t val)
{
    uint32_t reg, prot;
    int rc;

    if (!(offset == SCU_G6_STRAP1
                || offset == SCU_G6_STRAP2
                || offset == SCU_G6_STRAP3))
        return -EINVAL;

    /* STRAP1 and STRAP2 have W1S/W1C pairs, STRAP3 is RMW, because of course */
    if (offset == SCU_G6_STRAP3) {
        if ((rc = scu_readl(ctx, offset, &reg)) < 0)
            return rc;

        reg &= ~mask;
        reg |= (val & mask);

        if ((rc = scu_writel(ctx, offset, reg)) < 0)
            return rc;

        return 0;
    }

    /* Protection register for strapping */
    if ((rc = scu_readl(ctx, offset + 8, &prot)) < 0)
        return rc;

    if (prot & mask) {
        loge("Cannot update requested strapping bits in mask 0x%08" PRIx32 "\n",
             (prot & mask));
        return -EIO;
    }

    /* Value register for strapping */
    if ((rc = scu_writel(ctx, offset, (val & mask))) < 0)
        return rc;

    /* Clear register for strapping */
    if ((rc = scu_writel(ctx, offset + 4, (~val & mask))) < 0)
        return rc;

    return 0;
}

static const struct scu_ops ast2600_scu_ops = {
    .update_strapping = ast2600_scu_update_strapping,
};

static const struct soc_device_id scu_match[] = {
    { .compatible = "aspeed,ast2400-scu", .data = &ast2400_scu_ops },
    { .compatible = "aspeed,ast2500-scu", .data = &ast2500_scu_ops },
    { .compatible = "aspeed,ast2600-scu", .data = &ast2600_scu_ops },
};

int scu_init(struct scu *ctx, struct soc *soc,
             const struct soc_device_id *table)
{
    struct soc_device_node dn;
    int rc;

    if ((rc = soc_device_match_node(soc, scu_match, &dn)) < 0)
        return rc;

    if (table) {
        rc = soc_device_is_compatible(soc, table, &dn);
        if (rc < 0)
            return rc;

        if (!rc)
            return -ENOTSUP;

        ctx->match_data = soc_device_get_match_data(soc, table, &dn);
    } else {
        ctx->match_data = NULL;
    }

    if ((rc = soc_device_get_memory(soc, &dn, &ctx->iomem)) < 0)
        return rc;

    if (!(ctx->ops = soc_device_get_match_data(soc, scu_match, &dn)))
        return rc;

    ctx->soc = soc;

    return 0;
}

void scu_destroy(struct scu *ctx)
{
    ctx->soc = NULL;
}

int scu_update_strapping(struct scu *ctx, uint32_t offset, uint32_t mask,
                         uint32_t val)
{
    return ctx->ops->update_strapping(ctx, offset, mask, val);
}
