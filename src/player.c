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
#include <mm.h>
#include <mm_player.h>
#include <mm_player_audioeffect.h>
#include <mm_player_internal.h>
#include <mm_types.h>
#include <player.h>
#include <player_private.h>
#include <dlog.h>
#include <mm_ta.h>

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "TIZEN_N_PLAYER"

/*
* Internal Macros
*/

#define PLAYER_CHECK_CONDITION(condition,error,msg)	\
		if(condition) {} else \
		{ LOGE("[%s] %s(0x%08x)",__FUNCTION__, msg,error); return error;}; \

#define PLAYER_INSTANCE_CHECK(player)	\
	PLAYER_CHECK_CONDITION(player != NULL, PLAYER_ERROR_INVALID_PARAMETER,"PLAYER_ERROR_INVALID_PARAMETER")

#define PLAYER_STATE_CHECK(player,expected_state)	\
	PLAYER_CHECK_CONDITION(player->state == expected_state,PLAYER_ERROR_INVALID_STATE,"PLAYER_ERROR_INVALID_STATE")

#define PLAYER_NULL_ARG_CHECK(arg)	\
	PLAYER_CHECK_CONDITION(arg != NULL,PLAYER_ERROR_INVALID_PARAMETER,"PLAYER_ERROR_INVALID_PARAMETER")

/*
* Internal Implementation
*/
static int __convert_error_code(int code, char* func_name)
{
	int ret = PLAYER_ERROR_INVALID_OPERATION;
	char* msg="PLAYER_ERROR_INVALID_OPERATION";
	switch(code)
	{
		case MM_ERROR_NONE:
		case MM_ERROR_PLAYER_AUDIO_CODEC_NOT_FOUND:
		case MM_ERROR_PLAYER_VIDEO_CODEC_NOT_FOUND:
			ret = PLAYER_ERROR_NONE;
			msg = "PLAYER_ERROR_NONE";
			break;
		case MM_ERROR_INVALID_ARGUMENT:
			ret = PLAYER_ERROR_INVALID_PARAMETER;
			msg = "PLAYER_ERROR_INVALID_PARAMETER";
			break;
		case MM_ERROR_PLAYER_CODEC_NOT_FOUND:
		case MM_ERROR_PLAYER_STREAMING_UNSUPPORTED_AUDIO:
		case MM_ERROR_PLAYER_STREAMING_UNSUPPORTED_VIDEO:
		case MM_ERROR_PLAYER_STREAMING_UNSUPPORTED_MEDIA_TYPE:
		case MM_ERROR_PLAYER_NOT_SUPPORTED_FORMAT:
			ret = PLAYER_ERROR_NOT_SUPPORTED_FILE;
			msg = "PLAYER_ERROR_NOT_SUPPORTED_FILE";
			break;
		case MM_ERROR_PLAYER_INVALID_STATE:
		case MM_ERROR_PLAYER_NOT_INITIALIZED:
			ret = PLAYER_ERROR_INVALID_STATE;
			msg = "PLAYER_ERROR_INVALID_STATE";
			break;
		case MM_ERROR_PLAYER_INTERNAL:
		case MM_ERROR_PLAYER_INVALID_STREAM:
		case MM_ERROR_PLAYER_STREAMING_FAIL:
		case MM_ERROR_PLAYER_NO_OP:
		case MM_ERROR_NOT_SUPPORT_API:
			ret = PLAYER_ERROR_INVALID_OPERATION;
			msg = "PLAYER_ERROR_INVALID_OPERATION";
			break;
		case MM_ERROR_PLAYER_SOUND_EFFECT_INVALID_STATUS:
		case MM_ERROR_PLAYER_SOUND_EFFECT_NOT_SUPPORTED_FILTER:
			ret = PLAYER_ERROR_INVALID_OPERATION;
			msg = "PLAYER_ERROR_INVALID_OPERATION (NOT SUPPORTED AUDIO EFFECT)";
			break;
		case  MM_ERROR_PLAYER_NO_FREE_SPACE:
			ret = PLAYER_ERROR_FILE_NO_SPACE_ON_DEVICE;
			msg = "PLAYER_ERROR_FILE_NO_SPACE_ON_DEVICE";
			break;
		case MM_ERROR_PLAYER_FILE_NOT_FOUND:
			ret = PLAYER_ERROR_NO_SUCH_FILE;
			msg = "PLAYER_ERROR_NO_SUCH_FILE";
			break;
		case MM_ERROR_PLAYER_SEEK:
			ret = PLAYER_ERROR_SEEK_FAILED;
			msg = "PLAYER_ERROR_SEEK_FAILED";
			break;
		case MM_ERROR_PLAYER_INVALID_URI:
		case MM_ERROR_PLAYER_STREAMING_INVALID_URL:
			ret = PLAYER_ERROR_INVALID_URI;
			msg = "PLAYER_ERROR_INVALID_URI";
			break;
		case MM_ERROR_PLAYER_STREAMING_CONNECTION_FAIL:
		case MM_ERROR_PLAYER_STREAMING_DNS_FAIL	:
		case MM_ERROR_PLAYER_STREAMING_SERVER_DISCONNECTED:
		case MM_ERROR_PLAYER_STREAMING_INVALID_PROTOCOL:
		case MM_ERROR_PLAYER_STREAMING_UNEXPECTED_MSG:
		case MM_ERROR_PLAYER_STREAMING_OUT_OF_MEMORIES:
		case MM_ERROR_PLAYER_STREAMING_RTSP_TIMEOUT:
		case MM_ERROR_PLAYER_STREAMING_BAD_REQUEST:
		case MM_ERROR_PLAYER_STREAMING_NOT_AUTHORIZED:
		case MM_ERROR_PLAYER_STREAMING_PAYMENT_REQUIRED:
		case MM_ERROR_PLAYER_STREAMING_FORBIDDEN:
		case MM_ERROR_PLAYER_STREAMING_CONTENT_NOT_FOUND:
		case MM_ERROR_PLAYER_STREAMING_METHOD_NOT_ALLOWED:
		case MM_ERROR_PLAYER_STREAMING_NOT_ACCEPTABLE:
		case MM_ERROR_PLAYER_STREAMING_PROXY_AUTHENTICATION_REQUIRED:
		case MM_ERROR_PLAYER_STREAMING_SERVER_TIMEOUT:
		case MM_ERROR_PLAYER_STREAMING_GONE:
		case MM_ERROR_PLAYER_STREAMING_LENGTH_REQUIRED:
		case MM_ERROR_PLAYER_STREAMING_PRECONDITION_FAILED:
		case MM_ERROR_PLAYER_STREAMING_REQUEST_ENTITY_TOO_LARGE:
		case MM_ERROR_PLAYER_STREAMING_REQUEST_URI_TOO_LARGE:
		case MM_ERROR_PLAYER_STREAMING_PARAMETER_NOT_UNDERSTOOD:
		case MM_ERROR_PLAYER_STREAMING_CONFERENCE_NOT_FOUND:
		case MM_ERROR_PLAYER_STREAMING_NOT_ENOUGH_BANDWIDTH:
		case MM_ERROR_PLAYER_STREAMING_NO_SESSION_ID:
		case MM_ERROR_PLAYER_STREAMING_METHOD_NOT_VALID_IN_THIS_STATE:
		case MM_ERROR_PLAYER_STREAMING_HEADER_FIELD_NOT_VALID_FOR_SOURCE:
		case MM_ERROR_PLAYER_STREAMING_INVALID_RANGE:
		case MM_ERROR_PLAYER_STREAMING_PARAMETER_IS_READONLY:
		case MM_ERROR_PLAYER_STREAMING_AGGREGATE_OP_NOT_ALLOWED:
		case MM_ERROR_PLAYER_STREAMING_ONLY_AGGREGATE_OP_ALLOWED:
		case MM_ERROR_PLAYER_STREAMING_BAD_TRANSPORT:
		case MM_ERROR_PLAYER_STREAMING_DESTINATION_UNREACHABLE:
		case MM_ERROR_PLAYER_STREAMING_INTERNAL_SERVER_ERROR:
		case MM_ERROR_PLAYER_STREAMING_NOT_IMPLEMENTED:
		case MM_ERROR_PLAYER_STREAMING_BAD_GATEWAY:
		case MM_ERROR_PLAYER_STREAMING_SERVICE_UNAVAILABLE:
		case MM_ERROR_PLAYER_STREAMING_GATEWAY_TIME_OUT:
		case MM_ERROR_PLAYER_STREAMING_OPTION_NOT_SUPPORTED:
			ret = PLAYER_ERROR_CONNECTION_FAILED;
			msg = "PLAYER_ERROR_CONNECTION_FAILED";
			break;
		case MM_ERROR_POLICY_BLOCKED:
		case MM_ERROR_POLICY_INTERRUPTED:
		case MM_ERROR_POLICY_INTERNAL:
		case MM_ERROR_POLICY_DUPLICATED:
			ret = PLAYER_ERROR_SOUND_POLICY;
			msg = "PLAYER_ERROR_SOUND_POLICY";
			break;
		case MM_ERROR_PLAYER_DRM_EXPIRED:
			ret = PLAYER_ERROR_DRM_EXPIRED;
			msg = "PLAYER_ERROR_DRM_EXPIRED";
			break;
		case MM_ERROR_PLAYER_DRM_NOT_AUTHORIZED:
		case MM_ERROR_PLAYER_DRM_NO_LICENSE:
			ret = PLAYER_ERROR_DRM_NO_LICENSE;
			msg = "PLAYER_ERROR_DRM_NO_LICENSE";
			break;
		case MM_ERROR_PLAYER_DRM_FUTURE_USE:
			ret = PLAYER_ERROR_DRM_FUTURE_USE;
			msg = "PLAYER_ERROR_DRM_FUTURE_USE";
			break;
	}
	LOGE("[%s] %s(0x%08x) : core fw error(0x%x)",func_name,msg, ret, code);
	return ret;
}

static player_interrupted_code_e __convert_interrupted_code(int code)
{
	player_interrupted_code_e ret = PLAYER_INTERRUPTED_BY_RESOURCE_CONFLICT;
	switch(code)
	{
		case MM_MSG_CODE_INTERRUPTED_BY_ALARM_END:
		case MM_MSG_CODE_INTERRUPTED_BY_EMERGENCY_END:
			ret = PLAYER_INTERRUPTED_COMPLETED;
			break;
		case MM_MSG_CODE_INTERRUPTED_BY_MEDIA:
		case MM_MSG_CODE_INTERRUPTED_BY_OTHER_PLAYER_APP:
			ret = PLAYER_INTERRUPTED_BY_MEDIA;
			break;
		case MM_MSG_CODE_INTERRUPTED_BY_CALL_START:
			ret = PLAYER_INTERRUPTED_BY_CALL;
			break;
		case MM_MSG_CODE_INTERRUPTED_BY_EARJACK_UNPLUG:
			ret = PLAYER_INTERRUPTED_BY_EARJACK_UNPLUG;
			break;
		case MM_MSG_CODE_INTERRUPTED_BY_ALARM_START:
			ret = PLAYER_INTERRUPTED_BY_ALARM;
			break;
		case MM_MSG_CODE_INTERRUPTED_BY_EMERGENCY_START:
			ret = PLAYER_INTERRUPTED_BY_EMERGENCY;
			break;
		case MM_MSG_CODE_INTERRUPTED_BY_RESUMABLE_MEDIA:
			ret = PLAYER_INTERRUPTED_BY_RESUMABLE_MEDIA;
			break;
		case MM_MSG_CODE_INTERRUPTED_BY_RESOURCE_CONFLICT:
		default :
			ret = PLAYER_INTERRUPTED_BY_RESOURCE_CONFLICT;
			break;
	}
	LOGE("[%s] interrupted code(%d) => ret(%d)",__FUNCTION__,code, ret);
	return ret;
}

static player_state_e __convert_player_state(MMPlayerStateType state)
{
	if(state == MM_PLAYER_STATE_NONE)
		return PLAYER_STATE_NONE;
	else
		return state +1;
}

static bool __player_state_validate(player_s * handle, player_state_e threshold)
{
	if (handle->state < threshold)
		return FALSE;
	return TRUE;
}

static int __set_callback(_player_event_e type, player_h player, void* callback, void *user_data)
{
	PLAYER_INSTANCE_CHECK(player);
	PLAYER_NULL_ARG_CHECK(callback);
	player_s * handle = (player_s *) player;
	handle->user_cb[type] = callback;
	handle->user_data[type] = user_data;
	LOGI("[%s] Event type : %d ",__FUNCTION__, type);
	return PLAYER_ERROR_NONE;
}

static int __unset_callback(_player_event_e type, player_h player)
{
	PLAYER_INSTANCE_CHECK(player);
	player_s * handle = (player_s *) player;
	handle->user_cb[type] = NULL;
	handle->user_data[type] = NULL;
	LOGI("[%s] Event type : %d ",__FUNCTION__, type);
	return PLAYER_ERROR_NONE;
}

