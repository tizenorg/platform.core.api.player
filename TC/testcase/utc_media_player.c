/*
* Copyright (c) 2011 Samsung Electronics Co., Ltd All Rights Reserved
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
*/

#include <tet_api.h>
#include <media/player.h>
#include <glib.h>

#define MEDIA_PATH "/mnt/nfs/workspace/capi/media/player/TC/media_samsung.mp4"

enum {
	POSITIVE_TC_IDX = 0x01,
	NEGATIVE_TC_IDX,
};

//keeping ret and player global, since using startup function for all
player_h player;
int sRet;

static void startup(void);
static void cleanup(void);

void (*tet_startup)(void) = startup;
void (*tet_cleanup)(void) = cleanup;

static void utc_media_player_get_duration_p(void);
static void utc_media_player_get_duration_n(void);
static void utc_media_player_set_position_p(void);
static void utc_media_player_set_position_n(void);
static void utc_media_player_get_position_p(void);
static void utc_media_player_get_position_n(void);
static void utc_media_player_set_position_ratio_p(void);
static void utc_media_player_set_position_ratio_n(void);
static void utc_media_player_get_position_ratio_p(void);
static void utc_media_player_get_position_ratio_n(void);
static void utc_media_player_get_state_p(void);
static void utc_media_player_get_state_n(void);
static void utc_media_player_get_video_size_p(void);
static void utc_media_player_get_video_size_n(void);
static void utc_media_player_set_looping_p(void);
static void utc_media_player_set_looping_n(void);
static void utc_media_player_is_looping_p(void);
static void utc_media_player_is_looping_n(void);
static void utc_media_player_set_mute_p(void);
static void utc_media_player_set_mute_n(void);
static void utc_media_player_is_muted_p(void);
static void utc_media_player_is_muted_n(void);
static void utc_media_player_set_display_p(void);
static void utc_media_player_set_display_n(void);
static void utc_media_player_set_memory_buffer_p(void);
static void utc_media_player_set_memory_buffer_n(void);
static void utc_media_player_set_volume_p(void);
static void utc_media_player_set_volume_n(void);
static void utc_media_player_get_volume_p(void);
static void utc_media_player_get_volume_n(void);
static void utc_media_player_set_sound_type_p(void);
static void utc_media_player_set_sound_type_n(void);
static void utc_media_player_set_subtitle_path_p(void);
static void utc_media_player_set_subtitle_path_n(void);
static void utc_media_player_set_x11_display_visible_p(void);
static void utc_media_player_set_x11_display_visible_n(void);
static void utc_media_player_is_x11_display_visible_p(void);
static void utc_media_player_is_x11_display_visible_n(void);
static void utc_media_player_set_x11_display_rotation_p(void);
static void utc_media_player_set_x11_display_rotation_n(void);
static void utc_media_player_get_x11_display_rotation_p(void);
static void utc_media_player_get_x11_display_rotation_n(void);
static void utc_media_player_set_display_mode_p(void);
static void utc_media_player_set_display_mode_n(void);
static void utc_media_player_get_display_mode_p(void);
static void utc_media_player_get_display_mode_n(void);
static void utc_media_player_capture_video_p(void);
static void utc_media_player_capture_video_n(void);

