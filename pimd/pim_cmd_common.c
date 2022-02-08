/*
 * PIM for Quagga
 * Copyright (C) 2022  Vmware, Inc.
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
#include "pim_mroute.h"
#include "pim_cmd.h"
#include "pim6_cmd.h"
#include "pim_cmd_common.h"
#include "pim_time.h"
#include "pim_zebra.h"
#include "pim_zlookup.h"
#include "pim_iface.h"
#include "lib/linklist.h"
#include "pim_neighbor.h"

void json_object_pim_upstream_add(json_object *json, struct pim_upstream *up)
{
	if (up->flags & PIM_UPSTREAM_FLAG_MASK_DR_JOIN_DESIRED)
		json_object_boolean_true_add(json, "drJoinDesired");

	if (up->flags & PIM_UPSTREAM_FLAG_MASK_DR_JOIN_DESIRED_UPDATED)
		json_object_boolean_true_add(json, "drJoinDesiredUpdated");

	if (up->flags & PIM_UPSTREAM_FLAG_MASK_FHR)
		json_object_boolean_true_add(json, "firstHopRouter");

	if (up->flags & PIM_UPSTREAM_FLAG_MASK_SRC_IGMP)
		json_object_boolean_true_add(json, "sourceIgmp");

	if (up->flags & PIM_UPSTREAM_FLAG_MASK_SRC_PIM)
		json_object_boolean_true_add(json, "sourcePim");

	if (up->flags & PIM_UPSTREAM_FLAG_MASK_SRC_STREAM)
		json_object_boolean_true_add(json, "sourceStream");

	/* XXX: need to print ths flag in the plain text display as well */
	if (up->flags & PIM_UPSTREAM_FLAG_MASK_SRC_MSDP)
		json_object_boolean_true_add(json, "sourceMsdp");

	if (up->flags & PIM_UPSTREAM_FLAG_MASK_SEND_SG_RPT_PRUNE)
		json_object_boolean_true_add(json, "sendSGRptPrune");

	if (up->flags & PIM_UPSTREAM_FLAG_MASK_SRC_LHR)
		json_object_boolean_true_add(json, "lastHopRouter");

	if (up->flags & PIM_UPSTREAM_FLAG_MASK_DISABLE_KAT_EXPIRY)
		json_object_boolean_true_add(json, "disableKATExpiry");

	if (up->flags & PIM_UPSTREAM_FLAG_MASK_STATIC_IIF)
		json_object_boolean_true_add(json, "staticIncomingInterface");

	if (up->flags & PIM_UPSTREAM_FLAG_MASK_ALLOW_IIF_IN_OIL)
		json_object_boolean_true_add(json,
					     "allowIncomingInterfaceinOil");

	if (up->flags & PIM_UPSTREAM_FLAG_MASK_NO_PIMREG_DATA)
		json_object_boolean_true_add(json, "noPimRegistrationData");

	if (up->flags & PIM_UPSTREAM_FLAG_MASK_FORCE_PIMREG)
		json_object_boolean_true_add(json, "forcePimRegistration");

	if (up->flags & PIM_UPSTREAM_FLAG_MASK_SRC_VXLAN_ORIG)
		json_object_boolean_true_add(json, "sourceVxlanOrigination");

	if (up->flags & PIM_UPSTREAM_FLAG_MASK_SRC_VXLAN_TERM)
		json_object_boolean_true_add(json, "sourceVxlanTermination");

	if (up->flags & PIM_UPSTREAM_FLAG_MASK_MLAG_VXLAN)
		json_object_boolean_true_add(json, "mlagVxlan");

	if (up->flags & PIM_UPSTREAM_FLAG_MASK_MLAG_NON_DF)
		json_object_boolean_true_add(json,
					     "mlagNonDesignatedForwarder");
}

static const char *
pim_upstream_state2brief_str(enum pim_upstream_state join_state,
			     char *state_str, size_t state_str_len)
{
	switch (join_state) {
	case PIM_UPSTREAM_NOTJOINED:
		strlcpy(state_str, "NotJ", state_str_len);
		break;
	case PIM_UPSTREAM_JOINED:
		strlcpy(state_str, "J", state_str_len);
		break;
	default:
		strlcpy(state_str, "Unk", state_str_len);
	}
	return state_str;
}

static const char *pim_reg_state2brief_str(enum pim_reg_state reg_state,
					   char *state_str,
					   size_t state_str_len)
{
	switch (reg_state) {
	case PIM_REG_NOINFO:
		strlcpy(state_str, "RegNI", state_str_len);
		break;
	case PIM_REG_JOIN:
		strlcpy(state_str, "RegJ", state_str_len);
		break;
	case PIM_REG_JOIN_PENDING:
	case PIM_REG_PRUNE:
		strlcpy(state_str, "RegP", state_str_len);
		break;
	}
	return state_str;
}