static int __msg_callback(int message, void *param, void *user_data)
{
	player_s * handle = (player_s*)user_data;
	MMMessageParamType *msg = (MMMessageParamType*)param;
	LOGE("[%s] Start : Got message type : 0x%x" ,__FUNCTION__, message);
	player_error_e err_code = PLAYER_ERROR_NONE;
	switch(message)
	{
		case MM_MESSAGE_ERROR: //0x01
			err_code = __convert_error_code(msg->code,(char*)__FUNCTION__);
			break;
		case  MM_MESSAGE_STATE_CHANGED:	//0x03
			LOGE("STATE CHANGED INTERNALLY - from : %d,  to : %d (CAPI State : %d)", msg->state.previous, msg->state.current, handle->state);
			if ((handle->is_progressive_download && msg->state.previous == MM_PLAYER_STATE_NULL && msg->state.current == MM_PLAYER_STATE_READY) ||
				(msg->state.previous == MM_PLAYER_STATE_READY && msg->state.current == MM_PLAYER_STATE_PAUSED))
			{
				if(handle->user_cb[_PLAYER_EVENT_TYPE_PREPARE]) // asyc && prepared cb has been set
				{
					LOGE("[%s] Prepared! [current state : %d]", __FUNCTION__, handle->state);
					handle->state = PLAYER_STATE_READY;
					((player_prepared_cb)handle->user_cb[_PLAYER_EVENT_TYPE_PREPARE])(handle->user_data[_PLAYER_EVENT_TYPE_PREPARE]);
					handle->user_cb[_PLAYER_EVENT_TYPE_PREPARE] = NULL;
					handle->user_data[_PLAYER_EVENT_TYPE_PREPARE] = NULL;
				}
			}
			break;
		case MM_MESSAGE_READY_TO_RESUME: //0x05
			if( handle->user_cb[_PLAYER_EVENT_TYPE_INTERRUPT] )
			{
				((player_interrupted_cb)handle->user_cb[_PLAYER_EVENT_TYPE_INTERRUPT])(PLAYER_INTERRUPTED_COMPLETED,handle->user_data[_PLAYER_EVENT_TYPE_INTERRUPT]);
			}
			break;
		case MM_MESSAGE_BEGIN_OF_STREAM: //0x104
				MMTA_ACUM_ITEM_END("[CAPI] player_start ~ BOS", 0);
				LOGE("[%s] Ready to streaming information (BOS) [current state : %d]", __FUNCTION__, handle->state);
			break;
		case MM_MESSAGE_END_OF_STREAM://0x105
			if( handle->user_cb[_PLAYER_EVENT_TYPE_COMPLETE] )
			{
				((player_completed_cb)handle->user_cb[_PLAYER_EVENT_TYPE_COMPLETE])(handle->user_data[_PLAYER_EVENT_TYPE_COMPLETE]);
			}
			break;
		case MM_MESSAGE_BUFFERING: //0x103
			if( handle->user_cb[_PLAYER_EVENT_TYPE_BUFFERING] )
			{
				((player_buffering_cb)handle->user_cb[_PLAYER_EVENT_TYPE_BUFFERING])(msg->connection.buffering,handle->user_data[_PLAYER_EVENT_TYPE_BUFFERING]);
			}
			break;
		case MM_MESSAGE_STATE_INTERRUPTED: //0x04
			if( handle->user_cb[_PLAYER_EVENT_TYPE_INTERRUPT] )
			{
				handle->state = __convert_player_state(msg->state.current);
				if (handle->state == PLAYER_STATE_READY)
				{
					handle->is_stopped = TRUE;
				}
				((player_interrupted_cb)handle->user_cb[_PLAYER_EVENT_TYPE_INTERRUPT])(__convert_interrupted_code(msg->code),handle->user_data[_PLAYER_EVENT_TYPE_INTERRUPT]);
			}
			break;
		case MM_MESSAGE_CONNECTION_TIMEOUT: //0x102
			LOGE("[%s] PLAYER_ERROR_CONNECTION_FAILED (0x%08x) : CONNECTION_TIMEOUT" ,__FUNCTION__, PLAYER_ERROR_CONNECTION_FAILED);
			err_code = PLAYER_ERROR_CONNECTION_FAILED;
			break;
		case MM_MESSAGE_UPDATE_SUBTITLE: //0x109
			if( handle->user_cb[_PLAYER_EVENT_TYPE_SUBTITLE] )
			{
				((player_subtitle_updated_cb)handle->user_cb[_PLAYER_EVENT_TYPE_SUBTITLE])(msg->subtitle.duration, (char*)msg->data,handle->user_data[_PLAYER_EVENT_TYPE_SUBTITLE]);
			}
			break;
		case MM_MESSAGE_VIDEO_NOT_CAPTURED: //0x113
			LOGE("[%s] PLAYER_ERROR_VIDEO_CAPTURE_FAILED (0x%08x)",__FUNCTION__, PLAYER_ERROR_VIDEO_CAPTURE_FAILED);
			if( handle->user_cb[_PLAYER_EVENT_TYPE_ERROR] )
			{
				((player_error_cb)handle->user_cb[_PLAYER_EVENT_TYPE_ERROR])(PLAYER_ERROR_VIDEO_CAPTURE_FAILED,handle->user_data[_PLAYER_EVENT_TYPE_ERROR]);
			}
			break;
		case MM_MESSAGE_VIDEO_CAPTURED: //0x110
			if( handle->user_cb[_PLAYER_EVENT_TYPE_CAPTURE] )
			{
				int w;
				int h;
				int ret = mm_player_get_attribute(handle->mm_handle, NULL,MM_PLAYER_VIDEO_WIDTH ,&w,  MM_PLAYER_VIDEO_HEIGHT, &h, (char*)NULL);
				if(ret != MM_ERROR_NONE && handle->user_cb[_PLAYER_EVENT_TYPE_ERROR] )
				{
					LOGE("[%s] PLAYER_ERROR_VIDEO_CAPTURE_FAILED (0x%08x) : Failed to get video size on video captured (0x%x)" ,__FUNCTION__, PLAYER_ERROR_VIDEO_CAPTURE_FAILED, ret);
					err_code=PLAYER_ERROR_VIDEO_CAPTURE_FAILED;
				}
				else
				{
					MMPlayerVideoCapture* capture = (MMPlayerVideoCapture *)msg->data;
					((player_video_captured_cb)handle->user_cb[_PLAYER_EVENT_TYPE_CAPTURE])(capture->data, w, h, capture->size, handle->user_data[_PLAYER_EVENT_TYPE_CAPTURE]);

					if (capture->data)
					{
						g_free(capture->data);
						capture->data = NULL;
					}
				}
				handle->user_cb[_PLAYER_EVENT_TYPE_CAPTURE] = NULL;
				handle->user_data[_PLAYER_EVENT_TYPE_CAPTURE] = NULL;
			}
			break;
		case MM_MESSAGE_FILE_NOT_SUPPORTED: //0x10f
			LOGE("[%s] PLAYER_ERROR_NOT_SUPPORTED_FILE (0x%08x) : FILE_NOT_SUPPORTED" ,__FUNCTION__, PLAYER_ERROR_NOT_SUPPORTED_FILE);
			err_code = PLAYER_ERROR_NOT_SUPPORTED_FILE;
			break;
		case MM_MESSAGE_FILE_NOT_FOUND: //0x110
			LOGE("[%s] PLAYER_ERROR_NOT_SUPPORTED_FILE (0x%08x) : FILE_NOT_FOUND" ,__FUNCTION__, PLAYER_ERROR_NOT_SUPPORTED_FILE);
			err_code = PLAYER_ERROR_NOT_SUPPORTED_FILE;
			break;
		case MM_MESSAGE_SEEK_COMPLETED: //0x114
			if (handle->display_type != ((int)MM_DISPLAY_SURFACE_NULL) && handle->state == PLAYER_STATE_READY)
			{
				if(handle->is_display_visible)
				{
					int ret = MM_ERROR_NONE;
					ret = mm_player_set_attribute(handle->mm_handle, NULL,"display_visible" , 1, (char*)NULL);
					if(ret != MM_ERROR_NONE)
					{
						LOGW("[%s] Failed to set display visible (0x%x)" ,__FUNCTION__, ret);
					}

				}
			}
			if( handle->user_cb[_PLAYER_EVENT_TYPE_SEEK])
			{
				((player_seek_completed_cb)handle->user_cb[_PLAYER_EVENT_TYPE_SEEK])(handle->user_data[_PLAYER_EVENT_TYPE_SEEK]);
				handle->user_cb[_PLAYER_EVENT_TYPE_SEEK] = NULL;
				handle->user_data[_PLAYER_EVENT_TYPE_SEEK] = NULL;
			}
			break;
		case MM_MESSAGE_UNKNOWN: //0x00
		case MM_MESSAGE_WARNING: //0x02
		case MM_MESSAGE_CONNECTING: //0x100
		case MM_MESSAGE_CONNECTED: //0x101
		case MM_MESSAGE_BLUETOOTH_ON: //0x106
		case MM_MESSAGE_BLUETOOTH_OFF: //0x107
		case MM_MESSAGE_RTP_SENDER_REPORT: //0x10a
		case MM_MESSAGE_RTP_RECEIVER_REPORT: //0x10b
		case MM_MESSAGE_RTP_SESSION_STATUS: //0x10c
		case MM_MESSAGE_SENDER_STATE: //0x10d
		case MM_MESSAGE_RECEIVER_STATE: //0x10e
		default:
			break;
	}

	if(err_code != PLAYER_ERROR_NONE && handle->user_cb[_PLAYER_EVENT_TYPE_ERROR])
	{
		((player_error_cb)handle->user_cb[_PLAYER_EVENT_TYPE_ERROR])(err_code,handle->user_data[_PLAYER_EVENT_TYPE_ERROR]);
	}
	LOGE("[%s] End", __FUNCTION__);
	return 1;
}

static bool  __video_stream_callback(void *stream, int stream_size, void *user_data, int width, int height)
{
	player_s * handle = (player_s*)user_data;
	if( handle->user_cb[_PLAYER_EVENT_TYPE_VIDEO_FRAME])
	{
		if(handle->state==PLAYER_STATE_PLAYING)
			((player_video_frame_decoded_cb)handle->user_cb[_PLAYER_EVENT_TYPE_VIDEO_FRAME])((unsigned char *)stream, width, height, stream_size, handle->user_data[_PLAYER_EVENT_TYPE_VIDEO_FRAME]);
		else
			LOGE("[%s] Skip stream - current state : %d", __FUNCTION__,handle->state);
	}
	return TRUE;
}

static bool  __audio_stream_callback(void *stream, int stream_size, void *user_data)
{
	player_s * handle = (player_s*)user_data;
	if( handle->user_cb[_PLAYER_EVENT_TYPE_AUDIO_FRAME] )
	{
		if(handle->state==PLAYER_STATE_PLAYING)
			((player_audio_frame_decoded_cb)handle->user_cb[_PLAYER_EVENT_TYPE_AUDIO_FRAME])((unsigned char *)stream, stream_size, handle->user_data[_PLAYER_EVENT_TYPE_AUDIO_FRAME]);
		else
			LOGE("[%s] Skip stream - current state : %d", __FUNCTION__,handle->state);
	}
	return TRUE;
}

static bool  __video_frame_render_error_callback(void *param, void *user_data)
{
	player_s * handle = (player_s*)user_data;
	if( handle->user_cb[_PLAYER_EVENT_TYPE_VIDEO_FRAME_RENDER_ERROR])
	{
		((player_x11_pixmap_error_cb)handle->user_cb[_PLAYER_EVENT_TYPE_VIDEO_FRAME_RENDER_ERROR])((unsigned int *)param, handle->user_data[_PLAYER_EVENT_TYPE_VIDEO_FRAME_RENDER_ERROR]);
	}
	return TRUE;
}

static int __pd_message_callback(int message, void *param, void *user_data)
{
	player_s * handle = (player_s*)user_data;
	player_pd_message_type_e type;
	switch(message)
	{
		case MM_MESSAGE_PD_DOWNLOADER_START:
			type = PLAYER_PD_STARTED;
			break;
		case MM_MESSAGE_PD_DOWNLOADER_END:
			type = PLAYER_PD_COMPLETED;
			break;
		default:
			return 0;
	}

	if( handle->user_cb[_PLAYER_EVENT_TYPE_PD] )
	{
		((player_pd_message_cb)handle->user_cb[_PLAYER_EVENT_TYPE_PD])(type, handle->user_data[_PLAYER_EVENT_TYPE_PD]);
	}
	return 0;
}

static bool __supported_audio_effect_type (int  filter, int type, void *user_data)
{
	player_s * handle = (player_s*)user_data;
	if(filter != MM_AUDIO_EFFECT_TYPE_CUSTOM || type == MM_AUDIO_EFFECT_CUSTOM_EQ)
	{
		LOGI("[%s] Skip invalid filter: %d or type : %d",__FUNCTION__, filter, type);
		return TRUE;
	}

	if( handle->user_cb[_PLAYER_EVENT_TYPE_SUPPORTED_AUDIO_EFFECT] )
	{
		return ((player_audio_effect_supported_effect_cb)handle->user_cb[_PLAYER_EVENT_TYPE_SUPPORTED_AUDIO_EFFECT])(type, handle->user_data[_PLAYER_EVENT_TYPE_SUPPORTED_AUDIO_EFFECT]);
	}
	return FALSE;
}

