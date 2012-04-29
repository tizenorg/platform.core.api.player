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
 
#include <player.h>
#include <pthread.h>
#include <glib.h>
#include <dlfcn.h>
#include <appcore-efl.h>
#include <Elementary.h>
#include <Ecore_X.h>

#define PACKAGE "player_test"
#define MAX_STRING_LEN		2048
#define MMTS_SAMPLELIST_INI_DEFAULT_PATH "/opt/etc/mmts_filelist.ini"
#define INI_SAMPLE_LIST_MAX 9
char g_subtitle_uri[MAX_STRING_LEN];

enum
{
	CURRENT_STATUS_MAINMENU,
	CURRENT_STATUS_FILENAME,
	CURRENT_STATUS_VOLUME,
	CURRENT_STATUS_MUTE,
	CURRENT_STATUS_POSITION_TIME,
	CURRENT_STATUS_POSITION_PERCENT,
	CURRENT_STATUS_LOOPING,
	CURRENT_STATUS_DISPLAY_MODE,
	CURRENT_STATUS_DISPLAY_ROTATION,
	CURRENT_STATUS_DISPLAY_VISIBLE,
	CURRENT_STATUS_SUBTITLE_FILENAME
};

struct appdata
{
	Evas *evas;
	Ecore_Evas *ee;
	Evas_Object *win;

	Evas_Object *layout_main; /* layout widget based on EDJ */
	Ecore_X_Window xid;
	/* add more variables here */
};

static void win_del(void *data, Evas_Object *obj, void *event)
{
		elm_exit();
}

static Evas_Object* create_win(const char *name)
{
		Evas_Object *eo;
		int w, h;

		printf ("[%s][%d] name=%s\n", __func__, __LINE__, name);

		eo = elm_win_add(NULL, name, ELM_WIN_BASIC);
		if (eo) {
				elm_win_title_set(eo, name);
				elm_win_borderless_set(eo, EINA_TRUE);
				evas_object_smart_callback_add(eo, "delete,request",win_del, NULL);
				ecore_x_window_size_get(ecore_x_window_root_first_get(),&w, &h);
				evas_object_resize(eo, w, h);
		}

		return eo;
}

static int app_create(void *data)
{
		struct appdata *ad = data;
		Evas_Object *win;
		/* create window */
		win = create_win(PACKAGE);
		if (win == NULL)
				return -1;
		ad->win = win; 
		evas_object_show(win); 
		return 0;
}

static int app_terminate(void *data)
{
		struct appdata *ad = data;

		if (ad->win)
				evas_object_del(ad->win);

		return 0;
}

struct appcore_ops ops = {
		.create = app_create,
		.terminate = app_terminate, 	
};

struct appdata ad;
static player_h g_player = 0;
int g_menu_state = CURRENT_STATUS_MAINMENU;
char g_file_list[9][256];
gboolean quit_pushing;

static void paused_cb(void *user_data)
{
	g_print("[Player_Test] paused_cb!!!!\n");
}

static void started_cb(void *user_data)
{
	g_print("[Player_Test] started_cb!!!!\n");
}

static void buffering_cb(int percent, void *user_data)
{
	g_print("[Player_Test] buffering_cb!!!! percent : %d\n", percent);
}



static void completed_cb(void *user_data)
{
	g_print("[Player_Test] completed_cb!!!!\n");
}

static void subtitle_updated_cb(unsigned long duration, char *text, void *user_data)
{
	g_print("[Player_Test] subtitle_updated_cb!!!! [%ld] %s\n",duration, text);
}

static void video_captured_cb(unsigned char *data, int width, int height,unsigned int size, void *user_data)
{
	g_print("[Player_Test] video_captured_cb!!!! width: %d, height : %d, size : %d \n",width, height,size);
}

