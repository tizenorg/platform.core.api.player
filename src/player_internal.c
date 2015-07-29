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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlog.h>

#include <mm.h>
#include <mm_player.h>
#include <mm_player_internal.h>
#include <mm_player_mused.h>
#include <mm_types.h>
#include <player.h>
#include <player_internal.h>
#include <player_private.h>


/*
* Internal Macros
*/
#define PLAYER_SET_CALLBACK(event_type, handle, callback, user_data) \
do \
{ \
	PLAYER_INSTANCE_CHECK(handle); \
	PLAYER_NULL_ARG_CHECK(callback); \
	handle->user_cb[event_type] = callback; \
	handle->user_data[event_type] = user_data; \
	LOGI("[%s] Event type : %d ",__FUNCTION__, event_type); \
}while(0) \

bool  __audio_stream_callback_ex(MMPlayerAudioStreamDataType *stream, void *user_data)
{
	player_s * handle = (player_s*)user_data;
	if( handle->user_cb[_PLAYER_EVENT_TYPE_AUDIO_FRAME] )
	{
		if(handle->state==PLAYER_STATE_PLAYING || handle->state==PLAYER_STATE_PAUSED)
		{
			((player_audio_pcm_extraction_cb)handle->user_cb[_PLAYER_EVENT_TYPE_AUDIO_FRAME])((player_audio_raw_data_s *)stream, handle->user_data[_PLAYER_EVENT_TYPE_AUDIO_FRAME]);
		}
		else
		{
			LOGE("[%s] Skip stream - current state : %d", __FUNCTION__,handle->state);
		}
	}
	return TRUE;
}

int player_set_pcm_extraction_mode(player_h player, bool sync, player_audio_pcm_extraction_cb callback, void *user_data)
{
	PLAYER_INSTANCE_CHECK(player);
	PLAYER_NULL_ARG_CHECK(callback);
	player_s * handle = (player_s *) player;
	int ret = MM_ERROR_NONE;

	PLAYER_STATE_CHECK(handle, PLAYER_STATE_IDLE);

	ret = mm_player_set_attribute(handle->mm_handle, NULL, "pcm_extraction",TRUE, "pcm_extraction_start_msec", 0, "pcm_extraction_end_msec", 0, NULL);
	if(ret != MM_ERROR_NONE)
		return __player_convert_error_code(ret,(char*)__FUNCTION__);

	ret = mm_player_set_audio_stream_callback_ex(handle->mm_handle, sync,  __audio_stream_callback_ex, (void*)handle);
	if(ret != MM_ERROR_NONE)
		return __player_convert_error_code(ret,(char*)__FUNCTION__);

	PLAYER_SET_CALLBACK(_PLAYER_EVENT_TYPE_AUDIO_FRAME, handle, callback, user_data);
	return PLAYER_ERROR_NONE;
}

int player_set_pcm_spec(player_h player, const char *format, int samplerate, int channel)
{
	PLAYER_INSTANCE_CHECK(player);

	player_s * handle = (player_s *) player;
	int ret = MM_ERROR_NONE;

	LOGE("[%s] player_set_pcm_spec %s %d %d", __FUNCTION__, format, samplerate, channel);
	ret = mm_player_set_attribute(handle->mm_handle, NULL, "pcm_audioformat", format , strlen(format), NULL);
	if(ret != MM_ERROR_NONE)
		return __player_convert_error_code(ret,(char*)__FUNCTION__);

	ret = mm_player_set_pcm_spec(handle->mm_handle, samplerate, channel);
	if(ret != MM_ERROR_NONE)
		return __player_convert_error_code(ret,(char*)__FUNCTION__);

	return PLAYER_ERROR_NONE;
}

/*
* Internal Implementation for mused
*/
int player_set_shm_stream_path_for_mused (player_h player, const char *stream_path)
{
	PLAYER_INSTANCE_CHECK(player);
	PLAYER_NULL_ARG_CHECK(stream_path);

	player_s * handle = (player_s *) player;

	if(mm_player_set_shm_stream_path(handle->mm_handle, stream_path)
			!= MM_ERROR_NONE) {
		LOGE("Fail to set attribute stream path");
		return PLAYER_ERROR_INVALID_OPERATION;
	}
	return PLAYER_ERROR_NONE;
}

static MMDisplaySurfaceType __player_mused_convet_display_type(player_display_type_e type)
{
	switch(type) {
	case PLAYER_DISPLAY_TYPE_OVERLAY:
		return MM_DISPLAY_SURFACE_REMOTE;
	case PLAYER_DISPLAY_TYPE_EVAS:
		return MM_DISPLAY_SURFACE_EVAS;
	case PLAYER_DISPLAY_TYPE_NONE:
		return MM_DISPLAY_SURFACE_NULL;
	default :
		return MM_DISPLAY_SURFACE_NULL;
	}
}