static bool __supported_audio_effect_preset (int  filter, int type, void *user_data)
{
	player_s * handle = (player_s*)user_data;
	if(filter != MM_AUDIO_EFFECT_TYPE_PRESET)
	{
		LOGI("[%s] Skip invalid filter: %d or type : %d",__FUNCTION__, filter, type);
		return TRUE;
	}

	if( handle->user_cb[_PLAYER_EVENT_TYPE_SUPPORTED_AUDIO_EFFECT_PRESET] )
	{
		return ((player_audio_effect_supported_effect_cb)handle->user_cb[_PLAYER_EVENT_TYPE_SUPPORTED_AUDIO_EFFECT_PRESET])(type, handle->user_data[_PLAYER_EVENT_TYPE_SUPPORTED_AUDIO_EFFECT_PRESET]);
	}
	return FALSE;
}


/*
* Public Implementation
*/

int player_create (player_h *player)
{
	LOGE("[%s] Start", __FUNCTION__);
	PLAYER_INSTANCE_CHECK(player);
	MMTA_INIT();
	MMTA_ACUM_ITEM_BEGIN("[CoreAPI] player_create", 0);
	player_s * handle;
	handle = (player_s*)malloc( sizeof(player_s));
	if (handle != NULL) 
	{
		LOGE("[%s] Start, %p", __FUNCTION__, handle);
		memset(handle, 0 , sizeof(player_s));
	}
	else
	{
		LOGE("[%s] PLAYER_ERROR_OUT_OF_MEMORY(0x%08x)" ,__FUNCTION__,PLAYER_ERROR_OUT_OF_MEMORY);
		return PLAYER_ERROR_OUT_OF_MEMORY;
	}
	int ret = mm_player_create(&handle->mm_handle);
	MMTA_ACUM_ITEM_END("[CoreAPI] player_create", 0);
	if( ret != MM_ERROR_NONE)
	{
		LOGE("[%s] PLAYER_ERROR_INVALID_OPERATION(0x%08x)" ,__FUNCTION__,PLAYER_ERROR_INVALID_OPERATION);
		handle->state = PLAYER_STATE_NONE;
		free(handle);
		handle=NULL;
		return PLAYER_ERROR_INVALID_OPERATION;
	}
	else
	{
		*player = (player_h)handle;
		handle->state = PLAYER_STATE_IDLE;
		handle->display_type = MM_DISPLAY_SURFACE_NULL; // means DISPLAY_TYPE_NONE(3)
		handle->is_stopped=false;
		handle->is_display_visible=true;
		LOGE("[%s] End, new handle : %p", __FUNCTION__, *player);
		return PLAYER_ERROR_NONE;
	}
}

int player_destroy (player_h player)
{
	LOGE("[%s] Start, handle to destroy : %p", __FUNCTION__, player);
	PLAYER_INSTANCE_CHECK(player);
	player_s * handle = (player_s *) player;
	MMTA_ACUM_ITEM_SHOW_RESULT_TO(MMTA_SHOW_FILE);
	MMTA_RELEASE();

	if (handle->prepare_async_thread)
	{
		pthread_join(handle->prepare_async_thread, NULL);
		handle->prepare_async_thread = 0;
	}

	if (mm_player_destroy(handle->mm_handle)!= MM_ERROR_NONE)
	{
		LOGE("[%s] PLAYER_ERROR_INVALID_OPERATION(0x%08x)" ,__FUNCTION__,PLAYER_ERROR_INVALID_OPERATION);
		return PLAYER_ERROR_INVALID_OPERATION;
	}
	else
	{
		handle->state = PLAYER_STATE_NONE;
		free(handle);
		handle= NULL;
		LOGE("[%s] End", __FUNCTION__);
		return PLAYER_ERROR_NONE;
	}
}

static void *
__prepare_async_thread_func(void *data)
{
	player_s *handle = data;
	int ret = MM_ERROR_NONE;
	LOGE("[%s] Start", __FUNCTION__);

	ret = mm_player_pause(handle->mm_handle);
	if(ret != MM_ERROR_NONE) // MM_MESSAGE_ERROR should be posted through __msg_callback
	{
		LOGE("[%s] Failed to pause - core fw error(0x%x)", __FUNCTION__, ret);
	}
	LOGE("[%s] End", __FUNCTION__);
	return NULL;
}

int player_prepare_async (player_h player, player_prepared_cb callback, void* user_data)
{
	LOGE("[%s] Start", __FUNCTION__);
	PLAYER_INSTANCE_CHECK(player);
	player_s * handle = (player_s *) player;
	PLAYER_STATE_CHECK(handle,PLAYER_STATE_IDLE);

	if(handle->user_cb[_PLAYER_EVENT_TYPE_PREPARE])
	{
		LOGE("[%s] PLAYER_ERROR_INVALID_OPERATION (0x%08x) : preparing... we can't do any more " ,__FUNCTION__, PLAYER_ERROR_INVALID_OPERATION);
		return PLAYER_ERROR_INVALID_OPERATION;
	}
	else
	{
		LOGI("[%s] Event type : %d ",__FUNCTION__, _PLAYER_EVENT_TYPE_PREPARE);
		handle->user_cb[_PLAYER_EVENT_TYPE_PREPARE] = callback;
		handle->user_data[_PLAYER_EVENT_TYPE_PREPARE] = user_data;
	}

	int ret;
	ret = mm_player_set_message_callback(handle->mm_handle, __msg_callback, (void*)handle);
	if(ret != MM_ERROR_NONE)
	{
		LOGW("[%s] Failed to set message callback function (0x%x)" ,__FUNCTION__, ret);
	}

	if (handle->display_type==((int)MM_DISPLAY_SURFACE_NULL))
	{
		ret = mm_player_set_attribute(handle->mm_handle, NULL, "display_surface_type", MM_DISPLAY_SURFACE_NULL, (char*)NULL);
		if(ret != MM_ERROR_NONE)
		{
			LOGW("[%s] Failed to set display surface type 'MM_DISPLAY_SURFACE_NULL' (0x%x)" ,__FUNCTION__, ret);
		}
	}
	else
	{
		ret = mm_player_set_attribute(handle->mm_handle, NULL,"display_visible", 0, (char*)NULL);
                if(ret != MM_ERROR_NONE)
                {
                        LOGW("[%s] Failed to set display visible (0x%x)" ,__FUNCTION__, ret);
                }
	}

	ret = mm_player_set_attribute(handle->mm_handle, NULL, "profile_prepare_async", 1, (char*)NULL);
	if(ret != MM_ERROR_NONE)
	{
		LOGE("[%s] Failed to set profile_async_start '1' (0x%x)" ,__FUNCTION__, ret);
	}

	ret = mm_player_realize(handle->mm_handle);
	if(ret != MM_ERROR_NONE)
	{
		LOGE("[%s] Failed to realize - 0x%x", __FUNCTION__, ret);
		return __convert_error_code(ret,(char*)__FUNCTION__);
	}

	if (!handle->is_progressive_download)
	{
		ret = pthread_create(&handle->prepare_async_thread, NULL,
			(void *)__prepare_async_thread_func, (void *)handle);

		if (ret != 0)
		{
			LOGE("[%s] failed to create thread ret = %d", __FUNCTION__, ret);
			return PLAYER_ERROR_OUT_OF_MEMORY;
		}
	}

	LOGE("[%s] End", __FUNCTION__);
	return PLAYER_ERROR_NONE;
}

int player_prepare (player_h player)
{
	LOGE("[%s] Start", __FUNCTION__);
	PLAYER_INSTANCE_CHECK(player);
	MMTA_ACUM_ITEM_BEGIN("[CoreAPI] player_prepare", 0);
	player_s * handle = (player_s *) player;
	PLAYER_STATE_CHECK(handle,PLAYER_STATE_IDLE);

	int ret;
	ret = mm_player_set_message_callback(handle->mm_handle, __msg_callback, (void*)handle);
	if(ret != MM_ERROR_NONE)
	{
		LOGW("[%s] Failed to set message callback function (0x%x)" ,__FUNCTION__, ret);
	}

	if (handle->display_type==((int)MM_DISPLAY_SURFACE_NULL))
	{
		ret = mm_player_set_attribute(handle->mm_handle, NULL, "display_surface_type", MM_DISPLAY_SURFACE_NULL, (char*)NULL);
		if(ret != MM_ERROR_NONE)
		{
			LOGW("[%s] Failed to set display surface type 'MM_DISPLAY_SURFACE_NULL' (0x%x)" ,__FUNCTION__, ret);
		}
	}
	else
	{
		ret = mm_player_set_attribute(handle->mm_handle, NULL,"display_visible" , 0, (char*)NULL);
		if(ret != MM_ERROR_NONE)
		{
			LOGW("[%s] Failed to set display display_visible '0' (0x%x)" ,__FUNCTION__, ret);
		}
	}

	ret = mm_player_realize(handle->mm_handle);
	if(ret != MM_ERROR_NONE)
	{
		LOGE("[%s] Failed to realize - 0x%x", __FUNCTION__,ret);
		return __convert_error_code(ret,(char*)__FUNCTION__);
	}

	if (!handle->is_progressive_download)
		ret = mm_player_pause(handle->mm_handle);

	MMTA_ACUM_ITEM_END("[CoreAPI] player_prepare", 0);
	if(ret != MM_ERROR_NONE)
	{
		LOGE("[%s] Failed to pause - 0x%x", __FUNCTION__,ret);
		return __convert_error_code(ret,(char*)__FUNCTION__);
	}
	else
	{
		handle->state = PLAYER_STATE_READY;
		LOGE("[%s] End", __FUNCTION__);
		return PLAYER_ERROR_NONE;
	}
}

int player_unprepare (player_h player)
{
	LOGE("[%s] Start", __FUNCTION__);
	PLAYER_INSTANCE_CHECK(player);
	player_s * handle = (player_s *) player;
	if (!__player_state_validate(handle, PLAYER_STATE_READY))
	{
		LOGE("[%s] PLAYER_ERROR_INVALID_STATE(0x%08x) : current state - %d" ,__FUNCTION__,PLAYER_ERROR_INVALID_STATE, handle->state);
		return PLAYER_ERROR_INVALID_STATE;
	}

	int ret = mm_player_unrealize(handle->mm_handle);
	if(ret != MM_ERROR_NONE)
	{
		return __convert_error_code(ret,(char*)__FUNCTION__);
	}
	else
	{
		handle->state = PLAYER_STATE_IDLE;
		handle->display_type = MM_DISPLAY_SURFACE_NULL; // means DISPLAY_TYPE_NONE(3)
		handle->is_set_pixmap_cb = false;
		handle->is_stopped=false;
		handle->is_display_visible=true;
		handle->is_progressive_download=false;
		LOGE("[%s] End", __FUNCTION__);
		return PLAYER_ERROR_NONE;
	}
}

int player_set_uri (player_h player, const char *uri)
{
	PLAYER_INSTANCE_CHECK(player);
	PLAYER_NULL_ARG_CHECK(uri);
	player_s * handle = (player_s *) player;
	PLAYER_STATE_CHECK(handle,PLAYER_STATE_IDLE);

	int uri_length = strlen(uri);
	int ret = mm_player_set_attribute(handle->mm_handle, NULL,MM_PLAYER_CONTENT_URI , uri, uri_length, (char*)NULL);
	if(ret != MM_ERROR_NONE)
	{
		return __convert_error_code(ret,(char*)__FUNCTION__);
	}
	else
		return PLAYER_ERROR_NONE;
}

int player_set_memory_buffer (player_h player, const void *data, int size)
{
	PLAYER_INSTANCE_CHECK(player);
	PLAYER_NULL_ARG_CHECK(data);
	PLAYER_CHECK_CONDITION(size>=0,PLAYER_ERROR_INVALID_PARAMETER,"PLAYER_ERROR_INVALID_PARAMETER" );
	player_s * handle = (player_s *) player;
	PLAYER_STATE_CHECK(handle,PLAYER_STATE_IDLE);

	char uri[PATH_MAX] ;

	snprintf(uri, sizeof(uri),"mem:///ext=%s,size=%d","", size);
	int ret = mm_player_set_attribute(handle->mm_handle, NULL,MM_PLAYER_CONTENT_URI, uri, strlen(uri), MM_PLAYER_MEMORY_SRC, data,size,(char*)NULL);
	if(ret != MM_ERROR_NONE)
	{
		return __convert_error_code(ret,(char*)__FUNCTION__);
	}
	else
		return PLAYER_ERROR_NONE;
}

int player_get_state (player_h player, player_state_e *state)
{
	PLAYER_INSTANCE_CHECK(player);
	PLAYER_NULL_ARG_CHECK(state);
	player_s * handle = (player_s *) player;
	*state = handle->state;
	MMPlayerStateType currentStat = MM_PLAYER_STATE_NULL;
	mm_player_get_state(handle->mm_handle, &currentStat);
	LOGI("[%s] State : %d (FW state : %d)", __FUNCTION__,handle->state, currentStat);
	return PLAYER_ERROR_NONE;
}

int player_set_volume (player_h player, float left, float right)
{
	PLAYER_INSTANCE_CHECK(player);
	PLAYER_CHECK_CONDITION(left>=0 && left <= 1.0 ,PLAYER_ERROR_INVALID_PARAMETER,"PLAYER_ERROR_INVALID_PARAMETER" );
	PLAYER_CHECK_CONDITION(right>=0 && right <= 1.0 ,PLAYER_ERROR_INVALID_PARAMETER, "PLAYER_ERROR_INVALID_PARAMETER" );
	player_s * handle = (player_s *) player;
	MMPlayerVolumeType vol;
	vol.level[MM_VOLUME_CHANNEL_LEFT] = left;
	vol.level[MM_VOLUME_CHANNEL_RIGHT] = right;
	int ret = mm_player_set_volume(handle->mm_handle,&vol);
	if(ret != MM_ERROR_NONE)
	{
		return __convert_error_code(ret,(char*)__FUNCTION__);
	}
	else
	{
		return PLAYER_ERROR_NONE;
	}
}

