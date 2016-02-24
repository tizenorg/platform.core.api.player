/*
 * libmm-player
 *
 * Copyright (c) 2000 - 2011 Samsung Electronics Co., Ltd. All rights reserved.
 *
 * Contact: JongHyuk Choi <jhchoi.choi@samsung.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#ifndef __TIZEN_MEDIA_PLAYER_EVASCLIENT_H__
#define __TIZEN_MEDIA_PLAYER_EVASCLIENT_H__

/*===========================================================================================
|																							|
|  INCLUDE FILES																			|
|																							|
========================================================================================== */
#include <stdio.h>
#include <gst/gst.h>
#include <gst/video/gstvideosink.h>

#include <Evas.h>
#include <Ecore.h>

#include <tbm_bufmgr.h>
#include <tbm_surface.h>
#include <tbm_surface_internal.h>

#include "mm_types.h"
#include "mm_debug.h"

#include <media/media_packet.h>
#include <media/media_format.h>

#define MAX_PACKET_NUM 20

/*===========================================================================================
|																							|
|  GLOBAL DEFINITIONS AND DECLARATIONS FOR MODULE											|
|																							|
========================================================================================== */

/*---------------------------------------------------------------------------
|    GLOBAL #defines:														|
---------------------------------------------------------------------------*/

#ifdef __cplusplus
	extern "C" {
#endif
	typedef enum
	{
		UPDATE_VISIBILITY,
		UPDATE_TBM_SURF
	} update_info;

	typedef enum
	{
		VISIBLE_NONE, /* if user dont set visibility, it will be changed to true */
		VISIBLE_TRUE,
		VISIBLE_FALSE
	} visible_info;

	/* evas info for evas surface type */
	typedef struct
	{
		Evas_Object *eo;
		Ecore_Pipe *epipe;

		/* video width & height */
		guint w;
		guint h;

		visible_info visible;

		/* geometry information */
		GstVideoRectangle eo_size;
		gboolean use_ratio;
		guint rotate_angle;
		guint display_geometry_method;
		guint flip;

		tbm_surface_h tbm_surf;

		/* media packet */
		media_packet_h prev_mp;
		media_packet_h cur_mp;
		media_packet_h pkt[MAX_PACKET_NUM];
		gint prev_idx;
		gint cur_idx;

		/* count undestroyed media packet */
		guint sent_buffer_cnt;

		/* initialize values */
//		mm_player_ini_t ini;

		/* lock */
		GMutex free_lock;
	} mm_evas_info;

int player_set_evas_info(MMHandleType *player, Evas_Object *eo);
int player_unset_evas_info(MMHandleType player);
int player_set_visible_evas_info (MMHandleType player, bool visible);
int player_get_visible_evas_info (MMHandleType player, bool *visible);
int player_set_rotation_evas_info (MMHandleType player, int rotate);
int player_get_rotation_evas_info (MMHandleType player, int *rotate);
int player_set_geometry_evas_info (MMHandleType player, int mode);
int player_get_geometry_evas_info (MMHandleType player, int *mode);
int player_update_video_param(MMHandleType player);
void decoded_callback_for_evas (media_packet_h packet, void *data);
#if 0
int _player_create_evas_info (mm_evas_info ** evas_info);
int _player_destroy_evas_info (mm_evas_info * evas_info);
int _player_set_evas_info (mm_evas_info * evas_info, Evas_Object *eo);
int _player_reset_evas_info (mm_evas_info * evas_info);
void _player_update_geometry_evas_info (mm_evas_info *evas_info, GstVideoRectangle *result);
int _player_apply_geometry_evas_info (mm_evas_info *evas_info);
#endif
#ifdef __cplusplus
	}
#endif

#endif /* __TIZEN_MEDIA_PLAYER_EVASCLIENT_H__ */

