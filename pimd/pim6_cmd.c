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

DEFPY (show_ipv6_pim_rpf,
       show_ipv6_pim_rpf_cmd,
       "show ipv6 pim [vrf NAME] rpf [json$json]",
       SHOW_STR
       IPV6_STR
       PIM_STR
       VRF_CMD_HELP_STR
       "PIM cached source rpf information\n"
       JSON_STR)
{
	struct vrf *v =
		vrf_lookup_by_name(vrf ? vrf : VRF_DEFAULT_NAME);
	json_object *json_parent = NULL;

	if (!v)
		return CMD_WARNING;

	if (json)
		json_parent = json_object_new_object();

	pim_show_rpf(v->info, vty, json_parent);

	if (json)
		vty_json(vty, json_parent);

	return CMD_SUCCESS;
}

DEFPY (show_ipv6_pim_rpf_vrf_all,
       show_ipv6_pim_rpf_vrf_all_cmd,
       "show ipv6 pim vrf all rpf [json$json]",
       SHOW_STR
       IPV6_STR
       PIM_STR
       VRF_CMD_HELP_STR
       "PIM cached source rpf information\n"
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
		pim_show_rpf(vrf->info, vty, json_vrf);
		if (json)
			json_object_object_add(json_parent, vrf->name, json_vrf);
	}
	if (json)
		vty_json(vty, json_parent);

	return CMD_SUCCESS;
}

DEFPY (show_ipv6_pim_secondary,
       show_ipv6_pim_secondary_cmd,
       "show ipv6 pim [vrf NAME] secondary",
       SHOW_STR
       IPV6_STR
       PIM_STR
       VRF_CMD_HELP_STR
       "PIM neighbor addresses\n")
{
	struct vrf *v =
		vrf_lookup_by_name(vrf ? vrf : VRF_DEFAULT_NAME);

	if (!v)
		return CMD_WARNING;

	pim_show_neighbors_secondary(v->info, vty);

	return CMD_SUCCESS;
}

DEFPY (show_ipv6_pim_statistics,
       show_ipv6_pim_statistics_cmd,
       "show ipv6 pim [vrf NAME] statistics [interface WORD$word] [json]",
       SHOW_STR
       IPV6_STR
       PIM_STR
       VRF_CMD_HELP_STR
       "PIM statistics\n"
       INTERFACE_STR
       "PIM interface\n"
       JSON_STR)
{
	struct vrf *v =
		vrf_lookup_by_name(vrf ? vrf : VRF_DEFAULT_NAME);
	bool uj = use_json(argc, argv);

	if (!v)
		return CMD_WARNING;

	if (word)
		pim_show_statistics(v->info, vty, word, uj);
	else
		pim_show_statistics(v->info, vty, NULL, uj);

	return CMD_SUCCESS;
}

DEFPY (show_ipv6_pim_upstream,
       show_ipv6_pim_upstream_cmd,
       "show ipv6 pim [vrf NAME] upstream [X:X::X:X$s_or_g [X:X::X:X$g]] [json$json]",
       SHOW_STR
       IPV6_STR
       PIM_STR
       VRF_CMD_HELP_STR
       "PIM upstream information\n"
       "The Source or Group\n"
       "The Group\n"
       JSON_STR)
{
	pim_sgaddr sg = {0};
	struct vrf *v;
	bool uj = !!json;
	struct pim_instance *pim;
	json_object *json_parent = NULL;

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

	if (uj)
		json_parent = json_object_new_object();

	if (!IPV6_ADDR_SAME(&s_or_g, &in6addr_any)) {
		if (!IPV6_ADDR_SAME(&g, &in6addr_any)) {
			memcpy(&sg.src, &s_or_g, sizeof(s_or_g));
			memcpy(&sg.grp, &g, sizeof(g));
		} else
			memcpy(&sg.grp, &s_or_g, sizeof(s_or_g));
	}

	pim_show_upstream(pim, vty, &sg, json_parent);

	if (uj)
		vty_json(vty, json_parent);

	return CMD_SUCCESS;
}