int player_get_volume (player_h player, float *left, float *right)
{
	PLAYER_INSTANCE_CHECK(player);
	PLAYER_NULL_ARG_CHECK(left);
	PLAYER_NULL_ARG_CHECK(right);
	player_s * handle = (player_s *) player;
	MMPlayerVolumeType vol;
	int ret = mm_player_get_volume(handle->mm_handle,&vol);
	if(ret != MM_ERROR_NONE)
	{
		return __convert_error_code(ret,(char*)__FUNCTION__);
	}
	else
	{
		*left = vol.level[MM_VOLUME_CHANNEL_LEFT];
		*right = vol.level[MM_VOLUME_CHANNEL_RIGHT];
		return PLAYER_ERROR_NONE;
	}
}

int player_set_sound_type(player_h player, sound_type_e type)
{
	PLAYER_INSTANCE_CHECK(player);
	player_s * handle = (player_s *) player;

	if (!__player_state_validate(handle, PLAYER_STATE_IDLE))
	{
		LOGE("[%s] PLAYER_ERROR_INVALID_STATE(0x%08x) : current state - %d" ,__FUNCTION__,PLAYER_ERROR_INVALID_STATE, handle->state);
		return PLAYER_ERROR_INVALID_STATE;
	}

	int ret = mm_player_set_attribute(handle->mm_handle, NULL,"sound_volume_type" , type, (char*)NULL);
	if(ret != MM_ERROR_NONE)
	{
		return __convert_error_code(ret,(char*)__FUNCTION__);
	}
	else
		return PLAYER_ERROR_NONE;
}

int player_set_audio_latency_mode(player_h player, audio_latency_mode_e latency_mode)
{
	PLAYER_INSTANCE_CHECK(player);
	player_s * handle = (player_s *) player;

	int ret = mm_player_set_attribute(handle->mm_handle, NULL,"audio_latency_mode" , latency_mode, (char*)NULL);
	if(ret != MM_ERROR_NONE)
		return __convert_error_code(ret,(char*)__FUNCTION__);
	else
		return PLAYER_ERROR_NONE;
}

int player_get_audio_latency_mode(player_h player, audio_latency_mode_e *latency_mode)
{
	PLAYER_INSTANCE_CHECK(player);
	PLAYER_NULL_ARG_CHECK(latency_mode);
	player_s * handle = (player_s *) player;

	int ret = mm_player_get_attribute(handle->mm_handle, NULL,"audio_latency_mode" , latency_mode, (char*)NULL);
	if(ret != MM_ERROR_NONE)
		return __convert_error_code(ret,(char*)__FUNCTION__);
	else
		return PLAYER_ERROR_NONE;
}

int player_start (player_h player)
{
	LOGE("[%s] Start", __FUNCTION__);
	PLAYER_INSTANCE_CHECK(player);
	MMTA_ACUM_ITEM_BEGIN("[CAPI] player_start only", 0);
	MMTA_ACUM_ITEM_BEGIN("[CAPI] player_start ~ BOS", 0);
	player_s * handle = (player_s *) player;
	int ret;
	if ( handle->state  ==PLAYER_STATE_READY || handle->state ==PLAYER_STATE_PAUSED)
	{
		if(handle->display_type == PLAYER_DISPLAY_TYPE_X11 || handle->display_type == PLAYER_DISPLAY_TYPE_EVAS)
		{
			if(handle->is_display_visible)
			{
				ret = mm_player_set_attribute(handle->mm_handle, NULL,"display_visible" , 1, (char*)NULL);
				LOGE("[%s] show video display : %d",__FUNCTION__, ret);
			}
		}

		if(handle->is_stopped)
		{
			if (handle->is_progressive_download)
			{
				LOGE("[%s] PLAYER_ERROR_INVALID_OPERATION(0x%08x)" ,__FUNCTION__,PLAYER_ERROR_INVALID_OPERATION);
				return PLAYER_ERROR_INVALID_OPERATION;
			}

			ret = mm_player_start(handle->mm_handle);
			LOGE("[%s] stop -> start() ",__FUNCTION__);
		}
		else
		{
			if (handle->is_progressive_download && handle->state  ==PLAYER_STATE_READY)
				ret = mm_player_start(handle->mm_handle);
			else
				ret = mm_player_resume(handle->mm_handle);
		}
	}
	else
	{
		LOGE("[%s] PLAYER_ERROR_INVALID_STATE(0x%08x) : current state - %d" ,__FUNCTION__,PLAYER_ERROR_INVALID_STATE, handle->state);
		return PLAYER_ERROR_INVALID_STATE;
	}

	if(ret != MM_ERROR_NONE)
	{
		return __convert_error_code(ret,(char*)__FUNCTION__);
	}
	else
	{
		handle->is_stopped = FALSE;
		handle->state = PLAYER_STATE_PLAYING;
		LOGE("[%s] End", __FUNCTION__);
		return PLAYER_ERROR_NONE;
	}
}

int player_stop (player_h player)
{
	LOGE("[%s] Start", __FUNCTION__);
	PLAYER_INSTANCE_CHECK(player);
	player_s * handle = (player_s *) player;
	if (handle->state == PLAYER_STATE_PLAYING || handle->state == PLAYER_STATE_PAUSED)
	{
		int ret = mm_player_stop(handle->mm_handle);
		if(ret != MM_ERROR_NONE)
		{
			return __convert_error_code(ret,(char*)__FUNCTION__);
		}
		if(handle->display_type == PLAYER_DISPLAY_TYPE_X11 || handle->display_type == PLAYER_DISPLAY_TYPE_EVAS)
		{
			ret = mm_player_set_attribute(handle->mm_handle, NULL,"display_visible" , 0, (char*)NULL);
			if(ret != MM_ERROR_NONE)
			{
				return __convert_error_code(ret,(char*)__FUNCTION__);
			}
			LOGE("[%s] show video display : %d",__FUNCTION__, ret);
		}

		handle->state = PLAYER_STATE_READY;
		handle->is_stopped = TRUE;
		LOGE("[%s] End", __FUNCTION__);
		return PLAYER_ERROR_NONE;
	}
	else
	{
		LOGE("[%s] PLAYER_ERROR_INVALID_STATE(0x%08x) : current state - %d" ,__FUNCTION__,PLAYER_ERROR_INVALID_STATE, handle->state);
		return PLAYER_ERROR_INVALID_STATE;
	}
}

int player_pause (player_h player)
{
	LOGE("[%s] Start", __FUNCTION__);
	PLAYER_INSTANCE_CHECK(player);
	player_s * handle = (player_s *) player;
	PLAYER_STATE_CHECK(handle,PLAYER_STATE_PLAYING);

	int ret = mm_player_pause(handle->mm_handle);
	if(ret != MM_ERROR_NONE)
	{
		return __convert_error_code(ret,(char*)__FUNCTION__);
	}
	else
	{
		handle->state = PLAYER_STATE_PAUSED;
		LOGE("[%s] End", __FUNCTION__);
		return PLAYER_ERROR_NONE;
	}
}

int player_set_position (player_h player, int millisecond, player_seek_completed_cb callback, void *user_data)
{
	PLAYER_INSTANCE_CHECK(player);
	PLAYER_CHECK_CONDITION(millisecond>=0  ,PLAYER_ERROR_INVALID_PARAMETER ,"PLAYER_ERROR_INVALID_PARAMETER" );

	player_s * handle = (player_s *) player;
	if (!__player_state_validate(handle, PLAYER_STATE_READY))
	{
		LOGE("[%s] PLAYER_ERROR_INVALID_STATE(0x%08x) : current state - %d" ,__FUNCTION__,PLAYER_ERROR_INVALID_STATE, handle->state);
		return PLAYER_ERROR_INVALID_STATE;
	}

	if(handle->user_cb[_PLAYER_EVENT_TYPE_SEEK])
	{
		LOGE("[%s] PLAYER_ERROR_SEEK_FAILED (0x%08x) : seeking... we can't do any more " ,__FUNCTION__, PLAYER_ERROR_SEEK_FAILED);
		return PLAYER_ERROR_SEEK_FAILED;
	}
	else
	{
		LOGE("[%s] Event type : %d ",__FUNCTION__, _PLAYER_EVENT_TYPE_SEEK);
		handle->user_cb[_PLAYER_EVENT_TYPE_SEEK] = callback;
		handle->user_data[_PLAYER_EVENT_TYPE_SEEK] = user_data;
	}
	int ret = mm_player_set_attribute(handle->mm_handle, NULL, "accurate_seek", 1, (char*)NULL);
	if(ret != MM_ERROR_NONE)
	{
		return __convert_error_code(ret,(char*)__FUNCTION__);
	}

	ret = mm_player_set_position(handle->mm_handle, MM_PLAYER_POS_FORMAT_TIME, millisecond);
	if(ret != MM_ERROR_NONE)
	{
		return __convert_error_code(ret,(char*)__FUNCTION__);
	}
	else
	{
		return PLAYER_ERROR_NONE;
	}
}

int player_seek (player_h player, int millisecond, bool accurate, player_seek_completed_cb callback, void *user_data)
{
	PLAYER_INSTANCE_CHECK(player);
	PLAYER_CHECK_CONDITION(millisecond>=0  ,PLAYER_ERROR_INVALID_PARAMETER ,"PLAYER_ERROR_INVALID_PARAMETER" );
	player_s * handle = (player_s *) player;
	if (!__player_state_validate(handle, PLAYER_STATE_READY))
	{
		LOGE("[%s] PLAYER_ERROR_INVALID_STATE(0x%08x) : current state - %d" ,__FUNCTION__,PLAYER_ERROR_INVALID_STATE, handle->state);
		return PLAYER_ERROR_INVALID_STATE;
	}

	if(handle->user_cb[_PLAYER_EVENT_TYPE_SEEK])
	{
		LOGE("[%s] PLAYER_ERROR_SEEK_FAILED (0x%08x) : seeking... we can't do any more " ,__FUNCTION__, PLAYER_ERROR_SEEK_FAILED);
		return PLAYER_ERROR_SEEK_FAILED;
	}
	else
	{
		LOGE("[%s] Event type : %d, pos : %d ",__FUNCTION__, _PLAYER_EVENT_TYPE_SEEK, millisecond);
		handle->user_cb[_PLAYER_EVENT_TYPE_SEEK] = callback;
		handle->user_data[_PLAYER_EVENT_TYPE_SEEK] = user_data;
	}
	int accurated = accurate?1:0;
	int ret = mm_player_set_attribute(handle->mm_handle, NULL, "accurate_seek", accurated, (char*)NULL);
	if(ret != MM_ERROR_NONE)
	{
		return __convert_error_code(ret,(char*)__FUNCTION__);
	}
	ret = mm_player_set_position(handle->mm_handle, MM_PLAYER_POS_FORMAT_TIME, millisecond);
	if(ret != MM_ERROR_NONE)
	{
		return __convert_error_code(ret,(char*)__FUNCTION__);
	}
	else
	{
		return PLAYER_ERROR_NONE;
	}
}

int player_set_position_ratio (player_h player, int percent, player_seek_completed_cb callback, void *user_data)
{
	PLAYER_INSTANCE_CHECK(player);
	PLAYER_CHECK_CONDITION(percent>=0 && percent <= 100 ,PLAYER_ERROR_INVALID_PARAMETER ,"PLAYER_ERROR_INVALID_PARAMETER" );

	player_s * handle = (player_s *) player;
	if(handle->user_cb[_PLAYER_EVENT_TYPE_SEEK])
	{
		LOGE("[%s] PLAYER_ERROR_SEEK_FAILED (0x%08x) : seeking... we can't do any more " ,__FUNCTION__, PLAYER_ERROR_SEEK_FAILED);
		return PLAYER_ERROR_SEEK_FAILED;
	}
	else
	{
		LOGI("[%s] Event type : %d ",__FUNCTION__, _PLAYER_EVENT_TYPE_SEEK);
		handle->user_cb[_PLAYER_EVENT_TYPE_SEEK] = callback;
		handle->user_data[_PLAYER_EVENT_TYPE_SEEK] = user_data;
	}

	int ret = mm_player_set_position(handle->mm_handle, MM_PLAYER_POS_FORMAT_PERCENT , percent);
	if(ret != MM_ERROR_NONE)
	{
		return __convert_error_code(ret,(char*)__FUNCTION__);
	}
	else
	{
		return PLAYER_ERROR_NONE;
	}
}

int player_get_position (player_h player, int *millisecond)
{
	PLAYER_INSTANCE_CHECK(player);
	PLAYER_NULL_ARG_CHECK(millisecond);
	player_s * handle = (player_s *) player;
	if (!__player_state_validate(handle, PLAYER_STATE_READY))
	{
		LOGE("[%s] PLAYER_ERROR_INVALID_STATE(0x%08x) : current state - %d" ,__FUNCTION__,PLAYER_ERROR_INVALID_STATE, handle->state);
		return PLAYER_ERROR_INVALID_STATE;
	}
	int pos;
	int ret = mm_player_get_position(handle->mm_handle, MM_PLAYER_POS_FORMAT_TIME , &pos);
	if(ret != MM_ERROR_NONE)
	{
		return __convert_error_code(ret,(char*)__FUNCTION__);
	}
	else
	{
		*millisecond = pos;
		return PLAYER_ERROR_NONE;
	}
}