struct tet_testlist tet_testlist[] = {
	{ utc_media_player_get_duration_p , POSITIVE_TC_IDX },
	{ utc_media_player_get_duration_n , NEGATIVE_TC_IDX },
	{ utc_media_player_set_position_p , POSITIVE_TC_IDX },
	{ utc_media_player_set_position_n , NEGATIVE_TC_IDX },
	{ utc_media_player_get_position_p , POSITIVE_TC_IDX },
	{ utc_media_player_get_position_n , NEGATIVE_TC_IDX },
	{ utc_media_player_set_position_ratio_p , POSITIVE_TC_IDX },
	{ utc_media_player_set_position_ratio_n  , NEGATIVE_TC_IDX },
	{ utc_media_player_get_position_ratio_p , POSITIVE_TC_IDX },
	{ utc_media_player_get_position_ratio_n , NEGATIVE_TC_IDX },
	{ utc_media_player_get_state_p , POSITIVE_TC_IDX },
	{ utc_media_player_get_state_n  , NEGATIVE_TC_IDX },
	{ utc_media_player_get_video_size_p , POSITIVE_TC_IDX },
	{ utc_media_player_get_video_size_n  , NEGATIVE_TC_IDX },
	{ utc_media_player_set_looping_p  , POSITIVE_TC_IDX },
	{ utc_media_player_set_looping_n , NEGATIVE_TC_IDX },
	{ utc_media_player_is_looping_p , POSITIVE_TC_IDX },
	{ utc_media_player_is_looping_n  , NEGATIVE_TC_IDX },
	{ utc_media_player_set_mute_p , POSITIVE_TC_IDX },
	{ utc_media_player_set_mute_n , NEGATIVE_TC_IDX },
	{ utc_media_player_is_muted_p  , POSITIVE_TC_IDX },
	{ utc_media_player_is_muted_n , NEGATIVE_TC_IDX },
	{ utc_media_player_set_display_p , POSITIVE_TC_IDX },
	{ utc_media_player_set_display_n , NEGATIVE_TC_IDX },
	{ utc_media_player_set_memory_buffer_p  , POSITIVE_TC_IDX },
	{ utc_media_player_set_memory_buffer_n, NEGATIVE_TC_IDX },
	{ utc_media_player_set_volume_p , POSITIVE_TC_IDX },
	{ utc_media_player_set_volume_n , NEGATIVE_TC_IDX },
	{ utc_media_player_get_volume_p , POSITIVE_TC_IDX },
	{ utc_media_player_get_volume_n , NEGATIVE_TC_IDX },
	{ utc_media_player_set_sound_type_p , POSITIVE_TC_IDX },
	{ utc_media_player_set_sound_type_n , NEGATIVE_TC_IDX },
	{ utc_media_player_set_subtitle_path_p , POSITIVE_TC_IDX },
	{ utc_media_player_set_subtitle_path_n , NEGATIVE_TC_IDX },
	{ utc_media_player_set_x11_display_visible_p , POSITIVE_TC_IDX },
	{ utc_media_player_set_x11_display_visible_n , NEGATIVE_TC_IDX },
	{ utc_media_player_is_x11_display_visible_p , POSITIVE_TC_IDX },
	{ utc_media_player_is_x11_display_visible_n , NEGATIVE_TC_IDX },
	{ utc_media_player_set_x11_display_rotation_p , POSITIVE_TC_IDX },
	{ utc_media_player_set_x11_display_rotation_n , NEGATIVE_TC_IDX },
	{ utc_media_player_get_x11_display_rotation_p , POSITIVE_TC_IDX },
	{ utc_media_player_get_x11_display_rotation_n , NEGATIVE_TC_IDX },
	{ utc_media_player_set_display_mode_p , POSITIVE_TC_IDX },
	{ utc_media_player_set_display_mode_n , NEGATIVE_TC_IDX },
	{ utc_media_player_get_display_mode_p , POSITIVE_TC_IDX },
	{ utc_media_player_get_display_mode_n , NEGATIVE_TC_IDX },
	{ utc_media_player_capture_video_p , POSITIVE_TC_IDX },
	{ utc_media_player_capture_video_n , NEGATIVE_TC_IDX },
	{ NULL, 0 },
};

static GMainLoop *g_mainloop = NULL;
static GThread *event_thread;

gpointer GmainThread(gpointer data){
	g_mainloop = g_main_loop_new (NULL, 0);
	g_main_loop_run (g_mainloop);
	
	return NULL;
}

static void startup(void)
{
	if( !g_thread_supported() )
	{
		g_thread_init(NULL);
	}
	
	GError *gerr = NULL;
	event_thread = g_thread_create(GmainThread, NULL, 1, &gerr);

	if ((sRet = player_create(&player)) == PLAYER_ERROR_NONE)
	{
		sRet = player_set_uri(player, MEDIA_PATH);
	}
}

static void cleanup(void)
{
	player_destroy(player);
	g_main_loop_quit (g_mainloop);
	g_thread_join(event_thread);
}

static void player_video_capture_cb(unsigned char *data, int width, int height, unsigned int size, void *user_data)
{
	dts_message("Player", "player_video_capture_cb!!!");
}

static void reset(void)
{
	if ((sRet = player_unprepare(player)) == PLAYER_ERROR_NONE)
	{
		sRet = player_set_uri(player, MEDIA_PATH);
	}
}