static void show_rpf_refresh_stats(struct vty *vty, struct pim_instance *pim,
				   time_t now, json_object *json)
{
	char refresh_uptime[10];

	pim_time_uptime_begin(refresh_uptime, sizeof(refresh_uptime), now,
			      pim->rpf_cache_refresh_last);

	if (json) {
		json_object_int_add(json, "rpfCacheRefreshDelayMsecs",
				    router->rpf_cache_refresh_delay_msec);
		json_object_int_add(
			json, "rpfCacheRefreshTimer",
			pim_time_timer_remain_msec(pim->rpf_cache_refresher));
		json_object_int_add(json, "rpfCacheRefreshRequests",
				    pim->rpf_cache_refresh_requests);
		json_object_int_add(json, "rpfCacheRefreshEvents",
				    pim->rpf_cache_refresh_events);
		json_object_string_add(json, "rpfCacheRefreshLast",
				       refresh_uptime);
		json_object_int_add(json, "nexthopLookups",
				    pim->nexthop_lookups);
		json_object_int_add(json, "nexthopLookupsAvoided",
				    pim->nexthop_lookups_avoided);
	} else {
		vty_out(vty,
			"RPF Cache Refresh Delay:    %ld msecs\n"
			"RPF Cache Refresh Timer:    %ld msecs\n"
			"RPF Cache Refresh Requests: %lld\n"
			"RPF Cache Refresh Events:   %lld\n"
			"RPF Cache Refresh Last:     %s\n"
			"Nexthop Lookups:            %lld\n"
			"Nexthop Lookups Avoided:    %lld\n",
			router->rpf_cache_refresh_delay_msec,
			pim_time_timer_remain_msec(pim->rpf_cache_refresher),
			(long long)pim->rpf_cache_refresh_requests,
			(long long)pim->rpf_cache_refresh_events,
			refresh_uptime, (long long)pim->nexthop_lookups,
			(long long)pim->nexthop_lookups_avoided);
	}
}

void pim_show_rpf(struct pim_instance *pim, struct vty *vty, json_object *json)
{
	struct pim_upstream *up;
	time_t now = pim_time_monotonic_sec();
	json_object *json_group = NULL;
	json_object *json_row = NULL;

	show_rpf_refresh_stats(vty, pim, now, json);

	if (!json) {
		vty_out(vty, "\n");
		vty_out(vty,
			"Source          Group           RpfIface         RpfAddress      RibNextHop      Metric Pref\n");
	}

	frr_each (rb_pim_upstream, &pim->upstream_head, up) {
		char rpf_addr_str[PREFIX_STRLEN];
		char rib_nexthop_str[PREFIX_STRLEN];
		const char *rpf_ifname;
		struct pim_rpf *rpf = &up->rpf;

		pim_addr_dump("<rpf?>", &rpf->rpf_addr, rpf_addr_str,
			      sizeof(rpf_addr_str));
		pim_addr_dump("<nexthop?>",
			      &rpf->source_nexthop.mrib_nexthop_addr,
			      rib_nexthop_str, sizeof(rib_nexthop_str));

		rpf_ifname =
			rpf->source_nexthop.interface ? rpf->source_nexthop
								.interface->name
						      : "<ifname?>";

		if (json) {
			char src_str[PIM_ADDRSTRLEN];
			char grp_str[PIM_ADDRSTRLEN];

			snprintfrr(grp_str, sizeof(grp_str), "%pPAs",
				   &up->sg.grp);
			snprintfrr(src_str, sizeof(src_str), "%pPAs",
				   &up->sg.src);

			json_object_object_get_ex(json, grp_str, &json_group);

			if (!json_group) {
				json_group = json_object_new_object();
				json_object_object_add(json, grp_str,
						       json_group);
			}

			json_row = json_object_new_object();
			json_object_string_addf(json_row, "source", "%pPAs",
						&up->sg.src);
			json_object_string_addf(json_row, "group", "%pPAs",
						&up->sg.grp);
			json_object_string_add(json_row, "rpfInterface",
					       rpf_ifname);
			json_object_string_add(json_row, "rpfAddress",
					       rpf_addr_str);
			json_object_string_add(json_row, "ribNexthop",
					       rib_nexthop_str);
			json_object_int_add(
				json_row, "routeMetric",
				rpf->source_nexthop.mrib_route_metric);
			json_object_int_add(
				json_row, "routePreference",
				rpf->source_nexthop.mrib_metric_preference);
			json_object_object_add(json_group, src_str, json_row);

		} else {
			vty_out(vty,
				"%-15pPAs %-15pPAs %-16s %-15s %-15s %6d %4d\n",
				&up->sg.src, &up->sg.grp, rpf_ifname,
				rpf_addr_str, rib_nexthop_str,
				rpf->source_nexthop.mrib_route_metric,
				rpf->source_nexthop.mrib_metric_preference);
		}
	}
}