int player_get_position_ratio (player_h player,int *percent)
{
	PLAYER_INSTANCE_CHECK(player);
	PLAYER_NULL_ARG_CHECK(percent);
	player_s * handle = (player_s *) player;
	if (!__player_state_validate(handle, PLAYER_STATE_READY))
	{
		LOGE("[%s] PLAYER_ERROR_INVALID_STATE(0x%08x) : current state - %d" ,__FUNCTION__,PLAYER_ERROR_INVALID_STATE, handle->state);
		return PLAYER_ERROR_INVALID_STATE;
	}
	int pos;
	int ret = mm_player_get_position(handle->mm_handle, MM_PLAYER_POS_FORMAT_PERCENT, &pos);
	if(ret != MM_ERROR_NONE)
	{
		return __convert_error_code(ret,(char*)__FUNCTION__);
	}
	else
	{
		*percent=pos;
		return PLAYER_ERROR_NONE;
	}
}

int player_set_mute (player_h player, bool muted)
{
	PLAYER_INSTANCE_CHECK(player);
	player_s * handle = (player_s *) player;

	int ret = mm_player_set_mute(handle->mm_handle, muted);
	if(ret != MM_ERROR_NONE)
	{
		return __convert_error_code(ret,(char*)__FUNCTION__);
	}
	else
	{
		return PLAYER_ERROR_NONE;
	}
}

int player_is_muted (player_h player, bool *muted)
{
	PLAYER_INSTANCE_CHECK(player);
	PLAYER_NULL_ARG_CHECK(muted);
	player_s * handle = (player_s *) player;

	int _mute;
	int ret = mm_player_get_mute(handle->mm_handle, &_mute);
	if(ret != MM_ERROR_NONE)
	{
		return __convert_error_code(ret,(char*)__FUNCTION__);
	}
	else
	{
		if(_mute)
		{
			*muted = TRUE;
		}
		else
		{
			*muted = FALSE;
		}
		return PLAYER_ERROR_NONE;
	}
}

int 	player_set_looping (player_h player, bool looping)
{
	PLAYER_INSTANCE_CHECK(player);
	player_s * handle = (player_s *) player;
	if (!__player_state_validate(handle, PLAYER_STATE_IDLE))
	{
		LOGE("[%s] PLAYER_ERROR_INVALID_STATE(0x%08x) : current state - %d" ,__FUNCTION__,PLAYER_ERROR_INVALID_STATE, handle->state);
		return PLAYER_ERROR_INVALID_STATE;
	}

	int value = 0;
	if(looping==TRUE)
	{
		value = -1;
	}
	int ret = mm_player_set_attribute(handle->mm_handle, NULL,MM_PLAYER_PLAYBACK_COUNT , value, (char*)NULL);

	if(ret != MM_ERROR_NONE)
	{
		return __convert_error_code(ret,(char*)__FUNCTION__);
	}
	else
	{
		return PLAYER_ERROR_NONE;
	}
}

int player_is_looping (player_h player, bool *looping)
{
	PLAYER_INSTANCE_CHECK(player);
	PLAYER_NULL_ARG_CHECK(looping);
	player_s * handle = (player_s *) player;
	if (!__player_state_validate(handle, PLAYER_STATE_IDLE))
	{
		LOGE("[%s] PLAYER_ERROR_INVALID_STATE(0x%08x) : current state - %d" ,__FUNCTION__,PLAYER_ERROR_INVALID_STATE, handle->state);
		return PLAYER_ERROR_INVALID_STATE;
	}
	int count;
	int ret = mm_player_get_attribute(handle->mm_handle, NULL,MM_PLAYER_PLAYBACK_COUNT , &count, (char*)NULL);
	if(ret != MM_ERROR_NONE)
	{
		return __convert_error_code(ret,(char*)__FUNCTION__);
	}
	else
	{
		if(count==-1)
		{
			*looping = TRUE;
		}
		else
		{
			*looping = FALSE;
		}
		return PLAYER_ERROR_NONE;
	}
}

int player_get_duration (player_h player, int *duration)
{
	PLAYER_INSTANCE_CHECK(player);
	PLAYER_NULL_ARG_CHECK(duration);
	player_s * handle = (player_s *) player;
	if (!__player_state_validate(handle, PLAYER_STATE_IDLE))
	{
		LOGE("[%s] PLAYER_ERROR_INVALID_STATE(0x%08x) : current state - %d" ,__FUNCTION__,PLAYER_ERROR_INVALID_STATE, handle->state);
		return PLAYER_ERROR_INVALID_STATE;
	}
	int _duration;
	int ret = mm_player_get_attribute(handle->mm_handle, NULL,MM_PLAYER_CONTENT_DURATION, &_duration, (char*)NULL);
	if(ret != MM_ERROR_NONE)
	{
		return __convert_error_code(ret,(char*)__FUNCTION__);
	}
	else
	{
		*duration = _duration;
		LOGI("[%s] duration : %d",__FUNCTION__,_duration);
		return PLAYER_ERROR_NONE;
	}
}

int player_set_display(player_h player, player_display_type_e type, player_display_h display)
{
	PLAYER_INSTANCE_CHECK(player);
	player_s * handle = (player_s *) player;

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

	void* temp;
	if (handle->display_type == (int)MM_DISPLAY_SURFACE_NULL || type == handle->display_type) // first time or same type
	{
		temp = handle->display_handle;
		handle->display_handle = display;
		ret = mm_player_set_attribute(handle->mm_handle, NULL, "display_surface_type", type, "display_overlay" , type == PLAYER_DISPLAY_TYPE_X11 ? &handle->display_handle : display, sizeof(display), (char*)NULL);
		if (ret != MM_ERROR_NONE)
		{
			handle->display_handle = temp;
			LOGE("[%s] Failed to display surface change :%d",__FUNCTION__,ret);
		}
		else
		{
			handle->display_type = type;
			LOGE("[%s] video display has been changed- type :%d, addr : 0x%x",__FUNCTION__,handle->display_type, handle->display_handle);
		}
		LOGE("[%s] video display has been updated - type :%d",__FUNCTION__,type);
	}
	else //changing surface case
	{
		temp = handle->display_handle;
		handle->display_handle = display;

		ret = mm_player_change_videosink(handle->mm_handle, type, type == PLAYER_DISPLAY_TYPE_X11 ? &handle->display_handle : display);
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
			LOGE("[%s] video display has been changed- type :%d, addr : 0x%x",__FUNCTION__,handle->display_type, handle->display_handle);
		}
	}

	if(ret != MM_ERROR_NONE)
	{
		handle->display_type = MM_DISPLAY_SURFACE_NULL;
		return __convert_error_code(ret,(char*)__FUNCTION__);
	}
	else
	{
		LOGE("[%s] End",__FUNCTION__);
		return PLAYER_ERROR_NONE;
	}
}

int player_is_display_mode_changeable(player_h player, bool* changeable)
{
	PLAYER_INSTANCE_CHECK(player);
	PLAYER_NULL_ARG_CHECK(changeable);
	player_s * handle = (player_s *) player;
	switch(handle->display_type)
	{
		case PLAYER_DISPLAY_TYPE_X11:
			*changeable = TRUE;
			break;
		case PLAYER_DISPLAY_TYPE_EVAS:
		{
			char *sink_name = NULL;
			int length;
			int scaling;
			int ret = mm_player_get_attribute(handle->mm_handle, NULL,"display_evas_surface_sink" ,&sink_name, &length, "display_evas_do_scaling", &scaling, (char*)NULL);
			if(ret != MM_ERROR_NONE)
				*changeable = FALSE;
			if (!strncmp(sink_name,"evaspixmapsink",length) && scaling==1)
			{
				*changeable = TRUE;
			}
			break;
		}
		default:
			*changeable = FALSE;
			break;
	}
	return PLAYER_ERROR_NONE;
}

int player_set_display_mode(player_h player, player_display_mode_e mode)
{
	PLAYER_INSTANCE_CHECK(player);
	player_s * handle = (player_s *) player;

	int ret = mm_player_set_attribute(handle->mm_handle, NULL,"display_method" , mode, (char*)NULL);
	if(ret != MM_ERROR_NONE)
	{
		return __convert_error_code(ret,(char*)__FUNCTION__);
	}
	else
		return PLAYER_ERROR_NONE;
}

int player_get_display_mode(player_h player, player_display_mode_e *mode)
{
	PLAYER_INSTANCE_CHECK(player);
	PLAYER_NULL_ARG_CHECK(mode);
	player_s * handle = (player_s *) player;
	int ret = mm_player_get_attribute(handle->mm_handle, NULL,"display_method"  ,mode, (char*)NULL);
	if(ret != MM_ERROR_NONE)
	{
		return __convert_error_code(ret,(char*)__FUNCTION__);
	}
	else
	{
		return PLAYER_ERROR_NONE;
	}
}

int player_set_playback_rate(player_h player, float rate)
{
	LOGE("[%s] rate : %0.1f", __FUNCTION__, rate);
	PLAYER_INSTANCE_CHECK(player);
	PLAYER_CHECK_CONDITION(rate>=-5.0 && rate <= 5.0 ,PLAYER_ERROR_INVALID_PARAMETER,"PLAYER_ERROR_INVALID_PARAMETER" );
	player_s * handle = (player_s *) player;

	if (!__player_state_validate(handle, PLAYER_STATE_READY))
	{
		LOGE("[%s] PLAYER_ERROR_INVALID_STATE(0x%08x) : current state - %d" ,__FUNCTION__,PLAYER_ERROR_INVALID_STATE, handle->state);
		return PLAYER_ERROR_INVALID_STATE;
	}

	int ret = mm_player_set_play_speed(handle->mm_handle, rate);

	switch (ret)
	{
		case MM_ERROR_NONE:
		case MM_ERROR_PLAYER_NO_OP:
			ret = PLAYER_ERROR_NONE;
			break;
		case MM_ERROR_NOT_SUPPORT_API:
		case MM_ERROR_PLAYER_SEEK:
			LOGE("[%s] PLAYER_ERROR_INVALID_OPERATION(0x%08x) : seek error",__FUNCTION__, PLAYER_ERROR_INVALID_OPERATION);
			ret = PLAYER_ERROR_INVALID_OPERATION;
			break;
		default:
			return __convert_error_code(ret,(char*)__FUNCTION__);
	}
	return ret;
}


int player_set_x11_display_rotation(player_h player, player_display_rotation_e rotation)
{
	PLAYER_INSTANCE_CHECK(player);
	player_s * handle = (player_s *) player;

	int ret = mm_player_set_attribute(handle->mm_handle, NULL,MM_PLAYER_VIDEO_ROTATION , rotation, (char*)NULL);
	if(ret != MM_ERROR_NONE)
	{
		return __convert_error_code(ret,(char*)__FUNCTION__);
	}
	else
		return PLAYER_ERROR_NONE;
}

int player_get_x11_display_rotation( player_h player, player_display_rotation_e *rotation)
{
	PLAYER_INSTANCE_CHECK(player);
	PLAYER_NULL_ARG_CHECK(rotation);
	player_s * handle = (player_s *) player;
	int ret = mm_player_get_attribute(handle->mm_handle, NULL,MM_PLAYER_VIDEO_ROTATION ,rotation, (char*)NULL);
	if(ret != MM_ERROR_NONE)
	{
		return __convert_error_code(ret,(char*)__FUNCTION__);
	}
	else
	{
		return PLAYER_ERROR_NONE;
	}
}

int player_set_x11_display_visible(player_h player, bool visible)
{
	PLAYER_INSTANCE_CHECK(player);
	player_s * handle = (player_s *) player;

	int value = 0;
	if(visible==TRUE)
	{
		value = 1;
	}

	int ret = mm_player_set_attribute(handle->mm_handle, NULL,"display_visible" , value, (char*)NULL);
	if(ret != MM_ERROR_NONE)
	{
		return __convert_error_code(ret,(char*)__FUNCTION__);
	}
	else
	{
		handle->is_display_visible = visible;
		return PLAYER_ERROR_NONE;
	}
}

int player_is_x11_display_visible(player_h player, bool* visible)
{
	PLAYER_INSTANCE_CHECK(player);
	PLAYER_NULL_ARG_CHECK(visible);
	player_s * handle = (player_s *) player;
	int count;
	int ret = mm_player_get_attribute(handle->mm_handle, NULL,"display_visible" ,&count, (char*)NULL);
	if(ret != MM_ERROR_NONE)
	{
		return __convert_error_code(ret,(char*)__FUNCTION__);
	}
	else
	{
		if(count==0)
		{
			*visible = FALSE;
		}
		else
		{
			*visible = TRUE;
		}

		return PLAYER_ERROR_NONE;
	}
}