static void utc_media_player_get_duration_p(void)
{
	char* api_name = "player_get_duration";
	int ret;
	int duration;
	if (sRet == PLAYER_ERROR_NONE)
	{
		player_prepare(player);
		{
			if ((ret = player_start(player)) == PLAYER_ERROR_NONE)
			{			
				sleep(3);
				if ((ret = player_get_duration(player, &duration)) == PLAYER_ERROR_NONE)
				{
					player_stop(player);
					reset();
					dts_pass(api_name);
				}
				if ((sRet = player_stop(player)) == PLAYER_ERROR_NONE)
				{
					dts_message(api_name, "Call log: %d %d", sRet, ret);
				}
			}
			reset();
		}
	}
	dts_message(api_name, "Call log: %d %d", sRet, ret);
	dts_fail(api_name);
}

static void utc_media_player_get_duration_n(void)
{
	char* api_name = "player_get_duration";
	int ret;
	int duration;
	if (sRet == PLAYER_ERROR_NONE)
	{
		if ((ret = player_get_duration(player, &duration)) != PLAYER_ERROR_NONE)
		{
			dts_pass(api_name);
		}
	}
	dts_message(api_name, "Call log: %d %d", sRet, ret);
	dts_fail(api_name);
}

static void utc_media_player_set_position_p(void)
{
	char* api_name = "utc_media_player_set_position_p";
	int ret;
	int position = 10000;
	if (sRet == PLAYER_ERROR_NONE)
	{
		player_prepare(player);
		{
			if ((ret = player_set_position(player, position,NULL,NULL)) == PLAYER_ERROR_NONE)
			{
				reset();
				dts_pass(api_name);
			}
		}
		reset();
	}
	dts_message(api_name, "Call log: %d %d", sRet, ret);
	dts_fail(api_name);
}

static void utc_media_player_set_position_n(void)
{
	char* api_name = "utc_media_player_set_position_n";
	int ret;
	int position = 10000;
	player_h player1 = NULL;
	if ((ret = player_set_position(player1, position,NULL,NULL)) != PLAYER_ERROR_NONE)
	{
		dts_pass(api_name);
	}
	dts_message(api_name, "Call log: %d %d", sRet, ret);
	dts_fail(api_name);
}

static void utc_media_player_get_position_p(void)
{
	char* api_name = "player_get_position";
	int ret;
	int position;
	if (sRet == PLAYER_ERROR_NONE)
	{
		player_prepare(player);
		{
			if ((ret = player_get_position(player, &position)) == PLAYER_ERROR_NONE)
			{
				reset();
				dts_pass(api_name);
			}
		}
		reset();
	}
	dts_message(api_name, "Call log: %d %d", sRet, ret);
	dts_fail(api_name);
}

static void utc_media_player_get_position_n(void)
{
	char* api_name = "player_get_position";
	int ret;
	int position;
	player_h player1 = NULL;
	if ((ret = player_get_position(player1, &position)) != PLAYER_ERROR_NONE)
	{
		dts_pass(api_name);
	}
	dts_message(api_name, "Call log: %d %d", sRet, ret);
	dts_fail(api_name);
}

static void utc_media_player_set_position_ratio_p(void)
{
	char* api_name = "utc_media_player_set_position_ratio_p";
	int ret;
	int pos_ratio = 10;

	if (sRet == PLAYER_ERROR_NONE)
	{
		player_prepare(player);
		{
			if ((ret = player_set_position_ratio(player, pos_ratio,NULL,NULL)) == PLAYER_ERROR_NONE)
			{
				reset();
				dts_pass(api_name);
			}
		}
		reset();
	}
	dts_message(api_name, "Call log: %d %d", sRet, ret);
	dts_fail(api_name);
}

static void utc_media_player_set_position_ratio_n(void)
{
	char* api_name = "utc_media_player_set_position_ratio_n";
	int ret;
	int pos_ratio = 10;
	player_h player1 = NULL;
	if ((ret = player_get_position_ratio(player1, pos_ratio)) != PLAYER_ERROR_NONE)
	{
		dts_pass(api_name);
	}
	dts_message(api_name, "Call log: %d %d", sRet, ret);
	dts_fail(api_name);
}

static void utc_media_player_get_position_ratio_p(void)
{
	char* api_name = "player_get_position_ratio";
	int ret;
	int pos_ratio;

	if (sRet == PLAYER_ERROR_NONE)
	{
		player_prepare(player);
		{
			if ((ret = player_get_position_ratio(player, &pos_ratio)) == PLAYER_ERROR_NONE)
			{
				reset();
				dts_pass(api_name);
			}
		}
		reset();
	}
	dts_message(api_name, "Call log: %d %d", sRet, ret);
	dts_fail(api_name);
}