static void show_scan_oil_stats(struct pim_instance *pim, struct vty *vty,
				time_t now)
{
	char uptime_scan_oil[10];
	char uptime_mroute_add[10];
	char uptime_mroute_del[10];

	pim_time_uptime_begin(uptime_scan_oil, sizeof(uptime_scan_oil), now,
			      pim->scan_oil_last);
	pim_time_uptime_begin(uptime_mroute_add, sizeof(uptime_mroute_add), now,
			      pim->mroute_add_last);
	pim_time_uptime_begin(uptime_mroute_del, sizeof(uptime_mroute_del), now,
			      pim->mroute_del_last);

	vty_out(vty,
		"Scan OIL - Last: %s  Events: %lld\n"
		"MFC Add  - Last: %s  Events: %lld\n"
		"MFC Del  - Last: %s  Events: %lld\n",
		uptime_scan_oil, (long long)pim->scan_oil_events,
		uptime_mroute_add, (long long)pim->mroute_add_events,
		uptime_mroute_del, (long long)pim->mroute_del_events);
}

void show_multicast_interfaces(struct pim_instance *pim, struct vty *vty,
			       bool uj)
{
	struct interface *ifp;
	char buf[PREFIX_STRLEN];
	json_object *json = NULL;
	json_object *json_row = NULL;

	vty_out(vty, "\n");

	if (uj)
		json = json_object_new_object();
	else
		vty_out(vty,
			"Interface        Address            ifi Vif  PktsIn PktsOut    BytesIn   BytesOut\n");

	FOR_ALL_INTERFACES (pim->vrf, ifp) {
		struct pim_interface *pim_ifp;
		struct in_addr ifaddr;
		struct sioc_vif_req vreq;

		pim_ifp = ifp->info;

		if (!pim_ifp)
			continue;

		memset(&vreq, 0, sizeof(vreq));
		vreq.vifi = pim_ifp->mroute_vif_index;

		if (ioctl(pim->mroute_socket, SIOCGETVIFCNT, &vreq)) {
			zlog_warn(
				"ioctl(SIOCGETVIFCNT=%lu) failure for interface %s vif_index=%d: errno=%d: %s",
				(unsigned long)SIOCGETVIFCNT, ifp->name,
				pim_ifp->mroute_vif_index, errno,
				safe_strerror(errno));
		}

		ifaddr = pim_ifp->primary_address;
		if (uj) {
			json_row = json_object_new_object();
			json_object_string_add(json_row, "name", ifp->name);
			json_object_string_add(json_row, "state",
					       if_is_up(ifp) ? "up" : "down");
			json_object_string_addf(json_row, "address", "%pI4",
						&pim_ifp->primary_address);
			json_object_int_add(json_row, "ifIndex", ifp->ifindex);
			json_object_int_add(json_row, "vif",
					    pim_ifp->mroute_vif_index);
			json_object_int_add(json_row, "pktsIn",
					    (unsigned long)vreq.icount);
			json_object_int_add(json_row, "pktsOut",
					    (unsigned long)vreq.ocount);
			json_object_int_add(json_row, "bytesIn",
					    (unsigned long)vreq.ibytes);
			json_object_int_add(json_row, "bytesOut",
					    (unsigned long)vreq.obytes);
			json_object_object_add(json, ifp->name, json_row);
		} else {
			vty_out(vty,
				"%-16s %-15s %3d %3d %7lu %7lu %10lu %10lu\n",
				ifp->name,
				inet_ntop(AF_INET, &ifaddr, buf, sizeof(buf)),
				ifp->ifindex, pim_ifp->mroute_vif_index,
				(unsigned long)vreq.icount,
				(unsigned long)vreq.ocount,
				(unsigned long)vreq.ibytes,
				(unsigned long)vreq.obytes);
		}
	}

	if (uj)
		vty_json(vty, json);
}