DEFPY (show_ipv6_pim_upstream_vrf_all,
       show_ipv6_pim_upstream_vrf_all_cmd,
       "show ipv6 pim vrf all upstream [json$json]",
       SHOW_STR
       IPV6_STR
       PIM_STR
       VRF_CMD_HELP_STR
       "PIM upstream information\n"
       JSON_STR)
{
	pim_sgaddr sg = {0};
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
		pim_show_upstream(vrf->info, vty, &sg, json_vrf);
		if (json)
			json_object_object_add(json_parent, vrf->name,
					       json_vrf);
	}

	if (json)
		vty_json(vty, json_parent);

	return CMD_SUCCESS;
}

DEFPY (show_ipv6_pim_upstream_join_desired,
       show_ipv6_pim_upstream_join_desired_cmd,
       "show ipv6 pim [vrf NAME] upstream-join-desired [json$json]",
       SHOW_STR
       IPV6_STR
       PIM_STR
       VRF_CMD_HELP_STR
       "PIM upstream join-desired\n"
       JSON_STR)
{
	struct vrf *v =
		vrf_lookup_by_name(vrf ? vrf : VRF_DEFAULT_NAME);
	bool uj = !!json;

	if (!v)
		return CMD_WARNING;

	pim_show_join_desired(v->info, vty, uj);

	return CMD_SUCCESS;
}

DEFPY (show_ipv6_pim_upstream_rpf,
       show_ipv6_pim_upstream_rpf_cmd,
       "show ipv6 pim [vrf NAME] upstream-rpf [json$json]",
       SHOW_STR
       IPV6_STR
       PIM_STR
       VRF_CMD_HELP_STR
       "PIM upstream source rpf\n"
       JSON_STR)
{
	struct vrf *v =
		vrf_lookup_by_name(vrf ? vrf : VRF_DEFAULT_NAME);
	bool uj = !!json;

	if (!v)
		return CMD_WARNING;

	pim_show_upstream_rpf(v->info, vty, uj);

	return CMD_SUCCESS;
}

DEFPY (show_ipv6_pim_state,
       show_ipv6_pim_state_cmd,
       "show ipv6 pim [vrf NAME] state [X:X::X:X$s_or_g [X:X::X:X$g]] [json$json]",
       SHOW_STR
       IPV6_STR
       PIM_STR
       VRF_CMD_HELP_STR
       "PIM state information\n"
       "Unicast or Multicast address\n"
       "Multicast address\n"
       JSON_STR)
{
	struct vrf *v =
		vrf_lookup_by_name(vrf ? vrf : VRF_DEFAULT_NAME);
	json_object *json_parent = NULL;

	if (!v)
		return CMD_WARNING;

	if (json)
		json_parent = json_object_new_object();

	pim_show_state(v->info, vty, s_or_g_str, g_str, json_parent);

	if (json)
		vty_json(vty, json_parent);

	return CMD_SUCCESS;
}

DEFPY (show_ipv6_pim_state_vrf_all,
       show_ipv6_pim_state_vrf_all_cmd,
       "show ipv6 pim vrf all state [X:X::X:X$s_or_g [X:X::X:X$g]] [json$json]",
       SHOW_STR
       IPV6_STR
       PIM_STR
       VRF_CMD_HELP_STR
       "PIM state information\n"
       "Unicast or Multicast address\n"
       "Multicast address\n"
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
		pim_show_state(vrf->info, vty, s_or_g_str, g_str, json_vrf);
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
	install_element(VIEW_NODE, &show_ipv6_pim_rpf_cmd);
	install_element(VIEW_NODE, &show_ipv6_pim_rpf_vrf_all_cmd);
	install_element(VIEW_NODE, &show_ipv6_pim_secondary_cmd);
	install_element(VIEW_NODE, &show_ipv6_pim_statistics_cmd);
	install_element(VIEW_NODE, &show_ipv6_pim_upstream_cmd);
	install_element(VIEW_NODE, &show_ipv6_pim_upstream_vrf_all_cmd);
	install_element(VIEW_NODE, &show_ipv6_pim_upstream_join_desired_cmd);
	install_element(VIEW_NODE, &show_ipv6_pim_upstream_rpf_cmd);
	install_element(VIEW_NODE, &show_ipv6_pim_state_cmd);
	install_element(VIEW_NODE, &show_ipv6_pim_state_vrf_all_cmd);
}