#ifdef HAVE_WAYLAND
int player_set_display_wl_for_mused(player_h player, player_display_type_e type, intptr_t surface,
		int x, int y, int w, int h)
{
	PLAYER_INSTANCE_CHECK(player);
	player_s * handle = (player_s *) player;
	void *set_handle = NULL;
	MMDisplaySurfaceType mmType = __player_mused_convet_display_type(type);

	int ret;
	if (!__player_state_validate(handle, PLAYER_STATE_IDLE))
	{
		LOGE("[%s] PLAYER_ERROR_INVALID_STATE(0x%08x) : current state - %d" ,__FUNCTION__,PLAYER_ERROR_INVALID_STATE, handle->state);
		return PLAYER_ERROR_INVALID_STATE;
	}
	if (handle->is_set_pixmap_cb)
	{
		if (handle->state < PLAYER_STATE_READY)
		{
			/* just set below and go to "changing surface case" */
			handle->is_set_pixmap_cb = false;
		}
		else
		{
			LOGE("[%s] pixmap callback was set, try it again after calling player_unprepare()" ,__FUNCTION__,PLAYER_ERROR_INVALID_OPERATION);
			LOGE("[%s] PLAYER_ERROR_INVALID_OPERATION(0x%08x)" ,__FUNCTION__,PLAYER_ERROR_INVALID_OPERATION);
			return PLAYER_ERROR_INVALID_OPERATION;
		}
	}

	void* temp = NULL;
	if (type == PLAYER_DISPLAY_TYPE_NONE)
	{
		/* NULL surface */
		handle->display_handle = 0;
		handle->display_type = type;
		set_handle = NULL;
	}
	else
	{
		/* get handle from overlay or evas surface */
		temp = handle->display_handle;
		if (type == PLAYER_DISPLAY_TYPE_OVERLAY)
		{
			LOGI("Wayland overlay surface type");
			handle->display_handle = (void *)surface;
			set_handle = &(handle->display_handle);
		} else {
			LOGE("invalid surface type");
			return PLAYER_ERROR_INVALID_PARAMETER;
		}
	}

	/* set display handle */
	if (handle->display_type == PLAYER_DISPLAY_TYPE_NONE || type == handle->display_type) // first time or same type
	{
		LOGW("first time or same type");
		ret = mm_player_set_attribute(handle->mm_handle, NULL,
			"display_surface_type", mmType,
			"display_overlay", set_handle,
			sizeof(void *), (char*)NULL);

		if (ret != MM_ERROR_NONE)
		{
			handle->display_handle = temp;
			LOGE("[%s] Failed to display surface change :%d",__FUNCTION__,ret);
		}
		else
		{
			if (type != PLAYER_DISPLAY_TYPE_NONE)
			{
				handle->display_type = type;
				LOGI("[%s] video display has been changed- type :%d, addr : 0x%x",
					__FUNCTION__,handle->display_type, handle->display_handle);
			}
			else
				LOGI("NULL surface");
		}
		LOGI("get window rectangle: x(%d) y(%d) width(%d) height(%d)", x, y, w, h);
		ret = mm_player_set_attribute(handle->mm_handle, NULL,
			"wl_window_render_x", x,
			"wl_window_render_y", y,
			"wl_window_render_width", w,
			"wl_window_render_height", h,
			(char*)NULL);

		if (ret != MM_ERROR_NONE)
		{
			handle->display_handle = temp;
			LOGE("[%s] Failed to set wl_window render rectangle :%d",__FUNCTION__,ret);
		}
	}
	else //changing surface case
	{
		ret = mm_player_change_videosink(handle->mm_handle, mmType, set_handle);
		if (ret != MM_ERROR_NONE)
		{
			handle->display_handle = temp;
			if(ret == MM_ERROR_NOT_SUPPORT_API)
			{
				LOGE("[%s] change video sink is not available.",__FUNCTION__);
				ret = PLAYER_ERROR_NONE;
			}
			else
			{
				LOGE("[%s] Failed to display surface change :%d",__FUNCTION__,ret);
			}
		}
		else
		{
			handle->display_type = type;
			LOGI("[%s] video display has been changed- type :%d, addr : 0x%x",
				__FUNCTION__,handle->display_type, handle->display_handle);
		}
	}

	if(ret != MM_ERROR_NONE)
	{
		handle->display_type = PLAYER_DISPLAY_TYPE_NONE;
		return __player_convert_error_code(ret,(char*)__FUNCTION__);
	}
	else
	{
		return PLAYER_ERROR_NONE;
	}
}