void pim_cmd_show_ip_multicast_helper(struct pim_instance *pim, struct vty *vty)
{
	struct vrf *vrf = pim->vrf;
	time_t now = pim_time_monotonic_sec();
	char uptime[10];
	char mlag_role[80];

	pim = vrf->info;

	vty_out(vty, "Router MLAG Role: %s\n",
		mlag_role2str(router->mlag_role, mlag_role, sizeof(mlag_role)));
	vty_out(vty, "Mroute socket descriptor:");

	vty_out(vty, " %d(%s)\n", pim->mroute_socket, vrf->name);

	pim_time_uptime(uptime, sizeof(uptime),
			now - pim->mroute_socket_creation);
	vty_out(vty, "Mroute socket uptime: %s\n", uptime);

	vty_out(vty, "\n");

	pim_zebra_zclient_update(vty);
	pim_zlookup_show_ip_multicast(vty);

	vty_out(vty, "\n");
	vty_out(vty, "Maximum highest VifIndex: %d\n", PIM_MAX_USABLE_VIFS);

	vty_out(vty, "\n");
	vty_out(vty, "Upstream Join Timer: %d secs\n", router->t_periodic);
	vty_out(vty, "Join/Prune Holdtime: %d secs\n", PIM_JP_HOLDTIME);
	vty_out(vty, "PIM ECMP: %s\n", pim->ecmp_enable ? "Enable" : "Disable");
	vty_out(vty, "PIM ECMP Rebalance: %s\n",
		pim->ecmp_rebalance_enable ? "Enable" : "Disable");

	vty_out(vty, "\n");

	show_rpf_refresh_stats(vty, pim, now, NULL);

	vty_out(vty, "\n");

	show_scan_oil_stats(pim, vty, now);

	show_multicast_interfaces(pim, vty, false);
}

void pim_show_neighbors_secondary(struct pim_instance *pim, struct vty *vty)
{
	struct interface *ifp;

	vty_out(vty,
		"Interface        Address         Neighbor        Secondary      \n");

	FOR_ALL_INTERFACES (pim->vrf, ifp) {
		struct pim_interface *pim_ifp;
		struct in_addr ifaddr;
		struct listnode *neighnode;
		struct pim_neighbor *neigh;
		char buf[PREFIX_STRLEN];

		pim_ifp = ifp->info;

		if (!pim_ifp)
			continue;

		if (pim_ifp->pim_sock_fd < 0)
			continue;

		ifaddr = pim_ifp->primary_address;

		for (ALL_LIST_ELEMENTS_RO(pim_ifp->pim_neighbor_list, neighnode,
					  neigh)) {
			char neigh_src_str[INET_ADDRSTRLEN];
			struct listnode *prefix_node;
			struct prefix *p;

			if (!neigh->prefix_list)
				continue;

			pim_inet4_dump("<src?>", neigh->source_addr,
				       neigh_src_str, sizeof(neigh_src_str));

			for (ALL_LIST_ELEMENTS_RO(neigh->prefix_list,
						  prefix_node, p))
				vty_out(vty, "%-16s %-15s %-15s %-15pFX\n",
					ifp->name,
					inet_ntop(AF_INET, &ifaddr, buf,
						  sizeof(buf)),
					neigh_src_str, p);
		}
	}
}