static void input_filename(char *filename)
{
	int len = strlen(filename);

	if ( len < 0 || len > MAX_STRING_LEN )
		return;
	if(g_player!=NULL)
	{
		player_unprepare(g_player);
		player_destroy(g_player);
	}
	g_player = 0;

	if ( player_create(&g_player) != PLAYER_ERROR_NONE )
	{
		g_print("player create is failed\n");
	}


#if 0 //ned(APPSRC_TEST)
	gchar uri[100];
	gchar *ext;
	gsize file_size;
	GMappedFile *file;
	GError *error = NULL;
	guint8* g_media_mem = NULL;

	ext = filename;

	file = g_mapped_file_new (ext, FALSE, &error);
	file_size = g_mapped_file_get_length (file);
	g_media_mem = (guint8 *) g_mapped_file_get_contents (file);

	g_sprintf(uri, "mem://ext=%s,size=%d", ext ? ext : "", file_size);
	g_print("[uri] = %s\n", uri);

	mm_player_set_attribute(g_player,
								&g_err_name,
								"profile_uri", uri, strlen(uri),
								"profile_user_param", g_media_mem, file_size
								NULL);
#else
	player_set_uri(g_player, filename);
#endif /* APPSRC_TEST */

	int slen = strlen(g_subtitle_uri);

	if ( slen > 0 && slen < MAX_STRING_LEN )
	{
		g_print("0. set subtile path() - %s \n", g_subtitle_uri);
		player_set_subtitle_path(g_player,g_subtitle_uri);
		player_set_subtitle_updated_cb(g_player, subtitle_updated_cb, (void*)g_player);
	}
	player_set_display(g_player,PLAYER_DISPLAY_TYPE_X11,GET_DISPLAY(ad.xid));

	player_set_paused_cb(g_player, paused_cb, (void*)g_player);
	player_set_started_cb(g_player, started_cb, (void*)g_player);
	player_set_buffering_cb(g_player, buffering_cb, (void*)g_player);
	player_set_completed_cb(g_player, completed_cb, (void*)g_player);

	int ret;
	player_state_e state;
	ret = player_get_state(g_player, &state);
	g_print("1. After player_create() - Current State : %d \n", state);
	
	ret = player_prepare(g_player);
	if ( ret != PLAYER_ERROR_NONE )
	{
		g_print("prepare is failed (errno = %d) \n", ret);
	}
	
	ret = player_get_state(g_player, &state);
	g_print("2. After player_prepare() - Current State : %d \n", state);
}



static void _player_play()
{
	int bRet = FALSE;
	bRet = player_start(g_player);
}

static void _player_stop()
{
	int bRet = FALSE;
	bRet = player_stop(g_player);
}

static void _player_resume()
{
	int bRet = FALSE;
	bRet = player_start(g_player);
}

static void _player_pause()
{
	int bRet = FALSE;
	bRet = player_pause(g_player);
}

static void _player_state()
{
	player_state_e state;
	player_get_state(g_player, &state);
	g_print("                                                            ==> [Player_Test] Current Player State : %d\n", state);
}


static void set_volume(float volume)
{
	if ( player_set_volume(g_player, volume, volume) != PLAYER_ERROR_NONE )
	{
		g_print("failed to set volume\n");
	}
}

static void get_volume(float* left, float* right)
{
	player_get_volume(g_player, left, right);
	g_print("                                                            ==> [Player_Test] volume - left : %f, right : %f\n", *left, *right);
}

static void set_mute(bool mute)
{
	if ( player_set_mute(g_player, mute) != PLAYER_ERROR_NONE )
	{
		g_print("failed to set_mute\n");
	}
}

static void get_mute(bool *mute)
{
	player_is_muted(g_player, mute);
	g_print("                                                            ==> [Player_Test] mute = %d\n", *mute);
}

static void get_position()
{
	int position = 0;
	int percent = 0;
	int ret;
	ret = player_get_position(g_player, &position);
	g_print("                                                            ==> [Player_Test] player_get_position() return : %d\n",ret);
	ret = player_get_position_ratio(g_player, &percent);
	g_print("                                                            ==> [Player_Test] player_get_position_ratio() return : %d\n",ret);
	g_print("                                                            ==> [Player_Test] Pos: [%d ] msec\n", position);
	g_print("                                                            ==> [Player_Test] Pos: [%d] percent\n", percent);
}

static void set_position(int position)
{
	if ( player_set_position(g_player,  position) != PLAYER_ERROR_NONE )
	{
		g_print("failed to set position\n");
	}
}

static void set_position_ratio(int percent)
{
	if ( player_set_position_ratio(g_player, percent) != PLAYER_ERROR_NONE )
	{
		g_print("failed to set position ratio\n");
	}
}

static void get_duration()
{
	int duration = 0;
	int ret;
	ret = player_get_duration(g_player, &duration);
	g_print("                                                            ==> [Player_Test] player_get_duration() return : %d\n",ret);
	g_print("                                                            ==> [Player_Test] Duration: [%d ] msec\n",duration);
}

static void get_video_size()
{
	int w = 0;
	int h = 0;
	player_get_video_size(g_player, &w, &h);
	g_print("                                                            ==> [Player_Test] Width: [%d ] , Height: [%d ] \n",w,h);
}

static void set_looping(bool looping)
{
	if ( player_set_looping(g_player, looping) != PLAYER_ERROR_NONE )
	{
		g_print("failed to set_looping\n");
	}
}

