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

int player_set_pcm_extraction_mode(player_h player, bool sync, const char *format, player_audio_pcm_extraction_cb callback, void *user_data)
{
	PLAYER_INSTANCE_CHECK(player);
	PLAYER_NULL_ARG_CHECK(callback);
	player_s * handle = (player_s *) player;
	int ret = MM_ERROR_NONE;
	PLAYER_STATE_CHECK(handle, PLAYER_STATE_IDLE);

	ret = mm_player_set_attribute(handle->mm_handle, NULL, "pcm_extraction",TRUE, "pcm_extraction_start_msec", 0, "pcm_extraction_end_msec", 0, NULL);
	if(ret != MM_ERROR_NONE)
		return __player_convert_error_code(ret,(char*)__FUNCTION__);

	ret = mm_player_set_attribute(handle->mm_handle, NULL, "pcm_audioformat", format , strlen(format), NULL);
	if(ret != MM_ERROR_NONE)
		return __player_convert_error_code(ret,(char*)__FUNCTION__);

	ret = mm_player_set_audio_stream_callback_ex(handle->mm_handle, sync,  __audio_stream_callback_ex, (void*)handle);
	if(ret != MM_ERROR_NONE)
		return __player_convert_error_code(ret,(char*)__FUNCTION__);

	PLAYER_SET_CALLBACK(_PLAYER_EVENT_TYPE_AUDIO_FRAME, handle, callback, user_data);
	return PLAYER_ERROR_NONE;
}

