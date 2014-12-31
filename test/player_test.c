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
//#define _USE_X_DIRECT_
#ifdef _USE_X_DIRECT_
#include <X11/Xlib.h>
#endif
#define PACKAGE "player_test"
#define MAX_STRING_LEN	2048
#define MMTS_SAMPLELIST_INI_DEFAULT_PATH "/opt/etc/mmts_filelist.ini"
#define PLAYER_TEST_DUMP_PATH_PREFIX   "/opt/usr/media/dump_pcm_"
#define INI_SAMPLE_LIST_MAX 9
#define DEFAULT_HTTP_TIMEOUT -1

char g_uri[MAX_STRING_LEN];
char g_subtitle_uri[MAX_STRING_LEN];
FILE *g_pcm_fd;
enum
{
	CURRENT_STATUS_MAINMENU,
	CURRENT_STATUS_HANDLE_NUM,
	CURRENT_STATUS_FILENAME,
	CURRENT_STATUS_VOLUME,
	CURRENT_STATUS_SOUND_TYPE,
	CURRENT_STATUS_MUTE,
	CURRENT_STATUS_POSITION_TIME,
	CURRENT_STATUS_LOOPING,
	CURRENT_STATUS_DISPLAY_SURFACE_CHANGE,
	CURRENT_STATUS_DISPLAY_MODE,
	CURRENT_STATUS_DISPLAY_ROTATION,
	CURRENT_STATUS_DISPLAY_VISIBLE,
	CURRENT_STATUS_DISPLAY_ROI_MODE,
	CURRENT_STATUS_DISPLAY_DST_ROI,
	CURRENT_STATUS_DISPLAY_SRC_CROP,
	CURRENT_STATUS_SUBTITLE_FILENAME,
};

#define MAX_HANDLE 20

/* for video display */
#ifdef _USE_X_DIRECT_
Window g_xid;
Display *g_dpy;
GC g_gc;
#else
Evas_Object* g_xid;
#endif
Evas_Object* g_eo_win;
Evas_Object* g_eo[MAX_HANDLE] = {0};
int g_current_surface_type = PLAYER_DISPLAY_TYPE_OVERLAY;

struct appdata
{

	Evas_Object *win;
	Evas_Object *bg;
	Evas_Object *rect;

	Evas_Object *layout_main; /* layout widget based on EDJ */
	Ecore_X_Window xid;
	/* add more variables here */
};

