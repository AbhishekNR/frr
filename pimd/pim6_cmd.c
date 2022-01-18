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
#include "pimd/pim6_cmd_clippy.c"
#endif

DEFUN(interface_ipv6_mld,
      interface_ipv6_mld_cmd,
      "ipv6 mld",
      IPV6_STR
      IFACE_MLD_STR)
{
	nb_cli_enqueue_change(vty, "./enable", NB_OP_MODIFY, "true");

	return nb_cli_apply_changes(vty, FRR_GMP_INTERFACE_XPATH,
				    "frr-routing:ipv6");
}

DEFUN(interface_no_ipv6_mld,
      interface_no_ipv6_mld_cmd,
      "no ipv6 mld",
      NO_STR
      IPV6_STR
      IFACE_MLD_STR)
{
	const struct lyd_node *pim_enable_dnode;
	char pim_if_xpath[XPATH_MAXLEN + 20];

	snprintf(pim_if_xpath, sizeof(pim_if_xpath), "%s/frr-pim:pim",
		 VTY_CURR_XPATH);

	pim_enable_dnode = yang_dnode_getf(vty->candidate_config->dnode,
					   "%s/pim-enable", pim_if_xpath);
	if (!pim_enable_dnode) {
		nb_cli_enqueue_change(vty, pim_if_xpath, NB_OP_DESTROY, NULL);
		nb_cli_enqueue_change(vty, ".", NB_OP_DESTROY, NULL);
	} else {
		if (!yang_dnode_get_bool(pim_enable_dnode, ".")) {
			nb_cli_enqueue_change(vty, pim_if_xpath, NB_OP_DESTROY,
					      NULL);
			nb_cli_enqueue_change(vty, ".", NB_OP_DESTROY, NULL);
		} else
			nb_cli_enqueue_change(vty, "./enable", NB_OP_MODIFY,
					      "false");
	}

	return nb_cli_apply_changes(vty, FRR_GMP_INTERFACE_XPATH,
				    "frr-routing:ipv6");
}

DEFUN(interface_ipv6_mld_join, interface_ipv6_mld_join_cmd,
      "ipv6 mld join X:X::X:X [X:X::X:X]",
      IPV6_STR
      IFACE_MLD_STR
      "MLD join multicast group\n"
      "Multicast group address\n"
      "Source address\n")
{
	int idx_group = 3;
	int idx_source = 4;
	const char *source_str;
	char xpath[XPATH_MAXLEN];
	struct prefix_ipv6 p;

	if (argc == 5) {
		source_str = argv[idx_source]->arg;

		(void)str2prefix_ipv6(source_str, &p);

		if (IPV6_ADDR_SAME(&p.prefix, &in6addr_any)) {
			vty_out(vty, "Bad source address %s\n",
				argv[idx_source]->arg);
			return CMD_WARNING_CONFIG_FAILED;
		}
	} else
		source_str = "::";

	snprintf(xpath, sizeof(xpath), FRR_GMP_JOIN_XPATH, "frr-routing:ipv6",
		 argv[idx_group]->arg, source_str);

	nb_cli_enqueue_change(vty, xpath, NB_OP_CREATE, NULL);

	return nb_cli_apply_changes(vty, NULL);
}

DEFUN(interface_no_ipv6_mld_join, interface_no_ipv6_mld_join_cmd,
      "no ipv6 mld join X:X::X:X [X:X::X:X]",
      NO_STR
      IPV6_STR
      IFACE_MLD_STR
      "MLD join multicast group\n"
      "Multicast group address\n"
      "Source address\n")
{
	int idx_group = 4;
	int idx_source = 5;
	const char *source_str;
	char xpath[XPATH_MAXLEN];
	struct prefix_ipv6 p;

	if (argc == 6) {
		source_str = argv[idx_source]->arg;

		(void)str2prefix_ipv6(source_str, &p);

		if (IPV6_ADDR_SAME(&p.prefix, &in6addr_any)) {
			vty_out(vty, "Bad source address %s\n",
				argv[idx_source]->arg);
			return CMD_WARNING_CONFIG_FAILED;
		}
	} else
		source_str = "::";

	snprintf(xpath, sizeof(xpath), FRR_GMP_JOIN_XPATH, "frr-routing:ipv6",
		 argv[idx_group]->arg, source_str);

	nb_cli_enqueue_change(vty, xpath, NB_OP_DESTROY, NULL);

	return nb_cli_apply_changes(vty, NULL);
}

