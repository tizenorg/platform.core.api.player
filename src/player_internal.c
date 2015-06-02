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

#ifdef TIZEN_MOBILE
static bool  __video_frame_render_error_callback(void *param, void *user_data)
{
	player_s * handle = (player_s*)user_data;
	if( handle->user_cb[_PLAYER_EVENT_TYPE_VIDEO_FRAME_RENDER_ERROR])
	{
		((player_x11_pixmap_error_cb)handle->user_cb[_PLAYER_EVENT_TYPE_VIDEO_FRAME_RENDER_ERROR])((unsigned int *)param, handle->user_data[_PLAYER_EVENT_TYPE_VIDEO_FRAME_RENDER_ERROR]);
	}
	return TRUE;
}

int player_set_x11_display_pixmap (player_h player, player_x11_pixmap_updated_cb callback, void *user_data)
{
	PLAYER_INSTANCE_CHECK(player);
	PLAYER_NULL_ARG_CHECK(callback);
	player_s * handle = (player_s *) player;

	if (!__player_state_validate(handle, PLAYER_STATE_IDLE))
	{
		LOGE("[%s] PLAYER_ERROR_INVALID_STATE(0x%08x) : current state - %d" ,__FUNCTION__,PLAYER_ERROR_INVALID_STATE, handle->state);
		return PLAYER_ERROR_INVALID_STATE;
	}

	if (!handle->is_set_pixmap_cb && handle->display_type != ((int)MM_DISPLAY_SURFACE_NULL))
	{
		LOGE("[%s] player_set_display() was set, try it again after calling player_unprepare()" ,__FUNCTION__,PLAYER_ERROR_INVALID_OPERATION);
		LOGE("[%s] PLAYER_ERROR_INVALID_OPERATION(0x%08x)" ,__FUNCTION__,PLAYER_ERROR_INVALID_OPERATION);
		return PLAYER_ERROR_INVALID_OPERATION;
	}

	int ret = mm_player_set_attribute(handle->mm_handle, NULL, "display_surface_type", MM_DISPLAY_SURFACE_X_EXT, "display_overlay" , callback , sizeof(callback), "display_overlay_user_data", user_data, sizeof(user_data), (char*)NULL);
	if(ret != MM_ERROR_NONE)
	{
		return __player_convert_error_code(ret,(char*)__FUNCTION__);
	}
	else
	{
		handle->is_set_pixmap_cb = true;
		handle->display_type = MM_DISPLAY_SURFACE_X;
		handle->display_handle = callback;
		LOGI("[%s] video display has been changed- type :%d, pixmap_callback addr : 0x%x",__FUNCTION__,handle->display_type, handle->display_handle);
		return PLAYER_ERROR_NONE;
	}

}

int player_set_x11_display_pixmap_error_cb (player_h player, player_x11_pixmap_error_cb callback, void *user_data)
{
	PLAYER_INSTANCE_CHECK(player);
	PLAYER_NULL_ARG_CHECK(callback);
	player_s * handle = (player_s *) player;

	if (!__player_state_validate(handle, PLAYER_STATE_IDLE))
	{
		LOGE("[%s] PLAYER_ERROR_INVALID_STATE(0x%08x) : current state - %d" ,__FUNCTION__,PLAYER_ERROR_INVALID_STATE, handle->state);
		return PLAYER_ERROR_INVALID_STATE;
	}

	int ret = mm_player_set_video_frame_render_error_callback(handle->mm_handle, __video_frame_render_error_callback, (void*)handle);
	if(ret != MM_ERROR_NONE)
	{
		return __player_convert_error_code(ret,(char*)__FUNCTION__);
	}
	else
	{
		LOGI("[%s] set pixmap_error_cb(0x%08x) and user_data(0x%8x)" ,__FUNCTION__, callback, user_data);
		handle->user_cb[_PLAYER_EVENT_TYPE_VIDEO_FRAME_RENDER_ERROR] = callback;
		handle->user_data[_PLAYER_EVENT_TYPE_VIDEO_FRAME_RENDER_ERROR] = user_data;
		LOGI("[%s] Event type : %d ",__FUNCTION__, _PLAYER_EVENT_TYPE_VIDEO_FRAME_RENDER_ERROR);
		return PLAYER_ERROR_NONE;
	}
}
#endif

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
	PLAYER_STATE_CHECK(handle, PLAYER_STATE_IDLE);

	int ret = mm_player_set_attribute(handle->mm_handle, NULL, "pcm_extraction",TRUE, "pcm_extraction_start_msec", 0, "pcm_extraction_end_msec", 0, NULL);
	if(ret != MM_ERROR_NONE)
		return __player_convert_error_code(ret,(char*)__FUNCTION__);

	ret = mm_player_set_audio_stream_callback_ex(handle->mm_handle, sync,  __audio_stream_callback_ex, (void*)handle);
	if(ret != MM_ERROR_NONE)
		return __player_convert_error_code(ret,(char*)__FUNCTION__);

	PLAYER_SET_CALLBACK(_PLAYER_EVENT_TYPE_AUDIO_FRAME, handle, callback, user_data);
	return PLAYER_ERROR_NONE;
}