static void utc_media_player_get_position_ratio_n(void)
{
	char* api_name = "player_get_position_ratio";
	int ret;
	int pos_ratio;
	player_h player1 = NULL;
	if ((ret = player_get_position_ratio(player1, &pos_ratio)) != PLAYER_ERROR_NONE)
	{
		dts_pass(api_name);
	}
	dts_message(api_name, "Call log: %d %d", sRet, ret);
	dts_fail(api_name);
}

static void utc_media_player_get_state_p(void)
{
	char* api_name = "player_get_state";
	int ret;
	player_state_e cur_state;
	if (sRet == PLAYER_ERROR_NONE)
	{
		player_prepare(player);
		{
			if ((ret = player_get_state(player, &cur_state)) == PLAYER_ERROR_NONE)
			{
				dts_pass(api_name);
			}
		}
	}
	dts_message(api_name, "Call log: %d %d", sRet, ret);
	dts_fail(api_name);
}

static void utc_media_player_get_state_n(void)
{
	char* api_name = "player_get_state";
	int ret;
	if (sRet == PLAYER_ERROR_NONE)
	{
		if ((ret = player_get_state(player, NULL)) != PLAYER_ERROR_NONE)
		{
			dts_pass(api_name);
		}
	}
	dts_message(api_name, "Call log: %d %d", sRet, ret);
	dts_fail(api_name);
}

static void utc_media_player_get_video_size_p(void)
{
	char* api_name = "player_get_video_size";
	int ret;
	int width, height;
	if (sRet == PLAYER_ERROR_NONE)
	{
		player_prepare(player);
		if ((ret = player_get_video_size(player, &width, &height)) == PLAYER_ERROR_NONE)
		{
			reset();
			dts_pass(api_name);
		}
		if ((sRet = player_unprepare(player)) == PLAYER_ERROR_NONE)
		{
			sRet = player_set_uri(player, MEDIA_PATH);
		} 
	}
	dts_message(api_name, "Call log: %d %d", sRet, ret);
	dts_fail(api_name);
}

static void utc_media_player_get_video_size_n(void)
{
	char* api_name = "player_get_video_size";
	int ret;
	int width, height;
	if (sRet == PLAYER_ERROR_NONE)
	{
		if ((ret = player_get_video_size(player, &width, &height)) != PLAYER_ERROR_NONE)
		{
			dts_pass(api_name);
		}
	}
	dts_message(api_name, "Call log: %d %d", sRet, ret);
	dts_fail(api_name);
}

static void utc_media_player_set_looping_p(void)
{
	char* api_name = "player_set_looping";
	int ret;
	bool looping = true;
	if (sRet == PLAYER_ERROR_NONE)
	{
		player_prepare(player);
		{
			if ((ret = player_set_looping(player, looping)) == PLAYER_ERROR_NONE)
			{
				dts_pass(api_name);
			}
		}
	}
	dts_message(api_name, "Call log: %d %d", sRet, ret);
	dts_fail(api_name);
}

static void utc_media_player_set_looping_n(void)
{
	char* api_name = "player_set_looping";
	int ret;
	bool looping = true;
	if (sRet == PLAYER_ERROR_NONE)
	{
		if ((ret = player_set_looping(NULL, looping)) != PLAYER_ERROR_NONE)
		{
			dts_pass(api_name);
		}
	}
	dts_message(api_name, "Call log: %d %d", sRet, ret);
	dts_fail(api_name);
}

static void utc_media_player_is_looping_p(void)
{
	char* api_name = "player_is_looping";
	int ret;
	bool looping;
	if (sRet == PLAYER_ERROR_NONE)
	{
		player_prepare(player);
		{
			if ((ret = player_is_looping(player, &looping)) == PLAYER_ERROR_NONE)
			{
				reset();
				dts_pass(api_name);
			}
			reset();
		}
	}
	dts_message(api_name, "Call log: %d %d", sRet, ret);
	dts_fail(api_name);
}