void pim_show_state(struct pim_instance *pim, struct vty *vty,
		    const char *src_or_group, const char *group, bool uj)
{
	struct channel_oil *c_oil;
	json_object *json = NULL;
	json_object *json_group = NULL;
	json_object *json_ifp_in = NULL;
	json_object *json_ifp_out = NULL;
	json_object *json_source = NULL;
	time_t now;
	int first_oif;

	now = pim_time_monotonic_sec();

	if (uj) {
		json = json_object_new_object();
	} else {
		vty_out(vty,
			"Codes: J -> Pim Join, I -> IGMP Report, S -> Source, * -> Inherited from (*,G), V -> VxLAN, M -> Muted");
		vty_out(vty,
			"\nActive Source           Group            RPT  IIF               OIL\n");
	}

	frr_each (rb_pim_oil, &pim->channel_oil_head, c_oil) {
		char grp_str[INET_ADDRSTRLEN];
		char src_str[INET_ADDRSTRLEN];
		char in_ifname[INTERFACE_NAMSIZ + 1];
		char out_ifname[INTERFACE_NAMSIZ + 1];
		int oif_vif_index;
		struct interface *ifp_in;
		bool isRpt;

		first_oif = 1;

		if ((c_oil->up &&
		     PIM_UPSTREAM_FLAG_TEST_USE_RPT(c_oil->up->flags)) ||
		    c_oil->oil.mfcc_origin.s_addr == INADDR_ANY)
			isRpt = true;
		else
			isRpt = false;

		pim_inet4_dump("<group?>", c_oil->oil.mfcc_mcastgrp, grp_str,
			       sizeof(grp_str));
		pim_inet4_dump("<source?>", c_oil->oil.mfcc_origin, src_str,
			       sizeof(src_str));
		ifp_in = pim_if_find_by_vif_index(pim, c_oil->oil.mfcc_parent);

		if (ifp_in)
			strlcpy(in_ifname, ifp_in->name, sizeof(in_ifname));
		else
			strlcpy(in_ifname, "<iif?>", sizeof(in_ifname));

		if (src_or_group) {
			if (strcmp(src_or_group, src_str) &&
			    strcmp(src_or_group, grp_str))
				continue;

			if (group && strcmp(group, grp_str))
				continue;
		}

		if (uj) {

			/* Find the group, create it if it doesn't exist */
			json_object_object_get_ex(json, grp_str, &json_group);

			if (!json_group) {
				json_group = json_object_new_object();
				json_object_object_add(json, grp_str,
						       json_group);
			}

			/* Find the source nested under the group, create it if
			 * it doesn't exist
			 */
			json_object_object_get_ex(json_group, src_str,
						  &json_source);

			if (!json_source) {
				json_source = json_object_new_object();
				json_object_object_add(json_group, src_str,
						       json_source);
			}

			/* Find the inbound interface nested under the source,
			 * create it if it doesn't exist
			 */
			json_object_object_get_ex(json_source, in_ifname,
						  &json_ifp_in);

			if (!json_ifp_in) {
				json_ifp_in = json_object_new_object();
				json_object_object_add(json_source, in_ifname,
						       json_ifp_in);
				json_object_int_add(json_source, "Installed",
						    c_oil->installed);
				if (isRpt)
					json_object_boolean_true_add(
						json_source, "isRpt");
				else
					json_object_boolean_false_add(
						json_source, "isRpt");
				json_object_int_add(json_source, "RefCount",
						    c_oil->oil_ref_count);
				json_object_int_add(json_source, "OilListSize",
						    c_oil->oil_size);
				json_object_int_add(
					json_source, "OilRescan",
					c_oil->oil_inherited_rescan);
				json_object_int_add(json_source, "LastUsed",
						    c_oil->cc.lastused);
				json_object_int_add(json_source, "PacketCount",
						    c_oil->cc.pktcnt);
				json_object_int_add(json_source, "ByteCount",
						    c_oil->cc.bytecnt);
				json_object_int_add(json_source,
						    "WrongInterface",
						    c_oil->cc.wrong_if);
			}
		} else {
			vty_out(vty, "%-6d %-15s  %-15s  %-3s  %-16s  ",
				c_oil->installed, src_str, grp_str,
				isRpt ? "y" : "n", in_ifname);
		}

		for (oif_vif_index = 0; oif_vif_index < MAXVIFS;
		     ++oif_vif_index) {
			struct interface *ifp_out;
			char oif_uptime[10];
			int ttl;

			ttl = c_oil->oil.mfcc_ttls[oif_vif_index];
			if (ttl < 1)
				continue;

			ifp_out = pim_if_find_by_vif_index(pim, oif_vif_index);
			pim_time_uptime(
				oif_uptime, sizeof(oif_uptime),
				now - c_oil->oif_creation[oif_vif_index]);

			if (ifp_out)
				strlcpy(out_ifname, ifp_out->name,
					sizeof(out_ifname));
			else
				strlcpy(out_ifname, "<oif?>",
					sizeof(out_ifname));

			if (uj) {
				json_ifp_out = json_object_new_object();
				json_object_string_add(json_ifp_out, "source",
						       src_str);
				json_object_string_add(json_ifp_out, "group",
						       grp_str);
				json_object_string_add(json_ifp_out,
						       "inboundInterface",
						       in_ifname);
				json_object_string_add(json_ifp_out,
						       "outboundInterface",
						       out_ifname);
				json_object_int_add(json_ifp_out, "installed",
						    c_oil->installed);

				json_object_object_add(json_ifp_in, out_ifname,
						       json_ifp_out);
			} else {
				if (first_oif) {
					first_oif = 0;
					vty_out(vty, "%s(%c%c%c%c%c)",
						out_ifname,
						(c_oil->oif_flags
							 [oif_vif_index] &
						 PIM_OIF_FLAG_PROTO_IGMP)
							? 'I'
							: ' ',
						(c_oil->oif_flags
							 [oif_vif_index] &
						 PIM_OIF_FLAG_PROTO_PIM)
							? 'J'
							: ' ',
						(c_oil->oif_flags
							 [oif_vif_index] &
						 PIM_OIF_FLAG_PROTO_VXLAN)
							? 'V'
							: ' ',
						(c_oil->oif_flags
							 [oif_vif_index] &
						 PIM_OIF_FLAG_PROTO_STAR)
							? '*'
							: ' ',
						(c_oil->oif_flags
							 [oif_vif_index] &
						 PIM_OIF_FLAG_MUTE)
							? 'M'
							: ' ');
				} else
					vty_out(vty, ", %s(%c%c%c%c%c)",
						out_ifname,
						(c_oil->oif_flags
							 [oif_vif_index] &
						 PIM_OIF_FLAG_PROTO_IGMP)
							? 'I'
							: ' ',
						(c_oil->oif_flags
							 [oif_vif_index] &
						 PIM_OIF_FLAG_PROTO_PIM)
							? 'J'
							: ' ',
						(c_oil->oif_flags
							 [oif_vif_index] &
						 PIM_OIF_FLAG_PROTO_VXLAN)
							? 'V'
							: ' ',
						(c_oil->oif_flags
							 [oif_vif_index] &
						 PIM_OIF_FLAG_PROTO_STAR)
							? '*'
							: ' ',
						(c_oil->oif_flags
							 [oif_vif_index] &
						 PIM_OIF_FLAG_MUTE)
							? 'M'
							: ' ');
			}
		}

		if (!uj)
			vty_out(vty, "\n");
	}


	if (uj)
		vty_json(vty, json);
	else
		vty_out(vty, "\n");
}