int player_set_x11_display_zoom(player_h player, int level)
{
	PLAYER_INSTANCE_CHECK(player);
	PLAYER_CHECK_CONDITION(level>0 && level < 10 ,PLAYER_ERROR_INVALID_PARAMETER,"PLAYER_ERROR_INVALID_PARAMETER" );

	player_s * handle = (player_s *) player;
	int ret = mm_player_set_attribute(handle->mm_handle, NULL,"display_zoom" , level, (char*)NULL);
	if(ret != MM_ERROR_NONE)
	{
		return __convert_error_code(ret,(char*)__FUNCTION__);
	}
	else
		return PLAYER_ERROR_NONE;
}

int player_get_x11_display_zoom( player_h player, int *level)
{
	PLAYER_INSTANCE_CHECK(player);
	PLAYER_NULL_ARG_CHECK(level);
	player_s * handle = (player_s *) player;
	int _level;
	int ret = mm_player_get_attribute(handle->mm_handle, NULL,"display_zoom" , &_level, (char*)NULL);
	if(ret != MM_ERROR_NONE)
	{
		*level=-1;
		return __convert_error_code(ret,(char*)__FUNCTION__);
	}
	else
	{
		*level = _level;
		return PLAYER_ERROR_NONE;
	}
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
		return __convert_error_code(ret,(char*)__FUNCTION__);
	}
	else
	{
		handle->is_set_pixmap_cb = true;
		handle->display_type = MM_DISPLAY_SURFACE_X;
		handle->display_handle = callback;
		LOGE("[%s] video display has been changed- type :%d, pixmap_callback addr : 0x%x",__FUNCTION__,handle->display_type, handle->display_handle);
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
		return __convert_error_code(ret,(char*)__FUNCTION__);
	}
	else
	{
		LOGE("[%s] set pixmap_error_cb(0x%08x) and user_data(0x%8x)" ,__FUNCTION__, callback, user_data);
		handle->user_cb[_PLAYER_EVENT_TYPE_VIDEO_FRAME_RENDER_ERROR] = callback;
		handle->user_data[_PLAYER_EVENT_TYPE_VIDEO_FRAME_RENDER_ERROR] = user_data;
		LOGI("[%s] Event type : %d ",__FUNCTION__, _PLAYER_EVENT_TYPE_VIDEO_FRAME_RENDER_ERROR);
		return PLAYER_ERROR_NONE;
	}
}

int player_enable_evas_display_scaling(player_h player, bool enable)
{
	PLAYER_INSTANCE_CHECK(player);
	player_s * handle = (player_s *) player;

	int scaling = enable?1:0;
	int ret = mm_player_set_attribute(handle->mm_handle, NULL,"display_evas_do_scaling" , scaling, (char*)NULL);
	if(ret != MM_ERROR_NONE)
	{
		return __convert_error_code(ret,(char*)__FUNCTION__);
	}
	else
		return PLAYER_ERROR_NONE;
}

int player_get_content_info(player_h player, player_content_info_e key, char ** value)
{
	PLAYER_INSTANCE_CHECK(player);
	PLAYER_NULL_ARG_CHECK(value);
	player_s * handle = (player_s *) player;
	if (!__player_state_validate(handle, PLAYER_STATE_READY))
	{
		LOGE("[%s] PLAYER_ERROR_INVALID_STATE (0x%08x) :  current state - %d" ,__FUNCTION__, PLAYER_ERROR_INVALID_STATE, handle->state);
		return PLAYER_ERROR_INVALID_STATE;
	}

	char* attr=NULL;
	char* val=NULL;
	int val_len=0;

	switch(key)
	{
		case PLAYER_CONTENT_INFO_ALBUM:
			attr = MM_PLAYER_TAG_ALBUM;
			break;
		case PLAYER_CONTENT_INFO_ARTIST:
			attr = MM_PLAYER_TAG_ARTIST;
			break;
		case PLAYER_CONTENT_INFO_AUTHOR:
			attr = MM_PLAYER_TAG_AUTHOUR;
			break;
		case PLAYER_CONTENT_INFO_GENRE:
			attr = MM_PLAYER_TAG_GENRE;
			break;
		case PLAYER_CONTENT_INFO_TITLE:
			attr = MM_PLAYER_TAG_TITLE;
			break;
		case PLAYER_CONTENT_INFO_YEAR:
			attr = MM_PLAYER_TAG_DATE;
			break;
		default:
				attr=NULL;
	}

	int ret = mm_player_get_attribute(handle->mm_handle, NULL,attr ,&val, &val_len, (char*)NULL);
	if(ret != MM_ERROR_NONE)
	{
		return __convert_error_code(ret,(char*)__FUNCTION__);
	}
	else
	{
		*value = NULL;
		if(val!=NULL)
			*value = strndup(val,val_len);
		else
			*value = strndup("",0);

		if (*value == NULL)
		{
			LOGE("[%s] PLAYER_ERROR_OUT_OF_MEMORY(0x%08x) : fail to strdup ", __FUNCTION__,PLAYER_ERROR_OUT_OF_MEMORY);
			return PLAYER_ERROR_OUT_OF_MEMORY;
		}
		return PLAYER_ERROR_NONE;
	}
}

int player_get_codec_info(player_h player, char **audio_codec, char **video_codec)
{
	PLAYER_INSTANCE_CHECK(player);
	PLAYER_NULL_ARG_CHECK(audio_codec);
	PLAYER_NULL_ARG_CHECK(video_codec);
	player_s * handle = (player_s *) player;
	if (!__player_state_validate(handle, PLAYER_STATE_READY))
	{
		LOGE("[%s] PLAYER_ERROR_INVALID_STATE (0x%08x) :  current state - %d" ,__FUNCTION__, PLAYER_ERROR_INVALID_STATE, handle->state);
		return PLAYER_ERROR_INVALID_STATE;
	}

	char* audio=NULL;
	int audio_len=0;
	char* video=NULL;
	int video_len=0;

	int ret = mm_player_get_attribute(handle->mm_handle, NULL,MM_PLAYER_AUDIO_CODEC,&audio,&audio_len,MM_PLAYER_VIDEO_CODEC,&video,&video_len,(char*)NULL);
	if(ret != MM_ERROR_NONE)
	{
		return __convert_error_code(ret,(char*)__FUNCTION__);
	}
	else
	{
		*audio_codec = NULL;
		if(audio!=NULL)
			*audio_codec = strndup(audio,audio_len);
		else
			*audio_codec = strndup("",0);

		*video_codec = NULL;
		if(video!=NULL)
			*video_codec = strndup(video,video_len);
		else
			*video_codec = strndup("",0);

		return PLAYER_ERROR_NONE;
	}
}

int player_get_audio_stream_info(player_h player, int *sample_rate, int *channel, int *bit_rate)
{
	PLAYER_INSTANCE_CHECK(player);
	PLAYER_NULL_ARG_CHECK(sample_rate);
	PLAYER_NULL_ARG_CHECK(channel);
	PLAYER_NULL_ARG_CHECK(bit_rate);
	player_s * handle = (player_s *) player;
	if (!__player_state_validate(handle, PLAYER_STATE_READY))
	{
		LOGE("[%s] PLAYER_ERROR_INVALID_STATE (0x%08x) :  current state - %d" ,__FUNCTION__, PLAYER_ERROR_INVALID_STATE, handle->state);
		return PLAYER_ERROR_INVALID_STATE;
	}
	int ret = mm_player_get_attribute(handle->mm_handle, NULL,MM_PLAYER_AUDIO_SAMPLERATE,sample_rate,MM_PLAYER_AUDIO_CHANNEL,channel,MM_PLAYER_AUDIO_BITRATE,bit_rate,(char*)NULL);
	if(ret != MM_ERROR_NONE)
	{
		return __convert_error_code(ret,(char*)__FUNCTION__);
	}
	return PLAYER_ERROR_NONE;
}

int player_get_video_stream_info(player_h player, int *fps, int *bit_rate)
{
	PLAYER_INSTANCE_CHECK(player);
	PLAYER_NULL_ARG_CHECK(fps);
	PLAYER_NULL_ARG_CHECK(bit_rate);
	player_s * handle = (player_s *) player;
	if (!__player_state_validate(handle, PLAYER_STATE_READY))
	{
		LOGE("[%s] PLAYER_ERROR_INVALID_STATE (0x%08x) :  current state - %d" ,__FUNCTION__, PLAYER_ERROR_INVALID_STATE, handle->state);
		return PLAYER_ERROR_INVALID_STATE;
	}
	int ret = mm_player_get_attribute(handle->mm_handle, NULL,"content_video_fps",fps,"content_video_bitrate",bit_rate,(char*)NULL);
	if(ret != MM_ERROR_NONE)
	{
		return __convert_error_code(ret,(char*)__FUNCTION__);
	}
	return PLAYER_ERROR_NONE;
}

int player_get_video_size (player_h player, int *width, int *height)
{
	PLAYER_INSTANCE_CHECK(player);
	PLAYER_NULL_ARG_CHECK(width);
	PLAYER_NULL_ARG_CHECK(height);
	player_s * handle = (player_s *) player;
	if (!__player_state_validate(handle, PLAYER_STATE_READY))
	{
		LOGE("[%s] PLAYER_ERROR_INVALID_STATE (0x%08x) :  current state - %d" ,__FUNCTION__, PLAYER_ERROR_INVALID_STATE, handle->state);
		return PLAYER_ERROR_INVALID_STATE;
	}
	int w;
	int h;
	int ret = mm_player_get_attribute(handle->mm_handle, NULL,MM_PLAYER_VIDEO_WIDTH ,&w,  MM_PLAYER_VIDEO_HEIGHT, &h, (char*)NULL);
	if(ret != MM_ERROR_NONE)
	{
		return __convert_error_code(ret,(char*)__FUNCTION__);
	}
	else
	{
		*width = w;
		*height = h;
		LOGE("[%s] width : %d, height : %d",__FUNCTION__,w, h);
		return PLAYER_ERROR_NONE;
	}
}

int player_get_album_art(player_h player, void **album_art, int *size)
{
	PLAYER_INSTANCE_CHECK(player);
	PLAYER_NULL_ARG_CHECK(size);
	player_s * handle = (player_s *) player;
	if (!__player_state_validate(handle, PLAYER_STATE_READY))
	{
		LOGE("[%s] PLAYER_ERROR_INVALID_STATE (0x%08x) :  current state - %d" ,__FUNCTION__, PLAYER_ERROR_INVALID_STATE, handle->state);
		return PLAYER_ERROR_INVALID_STATE;
	}

	int ret = mm_player_get_attribute(handle->mm_handle, NULL,"tag_album_cover",album_art,size,(char*)NULL);
	if(ret != MM_ERROR_NONE)
	{
		return __convert_error_code(ret,(char*)__FUNCTION__);
	}
	return PLAYER_ERROR_NONE;
}

int player_get_track_count(player_h player, player_track_type_e type, int *count)
{
	PLAYER_INSTANCE_CHECK(player);
	PLAYER_NULL_ARG_CHECK(count);
	player_s * handle = (player_s *) player;
	if (!__player_state_validate(handle, PLAYER_STATE_READY))
	{
		LOGE("[%s] PLAYER_ERROR_INVALID_STATE (0x%08x) :  current state - %d" ,__FUNCTION__, PLAYER_ERROR_INVALID_STATE, handle->state);
		return PLAYER_ERROR_INVALID_STATE;
	}
	int ret = mm_player_get_track_count(handle->mm_handle, type, count);
	if(ret != MM_ERROR_NONE)
	{
		return __convert_error_code(ret,(char*)__FUNCTION__);
	}
	return PLAYER_ERROR_NONE;
}

int player_audio_effect_set_value(player_h player, audio_effect_e effect, int value)
{
	PLAYER_INSTANCE_CHECK(player);
	player_s * handle = (player_s *) player;
	int ret = mm_player_audio_effect_custom_set_level(handle->mm_handle,effect,0,value);
	if(ret != MM_ERROR_NONE)
	{
		return __convert_error_code(ret,(char*)__FUNCTION__);
	}
	else
	{
		ret = mm_player_audio_effect_custom_apply(handle->mm_handle);
		return (ret==MM_ERROR_NONE)?PLAYER_ERROR_NONE:__convert_error_code(ret,(char*)__FUNCTION__);
	}
}

int player_audio_effect_get_value(player_h player, audio_effect_e effect, int *value)
{
	PLAYER_INSTANCE_CHECK(player);
	PLAYER_NULL_ARG_CHECK(value);
	player_s * handle = (player_s *) player;
	int ret = mm_player_audio_effect_custom_get_level(handle->mm_handle,effect,0,value);
	if(ret != MM_ERROR_NONE)
		return __convert_error_code(ret,(char*)__FUNCTION__);
	else
		return PLAYER_ERROR_NONE;
}

int player_audio_effect_clear(player_h player, audio_effect_e effect)
{
	PLAYER_INSTANCE_CHECK(player);
	player_s * handle = (player_s *) player;
	int ret = mm_player_audio_effect_custom_clear_ext_all(handle->mm_handle);
	if(ret != MM_ERROR_NONE)
	{
		return __convert_error_code(ret,(char*)__FUNCTION__);
	}
	else
	{
		ret = mm_player_audio_effect_custom_apply(handle->mm_handle);
		return (ret==MM_ERROR_NONE)?PLAYER_ERROR_NONE:__convert_error_code(ret,(char*)__FUNCTION__);
	}
}

