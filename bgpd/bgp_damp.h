/* BGP flap dampening
 * Copyright (C) 2001 IP Infusion Inc.
 *
 * This file is part of GNU Zebra.
 *
 * GNU Zebra is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any
 * later version.
 *
 * GNU Zebra is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; see the file COPYING; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifndef _QUAGGA_BGP_DAMP_H
#define _QUAGGA_BGP_DAMP_H

#include "bgpd/bgp_table.h"

/* Structure maintained on a per-route basis. */
struct bgp_damp_info {
	/* Figure-of-merit.  */
	unsigned int penalty;

	/* Number of flapping.  */
	unsigned int flap;

	/* First flap time  */
	time_t start_time;

	/* Last time penalty was updated.  */
	time_t t_updated;

	/* Time of route start to be suppressed.  */
	time_t suppress_time;

	/* Back reference to associated dampening configuration. */
	struct bgp_damp_config *config;

	/* Back reference to bgp_path_info. */
	struct bgp_path_info *path;

	/* Back reference to bgp_node. */
	struct bgp_dest *dest;

	/* Current index in the reuse_list. */
	int index;
#define BGP_DAMP_NO_REUSE_LIST_INDEX                                           \
	(-1) /* index for elements on no_reuse_list */

	/* Last time message type. */
	uint8_t lastrecord;
#define BGP_RECORD_UPDATE	1U
#define BGP_RECORD_WITHDRAW	2U

	afi_t afi;
	safi_t safi;

	SLIST_ENTRY(bgp_damp_info) entry;
};

SLIST_HEAD(reuselist, bgp_damp_info);

/* Specified parameter set configuration. */
struct bgp_damp_config {
	/* Value over which routes suppressed.  */
	unsigned int suppress_value;

	/* Value below which suppressed routes reused.  */
	unsigned int reuse_limit;

	/* Max time a route can be suppressed.  */
	time_t max_suppress_time;

	/* Time during which accumulated penalty reduces by half.  */
	time_t half_life;

	/* Non-configurable parameters but fixed at implementation time.
	 * To change this values, init_bgp_damp() should be modified.
	 */
	time_t tmax; /* Max time previous instability retained */
	unsigned int reuse_list_size;  /* Number of reuse lists */
	unsigned int reuse_index_size; /* Size of reuse index array */

	/* Non-configurable parameters.  Most of these are calculated from
	 * the configurable parameters above.
	 */
	unsigned int ceiling;		  /* Max value a penalty can attain */
	unsigned int decay_rate_per_tick; /* Calculated from half-life */
	unsigned int decay_array_size; /* Calculated using config parameters */
	double scale_factor;
	unsigned int reuse_scale_factor;

	/* Decay array per-set based. */
	double *decay_array;

	/* Reuse index array per-set based. */
	int *reuse_index;

	/* Reuse list array per-set based. */
	struct reuselist *reuse_list;
	unsigned int reuse_offset;

	/* All dampening information which is not on reuse list.  */
	struct reuselist no_reuse_list;

	/* Reuse timer thread per-set base. */
	struct thread *t_reuse;

	afi_t afi;
	safi_t safi;
};

#define BGP_DAMP_NONE           0
#define BGP_DAMP_USED		1
#define BGP_DAMP_SUPPRESSED	2

/* Time granularity for reuse lists */
#define DELTA_REUSE	          10

/* Time granularity for decay arrays */
#define DELTA_T 	           5

#define DEFAULT_PENALTY         1000

#define DEFAULT_HALF_LIFE         15
#define DEFAULT_REUSE 	       	 750
#define DEFAULT_SUPPRESS 	2000

#define REUSE_LIST_SIZE          256
#define REUSE_ARRAY_SIZE        1024

extern struct bgp_damp_config *get_active_bdc_from_pi(struct bgp_path_info *pi,
						      afi_t afi, safi_t safi);
extern int bgp_damp_enable(struct bgp *, afi_t, safi_t, time_t, unsigned int,
			   unsigned int, time_t);
extern int bgp_damp_disable(struct bgp *, afi_t, safi_t);
extern int bgp_damp_withdraw(struct bgp_path_info *path, struct bgp_dest *dest,
			     afi_t afi, safi_t safi, int attr_change);
extern int bgp_damp_update(struct bgp_path_info *path, struct bgp_dest *dest,
			   afi_t afi, safi_t saff);
extern void bgp_damp_info_free(struct bgp_damp_info *bdi, int withdraw);
extern void bgp_damp_info_clean(struct bgp *bgp, struct bgp_damp_config *bdc,
				afi_t afi, safi_t safi);
extern void bgp_damp_config_clean(struct bgp_damp_config *bdc);
extern int bgp_damp_decay(time_t, int, struct bgp_damp_config *damp);
extern void bgp_config_write_damp(struct vty *vty, struct bgp *bgp, afi_t afi,
				  safi_t safi);
extern void bgp_damp_info_vty(struct vty *vty, struct bgp *bgp,
			      struct bgp_path_info *path, afi_t afi,
			      safi_t safi, json_object *json_path);
extern const char *bgp_damp_reuse_time_vty(struct vty *vty,
					   struct bgp_path_info *path,
					   char *timebuf, size_t len, afi_t afi,
					   safi_t safi, bool use_json,
					   json_object *json);
extern int bgp_show_dampening_parameters(struct vty *vty, afi_t, safi_t,
					 uint16_t);
extern void bgp_peer_damp_enable(struct peer *peer, afi_t afi, safi_t safi,
				 time_t half, unsigned int reuse,
				 unsigned int suppress, time_t max);
extern void bgp_peer_damp_disable(struct peer *peer, afi_t afi, safi_t safi);
extern void bgp_config_write_peer_damp(struct vty *vty, struct peer *peer,
				       afi_t afi, safi_t safi);
extern void bgp_show_peer_dampening_parameters(struct vty *vty,
					       struct peer *peer, afi_t afi,
					       safi_t safi, bool use_json);

#endif /* _QUAGGA_BGP_DAMP_H */
