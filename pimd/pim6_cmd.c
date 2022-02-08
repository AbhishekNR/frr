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
#include "pim_cmd_common.h"
#include "pim_vty.h"
#include "lib/northbound_cli.h"
#include "pim_errors.h"
#include "pim_nb.h"

#ifndef VTYSH_EXTRACT_PL
#include "pimd/pim6_cmd_clippy.c"
#endif

DEFPY (show_ipv6_pim_rp,
       show_ipv6_pim_rp_cmd,
       "show ipv6 pim [vrf NAME] rp-info [json$json]",
       SHOW_STR
       IPV6_STR
       PIM_STR
       VRF_CMD_HELP_STR
       "PIM RP information\n"
       JSON_STR)
{
	struct vrf *v =
		vrf_lookup_by_name(vrf ? vrf : VRF_DEFAULT_NAME);
	json_object *json_parent = NULL;

	if (!v)
		return CMD_WARNING;

	if (json)
		json_parent = json_object_new_object();

	pim_rp_show_information(v->info, vty, json_parent);

	if (json)
		vty_json(vty, json_parent);

	return CMD_SUCCESS;
}

DEFPY (show_ipv6_pim_rp_vrf_all,
       show_ipv6_pim_rp_vrf_all_cmd,
       "show ipv6 pim vrf all rp-info [json$json]",
       SHOW_STR
       IPV6_STR
       PIM_STR
       VRF_CMD_HELP_STR
       "PIM RP information\n"
       JSON_STR)
{
	struct vrf *vrf;
	json_object *json_parent = NULL;
	json_object *json_vrf = NULL;

	if (json)
		json_parent = json_object_new_object();

	RB_FOREACH (vrf, vrf_name_head, &vrfs_by_name) {
		if (!json)
			vty_out(vty, "VRF: %s\n", vrf->name);
		else
			json_vrf = json_object_new_object();
		pim_rp_show_information(vrf->info, vty, json_vrf);
		if (json)
			json_object_object_add(json_parent, vrf->name,
					       json_vrf);
	}
	if (json)
		vty_json(vty, json_parent);

	return CMD_SUCCESS;
}

void pim_cmd_init(void)
{
	if_cmd_init(pim_interface_config_write);
	install_element(VIEW_NODE, &show_ipv6_pim_rp_cmd);
	install_element(VIEW_NODE, &show_ipv6_pim_rp_vrf_all_cmd);
}