int player_audio_effect_get_value_range(player_h player, audio_effect_e effect, int* min, int* max)
{
	PLAYER_INSTANCE_CHECK(player);
	PLAYER_NULL_ARG_CHECK(min);
	PLAYER_NULL_ARG_CHECK(max);
	player_s * handle = (player_s *) player;
	int ret = mm_player_audio_effect_custom_get_level_range(handle->mm_handle, effect, min, max);
	if(ret != MM_ERROR_NONE)
		return __convert_error_code(ret,(char*)__FUNCTION__);
	else
		return PLAYER_ERROR_NONE;
}

int player_audio_effect_is_available(player_h player, audio_effect_e effect, bool *available)
{
	PLAYER_INSTANCE_CHECK(player);
	PLAYER_NULL_ARG_CHECK(available);
	player_s * handle = (player_s *) player;
	int ret = mm_player_is_supported_custom_effect_type(handle->mm_handle, effect);
	if(ret != MM_ERROR_NONE)
		*available = FALSE;
	else
		*available = TRUE;
	return PLAYER_ERROR_NONE;
}

int player_audio_effect_foreach_supported_effect(player_h player, player_audio_effect_supported_effect_cb callback, void *user_data)
{
	PLAYER_INSTANCE_CHECK(player);
	PLAYER_NULL_ARG_CHECK(callback);
	player_s * handle = (player_s *) player;

	LOGI("[%s] Event type : %d ",__FUNCTION__, _PLAYER_EVENT_TYPE_SUPPORTED_AUDIO_EFFECT);
	handle->user_cb[_PLAYER_EVENT_TYPE_SUPPORTED_AUDIO_EFFECT] = callback;
	handle->user_data[_PLAYER_EVENT_TYPE_SUPPORTED_AUDIO_EFFECT] = user_data;
	int ret = mm_player_get_foreach_present_supported_effect_type(handle->mm_handle, MM_AUDIO_EFFECT_TYPE_CUSTOM, __supported_audio_effect_type, (void*)handle);
	if(ret != MM_ERROR_NONE)
		return __convert_error_code(ret,(char*)__FUNCTION__);
	else
		return PLAYER_ERROR_NONE;
}

int player_audio_effect_set_preset(player_h player, audio_effect_preset_e preset)
{
	PLAYER_INSTANCE_CHECK(player);
	player_s * handle = (player_s *) player;
	int ret = mm_player_audio_effect_preset_apply(handle->mm_handle, preset);
	if(ret != MM_ERROR_NONE)
		return __convert_error_code(ret,(char*)__FUNCTION__);
	else
		return PLAYER_ERROR_NONE;
}

int player_audio_effect_preset_is_available(player_h player, audio_effect_preset_e preset, bool *available)
{
	PLAYER_INSTANCE_CHECK(player);
	PLAYER_NULL_ARG_CHECK(available);
	player_s * handle = (player_s *) player;
	int ret = mm_player_is_supported_preset_effect_type(handle->mm_handle, preset);
	if(ret != MM_ERROR_NONE)
		*available = FALSE;
	else
		*available = TRUE;
	return PLAYER_ERROR_NONE;
}

int player_audio_effect_foreach_supported_preset(player_h player, player_audio_effect_supported_preset_cb callback, void *user_data)
{
	PLAYER_INSTANCE_CHECK(player);
	PLAYER_NULL_ARG_CHECK(callback);
	player_s * handle = (player_s *) player;

	LOGI("[%s] Event type : %d ",__FUNCTION__, _PLAYER_EVENT_TYPE_SUPPORTED_AUDIO_EFFECT_PRESET);
	handle->user_cb[_PLAYER_EVENT_TYPE_SUPPORTED_AUDIO_EFFECT_PRESET] = callback;
	handle->user_data[_PLAYER_EVENT_TYPE_SUPPORTED_AUDIO_EFFECT_PRESET] = user_data;
	int ret = mm_player_get_foreach_present_supported_effect_type(handle->mm_handle, MM_AUDIO_EFFECT_TYPE_PRESET, __supported_audio_effect_preset, (void*)handle);
	if(ret != MM_ERROR_NONE)
		return __convert_error_code(ret,(char*)__FUNCTION__);
	else
		return PLAYER_ERROR_NONE;
}

int player_audio_effect_get_equalizer_bands_count (player_h player, int *count)
{
	PLAYER_INSTANCE_CHECK(player);
	PLAYER_NULL_ARG_CHECK(count);
	player_s * handle = (player_s *) player;
	int ret = mm_player_audio_effect_custom_get_eq_bands_number(handle->mm_handle, count);
	if(ret != MM_ERROR_NONE)
		return __convert_error_code(ret,(char*)__FUNCTION__);
	else
		return PLAYER_ERROR_NONE;
}

int player_audio_effect_set_equalizer_all_bands(player_h player, int *band_levels, int length)
{
	PLAYER_INSTANCE_CHECK(player);
	PLAYER_NULL_ARG_CHECK(band_levels);
	player_s * handle = (player_s *) player;
	int ret = mm_player_audio_effect_custom_set_level_eq_from_list(handle->mm_handle, band_levels, length);
	if(ret != MM_ERROR_NONE)
	{
		return __convert_error_code(ret,(char*)__FUNCTION__);
	}
	else
	{
		ret = mm_player_audio_effect_custom_apply(handle->mm_handle);
		return (ret==MM_ERROR_NONE)?PLAYER_ERROR_NONE:__convert_error_code(ret,(char*)__FUNCTION__);
	}
}

int player_audio_effect_set_equalizer_band_level(player_h player, int index, int level)
{
	PLAYER_INSTANCE_CHECK(player);
	player_s * handle = (player_s *) player;
	int ret = mm_player_audio_effect_custom_set_level(handle->mm_handle,MM_AUDIO_EFFECT_CUSTOM_EQ, index, level);
	if(ret != MM_ERROR_NONE)
	{
		return __convert_error_code(ret,(char*)__FUNCTION__);
	}
	else
	{
		ret = mm_player_audio_effect_custom_apply(handle->mm_handle);
		return (ret==MM_ERROR_NONE)?PLAYER_ERROR_NONE:__convert_error_code(ret,(char*)__FUNCTION__);
	}
}

int player_audio_effect_get_equalizer_band_level(player_h player, int index, int *level)
{
	PLAYER_INSTANCE_CHECK(player);
	PLAYER_NULL_ARG_CHECK(level);
	player_s * handle = (player_s *) player;
	int ret = mm_player_audio_effect_custom_get_level(handle->mm_handle,MM_AUDIO_EFFECT_CUSTOM_EQ, index, level);
	if(ret != MM_ERROR_NONE)
		return __convert_error_code(ret,(char*)__FUNCTION__);
	else
		return PLAYER_ERROR_NONE;
}

int player_audio_effect_get_equalizer_level_range(player_h player, int* min, int* max)
{
	PLAYER_INSTANCE_CHECK(player);
	PLAYER_NULL_ARG_CHECK(min);
	PLAYER_NULL_ARG_CHECK(max);
	player_s * handle = (player_s *) player;
	int ret = mm_player_audio_effect_custom_get_level_range(handle->mm_handle, MM_AUDIO_EFFECT_CUSTOM_EQ, min, max);
	if(ret != MM_ERROR_NONE)
		return __convert_error_code(ret,(char*)__FUNCTION__);
	else
		return PLAYER_ERROR_NONE;
}

int player_audio_effect_get_equalizer_band_frequency(player_h player, int index, int *frequency)
{
	PLAYER_INSTANCE_CHECK(player);
	PLAYER_NULL_ARG_CHECK(frequency);
	player_s * handle = (player_s *) player;
	int ret = mm_player_audio_effect_custom_get_eq_bands_freq(handle->mm_handle, index, frequency);
	if(ret != MM_ERROR_NONE)
		return __convert_error_code(ret,(char*)__FUNCTION__);
	else
		return PLAYER_ERROR_NONE;
}

int player_audio_effect_get_equalizer_band_frequency_range(player_h player, int index, int *range)
{
	PLAYER_INSTANCE_CHECK(player);
	PLAYER_NULL_ARG_CHECK(range);
	player_s * handle = (player_s *) player;
	int ret = mm_player_audio_effect_custom_get_eq_bands_width(handle->mm_handle, index, range);
	if(ret != MM_ERROR_NONE)
		return __convert_error_code(ret,(char*)__FUNCTION__);
	else
		return PLAYER_ERROR_NONE;
}

int player_audio_effect_equalizer_clear(player_h player)
{
	PLAYER_INSTANCE_CHECK(player);
	player_s * handle = (player_s *) player;
	int ret = mm_player_audio_effect_custom_clear_eq_all(handle->mm_handle);
	if(ret != MM_ERROR_NONE)
	{
		return __convert_error_code(ret,(char*)__FUNCTION__);
	}
	else
	{
		ret = mm_player_audio_effect_custom_apply(handle->mm_handle);
		return (ret==MM_ERROR_NONE)?PLAYER_ERROR_NONE:__convert_error_code(ret,(char*)__FUNCTION__);
	}
}

int player_audio_effect_equalizer_is_available(player_h player, bool *available)
{
	PLAYER_INSTANCE_CHECK(player);
	PLAYER_NULL_ARG_CHECK(available);
	player_s * handle = (player_s *) player;
	int ret = mm_player_is_supported_custom_effect_type(handle->mm_handle, MM_AUDIO_EFFECT_CUSTOM_EQ);
	if(ret != MM_ERROR_NONE)
		*available = FALSE;
	else
		*available = TRUE;
	return PLAYER_ERROR_NONE;
}

int player_set_subtitle_path(player_h player,const char* path)
{
	PLAYER_INSTANCE_CHECK(player);
	PLAYER_NULL_ARG_CHECK(path);
	player_s * handle = (player_s *) player;
	PLAYER_STATE_CHECK(handle,PLAYER_STATE_IDLE);

	int ret = mm_player_set_attribute(handle->mm_handle, NULL,"subtitle_uri" , path, strlen(path), (char*)NULL);
	if(ret != MM_ERROR_NONE)
	{
		return __convert_error_code(ret,(char*)__FUNCTION__);
	}
	else
		return PLAYER_ERROR_NONE;
}

int player_set_subtitle_position(player_h player, int millisecond)
{
	PLAYER_INSTANCE_CHECK(player);
	PLAYER_CHECK_CONDITION(millisecond>=0  ,PLAYER_ERROR_INVALID_PARAMETER ,"PLAYER_ERROR_INVALID_PARAMETER" );
	player_s * handle = (player_s *) player;
	if (!__player_state_validate(handle, PLAYER_STATE_PLAYING))
	{
		LOGE("[%s] PLAYER_ERROR_INVALID_STATE(0x%08x) : current state - %d" ,__FUNCTION__,PLAYER_ERROR_INVALID_STATE, handle->state);
		return PLAYER_ERROR_INVALID_STATE;
	}

	int ret = mm_player_adjust_subtitle_position(handle->mm_handle, MM_PLAYER_POS_FORMAT_TIME, millisecond);
	if(ret != MM_ERROR_NONE)
	{
		return __convert_error_code(ret,(char*)__FUNCTION__);
	}
	else
		return PLAYER_ERROR_NONE;
}

int player_set_progressive_download_path(player_h player, const char *path)
{
	PLAYER_INSTANCE_CHECK(player);
	PLAYER_NULL_ARG_CHECK(path);
	player_s * handle = (player_s *) player;
	PLAYER_STATE_CHECK(handle,PLAYER_STATE_IDLE);

	int ret = mm_player_set_attribute(handle->mm_handle, NULL,"pd_mode", MM_PLAYER_PD_MODE_URI, "pd_location", path, strlen(path), (char*)NULL);
	if(ret != MM_ERROR_NONE)
	{
		return __convert_error_code(ret,(char*)__FUNCTION__);
	}
	else
	{
		handle->is_progressive_download = 1;
		return PLAYER_ERROR_NONE;
	}
}

int player_get_progressive_download_status(player_h player, unsigned long *current, unsigned long *total_size)
{

	PLAYER_INSTANCE_CHECK(player);
	PLAYER_NULL_ARG_CHECK(current);
	PLAYER_NULL_ARG_CHECK(total_size);
	player_s * handle = (player_s *) player;
	if (handle->state != PLAYER_STATE_PLAYING && handle->state != PLAYER_STATE_PAUSED)
	{
		LOGE("[%s] PLAYER_ERROR_INVALID_STATE(0x%08x) : current state - %d" ,__FUNCTION__,PLAYER_ERROR_INVALID_STATE, handle->state);
		return PLAYER_ERROR_INVALID_STATE;
	}
	guint64 _current;
	guint64 _total;
	int ret = mm_player_get_pd_status(handle->mm_handle, &_current, &_total);
	if(ret != MM_ERROR_NONE)
	{
		return __convert_error_code(ret,(char*)__FUNCTION__);
	}
	else
	{
		*current = _current;
		*total_size = _total;
		return PLAYER_ERROR_NONE;
	}
}

