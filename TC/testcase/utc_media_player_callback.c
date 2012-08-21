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

static void utc_media_player_set_started_callback_p(void);
static void utc_media_player_set_started_callback_n(void);
static void utc_media_player_unset_started_callback_p(void);
static void utc_media_player_unset_started_callback_n(void);
static void utc_media_player_set_buffering_callback_p(void);
static void utc_media_player_set_buffering_callback_n(void);
static void utc_media_player_unset_buffering_callback_p(void);
static void utc_media_player_unset_buffering_callback_n(void);
static void utc_media_player_set_complete_callback_p(void);
static void utc_media_player_set_complete_callback_n(void);
static void utc_media_player_unset_complete_callback_p(void);
static void utc_media_player_unset_complete_callback_n(void);
static void utc_media_player_set_error_callback_p(void);
static void utc_media_player_set_error_callback_n(void);
static void utc_media_player_unset_error_callback_p(void);
static void utc_media_player_unset_error_callback_n(void);
static void utc_media_player_set_interrupt_callback_p(void);
static void utc_media_player_set_interrupt_callback_n(void);
static void utc_media_player_unset_interrupt_callback_p(void);
static void utc_media_player_unset_interrupt_callback_n(void);
static void utc_media_player_set_paused_callback_p(void);
static void utc_media_player_set_paused_callback_n(void);
static void utc_media_player_unset_paused_callback_p(void);
static void utc_media_player_unset_paused_callback_n(void);
static void utc_media_player_set_subtitle_updated_callback_p(void);
static void utc_media_player_set_subtitle_updated_callback_n(void);
static void utc_media_player_unset_subtitle_updated_callback_p(void);
static void utc_media_player_unset_subtitle_updated_callback_n(void);

struct tet_testlist tet_testlist[] = {
	{ utc_media_player_set_started_callback_p, POSITIVE_TC_IDX },
	{ utc_media_player_set_started_callback_n, NEGATIVE_TC_IDX },
	{ utc_media_player_unset_started_callback_p, POSITIVE_TC_IDX },
	{ utc_media_player_unset_started_callback_n, NEGATIVE_TC_IDX },
	{ utc_media_player_set_buffering_callback_p, POSITIVE_TC_IDX },
	{ utc_media_player_set_buffering_callback_n, NEGATIVE_TC_IDX },
	{ utc_media_player_unset_buffering_callback_p, POSITIVE_TC_IDX },
	{ utc_media_player_unset_buffering_callback_n, NEGATIVE_TC_IDX },
	{ utc_media_player_set_complete_callback_p, POSITIVE_TC_IDX },
	{ utc_media_player_set_complete_callback_n, NEGATIVE_TC_IDX },
	{ utc_media_player_unset_complete_callback_p, POSITIVE_TC_IDX },
	{ utc_media_player_unset_complete_callback_n, NEGATIVE_TC_IDX },
	{ utc_media_player_set_error_callback_p, POSITIVE_TC_IDX },
	{ utc_media_player_set_error_callback_n, NEGATIVE_TC_IDX },
	{ utc_media_player_unset_error_callback_p, POSITIVE_TC_IDX },
	{ utc_media_player_unset_error_callback_n, NEGATIVE_TC_IDX },
	{ utc_media_player_set_interrupt_callback_p, POSITIVE_TC_IDX },
	{ utc_media_player_set_interrupt_callback_n, NEGATIVE_TC_IDX },
	{ utc_media_player_unset_interrupt_callback_p, POSITIVE_TC_IDX },
	{ utc_media_player_unset_interrupt_callback_n, NEGATIVE_TC_IDX },
	{ utc_media_player_set_paused_callback_p, POSITIVE_TC_IDX },
	{ utc_media_player_set_paused_callback_n, NEGATIVE_TC_IDX },
	{ utc_media_player_unset_paused_callback_p, POSITIVE_TC_IDX },
	{ utc_media_player_unset_paused_callback_n, NEGATIVE_TC_IDX },
	{ utc_media_player_set_subtitle_updated_callback_p, POSITIVE_TC_IDX },
	{ utc_media_player_set_subtitle_updated_callback_n, NEGATIVE_TC_IDX },
	{ utc_media_player_unset_subtitle_updated_callback_p, POSITIVE_TC_IDX },
	{ utc_media_player_unset_subtitle_updated_callback_n, NEGATIVE_TC_IDX },
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
	sRet = player_create(&player);
}

static void cleanup(void)
{
	player_destroy(player);
	g_main_loop_quit (g_mainloop);
	g_thread_join(event_thread);
}

static void player_start_cb(void *data)
{
	dts_message("PlayerEvent", "Player Started!!!");
}

static void player_complete_cb(void *data)
{
	dts_message("PlayerEvent", "Player Started!!!");
}


