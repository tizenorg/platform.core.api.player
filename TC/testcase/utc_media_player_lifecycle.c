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
#define INVALID_MEDIA_PATH "./invalid.mp7"

enum {
	POSITIVE_TC_IDX = 0x01,
	NEGATIVE_TC_IDX,
};

static void startup(void);
static void cleanup(void);

void (*tet_startup)(void) = startup;
void (*tet_cleanup)(void) = cleanup;

static void utc_media_player_create_p(void);
static void utc_media_player_create_n(void);
static void utc_media_player_destroy_p(void);
static void utc_media_player_destroy_n(void);
static void utc_media_player_pause_p(void);
static void utc_media_player_pause_n(void);
static void utc_media_player_prepare_p(void);
static void utc_media_player_prepare_n(void);

struct tet_testlist tet_testlist[] = {
	{ utc_media_player_create_p, POSITIVE_TC_IDX },
	{ utc_media_player_create_n, NEGATIVE_TC_IDX },
	{ utc_media_player_destroy_p, POSITIVE_TC_IDX },
	{ utc_media_player_destroy_n, NEGATIVE_TC_IDX },
	{ utc_media_player_pause_p, POSITIVE_TC_IDX },
	{ utc_media_player_pause_n, NEGATIVE_TC_IDX },
	{ utc_media_player_prepare_p, POSITIVE_TC_IDX },
	{ utc_media_player_prepare_n, NEGATIVE_TC_IDX },
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
}

static void cleanup(void)
{
	g_main_loop_quit (g_mainloop);
	g_thread_join(event_thread);
}



/**
 * @brief Positive test case of player_create()
 */
static void utc_media_player_create_p(void)
{
	char* api_name = "player_create";
	player_h player;
	int ret;
	if ((ret = player_create(&player)) == PLAYER_ERROR_NONE)
	{
		player_destroy(player);
		dts_pass(api_name);
	}
	dts_message(api_name, "Call log: %d", ret);
	dts_fail(api_name);
}

static void utc_media_player_create_n(void)
{
	char* api_name = "player_create";
	int ret;
	if ((ret = player_create(NULL)) != PLAYER_ERROR_NONE)
	{
		dts_pass(api_name);
	}
	dts_message(api_name, "Call log: %d", ret);
	dts_fail(api_name);
}

static void utc_media_player_destroy_p(void)
{
	char* api_name = "player_destroy";
	player_h player;
	int ret;
	if ((ret = player_create(&player)) == PLAYER_ERROR_NONE)
	{
		if ((ret = player_destroy(player)) == PLAYER_ERROR_NONE)
		{
			dts_pass(api_name);
		}
	}
	dts_message(api_name, "Call log: %d", ret);
	dts_fail(api_name);
}

static void utc_media_player_destroy_n(void)
{
	char* api_name = "player_destroy";
	int ret;
	if ((ret = player_destroy(NULL)) != PLAYER_ERROR_NONE)
	{
		dts_pass(api_name);
	}
	dts_message(api_name, "Call log: %d", ret);
	dts_fail(api_name);
}

static void player_start_cb(void *data)
{
	char* api_name = "player_pause";
	player_h p = (player_h)data;
	dts_message(api_name, "Player Started !!!");
	player_state_e s;
	player_get_state(p, &s);
	dts_message(api_name, "Current state : %d", s);
}


static void utc_media_player_pause_p(void)
{
	char* api_name = "player_pause";
	player_h player;
	int ret;
	if ((ret = player_create(&player)) == PLAYER_ERROR_NONE)
	{
		ret =player_set_started_cb(player, player_start_cb, player);
		if ((ret = player_set_uri(player, MEDIA_PATH)) == PLAYER_ERROR_NONE)
		{
			if ((ret = player_prepare(player)) == PLAYER_ERROR_NONE)
			{
				if ((ret = player_start(player)) == PLAYER_ERROR_NONE)
				{
					int timeout = 3 ;
					player_state_e state;
					int i;
					for(i = 0; i<timeout;i++)
					{
						player_get_state(player, &state);
						dts_message(api_name, "Current state : %d", state);
						if(state == PLAYER_STATE_PLAYING)
						{
							timeout = 0;
							break;
						}
						sleep(1);	
					}
					
					if (timeout !=0)
					{
						dts_message(api_name, "Playing Timed out - state : %d", state);
						dts_fail(api_name);
					}
					
					if ((ret = player_pause(player)) == PLAYER_ERROR_NONE)
					{
						player_stop(player);
						player_unprepare(player);
						player_destroy(player);
						dts_pass(api_name);
					}
					player_stop(player);
				}
				player_unprepare(player);
			}
		}
		player_destroy(player);
	}
	dts_message(api_name, "Call log: %d", ret);
	dts_fail(api_name);
}

static void utc_media_player_pause_n(void)
{
	char* api_name = "player_pause";
	player_h player;
	int ret;
	if ((ret = player_create(&player)) == PLAYER_ERROR_NONE)
	{
		if ((ret = player_set_uri(player, MEDIA_PATH)) == PLAYER_ERROR_NONE)
		{
			if ((ret = player_prepare(player)) == PLAYER_ERROR_NONE)
			{
				if ((ret = player_pause(player)) != PLAYER_ERROR_NONE)
				{
					player_unprepare(player);
					player_destroy(player);
					dts_pass(api_name);
				}
				player_unprepare(player);
			}
		}
		player_destroy(player);
	}
	dts_message(api_name, "Call log: %d", ret);
	dts_fail(api_name);
}

static void utc_media_player_prepare_p(void)
{
	char* api_name = "player_prepare";
	player_h player;
	int ret;
	if ((ret = player_create(&player)) == PLAYER_ERROR_NONE)
	{
		if ((ret = player_set_uri(player, MEDIA_PATH)) == PLAYER_ERROR_NONE)
		{
			if ((ret = player_prepare(player)) == PLAYER_ERROR_NONE)
			{
				player_unprepare(player);
				player_destroy(player);
				dts_pass(api_name);
			}
			player_unprepare(player);
		}
		player_destroy(player);
	}
	dts_message(api_name, "Call log: %d", ret);
	dts_fail(api_name);
}

static void utc_media_player_prepare_n(void)
{
	char* api_name = "player_prepare";
	player_h player;
	int ret;
	if ((ret = player_create(&player)) == PLAYER_ERROR_NONE)
	{
		if ((ret = player_set_uri(player, INVALID_MEDIA_PATH)) == PLAYER_ERROR_NONE)
		{	dts_message(api_name, "Call log: %d", ret);

			if ((ret = player_prepare(player)) != PLAYER_ERROR_NONE)
			{
				player_unprepare(player);
				player_destroy(player);
				dts_pass(api_name);
			}
		}
		player_destroy(player);
	}
	dts_message(api_name, "Call log: %d", ret);
	dts_fail(api_name);
}