static void get_looping(bool *looping)
{
	player_is_looping(g_player, looping);
	g_print("                                                            ==> [Player_Test] looping = %d\n", *looping);
}

static void set_display_mode(int mode)
{
	if ( player_set_x11_display_mode(g_player, mode) != PLAYER_ERROR_NONE )
	{
		g_print("failed to player_set_x11_display_mode\n");
	}
}

static void get_display_mode()
{
	player_display_mode_e mode;
	player_get_x11_display_mode(g_player, &mode);
	g_print("                                                            ==> [Player_Test] X11 Display mode: [%d ] \n",mode);
}

static void set_display_rotation(int rotation)
{
	if ( player_set_x11_display_rotation(g_player, rotation) != PLAYER_ERROR_NONE )
	{
		g_print("failed to set_display_rotation\n");
	}
}

static void get_display_rotation()
{
	player_display_rotation_e rotation = 0;
	player_get_x11_display_rotation(g_player, &rotation);
	g_print("                                                            ==> [Player_Test] X11 Display rotation: [%d ] \n",rotation);
}


static void set_display_visible(bool visible)
{
	if ( player_set_x11_display_visible(g_player, visible) != PLAYER_ERROR_NONE )
	{
		g_print("failed to player_set_x11_display_visible\n");
	}
}

static void get_display_visible(bool *visible)
{
	player_is_x11_display_visible(g_player, visible);
	g_print("                                                            ==> [Player_Test] X11 Display Visible = %d\n", *visible);
}

static void input_subtitle_filename(char *subtitle_filename)
{
	int len = strlen(subtitle_filename);

	if ( len < 1 || len > MAX_STRING_LEN )
		return;

	strncpy (g_subtitle_uri, subtitle_filename,len);
	g_print("subtitle uri is set to %s\n", g_subtitle_uri);
}

static void capture_video()
{
	if( player_capture_video(g_player,video_captured_cb,NULL)!=PLAYER_ERROR_NONE)
	{
		g_print("failed to player_capture_video\n");
	}
}

void quit_program()
{
	player_unprepare(g_player);
	player_destroy(g_player);
	g_player = 0;
	elm_exit();
}

void play_with_ini(char *file_path)
{
	input_filename(file_path);
	_player_play();
}

void _interpret_main_menu(char *cmd)
{
	if ( strlen(cmd) == 1 )
	{
		if (strncmp(cmd, "a", 1) == 0)
		{
			g_menu_state = CURRENT_STATUS_FILENAME;
		}
		else if (strncmp(cmd, "1", 1) == 0)
		{
			play_with_ini(g_file_list[0]);
		}
		else if (strncmp(cmd, "2", 1) == 0)
		{
			play_with_ini(g_file_list[1]);
		}
		else if (strncmp(cmd, "3", 1) == 0)
		{
			play_with_ini(g_file_list[2]);
		}
		else if (strncmp(cmd, "4", 1) == 0)
		{
			play_with_ini(g_file_list[3]);
		}
		else if (strncmp(cmd, "5", 1) == 0)
		{
			play_with_ini(g_file_list[4]);
		}
		else if (strncmp(cmd, "6", 1) == 0)
		{
			play_with_ini(g_file_list[5]);
		}
		else if (strncmp(cmd, "7", 1) == 0)
		{
			play_with_ini(g_file_list[6]);
		}
		else if (strncmp(cmd, "8", 1) == 0)
		{
			play_with_ini(g_file_list[7]);
		}
		else if (strncmp(cmd, "9", 1) == 0)
		{
			play_with_ini(g_file_list[8]);
		}
		else if (strncmp(cmd, "b", 1) == 0)
		{
			_player_play();
		}
		else if (strncmp(cmd, "c", 1) == 0)
		{
			_player_stop();
		}
		else if (strncmp(cmd, "d", 1) == 0)
		{
			_player_resume();
		}
		else if (strncmp(cmd, "e", 1) == 0)
		{
			_player_pause();
		}
		else if (strncmp(cmd, "S", 1) == 0)
		{
			_player_state();
		}
		else if (strncmp(cmd, "f", 1) == 0)
		{
				g_menu_state = CURRENT_STATUS_VOLUME;
		}
		else if (strncmp(cmd, "g", 1) == 0)
		{
			float left;
			float right;
			get_volume(&left, &right);
		}
		else if (strncmp(cmd, "h", 1) == 0 )
		{
				g_menu_state = CURRENT_STATUS_MUTE;
		}
		else if (strncmp(cmd, "i", 1) == 0 )
		{
			bool mute;
			get_mute(&mute);
		}
		else if (strncmp(cmd, "j", 1) == 0 )
		{
				g_menu_state = CURRENT_STATUS_POSITION_TIME;
		}
		else if (strncmp(cmd, "k", 1) == 0 )
		{
				g_menu_state = CURRENT_STATUS_POSITION_PERCENT;
		}
		else if (strncmp(cmd, "l", 1) == 0 )
		{
				get_position();
		}
		else if (strncmp(cmd, "m", 1) == 0 )
		{
				get_duration();
		}
		else if (strncmp(cmd, "n", 1) == 0 )
		{
				get_video_size();
		}
		else if (strncmp(cmd, "o", 1) == 0 )
		{
			g_menu_state = CURRENT_STATUS_LOOPING;
		}
		else if (strncmp(cmd, "p", 1) == 0 )
		{
				bool looping;
				get_looping(&looping);
		}
		else if (strncmp(cmd, "r", 1) == 0 )
		{
			g_menu_state = CURRENT_STATUS_DISPLAY_MODE;
		}
		else if (strncmp(cmd, "s", 1) == 0 )
		{
			get_display_mode();
		}
		else if (strncmp(cmd, "t", 1) == 0 )
		{
			g_menu_state = CURRENT_STATUS_DISPLAY_ROTATION;
		}
		else if (strncmp(cmd, "u", 1) == 0 )
		{
			get_display_rotation();
		}
		else if (strncmp(cmd, "v", 1) == 0 )
		{
			g_menu_state = CURRENT_STATUS_DISPLAY_VISIBLE;
		}
		else if (strncmp(cmd, "w", 1) == 0 )
		{
			bool visible;
			get_display_visible(&visible);
		}
		else if (strncmp(cmd, "A", 1) == 0 )
		{
			g_menu_state=CURRENT_STATUS_SUBTITLE_FILENAME;
		}
		else if (strncmp(cmd, "C", 1) == 0 )
		{
			capture_video();
		}
		else if (strncmp(cmd, "q", 1) == 0)
		{
				quit_pushing = TRUE;
				quit_program();
		}
		else
		{
				g_print("unknown menu \n");
		}
	}
	else
	{
		g_print("unknown menu \n");
	}
}

