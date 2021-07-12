/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (C) 2018,2019 IBM Corp. */

#ifndef _AST_H
#define _AST_H

#include <stdbool.h>

#include "ahb.h"

enum ahb_bridge_state {
    bridge_state_unknown,
    bridge_state_absent,
    bridge_state_enabled,
    bridge_state_disabled,
};

enum ahb_bridge_mode {
    bridge_mode_none,
    bridge_mode_read,
    bridge_mode_readwrite,
};

extern const char *ahb_bridge_state_desc[4];

struct ast_cap_lpc {
    enum ahb_bridge_state superio;
    struct ahb_range ilpc;
};

enum ast_p2ab_ranges {
    p2ab_fw,
    p2ab_soc,
    p2ab_fmc,
    p2ab_spi,
    p2ab_rsvd,
    p2ab_lpch,
    p2ab_dram,
    p2ab_ranges_max,
};

struct ast_cap_pci {
    enum ahb_bridge_state vga;
    enum ahb_bridge_state vga_mmio;
    enum ahb_bridge_state vga_xdma;
    enum ahb_bridge_state bmc;
    enum ahb_bridge_state bmc_mmio;
    enum ahb_bridge_state bmc_xdma;
    struct ahb_range ranges[p2ab_ranges_max];
};

enum debug_uart { debug_uart1, debug_uart5 };

struct ast_cap_uart {
    enum ahb_bridge_state debug;
    enum debug_uart uart;
};

struct ast_cap_kernel {
    bool have_devmem;
};

struct ast_cap_xdma {
    bool unconstrained;
};

struct ast_interfaces {
    struct ast_cap_lpc lpc;
    struct ast_cap_pci pci;
    struct ast_cap_uart uart;
    struct ast_cap_kernel kernel;
    struct ast_cap_xdma xdma;
};

int ast_ahb_bridge_probe(struct ast_interfaces *state);
int ast_ahb_bridge_discover(struct ahb *ahb, struct ast_interfaces *state);
int ast_ahb_init(struct ahb *ahb, bool rw);

int ast_ahb_from_args(struct ahb *ahb, int argc, char *argv[]);
int ast_ahb_access(const char *name, int argc, char *argv[], struct ahb *ahb);

#endif
