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

DEFUN (show_ipv6_mld_statistics,
       show_ipv6_mld_statistics_cmd,
       "show ipv6 mld [vrf NAME] statistics [interface WORD] [json]",
       SHOW_STR
       IPV6_STR
       MLD_STR
       VRF_CMD_HELP_STR
       "MLD statistics\n"
       "interface\n"
       "MLD interface\n"
       JSON_STR)
{
    int idx = 2;
    struct vrf *vrf = pim_cmd_lookup_vrf(vty, argv, argc, &idx);
    bool uj = use_json(argc, argv);

    if (!vrf)
        return CMD_WARNING;

    if (argv_find(argv, argc, "WORD", &idx))
        //To be done
        //mld_show_statistics()
    else
        //To be done
        //mld_show_statistics()

    return CMD_SUCCESS;
}

DEFPY (show_ipv6_mroute,
      show_ipv6_mroute_cmd,
      "show ipv6 mroute [vrf NAME] [A.B.C.D$s_or_g [A.B.C.D$g]] [fill$fill] [json$json]",
      SHOW_STR
      IPV6_STR
      MROUTE_STR
      VRF_CMD_HELP_STR
      "The Source or Group\n"
      "The Group\n"
      "Fill in Assumed data\n"
      JSON_STR)
{
	struct prefix_sg sg = {0};
	struct pim_instance *pim;
	struct vrf *v;
	v = vrf_lookup_by_name(vrf ? vrf : VRF_DEFAULT_NAME);
	if (!v) {
		vty_out(vty, "%% Vrf specified: %s does not exist\n", vrf);
		return CMD_WARNING;
	}
	pim = pim_get_pim_instance(v->vrf_id);
	if (!pim) {
		vty_out(vty, "%% Unable to find pim instance\n");
		return CMD_WARNING;
	}
	if (s_or_g.s_addr != in6addr_any) {
		if (g.s_addr != in6addr_any) {
			sg.src = s_or_g;
			sg.grp = g;
		} else
			sg.grp = s_or_g;
	}
	show_mroute(pim, vty, &sg, !!fill, !!json);
	return CMD_SUCCESS;
}

DEFUN (show_ipv6_mroute_vrf_all,
      show_ipv6_mroute_vrf_all_cmd,
      "show ipv6 mroute vrf all [fill] [json]",
      SHOW_STR
      IPV6_STR
      MROUTE_STR
      VRF_CMD_HELP_STR
      "Fill in Assumed data\n"
      JSON_STR)
{
	struct prefix_sg sg = {0};
	bool uj = use_json(argc, argv);
	int idx = 4;
	struct vrf *vrf;
	bool first = true;
	bool fill = false;
	if (argv_find(argv, argc, "fill", &idx))
		fill = true;
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
		show_mroute(vrf->info, vty, &sg, fill, uj);
	}
	if (uj)
		vty_out(vty, "}\n");
	return CMD_SUCCESS;
}

DEFUN (show_ipv6_mroute_count,
      show_ipv6_mroute_count_cmd,
      "show ipv6 mroute [vrf NAME] count [json]",
      SHOW_STR
      IPV6_STR
      MROUTE_STR
      VRF_CMD_HELP_STR
      "Route and packet count data\n"
      JSON_STR)
{
	int idx = 2;
	bool uj = use_json(argc, argv);
	struct vrf *vrf = pim_cmd_lookup_vrf(vty, argv, argc, &idx);
	if (!vrf)
		return CMD_WARNING;
	show_mroute_count(vrf->info, vty, uj);
	return CMD_SUCCESS;
	}

DEFUN (show_ipv6_mroute_count_vrf_all,
      show_ipv6_mroute_count_vrf_all_cmd,
      "show ipv6 mroute vrf all count [json]",
      SHOW_STR
      IPV6_STR
      MROUTE_STR
      VRF_CMD_HELP_STR
      "Route and packet count data\n"
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
		show_mroute_count(vrf->info, vty, uj);
	}
	if (uj)
		vty_out(vty, "}\n");
	return CMD_SUCCESS;
}

DEFUN (show_ipv6_mroute_summary,
       show_ipv6_mroute_summary_cmd,
       "show ipv6 mroute [vrf NAME] summary [json]",
       SHOW_STR
       IPV6_STR
       MROUTE_STR
       VRF_CMD_HELP_STR
       "Summary of all mroutes\n"
       JSON_STR)
{
	int idx = 2;
	bool uj = use_json(argc, argv);
	struct vrf *vrf = pim_cmd_lookup_vrf(vty, argv, argc, &idx);
	json_object *json = NULL;

	if (uj)
		json = json_object_new_object();

	if (!vrf)
		return CMD_WARNING;

	show_mroute_summary(vrf->info, vty, json);

	if (uj) {
		vty_out(vty, "%s\n",
			json_object_to_json_string_ext(
				json, JSON_C_TO_STRING_PRETTY));
		json_object_free(json);
	}
	return CMD_SUCCESS;
}

DEFUN (show_ipv6_mroute_summary_vrf_all,
       show_ipv6_mroute_summary_vrf_all_cmd,
       "show ip mroute vrf all summary [json]",
       SHOW_STR
       IPV6_STR
       MROUTE_STR
       VRF_CMD_HELP_STR
       "Summary of all mroutes\n"
       JSON_STR)
{
	struct vrf *vrf;
	bool uj = use_json(argc, argv);
	json_object *json = NULL;
	json_object *json_vrf = NULL;

	if (uj)
		json = json_object_new_object();

	RB_FOREACH (vrf, vrf_name_head, &vrfs_by_name) {
		if (uj)
			json_vrf = json_object_new_object();
		else
			vty_out(vty, "VRF: %s\n", vrf->name);

		show_mroute_summary(vrf->info, vty, json_vrf);

		if (uj)
			json_object_object_add(json, vrf->name, json_vrf);
	}

	if (uj) {
		vty_out(vty, "%s\n",
			json_object_to_json_string_ext(
				json, JSON_C_TO_STRING_PRETTY));
		json_object_free(json);
	}

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
    install_element(VIEW_NODE, &show_ipv6_mld_statistics_cmd);
    install_element(VIEW_NODE, &show_ipv6_mroute_cmd);
    install_element(VIEW_NODE, &show_ipv6_mroute_vrf_all_cmd);
    install_element(VIEW_NODE, &show_ipv6_mroute_count_cmd);
    install_element(VIEW_NODE, &show_ipv6_mroute_count_vrf_all_cmd);
    install_element(VIEW_NODE, &show_ipv6_mroute_summary_cmd);
    install_element(VIEW_NODE, &show_ipv6_mroute_summary_vrf_all_cmd);
}