static void player_pause_cb(void *data)
{
	dts_message("PlayerEvent", "Player Paused!!!");
}

static void player_buffer_cb(int percent, void *data)
{
	dts_message("PlayerEvent", "Player Buffering : %d!!!", percent);
}

static void player_interrupt_cb(player_interrupted_code_e code, void *data)
{
	dts_message("PlayerEvent", "Player Interrupted!!!");
}

static  void player_subtitle_update_cb(unsigned long duration, char *text, void *user_data)
{
	dts_message("PlayerEvent", "Player Subtitle Updated  duration : %ld, text : %s!!!", duration, text);
}

static void utc_media_player_set_started_callback_p(void)
{
	char* api_name = "player_set_started_cb";
	int ret;
	if (sRet == PLAYER_ERROR_NONE)
	{
		if ((ret = player_set_started_cb(player, player_start_cb, &sRet)) == PLAYER_ERROR_NONE)
		{
			dts_pass(api_name);
		}
	}
	dts_message(api_name, "Call log: %d %d", sRet, ret);
	dts_fail(api_name);
}

static void utc_media_player_set_started_callback_n(void)
{
	char* api_name = "player_set_started_cb";
	int ret;
	if (sRet == PLAYER_ERROR_NONE)
	{
		if ((ret = player_set_started_cb(player, NULL, &sRet)) != PLAYER_ERROR_NONE)
		{
			dts_pass(api_name);
		}
	}
	dts_message(api_name, "Call log: %d %d", sRet, ret);
	dts_fail(api_name);
}

static void utc_media_player_unset_started_callback_p(void)
{
	char* api_name = "utc_media_player_unset_started_callback_p";
	int ret;
	if (sRet == PLAYER_ERROR_NONE)
	{
		if ((ret = player_unset_started_cb(player)) == PLAYER_ERROR_NONE)
		{
			dts_pass(api_name);
		}
	}
	dts_message(api_name, "Call log: %d %d", sRet, ret);
	dts_fail(api_name);
}

static void utc_media_player_unset_started_callback_n(void)
{
	char* api_name = "utc_media_player_unset_started_callback_n";
	int ret;
	if (sRet == PLAYER_ERROR_NONE)
	{
		if ((ret = player_unset_started_cb(NULL)) != PLAYER_ERROR_NONE)
		{
			dts_pass(api_name);
		}
	}
	dts_message(api_name, "Call log: %d %d", sRet, ret);
	dts_fail(api_name);
}

static void utc_media_player_set_buffering_callback_p(void)
{
	char* api_name = "player_set_buffering_cb";
	int ret;
	if (sRet == PLAYER_ERROR_NONE)
	{
		if ((ret = player_set_buffering_cb(player, player_buffer_cb, &sRet)) == PLAYER_ERROR_NONE)
		{
			dts_pass(api_name);
		}
	}
	dts_message(api_name, "Call log: %d %d", sRet, ret);
	dts_fail(api_name);
}

static void utc_media_player_set_buffering_callback_n(void)
{
	char* api_name = "player_set_buffering_cb";
	int ret;
	player_h player1 = NULL;
	if ((ret = player_set_buffering_cb(player1, player_buffer_cb, &sRet)) != PLAYER_ERROR_NONE)
	{
		dts_pass(api_name);
	}
	dts_message(api_name, "Call log: %d %d", sRet, ret);
	dts_fail(api_name);
}

static void utc_media_player_unset_buffering_callback_p(void)
{
	char* api_name = "utc_media_player_unset_buffering_callback_p";
	int ret;
	if (sRet == PLAYER_ERROR_NONE)
	{
		if ((ret = player_unset_buffering_cb(player)) == PLAYER_ERROR_NONE)
		{
			dts_pass(api_name);
		}
	}
	dts_message(api_name, "Call log: %d %d", sRet, ret);
	dts_fail(api_name);
}

static void utc_media_player_unset_buffering_callback_n(void)
{
	char* api_name = "utc_media_player_unset_buffering_callback_n";
	int ret;
	if (sRet == PLAYER_ERROR_NONE)
	{
		if ((ret = player_unset_buffering_cb(NULL)) != PLAYER_ERROR_NONE)
		{
			dts_pass(api_name);
		}
	}
	dts_message(api_name, "Call log: %d %d", sRet, ret);
	dts_fail(api_name);
}

static void utc_media_player_set_complete_callback_p(void)
{
	char* api_name = "utc_media_player_set_complete_callback_p";
	int ret;
	if (sRet == PLAYER_ERROR_NONE)
	{
		if ((ret = player_set_completed_cb(player, player_complete_cb, &sRet)) == PLAYER_ERROR_NONE)
		{
			dts_pass(api_name);
		}
	}
	dts_message(api_name, "Call log: %d %d", sRet, ret);
	dts_fail(api_name);
}