static void utc_media_player_is_looping_n(void)
{
	char* api_name = "player_is_looping";
	int ret;
	bool looping;
	if (sRet == PLAYER_ERROR_NONE)
	{
		if ((ret = player_is_looping(NULL, &looping)) != PLAYER_ERROR_NONE)
		{
			dts_pass(api_name);
		}
	}
	dts_message(api_name, "Call log: %d %d", sRet, ret);
	dts_fail(api_name);
}

static void utc_media_player_set_mute_p(void)
{
	char* api_name = "utc_media_player_set_mute_p";
	int ret;
	bool muted=false;
	if (sRet == PLAYER_ERROR_NONE)
	{
		player_prepare(player);
		{
			if ((ret = player_set_mute(player, muted)) == PLAYER_ERROR_NONE)
			{
				dts_pass(api_name);
			}
		}
	}
	dts_message(api_name, "Call log: %d %d", sRet, ret);
	dts_fail(api_name);
}

static void utc_media_player_set_mute_n(void)
{
	char* api_name = "utc_media_player_set_mute_n";
	int ret;
	player_h player1 = NULL;
	bool muted = false;
	if ((ret = player_set_mute(player1, muted)) != PLAYER_ERROR_NONE)
	{
		dts_pass(api_name);
	}
	dts_message(api_name, "Call log: %d %d", sRet, ret);
	dts_fail(api_name);
}



static void utc_media_player_is_muted_p(void)
{
	char* api_name = "player_is_muted";
	int ret;
	bool muted;
	if (sRet == PLAYER_ERROR_NONE)
	{
		player_prepare(player);
		if ((ret = player_is_muted(player, &muted)) == PLAYER_ERROR_NONE)
		{
			dts_pass(api_name);
		}
	}
	dts_message(api_name, "Call log: %d %d", sRet, ret);
	dts_fail(api_name);
}

static void utc_media_player_is_muted_n(void)
{
	char* api_name = "player_is_muted";
	int ret;
	player_h player1 = NULL;
	bool muted;
	if ((ret = player_is_muted(player1, &muted)) != PLAYER_ERROR_NONE)
	{
		dts_pass(api_name);
	}
	dts_message(api_name, "Call log: %d %d", sRet, ret);
	dts_fail(api_name);
}

static void utc_media_player_set_display_p(void)
{
	char* api_name = "player_set_display";
	int ret;
	reset();
	if (sRet == PLAYER_ERROR_NONE)
	{
		if ((ret = player_set_display(player, PLAYER_DISPLAY_TYPE_X11,0)) == PLAYER_ERROR_NONE)
		{
			dts_pass(api_name);
		}
	}
	dts_message(api_name, "Call log: %d %d", sRet, ret);
	dts_fail(api_name);
}

static void utc_media_player_set_display_n(void)
{
	char* api_name = "player_set_display";
	int ret;
	if (sRet == PLAYER_ERROR_NONE)
	{
		player_prepare(player);
		{
			if ((ret = player_set_display(player,PLAYER_DISPLAY_TYPE_X11, 0)) != PLAYER_ERROR_NONE)
			{
				reset();
				dts_pass(api_name);
			}
			reset();
		}
	}
	dts_message(api_name, "Call log: %d %d", sRet, ret);
	dts_fail(api_name);
}

static void utc_media_player_set_memory_buffer_p(void)
{
	char* api_name = "utc_media_player_set_memory_buffer_p";
	int ret;
	if (sRet == PLAYER_ERROR_NONE)
	{
		reset();
		GMappedFile *file;
		gsize file_size;
		guint8* g_media_mem = NULL;
		
		file = g_mapped_file_new (MEDIA_PATH, FALSE,NULL);
		file_size = g_mapped_file_get_length (file);
		g_media_mem = (guint8 *) g_mapped_file_get_contents (file);
		
		if ((ret = player_set_memory_buffer(player, (void*)g_media_mem ,file_size)) == PLAYER_ERROR_NONE)
		{
			player_prepare(player);
			reset();
			dts_pass(api_name);
		}
	} 
	dts_message(api_name, "Call log: %d %d", sRet, ret);
	dts_fail(api_name);
}

static void utc_media_player_set_memory_buffer_n(void)
{
	char* api_name = "utc_media_player_set_memory_buffer_n";
	int ret;
	if (sRet == PLAYER_ERROR_NONE)
	{
		if ((ret = player_set_memory_buffer(player, NULL, 0)) != PLAYER_ERROR_NONE)
		{
			dts_pass(api_name);
		}
	}
	dts_message(api_name, "Call log: %d %d", sRet, ret);
	dts_fail(api_name);
}