DEFUN(interface_ipv6_mld_version,
      interface_ipv6_mld_version_cmd,
      "ipv6 mld version (1-2)",
      IPV6_STR
      IFACE_MLD_STR
      "MLD version\n"
      "MLD version number\n")
{
	nb_cli_enqueue_change(vty, "./enable", NB_OP_MODIFY, "true");
	/* TBD
	 * nb_cli_enqueue_change(vty, "./mld-version", NB_OP_MODIFY,
	 * argv[3]->arg); return nb_cli_apply_changes(vty,
	 * FRR_GMP_INTERFACE_XPATH, "frr-routing:ipv6");
	 */
}

DEFUN(interface_no_ipv6_mld_version,
      interface_no_ipv6_mld_version_cmd,
      "no ipv6 mld version (1-2)",
      NO_STR
      IPV6_STR
      IFACE_MLD_STR
      "MLD version\n"
      "MLD version number\n")
{
	/* TBD
	 * nb_cli_enqueue_change(vty, "./mld-version", NB_OP_DESTROY, NULL);
	 * return nb_cli_apply_changes(vty, FRR_GMP_INTERFACE_XPATH,
	 *			    "frr-routing:ipv6");
	 */
}

DEFUN(interface_ipv6_mld_query_interval,
      interface_ipv6_mld_query_interval_cmd,
      "ipv6 mld query-interval (1-65535)",
      IPV6_STR
      IFACE_MLD_STR
      IFACE_MLD_QUERY_INTERVAL_STR
      "Query interval in seconds\n")
{
	const struct lyd_node *pim_enable_dnode;

	pim_enable_dnode =
		yang_dnode_getf(vty->candidate_config->dnode,
				"%s/frr-pim:pim/pim-enable", VTY_CURR_XPATH);
	if (!pim_enable_dnode) {
		nb_cli_enqueue_change(vty, "./enable", NB_OP_MODIFY, "true");
	} else {
		if (!yang_dnode_get_bool(pim_enable_dnode, "."))
			nb_cli_enqueue_change(vty, "./enable", NB_OP_MODIFY,
					      "true");
	}
	nb_cli_enqueue_change(vty, "./query-interval", NB_OP_MODIFY,
			      argv[3]->arg);
	return nb_cli_apply_changes(vty, FRR_GMP_INTERFACE_XPATH,
				    "frr-routing:ipv6");
}

DEFUN(interface_no_ipv6_mld_query_interval,
      interface_no_ipv6_mld_query_interval_cmd,
      "no ipv6 mld query-interval [(1-65535)]",
      NO_STR
      IPV6_STR
      IFACE_MLD_STR
      IFACE_MLD_QUERY_INTERVAL_STR
      IGNORED_IN_NO_STR)
{
	nb_cli_enqueue_change(vty, "./query-interval", NB_OP_DESTROY, NULL);
	return nb_cli_apply_changes(vty, FRR_GMP_INTERFACE_XPATH,
				    "frr-routing:ipv6");
}

void pim6_cmd_init(void)
{
	if_cmd_init(pim_interface_config_write);

	install_element(INTERFACE_NODE, &interface_ipv6_mld_cmd);
	install_element(INTERFACE_NODE, &interface_no_ipv6_mld_cmd);
	install_element(INTERFACE_NODE, &interface_ipv6_mld_join_cmd);
	install_element(INTERFACE_NODE, &interface_no_ipv6_mld_join_cmd);
	install_element(INTERFACE_NODE, &interface_ipv6_mld_version_cmd);
	install_element(INTERFACE_NODE, &interface_no_ipv6_mld_version_cmd);
	install_element(INTERFACE_NODE, &interface_ipv6_mld_query_interval_cmd);
	install_element(INTERFACE_NODE,
			&interface_no_ipv6_mld_query_interval_cmd);
}