/* pim statistics - just adding only bsm related now.
 * We can continue to add all pim related stats here.
 */
void pim_show_statistics(struct pim_instance *pim, struct vty *vty,
			 const char *ifname, bool uj)
{
	json_object *json = NULL;
	struct interface *ifp;

	if (uj) {
		json = json_object_new_object();
		json_object_int_add(json, "bsmRx", pim->bsm_rcvd);
		json_object_int_add(json, "bsmTx", pim->bsm_sent);
		json_object_int_add(json, "bsmDropped", pim->bsm_dropped);
	} else {
		vty_out(vty, "BSM Statistics :\n");
		vty_out(vty, "----------------\n");
		vty_out(vty, "Number of Received BSMs : %" PRIu64 "\n",
			pim->bsm_rcvd);
		vty_out(vty, "Number of Forwared BSMs : %" PRIu64 "\n",
			pim->bsm_sent);
		vty_out(vty, "Number of Dropped BSMs  : %" PRIu64 "\n",
			pim->bsm_dropped);
	}

	vty_out(vty, "\n");

	/* scan interfaces */
	FOR_ALL_INTERFACES (pim->vrf, ifp) {
		struct pim_interface *pim_ifp = ifp->info;

		if (ifname && strcmp(ifname, ifp->name))
			continue;

		if (!pim_ifp)
			continue;

		if (!uj) {
			vty_out(vty, "Interface : %s\n", ifp->name);
			vty_out(vty, "-------------------\n");
			vty_out(vty,
				"Number of BSMs dropped due to config miss : %u\n",
				pim_ifp->pim_ifstat_bsm_cfg_miss);
			vty_out(vty, "Number of unicast BSMs dropped : %u\n",
				pim_ifp->pim_ifstat_ucast_bsm_cfg_miss);
			vty_out(vty,
				"Number of BSMs dropped due to invalid scope zone : %u\n",
				pim_ifp->pim_ifstat_bsm_invalid_sz);
		} else {

			json_object *json_row = NULL;

			json_row = json_object_new_object();

			json_object_string_add(json_row, "If Name", ifp->name);
			json_object_int_add(json_row, "bsmDroppedConfig",
					    pim_ifp->pim_ifstat_bsm_cfg_miss);
			json_object_int_add(
				json_row, "bsmDroppedUnicast",
				pim_ifp->pim_ifstat_ucast_bsm_cfg_miss);
			json_object_int_add(json_row,
					    "bsmDroppedInvalidScopeZone",
					    pim_ifp->pim_ifstat_bsm_invalid_sz);
			json_object_object_add(json, ifp->name, json_row);
		}
		vty_out(vty, "\n");
	}

	if (uj)
		vty_json(vty, json);
}