static void utc_media_player_set_volume_p(void)
{
	char* api_name = "utc_media_player_set_volume_p";
	int ret;
	float left, right  = 1.0f; 
	{
		if (sRet == PLAYER_ERROR_NONE)
		{
			if ((ret = player_set_volume(player, left, right)) == PLAYER_ERROR_NONE)
			{
				dts_pass(api_name);
			}
		}
	}
	dts_message(api_name, "Call log: %d %d", sRet, ret);
	dts_fail(api_name);
}

static void utc_media_player_set_volume_n(void)
{
	char* api_name = "utc_media_player_set_volume_n";
	int ret;
	float left, right  = 1.0f;
	if (sRet == PLAYER_ERROR_NONE)
	{
		if ((ret = player_set_volume(NULL, left, right)) != PLAYER_ERROR_NONE)
		{
			dts_pass(api_name);
		}
	}
	dts_message(api_name, "Call log: %d %d", sRet, ret);
	dts_fail(api_name);
}

static void utc_media_player_get_volume_p(void)
{
	char* api_name = "player_get_volume";
	int ret;
	float left, right;
	if (sRet == PLAYER_ERROR_NONE)
	{
		if ((ret = player_get_volume(player, &left, &right)) == PLAYER_ERROR_NONE)
		{
			dts_pass(api_name);
		}
	}
	dts_message(api_name, "Call log: %d %d", sRet, ret);
	dts_fail(api_name);
}

static void utc_media_player_get_volume_n(void)
{
	char* api_name = "player_get_volume";
	int ret;
	float left, right;
	if (sRet == PLAYER_ERROR_NONE)
	{
		if ((ret = player_get_volume(player, &left, NULL)) != PLAYER_ERROR_NONE)
		{
			dts_pass(api_name);
		}
	}
	dts_message(api_name, "Call log: %d %d", sRet, ret);
	dts_fail(api_name);
}

static void utc_media_player_set_sound_type_p(void)
{
	char* api_name = "utc_media_player_set_sound_type_p";
	int ret;
	sound_type_e sound_type = SOUND_TYPE_MEDIA;
	if (sRet == PLAYER_ERROR_NONE)
	{
		if ((ret = player_set_sound_type(player,sound_type)) == PLAYER_ERROR_NONE)
		{
			dts_pass(api_name);
		}
	}
	dts_message(api_name, "Call log: %d %d", sRet, ret);
	dts_fail(api_name);
}

static void utc_media_player_set_sound_type_n(void)
{
	char* api_name = "utc_media_player_set_sound_type_n";
	int ret;
	if (sRet == PLAYER_ERROR_NONE)
	{
		if ((ret = player_set_sound_type(NULL, SOUND_TYPE_MEDIA)) != PLAYER_ERROR_NONE)
		{
			dts_pass(api_name);
		}
	}
	dts_message(api_name, "Call log: %d %d", sRet, ret);
	dts_fail(api_name);
}

static void utc_media_player_set_subtitle_path_p(void)
{
	char* api_name = "utc_media_player_set_subtitle_path_p";
	int ret;
	sound_type_e sound_type = SOUND_TYPE_MEDIA;
	if (sRet == PLAYER_ERROR_NONE)
	{
		if ((ret = player_set_subtitle_path(player,MEDIA_PATH)) == PLAYER_ERROR_NONE)
		{
			dts_pass(api_name);
		}
	}
	dts_message(api_name, "Call log: %d %d", sRet, ret);
	dts_fail(api_name);
}

static void utc_media_player_set_subtitle_path_n(void)
{
	char* api_name = "utc_media_player_set_subtitle_path_n";
	int ret;
	if (sRet == PLAYER_ERROR_NONE)
	{
		if ((ret = player_set_subtitle_path(NULL, NULL)) != PLAYER_ERROR_NONE)
		{
			dts_pass(api_name);
		}
	}
	dts_message(api_name, "Call log: %d %d", sRet, ret);
	dts_fail(api_name);
}

static void utc_media_player_set_x11_display_visible_p(void)
{
	char* api_name = "utc_media_player_set_x11_display_visible_p";
	int ret;
	if (sRet == PLAYER_ERROR_NONE)
	{
		if ((ret = player_set_display(player, PLAYER_DISPLAY_TYPE_X11,0)) == PLAYER_ERROR_NONE)
		{
			if ((ret = player_set_x11_display_visible(player, true)) == PLAYER_ERROR_NONE)
			{
				dts_pass(api_name);
			}
		}
	}
	dts_message(api_name, "Call log: %d %d", sRet, ret);
	dts_fail(api_name);
}

