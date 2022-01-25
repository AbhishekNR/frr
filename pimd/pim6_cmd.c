/*
 * PIM for IPv6 FRR
 * Copyright (C) 2022  Vmware, Inc.
 *		       Mobashshera Rasool <mrasool@vmware.com>
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
#include "pimd/pim6_cmd_clippy.c"
#endif

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
	json_object *json = NULL;

	if (uj)
		json = json_object_new_object();

	if (!vrf)
		return CMD_WARNING;

	/* 
	 * TBD Depends on MLD data structure changes
	 * mld_show_groups(vrf->info, vty, uj, json)
	 */

	if (uj) {
		vty_json(vty, json);
		json_object_free(json);
	}

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
	json_object *json = NULL;
	json_object *json_vrf = NULL;

	if (uj)
		json = json_object_new_object();

	RB_FOREACH (vrf, vrf_name_head, &vrfs_by_name) {
		if (!uj)
			vty_out(vty, "VRF: %s\n", vrf->name);
		else
			json_vrf = json_object_new_object();
		/* 
		 * TBD Depends on MLD data structure changes
		 * mld_show_groups(vrf->info, vty, uj, json_vrf)
		 */
		if (uj)
			json_object_object_add(json, vrf->name, json_vrf);
	}
	if (uj) {
		vty_json(vty, json);
		json_object_free(json);
	}

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
	json_object *json = NULL;

	if (!vrf)
		return CMD_WARNING;

	if (uj)
		json = json_object_new_object();

	/*
	 * if (argv_find(argv, argc, "detail", &idx)
	 *	   || argv_find(argv, argc, "WORD", &idx))
	 *	   TBD Depends on MLD data structure changes
	 *	   mld_show_interfaces_single(vrf->info, vty,
	 *					    argv[idx]->arg, uj, json)
	 * else
	 *	   TBD Depends on MLD data structure changes
	 *	   mld_show_interfaces(vrf->info, vty, uj, json)
	 */
	if (uj) {
		vty_json(vty, json);
		json_object_free(json);
	}

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
	/* int idx = 2; */
	bool uj = use_json(argc, argv);
	struct vrf *vrf;
	json_object *json = NULL;
	json_object *json_vrf = NULL;

	if (uj)
		json = json_object_new_object();

	RB_FOREACH (vrf, vrf_name_head, &vrfs_by_name) {
		if (!uj)
			vty_out(vty, "VRF: %s\n", vrf->name);
		else
			json_vrf = json_object_new_object();
		/*
		 * if (argv_find(argv, argc, "detail", &idx)
		 *	  || argv_find(argv, argc, "WORD", &idx))
		 *	  TBD Depends on MLD data structure changes
		 *	  mld_show_interfaces_single(vrf->info, vty,
		 *			argv[idx]->arg, uj, json_vrf)
		 * else
		 *	  TBD Depends on MLD data structure changes
		 *	  mld_show_interfaces(vrf->info, vty, uj, json_vrf)
		 */
		if (uj)
			json_object_object_add(json, vrf->name, json_vrf);
	}
	if (uj) {
		vty_json(vty, json);
		json_object_free(json);
	}

	return CMD_SUCCESS;
}

void pim_cmd_init(void)
{
	if_cmd_init(pim_interface_config_write);
	install_element(VIEW_NODE, &show_ipv6_mld_groups_cmd);
	install_element(VIEW_NODE, &show_ipv6_mld_groups_vrf_all_cmd);
	install_element(VIEW_NODE, &show_ipv6_mld_interface_cmd);
	install_element(VIEW_NODE, &show_ipv6_mld_interface_vrf_all_cmd);
}