#else

int player_set_display_for_mused(player_h player, player_display_type_e type, unsigned int xhandle)
{
	PLAYER_INSTANCE_CHECK(player);
	player_s * handle = (player_s *) player;
	void *set_handle = NULL;
	MMDisplaySurfaceType mmType = __player_mused_convet_display_type(type);

	int ret;
	if (!__player_state_validate(handle, PLAYER_STATE_IDLE))
	{
		LOGE("[%s] PLAYER_ERROR_INVALID_STATE(0x%08x) : current state - %d" ,__FUNCTION__,PLAYER_ERROR_INVALID_STATE, handle->state);
		return PLAYER_ERROR_INVALID_STATE;
	}

	if (handle->is_set_pixmap_cb)
	{
		if (handle->state < PLAYER_STATE_READY)
		{
			/* just set below and go to "changing surface case" */
			handle->is_set_pixmap_cb = false;
		}
		else
		{
			LOGE("[%s] pixmap callback was set, try it again after calling player_unprepare()" ,__FUNCTION__,PLAYER_ERROR_INVALID_OPERATION);
			LOGE("[%s] PLAYER_ERROR_INVALID_OPERATION(0x%08x)" ,__FUNCTION__,PLAYER_ERROR_INVALID_OPERATION);
			return PLAYER_ERROR_INVALID_OPERATION;
		}
	}

	void* temp = NULL;
	if (type == PLAYER_DISPLAY_TYPE_NONE)
	{
		/* NULL surface */
		handle->display_handle = 0;
		handle->display_type = PLAYER_DISPLAY_TYPE_NONE;
		set_handle = NULL;
	}
	else
	{
		/* get handle from overlay or evas surface */
		temp = handle->display_handle;
		if (type == PLAYER_DISPLAY_TYPE_OVERLAY /*&& !strcmp(object_type, "elm_win")*/)
		{
			/* x window overlay surface */
			LOGI("overlay surface type");
			handle->display_handle = (void *)xhandle;
			set_handle = &(handle->display_handle);
		}
		else
		{
			LOGE("invalid surface type");
			return PLAYER_ERROR_INVALID_PARAMETER;
		}
	}

	/* set display handle */
	if (handle->display_type == PLAYER_DISPLAY_TYPE_NONE || type == handle->display_type) // first time or same type
	{
		ret = mm_player_set_attribute(handle->mm_handle, NULL,
			"display_surface_type", mmType,
			"display_overlay", set_handle,
			sizeof(xhandle), (char*)NULL);

		if (ret != MM_ERROR_NONE)
		{
			handle->display_handle = temp;
			LOGE("[%s] Failed to display surface change :%d",__FUNCTION__,ret);
		}
		else
		{
			if (type != PLAYER_DISPLAY_TYPE_NONE)
			{
				handle->display_type = type;
				LOGI("[%s] video display has been changed- type :%d, addr : 0x%x",
					__FUNCTION__,handle->display_type, handle->display_handle);
			}
			else
				LOGI("NULL surface");
		}
	}
	else //changing surface case
	{
		ret = mm_player_change_videosink(handle->mm_handle, mmType, set_handle);
		if (ret != MM_ERROR_NONE)
		{
			handle->display_handle = temp;
			if(ret == MM_ERROR_NOT_SUPPORT_API)
			{
				LOGE("[%s] change video sink is not available.",__FUNCTION__);
				ret = PLAYER_ERROR_NONE;
			}
			else
			{
				LOGE("[%s] Failed to display surface change :%d",__FUNCTION__,ret);
			}
		}
		else
		{
			handle->display_type = type;
			LOGI("[%s] video display has been changed- type :%d, addr : 0x%x",
				__FUNCTION__,handle->display_type, handle->display_handle);
		}
	}

	if(ret != MM_ERROR_NONE)
	{
		handle->display_type = PLAYER_DISPLAY_TYPE_NONE;
		return __player_convert_error_code(ret,(char*)__FUNCTION__);
	}
	else
	{
		return PLAYER_ERROR_NONE;
	}
}
#endif

int player_get_raw_video_caps(player_h player, char **caps)
{
	int ret;
	PLAYER_INSTANCE_CHECK(player);
	PLAYER_NULL_ARG_CHECK(caps);
	player_s * handle = (player_s *) player;

	if (handle->display_type == PLAYER_DISPLAY_TYPE_NONE)
		return PLAYER_ERROR_INVALID_STATE;

	ret = mm_player_get_raw_video_caps(handle->mm_handle, caps);
	if(ret != MM_ERROR_NONE) {
		return __player_convert_error_code(ret,(char*)__FUNCTION__);
	}

	return PLAYER_ERROR_NONE;
}