static void utc_media_player_set_complete_callback_n(void)
{
	char* api_name = "player_set_completed_cb";
	int ret;
	player_h player1 = NULL;
	if ((ret = player_set_completed_cb(player1, player_complete_cb, &sRet)) != PLAYER_ERROR_NONE)
	{
		dts_pass(api_name);
	}
	dts_message(api_name, "Call log: %d %d", sRet, ret);
	dts_fail(api_name);
}

static void utc_media_player_unset_complete_callback_p(void)
{
	char* api_name = "utc_media_player_unset_complete_callback_p";
	int ret;
	if (sRet == PLAYER_ERROR_NONE)
	{
		if ((ret = player_unset_completed_cb(player)) == PLAYER_ERROR_NONE)
		{
			dts_pass(api_name);
		}
	}
	dts_message(api_name, "Call log: %d %d", sRet, ret);
	dts_fail(api_name);
}

static void utc_media_player_unset_complete_callback_n(void)
{
	char* api_name = "utc_media_player_unset_complete_callback_n";
	int ret;
	if (sRet == PLAYER_ERROR_NONE)
	{
		if ((ret = player_unset_completed_cb(NULL)) != PLAYER_ERROR_NONE)
		{
			dts_pass(api_name);
		}
	}
	dts_message(api_name, "Call log: %d %d", sRet, ret);
	dts_fail(api_name);
}

static void utc_media_player_set_error_callback_p(void)
{
	char* api_name = "player_set_error_cb";
	int ret;
	if (sRet == PLAYER_ERROR_NONE)
	{
		if ((ret = player_set_error_cb(player, player_buffer_cb, &sRet)) == PLAYER_ERROR_NONE)
		{
			dts_pass(api_name);
		}
	}
	dts_message(api_name, "Call log: %d %d", sRet, ret);
	dts_fail(api_name);
}

static void utc_media_player_set_error_callback_n(void)
{
	char* api_name = "player_set_error_cb";
	int ret;
	if (sRet == PLAYER_ERROR_NONE)
	{
		if ((ret = player_destroy(player)) == PLAYER_ERROR_NONE)
		{
			player = NULL;
			if ((ret = player_set_error_cb(player, player_buffer_cb, &sRet)) != PLAYER_ERROR_NONE)
			{
				sRet = player_create(&player);
				dts_pass(api_name);
			}
			sRet = player_create(&player);
		}
	}
	dts_message(api_name, "Call log: %d %d", sRet, ret);
	dts_fail(api_name);
}

static void utc_media_player_unset_error_callback_p(void)
{
	char* api_name = "utc_media_player_unset_error_callback_p";
	int ret;
	if (sRet == PLAYER_ERROR_NONE)
	{
		if ((ret = player_unset_error_cb(player)) == PLAYER_ERROR_NONE)
		{
			dts_pass(api_name);
		}
	}
	dts_message(api_name, "Call log: %d %d", sRet, ret);
	dts_fail(api_name);
}

static void utc_media_player_unset_error_callback_n(void)
{
	char* api_name = "utc_media_player_unset_error_callback_n";
	int ret;
	if (sRet == PLAYER_ERROR_NONE)
	{
		if ((ret = player_unset_error_cb(NULL)) != PLAYER_ERROR_NONE)
		{
			dts_pass(api_name);
		}
	}
	dts_message(api_name, "Call log: %d %d", sRet, ret);
	dts_fail(api_name);
}

static void utc_media_player_set_interrupt_callback_p(void)
{
	char* api_name = "player_set_interrupted_cb";
	int ret;
	if (sRet == PLAYER_ERROR_NONE)
	{
		if ((ret = player_set_interrupted_cb(player, player_interrupt_cb, &sRet)) == PLAYER_ERROR_NONE)
		{
			dts_pass(api_name);
		}
	}
	dts_message(api_name, "Call log: %d %d", sRet, ret);
	dts_fail(api_name);
}

static void utc_media_player_set_interrupt_callback_n(void)
{
	char* api_name = "player_set_interrupted_cb";
	int ret;
	if (sRet == PLAYER_ERROR_NONE)
	{
		if ((ret = player_set_interrupted_cb(player, NULL, &sRet)) != PLAYER_ERROR_NONE)
		{
			dts_pass(api_name);
		}
	}
	dts_message(api_name, "Call log: %d %d", sRet, ret);
	dts_fail(api_name);
}