int player_capture_video(player_h player, player_video_captured_cb callback, void *user_data)
{
	PLAYER_INSTANCE_CHECK(player);
	PLAYER_NULL_ARG_CHECK(callback);

	player_s * handle = (player_s *) player;
	if(handle->user_cb[_PLAYER_EVENT_TYPE_CAPTURE])
	{
		LOGE("[%s] PLAYER_ERROR_VIDEO_CAPTURE_FAILED (0x%08x) : capturing... we can't do any more " ,__FUNCTION__, PLAYER_ERROR_VIDEO_CAPTURE_FAILED);
		return PLAYER_ERROR_VIDEO_CAPTURE_FAILED;
	}
	else
	{
		LOGE("[%s] Event type : %d ",__FUNCTION__, _PLAYER_EVENT_TYPE_CAPTURE);
		handle->user_cb[_PLAYER_EVENT_TYPE_CAPTURE] = callback;
		handle->user_data[_PLAYER_EVENT_TYPE_CAPTURE] = user_data;
	}

	if(handle->state == PLAYER_STATE_PAUSED || handle->state == PLAYER_STATE_PLAYING )
	{
		int ret = mm_player_do_video_capture(handle->mm_handle);
		if(ret==MM_ERROR_PLAYER_NO_OP)
		{
			handle->user_cb[_PLAYER_EVENT_TYPE_CAPTURE] = NULL;
			handle->user_data[_PLAYER_EVENT_TYPE_CAPTURE] = NULL;
			LOGE("[%s] PLAYER_ERROR_INVALID_OPERATION (0x%08x) : video display must be set : %d" ,__FUNCTION__, PLAYER_ERROR_INVALID_OPERATION, handle->display_type);
			return PLAYER_ERROR_INVALID_OPERATION;
		}
		if(ret != MM_ERROR_NONE)
		{
			handle->user_cb[_PLAYER_EVENT_TYPE_CAPTURE] = NULL;
			handle->user_data[_PLAYER_EVENT_TYPE_CAPTURE] = NULL;
			return __convert_error_code(ret,(char*)__FUNCTION__);
		}
		else
			return PLAYER_ERROR_NONE;
	}
	else
	{
		LOGE("[%s] PLAYER_ERROR_INVALID_STATE (0x%08x) : current state - %d" ,__FUNCTION__, PLAYER_ERROR_INVALID_STATE, handle->state);
		handle->user_cb[_PLAYER_EVENT_TYPE_CAPTURE] = NULL;
		handle->user_data[_PLAYER_EVENT_TYPE_CAPTURE] = NULL;
		return PLAYER_ERROR_INVALID_STATE;
	}
}

int player_set_streaming_cookie(player_h player, const char *cookie, int size)
{
	PLAYER_INSTANCE_CHECK(player);
	PLAYER_NULL_ARG_CHECK(cookie);
	PLAYER_CHECK_CONDITION(size>=0,PLAYER_ERROR_INVALID_PARAMETER,"PLAYER_ERROR_INVALID_PARAMETER" );
	player_s * handle = (player_s *) player;
	PLAYER_STATE_CHECK(handle,PLAYER_STATE_IDLE);

	int ret = mm_player_set_attribute(handle->mm_handle, NULL,"streaming_cookie", cookie, size, (char*)NULL);
	if(ret != MM_ERROR_NONE)
	{
		return __convert_error_code(ret,(char*)__FUNCTION__);
	}
	else
		return PLAYER_ERROR_NONE;
}

int player_set_streaming_user_agent(player_h player, const char *user_agent, int size)
{
	PLAYER_INSTANCE_CHECK(player);
	PLAYER_NULL_ARG_CHECK(user_agent);
	PLAYER_CHECK_CONDITION(size>=0,PLAYER_ERROR_INVALID_PARAMETER,"PLAYER_ERROR_INVALID_PARAMETER" );
	player_s * handle = (player_s *) player;
	PLAYER_STATE_CHECK(handle,PLAYER_STATE_IDLE);

	int ret = mm_player_set_attribute(handle->mm_handle, NULL,"streaming_user_agent", user_agent, size, (char*)NULL);
	if(ret != MM_ERROR_NONE)
	{
		return __convert_error_code(ret,(char*)__FUNCTION__);
	}
	else
		return PLAYER_ERROR_NONE;
}

int player_get_streaming_download_progress(player_h player, int *start, int *current)
{
	PLAYER_INSTANCE_CHECK(player);
	PLAYER_NULL_ARG_CHECK(start);
	PLAYER_NULL_ARG_CHECK(current);
	player_s * handle = (player_s *) player;
	if (handle->state != PLAYER_STATE_PLAYING && handle->state != PLAYER_STATE_PAUSED)
	{
		LOGE("[%s] PLAYER_ERROR_INVALID_STATE(0x%08x) : current state - %d" ,__FUNCTION__,PLAYER_ERROR_INVALID_STATE, handle->state);
		return PLAYER_ERROR_INVALID_STATE;
	}
	int _current=0;
	int _start=0;
	int ret = mm_player_get_buffer_position(handle->mm_handle,MM_PLAYER_POS_FORMAT_PERCENT,&_start,&_current);
	if(ret != MM_ERROR_NONE)
	{
		return __convert_error_code(ret,(char*)__FUNCTION__);
	}
	else
	{
		*start = _start;
		*current = _current;
		return PLAYER_ERROR_NONE;
	}
}

int player_set_completed_cb (player_h player, player_completed_cb callback, void *user_data)
{
	return __set_callback(_PLAYER_EVENT_TYPE_COMPLETE,player,callback,user_data);
}

int player_unset_completed_cb (player_h player)
{
	return __unset_callback(_PLAYER_EVENT_TYPE_COMPLETE,player);
}

int player_set_interrupted_cb (player_h player, player_interrupted_cb callback, void *user_data)
{
	return __set_callback(_PLAYER_EVENT_TYPE_INTERRUPT,player,callback,user_data);
}

int player_unset_interrupted_cb (player_h player)
{
	return __unset_callback(_PLAYER_EVENT_TYPE_INTERRUPT,player);
}

int player_set_error_cb (player_h player, player_error_cb callback, void *user_data)
{
	return __set_callback(_PLAYER_EVENT_TYPE_ERROR,player,callback,user_data);
}

int player_unset_error_cb (player_h player)
{
	return __unset_callback(_PLAYER_EVENT_TYPE_ERROR,player);
}

int player_set_buffering_cb (player_h player, player_buffering_cb callback, void *user_data)
{
	return __set_callback(_PLAYER_EVENT_TYPE_BUFFERING,player,callback,user_data);
}

int player_unset_buffering_cb (player_h player)
{
	return __unset_callback(_PLAYER_EVENT_TYPE_BUFFERING,player);
}

int player_set_subtitle_updated_cb(player_h player, player_subtitle_updated_cb callback, void* user_data )
{
	return __set_callback(_PLAYER_EVENT_TYPE_SUBTITLE,player,callback,user_data);
}

int player_unset_subtitle_updated_cb (player_h player)
{
	return __unset_callback(_PLAYER_EVENT_TYPE_SUBTITLE,player);
}

int player_set_video_frame_decoded_cb(player_h player, player_video_frame_decoded_cb callback, void *user_data)
{
	PLAYER_INSTANCE_CHECK(player);
	PLAYER_NULL_ARG_CHECK(callback);
	player_s * handle = (player_s *) player;
	if (handle->state != PLAYER_STATE_IDLE )
	{
		LOGE("[%s] PLAYER_ERROR_INVALID_STATE(0x%08x) : current state - %d" ,__FUNCTION__,PLAYER_ERROR_INVALID_STATE, handle->state);
		return PLAYER_ERROR_INVALID_STATE;
	}

	int ret = mm_player_set_video_stream_callback(handle->mm_handle, __video_stream_callback, (void*)handle);
	if(ret != MM_ERROR_NONE)
		return __convert_error_code(ret,(char*)__FUNCTION__);

	handle->user_cb[_PLAYER_EVENT_TYPE_VIDEO_FRAME] = callback;
	handle->user_data[_PLAYER_EVENT_TYPE_VIDEO_FRAME] = user_data;
	LOGI("[%s] Event type : %d ",__FUNCTION__, _PLAYER_EVENT_TYPE_VIDEO_FRAME);
	return PLAYER_ERROR_NONE;
}

int player_unset_video_frame_decoded_cb(player_h player)
{
	PLAYER_INSTANCE_CHECK(player);
	player_s * handle = (player_s *) player;
	handle->user_cb[_PLAYER_EVENT_TYPE_VIDEO_FRAME] = NULL;
	handle->user_data[_PLAYER_EVENT_TYPE_VIDEO_FRAME] = NULL;
	LOGI("[%s] Event type : %d ",__FUNCTION__, _PLAYER_EVENT_TYPE_VIDEO_FRAME);
	int ret = mm_player_set_video_stream_callback(handle->mm_handle, NULL, NULL);
	if(ret != MM_ERROR_NONE)
		return __convert_error_code(ret,(char*)__FUNCTION__);
	else
		return PLAYER_ERROR_NONE;
}

int player_set_audio_frame_decoded_cb(player_h player, int start, int end, player_audio_frame_decoded_cb callback, void *user_data)
{
	PLAYER_INSTANCE_CHECK(player);
	PLAYER_NULL_ARG_CHECK(callback);
	PLAYER_CHECK_CONDITION(start>=0 ,PLAYER_ERROR_INVALID_PARAMETER,"PLAYER_ERROR_INVALID_PARAMETER" );
	PLAYER_CHECK_CONDITION(end>=start ,PLAYER_ERROR_INVALID_PARAMETER,"PLAYER_ERROR_INVALID_PARAMETER" );
	player_s * handle = (player_s *) player;
	if (handle->state != PLAYER_STATE_IDLE)
	{
		LOGE("[%s] PLAYER_ERROR_INVALID_STATE(0x%08x) : current state - %d" ,__FUNCTION__,PLAYER_ERROR_INVALID_STATE, handle->state);
		return PLAYER_ERROR_INVALID_STATE;
	}

	int ret = mm_player_set_attribute(handle->mm_handle, NULL, "pcm_extraction",TRUE, "pcm_extraction_start_msec", start, "pcm_extraction_end_msec", end, NULL);
	if(ret != MM_ERROR_NONE)
		return __convert_error_code(ret,(char*)__FUNCTION__);

	ret = mm_player_set_audio_stream_callback(handle->mm_handle, __audio_stream_callback, (void*)handle);
	if(ret != MM_ERROR_NONE)
		return __convert_error_code(ret,(char*)__FUNCTION__);

	handle->user_cb[_PLAYER_EVENT_TYPE_AUDIO_FRAME] = callback;
	handle->user_data[_PLAYER_EVENT_TYPE_AUDIO_FRAME] = user_data;
	LOGI("[%s] Event type : %d ",__FUNCTION__, _PLAYER_EVENT_TYPE_AUDIO_FRAME);
	return PLAYER_ERROR_NONE;
}

int player_unset_audio_frame_decoded_cb(player_h player)
{
	PLAYER_INSTANCE_CHECK(player);
	player_s * handle = (player_s *) player;

	handle->user_cb[_PLAYER_EVENT_TYPE_AUDIO_FRAME] = NULL;
	handle->user_data[_PLAYER_EVENT_TYPE_AUDIO_FRAME] = NULL;
	LOGI("[%s] Event type : %d ",__FUNCTION__, _PLAYER_EVENT_TYPE_AUDIO_FRAME);

	int ret = mm_player_set_attribute(handle->mm_handle, NULL, "pcm_extraction",FALSE, NULL);
	if(ret != MM_ERROR_NONE)
		return __convert_error_code(ret,(char*)__FUNCTION__);

	ret = mm_player_set_audio_stream_callback(handle->mm_handle, NULL, NULL);
	if(ret != MM_ERROR_NONE)
		return __convert_error_code(ret,(char*)__FUNCTION__);
	else
		return PLAYER_ERROR_NONE;
}

int player_set_progressive_download_message_cb(player_h player, player_pd_message_cb callback, void *user_data)
{
	PLAYER_INSTANCE_CHECK(player);
	PLAYER_NULL_ARG_CHECK(callback);
	player_s * handle = (player_s *) player;
	if (handle->state != PLAYER_STATE_IDLE  &&  handle->state != PLAYER_STATE_READY)
	{
		LOGE("[%s] PLAYER_ERROR_INVALID_STATE(0x%08x) : current state - %d" ,__FUNCTION__,PLAYER_ERROR_INVALID_STATE, handle->state);
		return PLAYER_ERROR_INVALID_STATE;
	}

	int ret = mm_player_set_pd_message_callback(handle->mm_handle, __pd_message_callback, (void*)handle);
	if(ret != MM_ERROR_NONE)
		return __convert_error_code(ret,(char*)__FUNCTION__);

	handle->user_cb[_PLAYER_EVENT_TYPE_PD] = callback;
	handle->user_data[_PLAYER_EVENT_TYPE_PD] = user_data;
	LOGI("[%s] Event type : %d ",__FUNCTION__, _PLAYER_EVENT_TYPE_PD);
	return PLAYER_ERROR_NONE;
}

int player_unset_progressive_download_message_cb(player_h player)
{
	PLAYER_INSTANCE_CHECK(player);
	player_s * handle = (player_s *) player;

	handle->user_cb[_PLAYER_EVENT_TYPE_PD] = NULL;
	handle->user_data[_PLAYER_EVENT_TYPE_PD] = NULL;
	LOGI("[%s] Event type : %d ",__FUNCTION__, _PLAYER_EVENT_TYPE_PD);

	int ret = mm_player_set_pd_message_callback(handle->mm_handle, NULL, NULL);
	if(ret != MM_ERROR_NONE)
		return __convert_error_code(ret,(char*)__FUNCTION__);
	else
		return PLAYER_ERROR_NONE;
}