static void utc_media_player_set_x11_display_visible_n(void)
{
	char* api_name = "utc_media_player_set_x11_display_visible_n";
	int ret;
	if (sRet == PLAYER_ERROR_NONE)
	{
		if ((ret = player_set_display(player, PLAYER_DISPLAY_TYPE_X11,0)) == PLAYER_ERROR_NONE)
		{
			if ((ret = player_set_x11_display_visible(NULL, true)) != PLAYER_ERROR_NONE)
			{ 
				dts_pass(api_name);
			}
		}
	}
	dts_message(api_name, "Call log: %d %d", sRet, ret);
	dts_fail(api_name);
}

static void utc_media_player_is_x11_display_visible_p(void)
{
	char* api_name = "utc_media_player_is_x11_display_visible_p";
	int ret;
	if (sRet == PLAYER_ERROR_NONE)
	{
		if ((ret = player_set_display(player, PLAYER_DISPLAY_TYPE_X11,0)) == PLAYER_ERROR_NONE)
		{
			bool isVisible;
			if ((ret = player_is_x11_display_visible(player, &isVisible)) == PLAYER_ERROR_NONE)
			{
				dts_pass(api_name);
			}
		}
	}
	dts_message(api_name, "Call log: %d %d", sRet, ret);
	dts_fail(api_name);
}

static void utc_media_player_is_x11_display_visible_n(void)
{
	char* api_name = "utc_media_player_is_x11_display_visible_n";
	int ret;
	if (sRet == PLAYER_ERROR_NONE)
	{
		if ((ret = player_set_display(player, PLAYER_DISPLAY_TYPE_X11,0)) == PLAYER_ERROR_NONE)
		{
			if ((ret = player_is_x11_display_visible(NULL, NULL)) != PLAYER_ERROR_NONE)
			{ 
				dts_pass(api_name);
			}
		}
	}
	dts_message(api_name, "Call log: %d %d", sRet, ret);
	dts_fail(api_name);
}

static void utc_media_player_set_x11_display_rotation_p(void)
{
	char* api_name = "utc_media_player_set_x11_display_rotation_p";
	int ret;
	if (sRet == PLAYER_ERROR_NONE)
	{
		if ((ret = player_set_display(player, PLAYER_DISPLAY_TYPE_X11,0)) == PLAYER_ERROR_NONE)
		{
			if ((ret = player_set_x11_display_rotation(player, PLAYER_DISPLAY_ROTATION_180)) == PLAYER_ERROR_NONE)
			{ 
				dts_pass(api_name);
			}
		}
	}
	dts_message(api_name, "Call log: %d %d", sRet, ret);
	dts_fail(api_name);
}

static void utc_media_player_set_x11_display_rotation_n(void)
{
	char* api_name = "utc_media_player_set_x11_display_rotation_n";
	int ret;
	if (sRet == PLAYER_ERROR_NONE)
	{
		if ((ret = player_set_display(player, PLAYER_DISPLAY_TYPE_X11,0)) == PLAYER_ERROR_NONE)
		{
			if ((ret = player_set_x11_display_rotation(NULL, PLAYER_DISPLAY_ROTATION_180)) != PLAYER_ERROR_NONE)
			{
				dts_pass(api_name);
			}
		}
	}
	dts_message(api_name, "Call log: %d %d", sRet, ret);
	dts_fail(api_name);
}

static void utc_media_player_get_x11_display_rotation_p(void)
{
	char* api_name = "utc_media_player_get_x11_display_rotation_p";
	int ret;
	if (sRet == PLAYER_ERROR_NONE)
	{
		if ((ret = player_set_display(player, PLAYER_DISPLAY_TYPE_X11,0)) == PLAYER_ERROR_NONE)
		{
			player_display_rotation_e rotation;
			if ((ret = player_get_x11_display_rotation(player, &rotation)) == PLAYER_ERROR_NONE)
			{ 
				dts_pass(api_name);
			}
		}
	}
	dts_message(api_name, "Call log: %d %d", sRet, ret);
	dts_fail(api_name);
}