static void utc_media_player_unset_interrupt_callback_p(void)
{
	char* api_name = "utc_media_player_unset_interrupt_callback_p";
	int ret;
	if (sRet == PLAYER_ERROR_NONE)
	{
		if ((ret = player_unset_interrupted_cb(player)) == PLAYER_ERROR_NONE)
		{
			dts_pass(api_name);
		}
	}
	dts_message(api_name, "Call log: %d %d", sRet, ret);
	dts_fail(api_name);
}

static void utc_media_player_unset_interrupt_callback_n(void)
{
	char* api_name = "utc_media_player_unset_interrupt_callback_n";
	int ret;
	if (sRet == PLAYER_ERROR_NONE)
	{
		if ((ret = player_unset_interrupted_cb(NULL)) != PLAYER_ERROR_NONE)
		{
			dts_pass(api_name);
		}
	}
	dts_message(api_name, "Call log: %d %d", sRet, ret);
	dts_fail(api_name);
}

static void utc_media_player_set_paused_callback_p(void)
{
	char* api_name = "utc_media_player_set_paused_callback_p";
	int ret;
	if (sRet == PLAYER_ERROR_NONE)
	{
		if ((ret = player_set_paused_cb(player, player_pause_cb, &sRet)) == PLAYER_ERROR_NONE)
		{
			dts_pass(api_name);
		}
	}
	dts_message(api_name, "Call log: %d %d", sRet, ret);
	dts_fail(api_name);
}

static void utc_media_player_set_paused_callback_n(void)
{
	char* api_name = "utc_media_player_set_paused_callback_n";
	int ret;
	if (sRet == PLAYER_ERROR_NONE)
	{
		if ((ret = player_set_paused_cb(player, NULL, &sRet)) != PLAYER_ERROR_NONE)
		{
			dts_pass(api_name);
		}
	}
	dts_message(api_name, "Call log: %d %d", sRet, ret);
	dts_fail(api_name);
}

static void utc_media_player_unset_paused_callback_p(void)
{
	char* api_name = "utc_media_player_unset_paused_callback_p";
	int ret;
	if (sRet == PLAYER_ERROR_NONE)
	{
		if ((ret = player_unset_paused_cb(player)) == PLAYER_ERROR_NONE)
		{
			dts_pass(api_name);
		}
	}
	dts_message(api_name, "Call log: %d %d", sRet, ret);
	dts_fail(api_name);
}

static void utc_media_player_unset_paused_callback_n(void)
{
	char* api_name = "utc_media_player_unset_paused_callback_n";
	int ret;
	if (sRet == PLAYER_ERROR_NONE)
	{
		if ((ret = player_unset_paused_cb(NULL)) != PLAYER_ERROR_NONE)
		{
			dts_pass(api_name);
		}
	}
	dts_message(api_name, "Call log: %d %d", sRet, ret);
	dts_fail(api_name);
}

static void utc_media_player_set_subtitle_updated_callback_p(void)
{
	char* api_name = "utc_media_player_set_subtitle_updated_callback_p";
	int ret;
	if (sRet == PLAYER_ERROR_NONE)
	{
		if ((ret = player_set_subtitle_updated_cb(player, player_subtitle_update_cb, &sRet)) == PLAYER_ERROR_NONE)
		{
			dts_pass(api_name);
		}
	}
	dts_message(api_name, "Call log: %d %d", sRet, ret);
	dts_fail(api_name);
}

static void utc_media_player_set_subtitle_updated_callback_n(void)
{
	char* api_name = "utc_media_player_set_subtitle_updated_callback_n";
	int ret;
	if (sRet == PLAYER_ERROR_NONE)
	{
		if ((ret = player_set_subtitle_updated_cb(player, NULL, &sRet)) != PLAYER_ERROR_NONE)
		{
			dts_pass(api_name);
		}
	}
	dts_message(api_name, "Call log: %d %d", sRet, ret);
	dts_fail(api_name);
}

static void utc_media_player_unset_subtitle_updated_callback_p(void)
{
	char* api_name = "utc_media_player_unset_subtitle_updated_callback_p";
	int ret;
	if (sRet == PLAYER_ERROR_NONE)
	{
		if ((ret = player_unset_subtitle_updated_cb(player)) == PLAYER_ERROR_NONE)
		{
			dts_pass(api_name);
		}
	}
	dts_message(api_name, "Call log: %d %d", sRet, ret);
	dts_fail(api_name);
}

static void utc_media_player_unset_subtitle_updated_callback_n(void)
{
	char* api_name = "utc_media_player_unset_subtitle_updated_callback_n";
	int ret;
	if (sRet == PLAYER_ERROR_NONE)
	{
		if ((ret = player_unset_subtitle_updated_cb(NULL)) != PLAYER_ERROR_NONE)
		{
			dts_pass(api_name);
		}
	}
	dts_message(api_name, "Call log: %d %d", sRet, ret);
	dts_fail(api_name);
}