void display_sub_basic()
{
	int idx;
	g_print("\n");
	g_print("=========================================================================================\n");
	g_print("                          Player Test (press q to quit) \n");
	g_print("-----------------------------------------------------------------------------------------\n");
	g_print("*. Sample List in [%s]\n", MMTS_SAMPLELIST_INI_DEFAULT_PATH);

	for( idx = 1; idx <= INI_SAMPLE_LIST_MAX ; idx++ )
	{
		if (strlen (g_file_list[idx-1]) > 0)
			g_print("%d. Play [%s]\n", idx, g_file_list[idx-1]);
	}
	g_print("-----------------------------------------------------------------------------------------\n");
	g_print("[playback] a. Initialize Media\t");
	g_print("b. Play  \t");
	g_print("c. Stop  \t");
	g_print("d. Resume\t");
	g_print("e. Pause \t");
	g_print("S. Player State \n");
	g_print("[ volume ] f. Set Volume\t");
	g_print("g. Get Volume\n");
	g_print("[ mute ] h. Set Mute\t");
	g_print("i. Get Mute\n");
	g_print("[position] j. Set Position (T)\t");
	g_print("k. Set Position (%%)\t");
	g_print("l. Get Position\n");
	g_print("[duration] m. Get Duration\n");
	g_print("[video size] n. Get Video Size\n");
	g_print("[Looping] o. Set Looping\t");
	g_print("p. Get Looping\n");
	g_print("[x display] r. Set display mode\t");
	g_print("s. Get display mode\t");
	g_print("t. Set display Rotation\t");
	g_print("u. Get display Rotation\n");
	g_print("[x display] v. Set display visible\t");
	g_print("w. Get display visible\n");
	g_print("[subtitle] A. Set subtitle path\n");
	g_print("[Video Capture] C. Capture \n");
	g_print("\n");
	g_print("=========================================================================================\n");
}