void pim_show_upstream(struct pim_instance *pim, struct vty *vty,
		       pim_sgaddr *sg, bool uj)
{
	struct pim_upstream *up;
	time_t now;
	json_object *json = NULL;
	json_object *json_group = NULL;
	json_object *json_row = NULL;

	now = pim_time_monotonic_sec();

	if (uj)
		json = json_object_new_object();
	else
		vty_out(vty,
			"Iif             Source          Group           State       Uptime   JoinTimer RSTimer   KATimer   RefCnt\n");

	frr_each (rb_pim_upstream, &pim->upstream_head, up) {
		char src_str[INET_ADDRSTRLEN];
		char grp_str[INET_ADDRSTRLEN];
		char uptime[10];
		char join_timer[10];
		char rs_timer[10];
		char ka_timer[10];
		char msdp_reg_timer[10];
		char state_str[PIM_REG_STATE_STR_LEN];

		if (sg->grp.s_addr != INADDR_ANY &&
		    sg->grp.s_addr != up->sg.grp.s_addr)
			continue;
		if (sg->src.s_addr != INADDR_ANY &&
		    sg->src.s_addr != up->sg.src.s_addr)
			continue;

		pim_inet4_dump("<src?>", up->sg.src, src_str, sizeof(src_str));
		pim_inet4_dump("<grp?>", up->sg.grp, grp_str, sizeof(grp_str));
		pim_time_uptime(uptime, sizeof(uptime),
				now - up->state_transition);
		pim_time_timer_to_hhmmss(join_timer, sizeof(join_timer),
					 up->t_join_timer);

		/*
		 * If the upstream is not dummy and it has a J/P timer for the
		 * neighbor display that
		 */
		if (!up->t_join_timer && up->rpf.source_nexthop.interface) {
			struct pim_neighbor *nbr;

			nbr = pim_neighbor_find(
				up->rpf.source_nexthop.interface,
				up->rpf.rpf_addr.u.prefix4);
			if (nbr)
				pim_time_timer_to_hhmmss(join_timer,
							 sizeof(join_timer),
							 nbr->jp_timer);
		}

		pim_time_timer_to_hhmmss(rs_timer, sizeof(rs_timer),
					 up->t_rs_timer);
		pim_time_timer_to_hhmmss(ka_timer, sizeof(ka_timer),
					 up->t_ka_timer);
		pim_time_timer_to_hhmmss(msdp_reg_timer, sizeof(msdp_reg_timer),
					 up->t_msdp_reg_timer);

		pim_upstream_state2brief_str(up->join_state, state_str,
					     sizeof(state_str));
		if (up->reg_state != PIM_REG_NOINFO) {
			char tmp_str[PIM_REG_STATE_STR_LEN];
			char tmp[sizeof(state_str) + 1];

			snprintf(tmp, sizeof(tmp), ",%s",
				 pim_reg_state2brief_str(up->reg_state, tmp_str,
							 sizeof(tmp_str)));
			strlcat(state_str, tmp, sizeof(state_str));
		}

		if (uj) {
			json_object_object_get_ex(json, grp_str, &json_group);

			if (!json_group) {
				json_group = json_object_new_object();
				json_object_object_add(json, grp_str,
						       json_group);
			}

			json_row = json_object_new_object();
			json_object_pim_upstream_add(json_row, up);
			json_object_string_add(
				json_row, "inboundInterface",
				up->rpf.source_nexthop.interface
				? up->rpf.source_nexthop.interface->name
				: "Unknown");

			/*
			 * The RPF address we use is slightly different
			 * based upon what we are looking up.
			 * If we have a S, list that unless
			 * we are the FHR, else we just put
			 * the RP as the rpfAddress
			 */
			if (up->flags & PIM_UPSTREAM_FLAG_MASK_FHR ||
			    up->sg.src.s_addr == INADDR_ANY) {
				char rpf[PREFIX_STRLEN];
				struct pim_rpf *rpg;

				rpg = RP(pim, up->sg.grp);
				pim_inet4_dump("<rpf?>",
					       rpg->rpf_addr.u.prefix4, rpf,
					       sizeof(rpf));
				json_object_string_add(json_row, "rpfAddress",
						       rpf);
			} else {
				json_object_string_add(json_row, "rpfAddress",
						       src_str);
			}

			json_object_string_add(json_row, "source", src_str);
			json_object_string_add(json_row, "group", grp_str);
			json_object_string_add(json_row, "state", state_str);
			json_object_string_add(
				json_row, "joinState",
				pim_upstream_state2str(up->join_state));
			json_object_string_add(
				json_row, "regState",
				pim_reg_state2str(up->reg_state, state_str,
						  sizeof(state_str)));
			json_object_string_add(json_row, "upTime", uptime);
			json_object_string_add(json_row, "joinTimer",
					       join_timer);
			json_object_string_add(json_row, "resetTimer",
					       rs_timer);
			json_object_string_add(json_row, "keepaliveTimer",
					       ka_timer);
			json_object_string_add(json_row, "msdpRegTimer",
					       msdp_reg_timer);
			json_object_int_add(json_row, "refCount",
					    up->ref_count);
			json_object_int_add(json_row, "sptBit", up->sptbit);
			json_object_object_add(json_group, src_str, json_row);
		} else {
			vty_out(vty,
				"%-16s%-15s %-15s %-11s %-8s %-9s %-9s %-9s %6d\n",
				up->rpf.source_nexthop.interface
				? up->rpf.source_nexthop.interface->name
				: "Unknown",
				src_str, grp_str, state_str, uptime, join_timer,
				rs_timer, ka_timer, up->ref_count);
		}
	}

	if (uj)
		vty_json(vty, json);
}

