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

#ifndef PIM_CMD_COMMON_H
#define PIM_CMD_COMMON_H

void json_object_pim_upstream_add(json_object *json, struct pim_upstream *up);
void pim_show_rpf(struct pim_instance *pim, struct vty *vty, json_object *json);
void pim_cmd_show_ip_multicast_helper(struct pim_instance *pim,
				      struct vty *vty);
void show_multicast_interfaces(struct pim_instance *pim, struct vty *vty,
			       bool uj);
void pim_show_neighbors_secondary(struct pim_instance *pim, struct vty *vty);
void pim_show_state(struct pim_instance *pim, struct vty *vty,
		    const char *src_or_group, const char *group, bool uj);
void pim_show_statistics(struct pim_instance *pim, struct vty *vty,
			 const char *ifname, bool uj);
void pim_show_upstream(struct pim_instance *pim, struct vty *vty,
		       pim_sgaddr *sg, json_object *json);
void pim_show_join_desired(struct pim_instance *pim, struct vty *vty, bool uj);
void pim_show_upstream_rpf(struct pim_instance *pim, struct vty *vty, bool uj);
bool pim_sgaddr_match(pim_sgaddr item, pim_sgaddr match);

/*
 * Special Macro to allow us to get the correct pim_instance;
 */
#define PIM_DECLVAR_CONTEXT(A, B)                                              \
	struct vrf *A = VTY_GET_CONTEXT(vrf);                                  \
	struct pim_instance *B =                                               \
		(vrf) ? vrf->info : pim_get_pim_instance(VRF_DEFAULT);         \
	vrf = (vrf) ? vrf : pim->vrf;

#endif /* PIM_CMD_COMMON_H */