static void displaymenu()
{
	if (g_menu_state == CURRENT_STATUS_MAINMENU)
	{
		display_sub_basic();
	}
	else if (g_menu_state == CURRENT_STATUS_FILENAME)
	{
		g_print("*** input mediapath.\n");
	}
	else if (g_menu_state == CURRENT_STATUS_VOLUME)
	{
		g_print("*** input volume value.(0~1.0)\n");
	}
	else if (g_menu_state == CURRENT_STATUS_MUTE)
	{
		g_print("*** input mute value.(0: Not Mute, 1: Mute) \n");
	}
	else if (g_menu_state == CURRENT_STATUS_POSITION_TIME)
	{
		g_print("*** input position value(msec)\n");
	}
	else if (g_menu_state == CURRENT_STATUS_POSITION_PERCENT)
	{
		g_print("*** input position percent(%%)\n");
	}
	else if (g_menu_state == CURRENT_STATUS_LOOPING)
	{
		g_print("*** input looping value.(0: Not Looping, 1: Looping) \n");
	}
	else if (g_menu_state == CURRENT_STATUS_DISPLAY_MODE)
	{
		g_print("*** input display mode value.(0: LETTER BOX, 1: ORIGIN SIZE, 2: FULL_SCREEN, 3: CROPPED_FULL) \n");
	}
	else if (g_menu_state == CURRENT_STATUS_DISPLAY_ROTATION)
	{
		g_print("*** input display rotation value.(0: NONE, 1: 90, 2: 180, 3: 270, 4:F LIP_HORZ, 5: FLIP_VERT ) \n");
	}
	else if (g_menu_state == CURRENT_STATUS_DISPLAY_VISIBLE)
	{
		g_print("*** input display visible value.(0: HIDE, 1: SHOW) \n");
	}
	else if (g_menu_state == CURRENT_STATUS_SUBTITLE_FILENAME)
	{
		g_print(" *** input  subtitle file path.\n");
	}
	else
	{
		g_print("*** unknown status.\n");
		quit_program();
	}
	g_print(" >>> ");
}

gboolean timeout_menu_display(void* data)
{
	displaymenu();
	return FALSE;
}

gboolean timeout_quit_program(void* data)
{
	quit_program();
	return FALSE;
}

void reset_menu_state(void)
{
	g_menu_state = CURRENT_STATUS_MAINMENU;
}

static void interpret (char *cmd)
{
	switch (g_menu_state)
	{
		case CURRENT_STATUS_MAINMENU:
		{
			_interpret_main_menu(cmd);
		}
		break;
		case CURRENT_STATUS_FILENAME:
		{
			input_filename(cmd);
			reset_menu_state();
		}
		break;
		case CURRENT_STATUS_VOLUME:
		{
			float level = atof(cmd);
			set_volume(level);
			reset_menu_state();
		}
		break;
		case CURRENT_STATUS_MUTE:
		{
			int mute = atoi(cmd);
			set_mute(mute);
			reset_menu_state();
		}
		break;
		case CURRENT_STATUS_POSITION_TIME:
		{
			long position = atol(cmd);
			set_position(position);
			reset_menu_state();
		}
		break;
		case CURRENT_STATUS_POSITION_PERCENT:
		{
			long percent = atol(cmd);
			set_position_ratio(percent);
			reset_menu_state();
		}
		break;
		case CURRENT_STATUS_LOOPING:
		{
			int looping = atoi(cmd);
			set_looping(looping);
			reset_menu_state();
		}
		break;
		case CURRENT_STATUS_DISPLAY_MODE:
		{
			int mode = atoi(cmd);
			set_display_mode(mode);
			reset_menu_state();
		}
		break;
		case CURRENT_STATUS_DISPLAY_ROTATION:
		{
			int rotation = atoi(cmd);
			set_display_rotation(rotation);
			reset_menu_state();
		}
		break;
		case CURRENT_STATUS_DISPLAY_VISIBLE:
		{
			int visible = atoi(cmd);
			set_display_visible(visible);
			reset_menu_state();
		}
		case CURRENT_STATUS_SUBTITLE_FILENAME:
		{
			input_subtitle_filename(cmd);
			reset_menu_state();
		}
		break;
	}
	g_timeout_add(100, timeout_menu_display, 0);
}

gboolean input (GIOChannel *channel)
{
    char buf[MAX_STRING_LEN + 3];
    gsize read;

    g_io_channel_read(channel, buf, MAX_STRING_LEN, &read);
    buf[read] = '\0';
	g_strstrip(buf);
    interpret (buf);
    return TRUE;
}

int main(int argc, char *argv[])
{
	GIOChannel *stdin_channel;
	stdin_channel = g_io_channel_unix_new(0);
	g_io_add_watch(stdin_channel, G_IO_IN, (GIOFunc)input, NULL);
	displaymenu();
	memset(&ad, 0x0, sizeof(struct appdata));
	ops.data = &ad;
	
	return appcore_efl_main(PACKAGE, &argc, &argv, &ops);
}