static void pim_show_join_desired_helper(struct pim_instance *pim,
					 struct vty *vty,
					 struct pim_upstream *up,
					 json_object *json, bool uj)
{
	json_object *json_group = NULL;
	char src_str[INET_ADDRSTRLEN];
	char grp_str[INET_ADDRSTRLEN];
	json_object *json_row = NULL;

	pim_inet4_dump("<src?>", up->sg.src, src_str, sizeof(src_str));
	pim_inet4_dump("<grp?>", up->sg.grp, grp_str, sizeof(grp_str));

	if (uj) {
		json_object_object_get_ex(json, grp_str, &json_group);

		if (!json_group) {
			json_group = json_object_new_object();
			json_object_object_add(json, grp_str, json_group);
		}

		json_row = json_object_new_object();
		json_object_pim_upstream_add(json_row, up);
		json_object_string_add(json_row, "source", src_str);
		json_object_string_add(json_row, "group", grp_str);

		if (pim_upstream_evaluate_join_desired(pim, up))
			json_object_boolean_true_add(json_row,
						     "evaluateJoinDesired");

		json_object_object_add(json_group, src_str, json_row);

	} else {
		vty_out(vty, "%-15s %-15s %-6s\n", src_str, grp_str,
			pim_upstream_evaluate_join_desired(pim, up) ? "yes"
								    : "no");
	}
}

void pim_show_join_desired(struct pim_instance *pim, struct vty *vty, bool uj)
{
	struct pim_upstream *up;

	json_object *json = NULL;

	if (uj)
		json = json_object_new_object();
	else
		vty_out(vty, "Source          Group           EvalJD\n");

	frr_each (rb_pim_upstream, &pim->upstream_head, up) {
		/* scan all interfaces */
		pim_show_join_desired_helper(pim, vty, up, json, uj);
	}

	if (uj)
		vty_json(vty, json);
}

void pim_show_upstream_rpf(struct pim_instance *pim, struct vty *vty, bool uj)
{
	struct pim_upstream *up;
	json_object *json = NULL;
	json_object *json_group = NULL;
	json_object *json_row = NULL;

	if (uj)
		json = json_object_new_object();
	else
		vty_out(vty,
			"Source          Group           RpfIface         RibNextHop      RpfAddress     \n");

	frr_each (rb_pim_upstream, &pim->upstream_head, up) {
		char src_str[INET_ADDRSTRLEN];
		char grp_str[INET_ADDRSTRLEN];
		char rpf_nexthop_str[PREFIX_STRLEN];
		char rpf_addr_str[PREFIX_STRLEN];
		struct pim_rpf *rpf;
		const char *rpf_ifname;

		rpf = &up->rpf;

		pim_inet4_dump("<src?>", up->sg.src, src_str, sizeof(src_str));
		pim_inet4_dump("<grp?>", up->sg.grp, grp_str, sizeof(grp_str));
		pim_addr_dump("<nexthop?>",
			      &rpf->source_nexthop.mrib_nexthop_addr,
			      rpf_nexthop_str, sizeof(rpf_nexthop_str));
		pim_addr_dump("<rpf?>", &rpf->rpf_addr, rpf_addr_str,
			      sizeof(rpf_addr_str));

		rpf_ifname =
			rpf->source_nexthop.interface ? rpf->source_nexthop
								.interface->name
						      : "<ifname?>";

		if (uj) {
			json_object_object_get_ex(json, grp_str, &json_group);

			if (!json_group) {
				json_group = json_object_new_object();
				json_object_object_add(json, grp_str,
						       json_group);
			}

			json_row = json_object_new_object();
			json_object_pim_upstream_add(json_row, up);
			json_object_string_add(json_row, "source", src_str);
			json_object_string_add(json_row, "group", grp_str);
			json_object_string_add(json_row, "rpfInterface",
					       rpf_ifname);
			json_object_string_add(json_row, "ribNexthop",
					       rpf_nexthop_str);
			json_object_string_add(json_row, "rpfAddress",
					       rpf_addr_str);
			json_object_object_add(json_group, src_str, json_row);
		} else {
			vty_out(vty, "%-15s %-15s %-16s %-15s %-15s\n", src_str,
				grp_str, rpf_ifname, rpf_nexthop_str,
				rpf_addr_str);
		}
	}

	if (uj)
		vty_json(vty, json);
}
