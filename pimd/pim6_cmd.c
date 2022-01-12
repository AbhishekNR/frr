/*
 * PIM for IPv6 FRR
 * Copyright (C) 2022  Mobashshera Rasool <mrasool@vmware.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; see the file COPYING; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include <zebra.h>

#include "lib/json.h"
#include "command.h"
#include "if.h"
#include "prefix.h"
#include "zclient.h"
#include "plist.h"
#include "hash.h"
#include "nexthop.h"
#include "vrf.h"
#include "ferr.h"

#include "pimd.h"
#include "pim6_cmd.h"
#include "pim_vty.h"
#include "lib/northbound_cli.h"
#include "pim_errors.h"
#include "pim_nb.h"

#ifndef VTYSH_EXTRACT_PL
#include "pimd/pim_cmd_clippy.c"
#endif

static struct cmd_node debug_node = {
	.name = "debug",
	.node = DEBUG_NODE,
	.prompt = "",
	.config_write = pim_debug_config_write,
};

DEFUN (show_ipv6_mld_groups,
       show_ipv6_mld_groups_cmd,
       "show ipv6 mld [vrf NAME] groups [json]",
       SHOW_STR
       IPV6_STR
       MLD_STR
       VRF_CMD_HELP_STR
       MLD_GROUP_STR
       JSON_STR)
{
    int idx = 2;
    struct vrf *vrf = pim_cmd_lookup_vrf(vty, argv, argc, &idx);
    bool uj = use_json(argc, argv);

    if (!vrf)
        return CMD_WARNING;

    //To be done
    //mld_show_groups()

    return CMD_SUCCESS;
}

DEFUN (show_ipv6_mld_groups_vrf_all,
       show_ipv6_mld_groups_vrf_all_cmd,
       "show ipv6 mld vrf all groups [json]",
       SHOW_STR
       IPV6_STR
       MLD_STR
       VRF_CMD_HELP_STR
       MLD_GROUP_STR
       JSON_STR)
{
    bool uj = use_json(argc, argv);
    struct vrf *vrf;
    bool first = true;

    if (uj)
        vty_out(vty, "{ ");
    RB_FOREACH (vrf, vrf_name_head, &vrfs_by_name) {
        if (uj) {
            if (!first)
                vty_out(vty, ", ");
            vty_out(vty, " \"%s\": ", vrf->name);
            first = false;
        } else
            vty_out(vty, "VRF: %s\n", vrf->name);
        //To be done
        //mld_show_groups()
    }
    if (uj)
        vty_out(vty, "}\n");

    return CMD_SUCCESS;
}

DEFUN (show_ipv6_mld_interface,
       show_ipv6_mld_interface_cmd,
       "show ipv6 mld [vrf NAME] interface [detail|WORD] [json]",
       SHOW_STR
       IPV6_STR
       MLD_STR
       VRF_CMD_HELP_STR
       "MLD interface information\n"
       "Detailed output\n"
       "interface name\n"
       JSON_STR)
{
    int idx = 2;
    struct vrf *vrf = pim_cmd_lookup_vrf(vty, argv, argc, &idx);
    bool uj = use_json(argc, argv);

    if (!vrf)
        return CMD_WARNING;

    if (argv_find(argv, argc, "detail", &idx)
        || argv_find(argv, argc, "WORD", &idx))
        //To be done
        //mld_show_interfaces_single()
    else
        //To be done
        //mld_show_interfaces()

    return CMD_SUCCESS;
}

DEFUN (show_ipv6_mld_interface_vrf_all,
       show_ipv6_mld_interface_vrf_all_cmd,
       "show ipv6 mld vrf all interface [detail|WORD] [json]",
       SHOW_STR
       IPV6_STR
       MLD_STR
       VRF_CMD_HELP_STR
       "MLD interface information\n"
       "Detailed output\n"
       "interface name\n"
       JSON_STR)
{
    int idx = 2;
    bool uj = use_json(argc, argv);
    struct vrf *vrf;
    bool first = true;

    if (uj)
        vty_out(vty, "{ ");
    RB_FOREACH (vrf, vrf_name_head, &vrfs_by_name) {
        if (uj) {
            if (!first)
                vty_out(vty, ", ");
            vty_out(vty, " \"%s\": ", vrf->name);
            first = false;
        } else
            vty_out(vty, "VRF: %s\n", vrf->name);
        if (argv_find(argv, argc, "detail", &idx)
            || argv_find(argv, argc, "WORD", &idx))
            //To be done
            //mld_show_interfaces_single()
        else
            //To be done
            //mld_show_interfaces()
    }
    if (uj)
        vty_out(vty, "}\n");

    return CMD_SUCCESS;
}

void pim6_cmd_init(void)
{
    if_cmd_init(pim_interface_config_write);

    install_node(&debug_node);

    install_element(VIEW_NODE, &show_ipv6_mld_groups_cmd);
    install_element(VIEW_NODE, &show_ipv6_mld_groups_vrf_all_cmd);
    install_element(VIEW_NODE, &show_ipv6_mld_interface_cmd);
    install_element(VIEW_NODE, &show_ipv6_mld_interface_vrf_all_cmd);
}