static Evas_Object *create_bg(Evas_Object *pParent)
{
	if(!pParent) {
		return NULL;
	}

	Evas_Object *pObj = NULL;

	pObj = elm_bg_add(pParent);
	evas_object_size_hint_weight_set(pObj, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_win_resize_object_add(pParent, pObj);
	evas_object_color_set(pObj, 0, 0, 0, 0);
	evas_object_show(pObj);
	return pObj;
}

static void win_del(void *data, Evas_Object *obj, void *event)
{
		elm_exit();
}

static Evas_Object* create_win(const char *name)
{
	Evas_Object *eo = NULL;

		printf ("[%s][%d] name=%s\n", __func__, __LINE__, name);

		eo = elm_win_add(NULL, name, ELM_WIN_BASIC);
		if (eo) {
				elm_win_title_set(eo, name);
				elm_win_borderless_set(eo, EINA_TRUE);
				evas_object_smart_callback_add(eo, "delete,request",win_del, NULL);
				elm_win_autodel_set(eo, EINA_TRUE);
		}
		return eo;
}

static Evas_Object *create_image_object(Evas_Object *eo_parent)
{
	if(!eo_parent) {
		return NULL;
	}
	Evas *evas = evas_object_evas_get(eo_parent);
	Evas_Object *eo = NULL;

	eo = evas_object_image_add(evas);

	return eo;
}

static Evas_Object *create_render_rect(Evas_Object *pParent)
{
	if(!pParent) {
		return NULL;
	}

	Evas *pEvas = evas_object_evas_get(pParent);
	Evas_Object *pObj = evas_object_rectangle_add(pEvas);
	if(pObj == NULL) {
		return NULL;
	}

	evas_object_size_hint_weight_set(pObj, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_color_set(pObj, 0, 0, 0, 0);
	evas_object_render_op_set(pObj, EVAS_RENDER_COPY);
	evas_object_show(pObj);
	elm_win_resize_object_add(pParent, pObj);

	return pObj;
}

static int app_create(void *data)
{
	struct appdata *ad = data;
	Evas_Object *win = NULL;

	/* use gl backend */
	elm_config_preferred_engine_set("opengl_x11");

	/* create window */
	win = create_win(PACKAGE);
	if (win == NULL)
			return -1;
	ad->win = win;
	g_eo_win = win;
	ad->bg = create_bg(ad->win);
	ad->rect = create_render_rect(ad->win);
	g_xid = g_eo_win;
	/* Create evas image object for EVAS surface */
	g_eo[0] = create_image_object(ad->win);
	evas_object_image_size_set(g_eo[0], 500, 500);
	evas_object_image_fill_set(g_eo[0], 0, 0, 500, 500);
	evas_object_resize(g_eo[0], 500, 500);

	elm_win_activate(win);
	evas_object_show(win);

	return 0;
}

static int app_terminate(void *data)
{
	struct appdata *ad = data;
	int i = 0;

	for (i = 0 ; i < MAX_HANDLE; i++)
	{
		if (g_eo[i])
		{
			evas_object_del(g_eo[i]);
			g_eo[i] = NULL;
		}
	}
	if (g_eo_win) {
		evas_object_del(g_eo_win);
		g_eo_win = NULL;
	}
	ad->win = NULL;
#ifdef _USE_X_DIRECT_
	if(g_dpy)
	{
		if(g_gc)
			XFreeGC (g_dpy, g_gc);
		if(g_xid)
			XDestroyWindow (g_dpy, g_xid);
		XCloseDisplay (g_dpy);
		g_xid = 0;
		g_dpy = NULL;
	}
#endif
	return 0;
}

struct appcore_ops ops = {
	.create = app_create,
	.terminate = app_terminate,
};

struct appdata ad;
static player_h g_player[MAX_HANDLE] = {0};
int g_handle_num = 1;
int g_menu_state = CURRENT_STATUS_MAINMENU;
char g_file_list[9][256];
gboolean quit_pushing;

static void prepared_cb(void *user_data)
{
	g_print("[Player_Test] prepared_cb!!!!\n");
}

static void progress_down_cb(player_pd_message_type_e type, void *user_data)
{
	g_print("[Player_Test] progress_down_cb!!!! type : %d\n", type);
}

static void buffering_cb(int percent, void *user_data)
{
	g_print("[Player_Test] buffering_cb!!!! percent : %d\n", percent);
}

static void seek_completed_cb(void *user_data)
{
	g_print("[Player_Test] seek_completed_cb!!! \n");
}

static void completed_cb(void *user_data)
{
	g_print("[Player_Test] completed_cb!!!!\n");
}

static void error_cb(int code, void *user_data)
{
	g_print("[Player_Test] error_cb!!!! code : %d\n", code);
}

static void interrupted_cb(player_interrupted_code_e code, void *user_data)
{
	g_print("[Player_Test] interrupted_cb!!!! code : %d\n", code);
}

#if 0
static void audio_frame_decoded_cb(unsigned char *data, unsigned int size, void *user_data)
{
	int pos=0;

	if (data && g_pcm_fd)
	{
		fwrite(data, 1, size, g_pcm_fd);
	}
	player_get_play_position(g_player[0], &pos);
	g_print("[Player_Test] audio_frame_decoded_cb [size: %d] --- current pos : %d!!!!\n", size, pos);
}
#endif

static void subtitle_updated_cb(unsigned long duration, char *text, void *user_data)
{
	g_print("[Player_Test] subtitle_updated_cb!!!! [%ld] %s\n",duration, text);
}

static void video_captured_cb(unsigned char *data, int width, int height,unsigned int size, void *user_data)
{
	g_print("[Player_Test] video_captured_cb!!!! width: %d, height : %d, size : %d \n",width, height,size);
}

int	_save(unsigned char * src, int length)
{	//unlink(CAPTUERD_IMAGE_SAVE_PATH);
	FILE* fp;
	char filename[256] = {0,};
	static int WRITE_COUNT = 0;

	//gchar *filename  = CAPTUERD_IMAGE_SAVE_PATH;
	sprintf (filename, "ALBUM_ART_IMAGE_%d", WRITE_COUNT);
	WRITE_COUNT++;
	fp=fopen(filename, "w+");
	if(fp==NULL)
	{
		g_print("file open error!!\n");
		return FALSE;
	}
	else
	{
		g_print("open success\n");
		if(fwrite(src, 1, length, fp )!=1)
		{
			g_print("file write error!!\n");
			fclose(fp);
			return FALSE;
		}
		g_print("write success(%s)\n", filename);
		fclose(fp);
	}

	return TRUE;
}

static void reset_display()
{
	int i = 0;

	/* delete evas window, if it is */
	for (i = 0 ; i < MAX_HANDLE; i++)
	{
		if (g_eo[i])
		{
			evas_object_del(g_eo[i]);
			g_eo[i] = NULL;
		}
	}

#ifdef _USE_X_DIRECT_
	/* delete x window, if it is */
	if(g_dpy)
	{
		if(g_gc)
			XFreeGC (g_dpy, g_gc);
		if(g_xid)
			XDestroyWindow (g_dpy, g_xid);
		XCloseDisplay (g_dpy);
		g_xid = 0;
		g_dpy = NULL;
	}
#endif
}

static void input_filename(char *filename)
{
	int len = strlen(filename);
	int i = 0;

	if ( len < 0 || len > MAX_STRING_LEN )
		return;

	for (i = 0; i < g_handle_num; i++)
	{
		if(g_player[i]!=NULL)
		{
			player_unprepare(g_player[i]);
			player_destroy(g_player[i]);
		}
		g_player[i] = 0;

		if ( player_create(&g_player[i]) != PLAYER_ERROR_NONE )
		{
			g_print("player create is failed\n");
		}
	}

	strncpy (g_uri, filename,len);

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

	mm_player_set_attribute(g_player[0],
								&g_err_name,
								"profile_uri", uri, strlen(uri),
								"profile_user_param", g_media_mem, file_size
								NULL);
#else
	//player_set_uri(g_player[0], filename);
#endif /* APPSRC_TEST */

	int ret;
	player_state_e state;
	for (i = 0; i < g_handle_num; i++)
	{
		ret = player_get_state(g_player[i], &state);
		g_print("player_get_state returned [%d]", ret);
		g_print("1. After player_create() - Current State : %d \n", state);
	}
}
#if 0
// use this API instead of player_set_uri
static void player_set_memory_buffer_test()
{
	if(!g_uri)
		g_print("g_uri is NULL\n");
	GMappedFile *file;
    gsize file_size;
    guint8* g_media_mem = NULL;

	file = g_mapped_file_new (g_uri, FALSE, NULL);
    file_size = g_mapped_file_get_length (file);
    g_media_mem = (guint8 *) g_mapped_file_get_contents (file);

    int ret = player_set_memory_buffer(g_player[0], (void*)g_media_mem, file_size);
    g_print("player_set_memory_buffer ret : %d\n", ret);
}
#endif
static void _player_prepare(bool async)
{
	int ret = FALSE;
	int slen = strlen(g_subtitle_uri);

	if ( slen > 0 && slen < MAX_STRING_LEN )
	{
		g_print("0. set subtile path() (size : %d) - %s  \n", slen, g_subtitle_uri);
		player_set_subtitle_path(g_player[0],g_subtitle_uri);
		player_set_subtitle_updated_cb(g_player[0], subtitle_updated_cb, (void*)g_player[0]);
	}
	if (g_current_surface_type == PLAYER_DISPLAY_TYPE_OVERLAY)
	{
		player_set_display(g_player[0], g_current_surface_type, GET_DISPLAY(g_xid));
		player_set_buffering_cb(g_player[0], buffering_cb, (void*)g_player[0]);
		player_set_completed_cb(g_player[0], completed_cb, (void*)g_player[0]);
		player_set_interrupted_cb(g_player[0], interrupted_cb, (void*)g_player[0]);
		player_set_error_cb(g_player[0], error_cb, (void*)g_player[0]);
		player_set_uri(g_player[0], g_uri);
		//player_set_memory_buffer_test();
	}
	else
	{
		int i = 0;
		for (i = 0; i < g_handle_num; i++)
		{
			player_set_display(g_player[i], g_current_surface_type, g_eo[i]);
			player_set_buffering_cb(g_player[i], buffering_cb, (void*)g_player[i]);
			player_set_completed_cb(g_player[i], completed_cb, (void*)g_player[i]);
			player_set_interrupted_cb(g_player[i], interrupted_cb, (void*)g_player[i]);
			player_set_error_cb(g_player[i], error_cb, (void*)g_player[i]);
			player_set_uri(g_player[i], g_uri);
			//player_set_memory_buffer_test();
		}
	}

	if (g_current_surface_type == PLAYER_DISPLAY_TYPE_OVERLAY)
	{
		if ( async )
		{
			ret = player_prepare_async(g_player[0], prepared_cb, (void*) g_player[0]);
		}
		else
			ret = player_prepare(g_player[0]);
	}
	else
	{
		int i = 0;
		for (i = 0; i < g_handle_num; i++)
		{
			if ( async )
			{
				ret = player_prepare_async(g_player[i], prepared_cb, (void*) g_player[i]);
			}
			else
				ret = player_prepare(g_player[i]);
		}
	}

	if ( ret != PLAYER_ERROR_NONE )
	{
		g_print("prepare is failed (errno = %d) \n", ret);
	}
	player_state_e state;
	if (g_current_surface_type == PLAYER_DISPLAY_TYPE_OVERLAY)
	{
		ret = player_get_state(g_player[0], &state);
		g_print("After player_prepare() - Current State : %d \n", state);
	}
	else
	{
		int i = 0;
		for (i = 0; i < g_handle_num; i++)
		{
			ret = player_get_state(g_player[i], &state);
			g_print("After player_prepare() - Current State : %d \n", state);
		}
	}

}

static void _player_unprepare()
{
	int ret = FALSE;
	int i = 0;
	if (g_current_surface_type == PLAYER_DISPLAY_TYPE_OVERLAY)
	{
		ret = player_unprepare(g_player[0]);
		if ( ret != PLAYER_ERROR_NONE )
		{
			g_print("unprepare is failed (errno = %d) \n", ret);
		}
		ret = player_unset_subtitle_updated_cb(g_player[0]);
		g_print("player_unset_subtitle_updated_cb ret %d\n", ret);

		ret = player_unset_buffering_cb(g_player[0]);
		g_print("player_unset_buffering_cb ret %d\n", ret);

		ret = player_unset_completed_cb(g_player[0]);
		g_print("player_unset_completed_cb ret %d\n", ret);

		ret = player_unset_interrupted_cb(g_player[0]);
		g_print("player_unset_interrupted_cb ret %d\n", ret);

		ret = player_unset_error_cb(g_player[0]);
		g_print("player_unset_error_cb ret %d\n", ret);

	}
	else
	{
		for (i = 0; i < g_handle_num ; i++)
		{
			if(g_player[i]!=NULL)
			{
				ret = player_unprepare(g_player[i]);
				if ( ret != PLAYER_ERROR_NONE )
				{
					g_print("unprepare is failed (errno = %d) \n", ret);
				}
				ret = player_unset_subtitle_updated_cb(g_player[i]);
				g_print("player_unset_subtitle_updated_cb [%d] ret %d\n", i, ret);

				ret = player_unset_buffering_cb(g_player[i]);
				g_print("player_unset_buffering_cb [%d] ret %d\n", i, ret);

				ret = player_unset_completed_cb(g_player[i]);
				g_print("player_unset_completed_cb [%d] ret %d\n", i, ret);

				ret = player_unset_interrupted_cb(g_player[i]);
				g_print("player_unset_interrupted_cb [%d] ret %d\n", i, ret);

				ret = player_unset_error_cb(g_player[i]);
				g_print("player_unset_error_cb [%d] ret %d\n", i, ret);
			}
		}
	}
	reset_display();
	memset(g_subtitle_uri, 0 , sizeof(g_subtitle_uri));
	player_state_e state;
	if (g_current_surface_type == PLAYER_DISPLAY_TYPE_OVERLAY)
	{
		ret = player_get_state(g_player[0], &state);
		g_print(" After player_unprepare() - Current State : %d \n", state);
	}
	else
	{
		for (i = 0; i < g_handle_num ; i++)
		{
			ret = player_get_state(g_player[i], &state);
			g_print(" After player_unprepare() - Current State : %d \n", state);
		}
	}
}

static void _player_destroy()
{
	int i = 0;
	if (g_current_surface_type == PLAYER_DISPLAY_TYPE_OVERLAY)
	{
		player_unprepare(g_player[0]);
		for (i = 0; i < g_handle_num ; i++)
		{
			player_destroy(g_player[i]);
			g_player[i] = 0;
		}
	}
	else
	{
		for (i = 0; i < g_handle_num ; i++)
		{
			if(g_player[i]!=NULL)
			{
				player_unprepare(g_player[i]);
				player_destroy(g_player[i]);
				g_player[i] = 0;
			}
		}
	}
}

static void _player_play()
{
	int bRet = FALSE;
	int i = 0;
	if (g_current_surface_type == PLAYER_DISPLAY_TYPE_OVERLAY)
	{
		bRet = player_start(g_player[0]);
		g_print("player_start returned [%d]", bRet);
	}
	else
	{
		for (i = 0; i < g_handle_num ; i++)
		{
			bRet = player_start(g_player[i]);
			g_print("player_start returned [%d]", bRet);
		}
	}
}

static void _player_stop()
{
	int bRet = FALSE;
	int i = 0;
	if (g_current_surface_type == PLAYER_DISPLAY_TYPE_OVERLAY)
	{
		bRet = player_stop(g_player[0]);
		g_print("player_stop returned [%d]", bRet);
	}
	else
	{
		for (i = 0; i < g_handle_num ; i++)
		{
			bRet = player_stop(g_player[i]);
			g_print("player_stop returned [%d]", bRet);
		}
	}
}

static void _player_resume()
{
	int bRet = FALSE;
	int i = 0;
	if (g_current_surface_type == PLAYER_DISPLAY_TYPE_OVERLAY)
	{
		bRet = player_start(g_player[0]);
		g_print("player_start returned [%d]", bRet);
	}
	else
	{
		for (i = 0; i < g_handle_num ; i++)
		{
			bRet = player_start(g_player[i]);
			g_print("player_start returned [%d]", bRet);
		}
	}
}

static void _player_pause()
{
	int bRet = FALSE;
	int i = 0;
	if (g_current_surface_type == PLAYER_DISPLAY_TYPE_OVERLAY)
	{
		bRet = player_pause(g_player[0]);
		g_print("player_pause returned [%d]", bRet);
	}
	else
	{
		for (i = 0; i < g_handle_num ; i++)
		{
			bRet = player_pause(g_player[i]);
			g_print("player_pause returned [%d]", bRet);
		}
	}
}

static void _player_state()
{
	player_state_e state;
	player_get_state(g_player[0], &state);
	g_print("                                                            ==> [Player_Test] Current Player State : %d\n", state);
}

static void _player_set_progressive_download()
{
	player_set_progressive_download_path(g_player[0], "/opt/test.pd");
	player_set_progressive_download_message_cb(g_player[0], progress_down_cb, (void*)g_player[0]);
}

static void set_volume(float volume)
{
	if ( player_set_volume(g_player[0], volume, volume) != PLAYER_ERROR_NONE )
	{
		g_print("failed to set volume\n");
	}
}

static void get_volume(float* left, float* right)
{
	player_get_volume(g_player[0], left, right);
	g_print("                                                            ==> [Player_Test] volume - left : %f, right : %f\n", *left, *right);
}

static void set_mute(bool mute)
{
	if ( player_set_mute(g_player[0], mute) != PLAYER_ERROR_NONE )
	{
		g_print("failed to set_mute\n");
	}
}

static void get_mute(bool *mute)
{
	player_is_muted(g_player[0], mute);
	g_print("                                                            ==> [Player_Test] mute = %d\n", *mute);
}

static void set_sound_type(sound_type_e type)
{
	if ( player_set_sound_type(g_player[0], type) != PLAYER_ERROR_NONE )
	{
		g_print("failed to set sound type(%d)\n", type);
	}
	else
		g_print("set sound type(%d) success", type);
}

static void get_position()
{
	int position = 0;
	int ret;
	ret = player_get_play_position(g_player[0], &position);
	g_print("                                                            ==> [Player_Test] player_get_play_position()%d return : %d\n", ret, position);
}

static void set_position(int position)
{
	if ( player_set_play_position(g_player[0],  position, TRUE, seek_completed_cb, g_player[0]) != PLAYER_ERROR_NONE )
	{
		g_print("failed to set position\n");
	}
}

static void get_duration()
{
	int duration = 0;
	int ret;
	ret = player_get_duration(g_player[0], &duration);
	g_print("                                                            ==> [Player_Test] player_get_duration() return : %d\n",ret);
	g_print("                                                            ==> [Player_Test] Duration: [%d ] msec\n",duration);
}

static void get_stream_info()
{
	int w = 0;
	int h = 0;

	char *value = NULL;
	player_get_content_info(g_player[0], PLAYER_CONTENT_INFO_ALBUM,  &value);
	g_print("                                                            ==> [Player_Test] PLAYER_CONTENT_INFO_ALBUM: [%s ] \n",value);
	player_get_content_info(g_player[0], PLAYER_CONTENT_INFO_ARTIST,  &value);
	g_print("                                                            ==> [Player_Test] PLAYER_CONTENT_INFO_ARTIST: [%s ] \n",value);
	player_get_content_info(g_player[0], PLAYER_CONTENT_INFO_AUTHOR,  &value);
	g_print("                                                            ==> [Player_Test] PLAYER_CONTENT_INFO_AUTHOR: [%s ] \n",value);
	player_get_content_info(g_player[0], PLAYER_CONTENT_INFO_GENRE,  &value);
	g_print("                                                            ==> [Player_Test] PLAYER_CONTENT_INFO_GENRE: [%s ] \n",value);
	player_get_content_info(g_player[0], PLAYER_CONTENT_INFO_TITLE,  &value);
	g_print("                                                            ==> [Player_Test] PLAYER_CONTENT_INFO_TITLE: [%s ] \n",value);
	void *album;
	int size;
	player_get_album_art(g_player[0], &album, &size);
	g_print("                                                            ==> [Player_Test] Album art : [ data : %p, size : %d ]\n", (unsigned int *)album, size);
	if(value!=NULL)
	{
		free(value);
		value = NULL;
	}

	int sample_rate;
	int channel;
	int bit_rate;
	int fps, v_bit_rate;
	player_get_audio_stream_info(g_player[0], &sample_rate, &channel, &bit_rate);
	g_print("                                                            ==> [Player_Test] Sample Rate: [%d ] , Channel: [%d ] , Bit Rate: [%d ] \n",sample_rate,channel,bit_rate);

	player_get_video_stream_info(g_player[0], &fps, &v_bit_rate);
	g_print("                                                            ==> [Player_Test] fps: [%d ] , Bit Rate: [%d ] \n",fps,v_bit_rate);

	char *audio_codec = NULL;
	char *video_codec = NULL;
	player_get_codec_info(g_player[0], &audio_codec, &video_codec);
	if(audio_codec!=NULL)
	{
		g_print("                                                            ==> [Player_Test] Audio Codec: [%s ] \n",audio_codec);
		free(audio_codec);
		audio_codec = NULL;
	}
	if(video_codec!=NULL)
	{
		g_print("                                                            ==> [Player_Test] Video Codec: [%s ] \n",video_codec);
		free(video_codec);
		video_codec = NULL;
	}
	player_get_video_size(g_player[0], &w, &h);
	g_print("                                                            ==> [Player_Test] Width: [%d ] , Height: [%d ] \n",w,h);
 }

static void set_looping(bool looping)
{
	if (g_current_surface_type == PLAYER_DISPLAY_TYPE_OVERLAY)
	{
		if ( player_set_looping(g_player[0], looping) != PLAYER_ERROR_NONE )
		{
			g_print("failed to set_looping\n");
		}
	}
	else
	{
		int i = 0;
		for (i = 0; i < g_handle_num; i++)
		{
			if ( player_set_looping(g_player[i], looping) != PLAYER_ERROR_NONE )
			{
				g_print("failed to set_looping\n");
			}
		}
	}
}

static void get_looping(bool *looping)
{
	player_is_looping(g_player[0], looping);
	g_print("                                                            ==> [Player_Test] looping = %d\n", *looping);
}

static void change_surface(int option)
{
	player_display_type_e surface_type = 0;
	int ret = PLAYER_ERROR_NONE;

	switch (option)
	{
	case 0: /* X surface */
		surface_type = PLAYER_DISPLAY_TYPE_OVERLAY;
		g_print("change surface type to X\n");
		break;
	case 1: /* EVAS surface */
		surface_type = PLAYER_DISPLAY_TYPE_EVAS;
		g_print("change surface type to EVAS\n");
		break;
	default:
		g_print("invalid surface type\n");
		return;
	}

	if (surface_type == g_current_surface_type)
	{
		g_print("same with the previous surface type(%d)\n", g_current_surface_type);
		return;
	}
	else
	{
		player_state_e player_state = PLAYER_STATE_NONE;
		ret = player_get_state(g_player[0], &player_state);
		if (ret)
		{
			g_print("failed to player_get_state(), ret(0x%x)\n", ret);
		}
		/* state check */
		if (player_state == PLAYER_STATE_NONE || player_state == PLAYER_STATE_IDLE )
		{
			reset_display();

			if (surface_type == PLAYER_DISPLAY_TYPE_OVERLAY)
			{
#ifdef _USE_X_DIRECT_
				/* Create xwindow for X surface */
				if(!g_dpy)
				{
					g_dpy = XOpenDisplay (NULL);
					g_xid = create_window (g_dpy, 0, 0, 500, 500);
					g_gc = XCreateGC (g_dpy, g_xid, 0, 0);
				}
				g_print("create x window dpy(%p), gc(%x), xid(%d)\n", g_dpy, (unsigned int)g_gc, (int)g_xid);
				XImage *xim = make_transparent_image (g_dpy, 500, 500);
				XPutImage (g_dpy, g_xid, g_gc, xim, 0, 0, 0, 0, 500, 500);
				XSync (g_dpy, False);
#else
				g_xid = g_eo_win;
#endif
				ret = player_set_display(g_player[0], surface_type, GET_DISPLAY(g_xid));
			}
			else
			{
				int i = 0;
				for (i = 0; i < g_handle_num ; i++)
				{
					/* Create evas image object for EVAS surface */
					if (!g_eo[i])
					{
						g_eo[i] = create_image_object(g_eo_win);
						evas_object_image_size_set(g_eo[i], 500, 500);
						evas_object_image_fill_set(g_eo[i], 0, 0, 500, 500);
						evas_object_resize(g_eo[i], 500, 500);
						evas_object_move(g_eo[i], i*20, i*20);
					}
					ret = player_set_display(g_player[i], surface_type, g_eo[i]);
				}
			}
			if (ret)
			{
				g_print("failed to set display, surface_type(%d)\n", surface_type);
				return;
			}
			g_current_surface_type = surface_type;
		}
		else
		{
			g_print("could not change surface type, current_state(%d)\n", player_state);
		}
	}
	return;
}

static void set_display_mode(int mode)
{
	if ( player_set_display_mode(g_player[0], mode) != PLAYER_ERROR_NONE )
	{
		g_print("failed to player_set_display_mode\n");
	}
}

static void get_display_mode()
{
	player_display_mode_e mode;
	player_get_display_mode(g_player[0], &mode);
	g_print("                                                            ==> [Player_Test] Display mode: [%d ] \n",mode);
}

static void set_display_rotation(int rotation)
{
	if ( player_set_display_rotation(g_player[0], rotation) != PLAYER_ERROR_NONE )
	{
		g_print("failed to set_display_rotation\n");
	}
}

static void get_display_rotation()
{
	player_display_rotation_e rotation = 0;
	player_get_display_rotation(g_player[0], &rotation);
	g_print("                                                            ==> [Player_Test] X11 Display rotation: [%d ] \n",rotation);
}


static void set_display_visible(bool visible)
{
	if ( player_set_display_visible(g_player[0], visible) != PLAYER_ERROR_NONE )
	{
		g_print("failed to player_set_x11_display_visible\n");
	}
}

static void get_display_visible(bool *visible)
{
	player_is_display_visible(g_player[0], visible);
	g_print("                                                            ==> [Player_Test] X11 Display Visible = %d\n", *visible);
}

static void set_display_dst_roi(int x, int y, int w, int h)
{
#if 0
	if ( player_set_x11_display_dst_roi(g_player[0], x, y, w, h) != PLAYER_ERROR_NONE )
	{
		g_print("failed to player_set_x11_display_dst_roi\n");
	} else {
		g_print("                                                            ==> [Player_Test] set X11 Display DST ROI (x:%d, y:%d, w:%d, h:%d)\n", x, y, w, h);
	}
#endif
}

static void get_display_dst_roi()
{
#if 0
	int x = 0;
	int y = 0;
	int w = 0;
	int h = 0;

	if ( player_get_x11_display_dst_roi(g_player[0], &x, &y, &w, &h) != PLAYER_ERROR_NONE )
	{
		g_print("failed to player_get_x11_display_dst_roi\n");
	} else {
		g_print("                                                            ==> [Player_Test] got X11 Display DST ROI (x:%d, y:%d, w:%d, h:%d)\n", x, y, w, h);
	}
#endif
}

static void set_display_roi_mode(int mode)
{
#if 0
	if ( player_set_x11_display_roi_mode(g_player[0], (player_display_roi_mode_e)mode) != PLAYER_ERROR_NONE )
	{
		g_print("failed to player_set_x11_display_roi_mode\n");
	} else {
		g_print("                                                            ==> [Player_Test] set X11 Display ROI mode (%d)\n", mode);
	}
#endif
}

static void get_display_roi_mode()
{
#if 0
	player_display_roi_mode_e mode;
	if ( player_get_x11_display_roi_mode(g_player[0], &mode) != PLAYER_ERROR_NONE )
	{
		g_print("failed to player_get_x11_display_roi_mode\n");
	} else {
		g_print("                                                            ==> [Player_Test] got X11 Display ROI mode (%d)\n", mode);
	}
#endif
}

static void set_display_src_crop(int x, int y, int w, int h)
{
#if 0
	if ( player_set_x11_display_src_crop(g_player[0], x, y, w, h) != PLAYER_ERROR_NONE )
	{
		g_print("failed to player_set_x11_display_src_crop\n");
	} else {
		g_print("                                                            ==> [Player_Test] set X11 Display SRC CROP (x:%d, y:%d, w:%d, h:%d)\n", x, y, w, h);
	}
#endif
}

static void get_display_src_crop()
{
#if 0
	int x = 0;
	int y = 0;
	int w = 0;
	int h = 0;

	if ( player_get_x11_display_src_crop(g_player[0], &x, &y, &w, &h) != PLAYER_ERROR_NONE )
	{
		g_print("failed to player_get_x11_display_src_crop\n");
	} else {
		g_print("                                                            ==> [Player_Test] got X11 Display SRC CROP (x:%d, y:%d, w:%d, h:%d)\n", x, y, w, h);
	}
#endif
}

static void input_subtitle_filename(char *subtitle_filename)
{
	int len = strlen(subtitle_filename);

	if ( len < 1 || len > MAX_STRING_LEN )
		return;

	strncpy (g_subtitle_uri, subtitle_filename,len);
	g_print("subtitle uri is set to %s\n", g_subtitle_uri);
	player_set_subtitle_path (g_player[0], g_subtitle_uri);
}

static void capture_video()
{
	if( player_capture_video(g_player[0],video_captured_cb,NULL)!=PLAYER_ERROR_NONE)
	{
		g_print("failed to player_capture_video\n");
	}
}

static void decoding_audio()
{
#if 0
	int ret;
        char *suffix, *dump_path;
        GDateTime *time = g_date_time_new_now_local();

        suffix = g_date_time_format(time, "%Y%m%d_%H%M%S.pcm");
        dump_path = g_strjoin(NULL, PLAYER_TEST_DUMP_PATH_PREFIX, suffix, NULL);
        g_pcm_fd = fopen(dump_path, "w+");
        g_free(dump_path);
        g_free(suffix);
        g_date_time_unref(time);
        if(!g_pcm_fd) {
            g_print("Can not create debug dump file");
        }

	ret =player_set_audio_frame_decoded_cb(g_player[0], 0, 0,audio_frame_decoded_cb, (void*)g_player[0]);
	if ( ret != PLAYER_ERROR_NONE )
	{
		g_print("player_set_audio_frame_decoded_cb is failed (errno = %d) \n", ret);
	}
#endif
}

void quit_program()
{
	int i = 0;

	if(g_pcm_fd)
	{
		fclose(g_pcm_fd);
	}

	for (i = 0; i < g_handle_num; i++)
	{
		if(g_player[i]!=NULL)
		{
			player_unprepare(g_player[i]);
			player_destroy(g_player[i]);
			g_player[i] = 0;
		}
	}
	elm_exit();
}

void play_with_ini(char *file_path)
{
	input_filename(file_path);
	_player_play();
}

void _interpret_main_menu(char *cmd)
{
	int len =  strlen(cmd);
	if ( len == 1 )
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
		else if (strncmp(cmd, "z", 1) == 0)
		{
			g_menu_state = CURRENT_STATUS_SOUND_TYPE;
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
			get_stream_info();
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
		else if (strncmp(cmd, "x", 1) == 0 )
		{
			g_menu_state = CURRENT_STATUS_DISPLAY_DST_ROI;
		}
		else if (strncmp(cmd, "y", 1) == 0 )
		{
			get_display_dst_roi();
		}
		else if (strncmp(cmd, "M", 1) == 0 )
		{
			g_menu_state = CURRENT_STATUS_DISPLAY_ROI_MODE;
		}
		else if (strncmp(cmd, "N", 1) == 0 )
		{
			get_display_roi_mode();
		}
		else if (strncmp(cmd, "F", 1) == 0 )
		{
			g_menu_state = CURRENT_STATUS_DISPLAY_SRC_CROP;
		}
		else if (strncmp(cmd, "G", 1) == 0 )
		{
			get_display_src_crop();
		}
		else if (strncmp(cmd, "A", 1) == 0 )
		{
			g_menu_state = CURRENT_STATUS_SUBTITLE_FILENAME;
		}
 		else if (strncmp(cmd, "C", 1) == 0 )
		{
			capture_video();
		}
		else if (strncmp(cmd, "D", 1) == 0 )
		{
			decoding_audio();
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
	else if(len == 2)
	{
	    if (strncmp(cmd, "pr", 2) == 0)
		{
			_player_prepare(FALSE); // sync
		}
  		else if (strncmp(cmd, "pa", 2) == 0)
		{
			_player_prepare(TRUE); // async
		}
		else if (strncmp(cmd, "un", 2) == 0)
		{
			_player_unprepare();
		}
		else if (strncmp(cmd, "dt", 2) == 0)
		{
			_player_destroy();
		}
		else if (strncmp(cmd, "sp", 2) == 0)
		{
			_player_set_progressive_download();
		}
   		else if (strncmp(cmd, "ds", 2) == 0 )
		{
			g_menu_state = CURRENT_STATUS_DISPLAY_SURFACE_CHANGE;
		}
		else if (strncmp(cmd, "nb", 2) == 0 )
		{
			g_menu_state = CURRENT_STATUS_HANDLE_NUM;
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
	g_print("*. Sample List in [%s]      \t", MMTS_SAMPLELIST_INI_DEFAULT_PATH);
	g_print("nb. num. of handles \n");
	for( idx = 1; idx <= INI_SAMPLE_LIST_MAX ; idx++ )
	{
		if (strlen (g_file_list[idx-1]) > 0)
			g_print("%d. Play [%s]\n", idx, g_file_list[idx-1]);
	}
	g_print("-----------------------------------------------------------------------------------------\n");
	g_print("[playback] a. Create\t");
	g_print("pr. Prepare  \t");
	g_print("pa. Prepare async \t");
	g_print("b. Play  \t");
	g_print("c. Stop  \t");
	g_print("d. Resume\t");
	g_print("e. Pause \t");
	g_print("un. Unprepare \t");
	g_print("dt. Destroy \n");
 	g_print("[State] S. Player State \n");
	g_print("[ volume ] f. Set Volume\t");
	g_print("g. Get Volume\t");
	g_print("z. Set Sound type\t");
	g_print("[ mute ] h. Set Mute\t");
	g_print("i. Get Mute\n");
 	g_print("[position] j. Set Position \t");
	g_print("l. Get Position\n");
	g_print("[duration] m. Get Duration\n");
	g_print("[Stream Info] n. Get stream info (Video Size, codec, audio stream info, and tag info)\n");
	g_print("[Looping] o. Set Looping\t");
	g_print("p. Get Looping\n");
	g_print("[display] v. Set display visible\t");
	g_print("w. Get display visible\n");
	g_print("[display] ds. Change display surface type\n");
	g_print("[x display] r. Set display mode\t");
	g_print("s. Get display mode\n");
	g_print("[x display] t. Set display Rotation\t");
	g_print("[Track] tl. Get Track language info(single only)\n");
	g_print("[subtitle] A. Set(or change) subtitle path\n");
	g_print("[Video Capture] C. Capture \n");
	g_print("[etc] sp. Set Progressive Download\n");
	g_print("\n");
	g_print("=========================================================================================\n");
}

static void displaymenu()
{
	if (g_menu_state == CURRENT_STATUS_MAINMENU)
	{
		display_sub_basic();
	}
	else if (g_menu_state == CURRENT_STATUS_HANDLE_NUM)
	{
		g_print("*** input number of handles.(recommended only for EVAS surface)\n");
	}
	else if (g_menu_state == CURRENT_STATUS_FILENAME)
	{
		g_print("*** input mediapath.\n");
	}
	else if (g_menu_state == CURRENT_STATUS_VOLUME)
	{
		g_print("*** input volume value.(0~1.0)\n");
	}
	else if (g_menu_state == CURRENT_STATUS_SOUND_TYPE)
	{
		g_print("*** input sound type.(0:SYSTEM 1:NOTIFICATION 2:ALARM 3:RINGTONE 4:MEDIA 5:CALL 6:VOIP 7:FIXED)\n");
	}
	else if (g_menu_state == CURRENT_STATUS_MUTE)
	{
		g_print("*** input mute value.(0: Not Mute, 1: Mute) \n");
	}
 	else if (g_menu_state == CURRENT_STATUS_POSITION_TIME)
	{
		g_print("*** input position value(msec)\n");
	}
	else if (g_menu_state == CURRENT_STATUS_LOOPING)
	{
		g_print("*** input looping value.(0: Not Looping, 1: Looping) \n");
	}
	else if (g_menu_state == CURRENT_STATUS_DISPLAY_SURFACE_CHANGE) {
		g_print("*** input display surface type.(0: X surface, 1: EVAS surface) \n");
	}
	else if (g_menu_state == CURRENT_STATUS_DISPLAY_MODE)
	{
		g_print("*** input display mode value.(0: LETTER BOX, 1: ORIGIN SIZE, 2: FULL_SCREEN, 3: CROPPED_FULL, 4: ORIGIN_OR_LETTER, 5: ROI) \n");
	}
 	else if (g_menu_state == CURRENT_STATUS_DISPLAY_ROTATION)
	{
		g_print("*** input display rotation value.(0: NONE, 1: 90, 2: 180, 3: 270, 4:F LIP_HORZ, 5: FLIP_VERT ) \n");
	}
	else if (g_menu_state == CURRENT_STATUS_DISPLAY_VISIBLE)
	{
		g_print("*** input display visible value.(0: HIDE, 1: SHOW) \n");
	}
	else if (g_menu_state == CURRENT_STATUS_DISPLAY_ROI_MODE)
	{
		g_print("*** input display roi mode.(0: FULL_SCREEN, 1: LETTER BOX)\n");
	}
	else if (g_menu_state == CURRENT_STATUS_DISPLAY_DST_ROI)
	{
		g_print("*** input display roi value sequentially.(x, y, w, h)\n");
	}
	else if (g_menu_state == CURRENT_STATUS_DISPLAY_SRC_CROP)
	{
		g_print("*** input display source crop value sequentially.(x, y, w, h)\n");
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
		case CURRENT_STATUS_HANDLE_NUM:
		{
			int num_handle = atoi(cmd);
			if (0 >= num_handle || num_handle > MAX_HANDLE)
			{
				g_print("not supported this number for handles(%d)\n", num_handle);
			}
			else
			{
				g_handle_num = num_handle;
			}
			reset_menu_state();
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
		case CURRENT_STATUS_SOUND_TYPE:
		{
			int type = atoi(cmd);
			set_sound_type(type);
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
		case CURRENT_STATUS_LOOPING:
		{
			int looping = atoi(cmd);
			set_looping(looping);
			reset_menu_state();
		}
		break;
		case CURRENT_STATUS_DISPLAY_SURFACE_CHANGE:
		{
			int type = atoi(cmd);
			change_surface(type);
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
		break;
		case CURRENT_STATUS_DISPLAY_DST_ROI:
		{
			int value = atoi(cmd);
			static int roi_x = 0;
			static int roi_y = 0;
			static int roi_w = 0;
			static int roi_h = 0;
			static int cnt = 0;
			switch (cnt) {
			case 0:
				roi_x = value;
				cnt++;
				break;
			case 1:
				roi_y = value;
				cnt++;
				break;
			case 2:
				roi_w = value;
				cnt++;
				break;
			case 3:
				cnt = 0;
				roi_h = value;
				set_display_dst_roi(roi_x, roi_y, roi_w, roi_h);
				roi_x = roi_y = roi_w = roi_h = 0;
				reset_menu_state();
				break;
			default:
				break;
			}
		}
		break;
		case CURRENT_STATUS_DISPLAY_SRC_CROP:
		{
			int value = atoi(cmd);
			static int crop_x = 0;
			static int crop_y = 0;
			static int crop_w = 0;
			static int crop_h = 0;
			static int crop_cnt = 0;
			switch (crop_cnt) {
			case 0:
				crop_x = value;
				crop_cnt++;
				break;
			case 1:
				crop_y = value;
				crop_cnt++;
				break;
			case 2:
				crop_w = value;
				crop_cnt++;
				break;
			case 3:
				crop_cnt = 0;
				crop_h = value;
				set_display_src_crop(crop_x, crop_y, crop_w, crop_h);
				crop_x = crop_y = crop_w = crop_h = 0;
				reset_menu_state();
				break;
			default:
				break;
			}
		}
		break;
		case CURRENT_STATUS_DISPLAY_ROI_MODE:
		{
			int value = atoi(cmd);
			set_display_roi_mode(value);
			reset_menu_state();
		}
		break;
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
    gchar buf[MAX_STRING_LEN];
    gsize read;
    GError *error = NULL;

    g_io_channel_read_chars(channel, buf, MAX_STRING_LEN, &read, &error);
    buf[read] = '\0';
    g_strstrip(buf);
    interpret (buf);

    return TRUE;
}

int main(int argc, char *argv[])
{
	GIOChannel *stdin_channel;
	stdin_channel = g_io_channel_unix_new(0);
	g_io_channel_set_flags (stdin_channel, G_IO_FLAG_NONBLOCK, NULL);
	g_io_add_watch(stdin_channel, G_IO_IN, (GIOFunc)input, NULL);

	displaymenu();
	memset(&ad, 0x0, sizeof(struct appdata));
	ops.data = &ad;

	return appcore_efl_main(PACKAGE, &argc, &argv, &ops);
}