static void utc_media_player_get_x11_display_rotation_n(void)
{
	char* api_name = "utc_media_player_get_x11_display_rotation_n";
	int ret;
	if (sRet == PLAYER_ERROR_NONE)
	{
		if ((ret = player_get_x11_display_rotation(NULL, PLAYER_DISPLAY_ROTATION_180)) != PLAYER_ERROR_NONE)
		{
				dts_pass(api_name);
		}
	}
	dts_message(api_name, "Call log: %d %d", sRet, ret);
	dts_fail(api_name);
}

static void utc_media_player_set_display_mode_p(void)
{
	char* api_name = "utc_media_player_set_display_mode_p";
	int ret;
	if (sRet == PLAYER_ERROR_NONE)
	{
		if ((ret = player_set_display(player, PLAYER_DISPLAY_TYPE_X11,0)) == PLAYER_ERROR_NONE)
		{
			if ((ret = player_set_display_mode(player, PLAYER_DISPLAY_MODE_FULL_SCREEN)) == PLAYER_ERROR_NONE)
			{ 
				dts_pass(api_name);
			}
		}
	}
	dts_message(api_name, "Call log: %d %d", sRet, ret);
	dts_fail(api_name);
}

static void utc_media_player_set_display_mode_n(void)
{
	char* api_name = "utc_media_player_set_display_mode_n";
	int ret;
	if (sRet == PLAYER_ERROR_NONE)
	{
		if ((ret = player_set_display(player, PLAYER_DISPLAY_TYPE_X11,0)) == PLAYER_ERROR_NONE)
		{
			if ((ret = player_set_display_mode(NULL, PLAYER_DISPLAY_MODE_FULL_SCREEN)) != PLAYER_ERROR_NONE)
			{
				dts_pass(api_name);
			}
		}
	}
	dts_message(api_name, "Call log: %d %d", sRet, ret);
	dts_fail(api_name);
}

static void utc_media_player_get_display_mode_p(void)
{
	char* api_name = "utc_media_player_get_display_mode_p";
	int ret;
	if (sRet == PLAYER_ERROR_NONE)
	{
		if ((ret = player_set_display(player, PLAYER_DISPLAY_TYPE_X11,0)) == PLAYER_ERROR_NONE)
		{
			player_display_mode_e mode;
			if ((ret = player_get_display_mode(player, &mode)) == PLAYER_ERROR_NONE)
			{
				dts_pass(api_name);
			}
		}
	}
	dts_message(api_name, "Call log: %d %d", sRet, ret);
	dts_fail(api_name);
}

static void utc_media_player_get_display_mode_n(void)
{
	char* api_name = "utc_media_player_get_display_mode_n";
	int ret;
	if (sRet == PLAYER_ERROR_NONE)
	{
		if ((ret = player_get_display_mode(NULL, PLAYER_DISPLAY_MODE_FULL_SCREEN)) != PLAYER_ERROR_NONE)
		{
				dts_pass(api_name);
		}
	}
	dts_message(api_name, "Call log: %d %d", sRet, ret);
	dts_fail(api_name);
}

static void utc_media_player_capture_video_p(void)
{
	char* api_name = "utc_media_player_capture_video_p";
	int ret;

	if (sRet == PLAYER_ERROR_NONE)
	{
		player_prepare(player);
		{
			if ((ret = player_start(player)) == PLAYER_ERROR_NONE)
			{			
				sleep(3);
				if ((ret = player_capture_video(player,player_video_capture_cb ,NULL)) == PLAYER_ERROR_NONE)
				{
					player_stop(player);
					reset();
					dts_pass(api_name);
				}
				if ((sRet = player_stop(player)) == PLAYER_ERROR_NONE)
				{
					dts_message(api_name, "Call log: %d %d", sRet, ret);
				}
			}
			reset();
		}
	}
	dts_message(api_name, "Call log: %d %d", sRet, ret);
	dts_fail(api_name);
}

static void utc_media_player_capture_video_n(void)
{
	char* api_name = "utc_media_player_capture_video_n";
	int ret;
	if (sRet == PLAYER_ERROR_NONE)
	{
		if ((ret = player_capture_video(NULL,player_video_capture_cb ,NULL)) != PLAYER_ERROR_NONE)
		{
				dts_pass(api_name);
		}
	}
	dts_message(api_name, "Call log: %d %d", sRet, ret);
	dts_fail(api_name);
}