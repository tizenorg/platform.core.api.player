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
#include <mm_player_sndeffect.h>
#include <mm_player_internal.h>
#include <mm_types.h>
#include <player.h>
#include <player_private.h>
#include <dlog.h>

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
	int ret = PLAYER_ERROR_NONE;
	char* msg="PLAYER_ERROR_NONE";
	switch(code)
	{
		case MM_ERROR_NONE:
			ret = PLAYER_ERROR_NONE;
			msg = "PLAYER_ERROR_NONE";
			break;
		case MM_ERROR_PLAYER_CODEC_NOT_FOUND:
		case MM_ERROR_PLAYER_AUDIO_CODEC_NOT_FOUND:
		case MM_ERROR_PLAYER_VIDEO_CODEC_NOT_FOUND:
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
			ret = PLAYER_ERROR_INVALID_OPERATION;
			msg = "PLAYER_ERROR_INVALID_OPERATION";
			break;
		case  MM_ERROR_PLAYER_NO_FREE_SPACE:
			ret = PLAYER_ERROR_OUT_OF_MEMORY;
			msg = "PLAYER_ERROR_OUT_OF_MEMORY";
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
	}
	LOGE("[%s] %s(0x%08x) : core fw error(0x%x)",func_name,msg, ret, code);
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
	LOGI("[%s] Got message type : 0x%x" ,__FUNCTION__, message);
	player_error_e err_code = PLAYER_ERROR_NONE;
	switch(message)
	{
		case MM_MESSAGE_ERROR: //0x01
			err_code = __convert_error_code(msg->code,(char*)__FUNCTION__);
			break;
		case  MM_MESSAGE_STATE_CHANGED:	//0x03
			handle->state = __convert_player_state(msg->state.current);
			if(handle->state == PLAYER_STATE_PAUSED &&  handle->user_cb[_PLAYER_EVENT_TYPE_PAUSE] )
			{
				((player_paused_cb)handle->user_cb[_PLAYER_EVENT_TYPE_PAUSE])(handle->user_data[_PLAYER_EVENT_TYPE_PAUSE]);
			}	
			break;
		case MM_MESSAGE_READY_TO_RESUME: //0x05
			if( handle->user_cb[_PLAYER_EVENT_TYPE_INTERRUPT] )
			{
				((player_interrupted_cb)handle->user_cb[_PLAYER_EVENT_TYPE_INTERRUPT])(PLAYER_INTERRUPTED_COMPLETED,handle->user_data[_PLAYER_EVENT_TYPE_INTERRUPT]);
			}
			break;
		case MM_MESSAGE_BEGIN_OF_STREAM: //0x104
			if( handle->user_cb[_PLAYER_EVENT_TYPE_BEGIN] )
			{
				((player_started_cb)handle->user_cb[_PLAYER_EVENT_TYPE_BEGIN])(handle->user_data[_PLAYER_EVENT_TYPE_BEGIN]);
			}	
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
				MMPlayerStateType currentStat = MM_PLAYER_STATE_PAUSED;
				int ret = mm_player_get_state(handle->mm_handle, &currentStat);
				handle->state = (ret != MM_ERROR_NONE )? currentStat: __convert_player_state(currentStat);
				((player_interrupted_cb)handle->user_cb[_PLAYER_EVENT_TYPE_INTERRUPT])(msg->code,handle->user_data[_PLAYER_EVENT_TYPE_INTERRUPT]);
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
		case MM_MESSAGE_DRM_NOT_AUTHORIZED: //0x111
			LOGE("[%s] PLAYER_ERROR_NOT_SUPPORTED_FILE (0x%08x) : DRM_NOT_AUTHORIZED" ,__FUNCTION__, PLAYER_ERROR_NOT_SUPPORTED_FILE);
			err_code = PLAYER_ERROR_NOT_SUPPORTED_FILE;
			break;
		case MM_MESSAGE_UNKNOWN: //0x00
		case MM_MESSAGE_WARNING: //0x02
		case MM_MESSAGE_CONNECTING: //0x100
		case MM_MESSAGE_CONNECTED: //0x101
		case MM_MESSAGE_BLUETOOTH_ON: //0x106
		case MM_MESSAGE_BLUETOOTH_OFF: //0x107
		case MM_MESSAGE_RESUMED_BY_REW: //0x108
		case MM_MESSAGE_RTP_SENDER_REPORT: //0x10a
		case MM_MESSAGE_RTP_RECEIVER_REPORT: //0x10b
		case MM_MESSAGE_RTP_SESSION_STATUS: //0x10c
		case MM_MESSAGE_SENDER_STATE: //0x10d
		case MM_MESSAGE_RECEIVER_STATE: //0x10e
		case MM_MESSAGE_SEEK_COMPLETED: //0x114
		default:
			break;
	}

	if(err_code != PLAYER_ERROR_NONE && handle->user_cb[_PLAYER_EVENT_TYPE_ERROR])
	{
		((player_error_cb)handle->user_cb[_PLAYER_EVENT_TYPE_ERROR])(err_code,handle->user_data[_PLAYER_EVENT_TYPE_ERROR]);
	}
	return 1;
}

static bool  __video_stream_callback(void *stream, int stream_size, void *user_data, int width, int height)
{
	player_s * handle = (player_s*)user_data;
	if( handle->user_cb[_PLAYER_EVENT_TYPE_VIDEO_FRAME] )
	{
		((player_video_frame_decoded_cb)handle->user_cb[_PLAYER_EVENT_TYPE_VIDEO_FRAME])((unsigned char *)stream, width, height, stream_size, handle->user_data[_PLAYER_EVENT_TYPE_VIDEO_FRAME]);
	}	
	return TRUE;
}

static bool  __audio_stream_callback(void *stream, int stream_size, void *user_data)
{
	player_s * handle = (player_s*)user_data;
	if( handle->user_cb[_PLAYER_EVENT_TYPE_AUDIO_FRAME] )
	{
		((player_audio_frame_decoded_cb)handle->user_cb[_PLAYER_EVENT_TYPE_AUDIO_FRAME])((unsigned char *)stream, stream_size, handle->user_data[_PLAYER_EVENT_TYPE_AUDIO_FRAME]);
	}	
	return TRUE;
}


/*
* Public Implementation
*/

int player_create (player_h *player)
{
	PLAYER_INSTANCE_CHECK(player);
	player_s * handle;
	handle = (player_s*)malloc( sizeof(player_s));
	if (handle != NULL)
		memset(handle, 0 , sizeof(player_s));
	else
	{
		LOGE("[%s] PLAYER_ERROR_OUT_OF_MEMORY" ,__FUNCTION__,PLAYER_ERROR_OUT_OF_MEMORY);
		return PLAYER_ERROR_OUT_OF_MEMORY;
	}
	int ret = mm_player_create(&handle->mm_handle);
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
		handle->display_type = PLAYER_DISPLAY_TYPE_NONE;
		return PLAYER_ERROR_NONE;
	}
}


int player_destroy (player_h player)
{
	PLAYER_INSTANCE_CHECK(player);
	player_s * handle = (player_s *) player;
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
		return PLAYER_ERROR_NONE;
	}
}

int 	player_prepare (player_h player)
{
	PLAYER_INSTANCE_CHECK(player);
	player_s * handle = (player_s *) player;
	PLAYER_STATE_CHECK(handle,PLAYER_STATE_IDLE);

	int ret;
	ret = mm_player_set_message_callback(handle->mm_handle, __msg_callback, (void*)handle);
	if(ret != MM_ERROR_NONE)
	{
		LOGW("[%s] Failed to set message callback function (0x%x)" ,__FUNCTION__, ret);
	}
	ret = mm_player_realize(handle->mm_handle);
	if(ret != MM_ERROR_NONE)
	{
		return __convert_error_code(ret,(char*)__FUNCTION__);
	}
	else
	{
		handle->state = PLAYER_STATE_READY;
		return PLAYER_ERROR_NONE;
	}
}

int 	player_unprepare (player_h player)
{
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
		return PLAYER_ERROR_NONE;
	}
}

int 	player_set_uri (player_h player, const char *uri)
{
	PLAYER_INSTANCE_CHECK(player);
	PLAYER_NULL_ARG_CHECK(uri);
	player_s * handle = (player_s *) player;
	PLAYER_STATE_CHECK(handle,PLAYER_STATE_IDLE);
	
	int ret = mm_player_set_attribute(handle->mm_handle, NULL,MM_PLAYER_CONTENT_URI , uri, strlen(uri), (char*)NULL);
	if(ret != MM_ERROR_NONE)
	{
		return __convert_error_code(ret,(char*)__FUNCTION__);
	}
	else
		return PLAYER_ERROR_NONE;
}

int 	player_set_memory_buffer (player_h player, const void *data, int size)
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

int 	player_get_state (player_h player, player_state_e *state)
{
	PLAYER_INSTANCE_CHECK(player);
	PLAYER_NULL_ARG_CHECK(state);
	player_s * handle = (player_s *) player;
	MMPlayerStateType currentStat = MM_PLAYER_STATE_NULL;
	int ret = mm_player_get_state(handle->mm_handle, &currentStat);
	if(ret != MM_ERROR_NONE)
	{
		*state = handle->state;
		return __convert_error_code(ret,(char*)__FUNCTION__);
	}
	else
	{
		handle->state  = __convert_player_state(currentStat);
		*state = handle->state;
		return PLAYER_ERROR_NONE;
	}
}

int 	player_set_volume (player_h player, float left, float right)
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

int 	player_get_volume (player_h player, float *left, float *right)
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

	int ret = mm_player_set_attribute(handle->mm_handle, NULL,"sound_volume_type" , type, (char*)NULL);
	if(ret != MM_ERROR_NONE)
	{
		return __convert_error_code(ret,(char*)__FUNCTION__);
	}
	else
		return PLAYER_ERROR_NONE;
}

int 	player_start (player_h player)
{
	PLAYER_INSTANCE_CHECK(player);
	player_s * handle = (player_s *) player;
	int ret;
	if (handle->state == PLAYER_STATE_READY )
	{
		ret = mm_player_start(handle->mm_handle);
	}
	else if ( handle->state  == PLAYER_STATE_PAUSED)
	{
		ret = mm_player_resume(handle->mm_handle);
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
		handle->state = PLAYER_STATE_PLAYING;
		return PLAYER_ERROR_NONE;
	}
}

int 	player_stop (player_h player)
{
	PLAYER_INSTANCE_CHECK(player);
	player_s * handle = (player_s *) player;
	if (handle->state == PLAYER_STATE_PLAYING || handle->state == PLAYER_STATE_PAUSED)
	{
		int ret = mm_player_stop(handle->mm_handle);
		if(ret != MM_ERROR_NONE)
		{
			return __convert_error_code(ret,(char*)__FUNCTION__);
		}
		else
		{
			handle->state = PLAYER_STATE_READY;
			return PLAYER_ERROR_NONE;
		}
	}
	else
	{
		LOGE("[%s] PLAYER_ERROR_INVALID_STATE (current state : %d)" ,__FUNCTION__, handle->state);
		return PLAYER_ERROR_INVALID_STATE;
	}
}

int 	player_pause (player_h player)
{
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
		return PLAYER_ERROR_NONE;
	}
}

int 	player_set_position (player_h player, int millisecond)
{
	PLAYER_INSTANCE_CHECK(player);
	PLAYER_CHECK_CONDITION(millisecond>=0  ,PLAYER_ERROR_INVALID_PARAMETER ,"PLAYER_ERROR_INVALID_PARAMETER" );
	player_s * handle = (player_s *) player;

	int ret = mm_player_set_position(handle->mm_handle, MM_PLAYER_POS_FORMAT_TIME, millisecond);
	if(ret != MM_ERROR_NONE)
	{
		return __convert_error_code(ret,(char*)__FUNCTION__);
	}
	else
	{
		return PLAYER_ERROR_NONE;
	}
}


int 	player_set_position_ratio (player_h player, int percent)
{
	PLAYER_INSTANCE_CHECK(player);
	PLAYER_CHECK_CONDITION(percent>=0 && percent <= 100 ,PLAYER_ERROR_INVALID_PARAMETER ,"PLAYER_ERROR_INVALID_PARAMETER" );
	player_s * handle = (player_s *) player;

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


int 	player_get_position (player_h player, int *millisecond)
{
	PLAYER_INSTANCE_CHECK(player);
	PLAYER_NULL_ARG_CHECK(millisecond);
	player_s * handle = (player_s *) player;
	if (!__player_state_validate(handle, PLAYER_STATE_READY))
	{
		LOGE("[%s] PLAYER_ERROR_INVALID_STATE (current state : %d)" ,__FUNCTION__, handle->state);
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

int 	player_get_position_ratio (player_h player,int *percent)
{
	PLAYER_INSTANCE_CHECK(player);
	PLAYER_NULL_ARG_CHECK(percent);
	player_s * handle = (player_s *) player;
	if (!__player_state_validate(handle, PLAYER_STATE_READY))
	{
		LOGE("[%s] PLAYER_ERROR_INVALID_STATE (current state : %d)" ,__FUNCTION__, handle->state);
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

int 	player_set_mute (player_h player, bool muted)
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

int 	player_is_muted (player_h player, bool *muted)
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
		LOGE("[%s] PLAYER_ERROR_INVALID_STATE (current state : %d)" ,__FUNCTION__, handle->state);
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

int 	player_is_looping (player_h player, bool *looping)
{
	PLAYER_INSTANCE_CHECK(player);
	PLAYER_NULL_ARG_CHECK(looping);
	player_s * handle = (player_s *) player;
	if (!__player_state_validate(handle, PLAYER_STATE_IDLE))
	{
		LOGE("[%s] PLAYER_ERROR_INVALID_STATE (current state : %d)" ,__FUNCTION__, handle->state);
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

int 	player_get_duration (player_h player, int *duration)
{
	PLAYER_INSTANCE_CHECK(player);
	PLAYER_NULL_ARG_CHECK(duration);
	player_s * handle = (player_s *) player;
	if (!__player_state_validate(handle, PLAYER_STATE_PLAYING))
	{
		LOGE("[%s] PLAYER_ERROR_INVALID_STATE (current state : %d)" ,__FUNCTION__, handle->state);
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
		return PLAYER_ERROR_NONE;
	}
}

int player_set_display(player_h player, player_display_type_e type, player_display_h display)
{
	PLAYER_INSTANCE_CHECK(player);
	player_s * handle = (player_s *) player;
	PLAYER_STATE_CHECK(handle,PLAYER_STATE_IDLE);

	handle->display_handle = display;
	handle->display_type = type;
	int ret = mm_player_set_attribute(handle->mm_handle, NULL, "display_surface_type", type, "display_overlay" , type == PLAYER_DISPLAY_TYPE_X11 ? &handle->display_handle : display, sizeof(display), (char*)NULL);
	if(ret != MM_ERROR_NONE)
	{
		return __convert_error_code(ret,(char*)__FUNCTION__);
	}
	else
		return PLAYER_ERROR_NONE;
}

int player_set_playback_rate(player_h player, float rate)
{
	PLAYER_INSTANCE_CHECK(player);
	PLAYER_CHECK_CONDITION(rate>=-5.0 && rate <= 5.0 ,PLAYER_ERROR_INVALID_PARAMETER,"PLAYER_ERROR_INVALID_PARAMETER" );
	player_s * handle = (player_s *) player;
	PLAYER_STATE_CHECK(handle,PLAYER_STATE_PLAYING);

	int ret = mm_player_set_play_speed(handle->mm_handle, rate);

	switch (ret)
	{
	case MM_ERROR_NONE:
	case MM_ERROR_PLAYER_NO_OP:
		ret = PLAYER_ERROR_NONE;
		break;
	case MM_ERROR_NOT_SUPPORT_API:
	case MM_ERROR_PLAYER_SEEK:		
		LOGE("[%s] %s(0x%08x)",__FUNCTION__, PLAYER_ERROR_INVALID_OPERATION,"PLAYER_ERROR_INVALID_OPERATION" );
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
		return PLAYER_ERROR_NONE;
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


int player_set_x11_display_mode(player_h player, player_display_mode_e mode)
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

int player_get_x11_display_mode(player_h player, player_display_mode_e *mode)
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

int player_set_audio_effect_output(player_h player,  sound_device_out_e output)
{
	PLAYER_INSTANCE_CHECK(player);
	player_s * handle = (player_s *) player;

	MMAudioFilterOutputMode _mode=MM_AUDIO_FILTER_OUTPUT_SPK;
	switch(output)
	{
	case SOUND_DEVICE_OUT_SPEAKER:
		 _mode = MM_AUDIO_FILTER_OUTPUT_SPK;
		break;
	case SOUND_DEVICE_OUT_WIRED_ACCESSORY:
	case SOUND_DEVICE_OUT_BT_A2DP:
		 _mode = MM_AUDIO_FILTER_OUTPUT_EAR;
		break;
	case SOUND_DEVICE_OUT_BT_SCO:
	case SOUND_DEVICE_OUT_RECEIVER:
		LOGE("[%s] PLAYER_ERROR_INVALID_OPERATION BT_SCO and RECEIVER doesn't supported( output : %d)" ,__FUNCTION__, output);
		return PLAYER_ERROR_INVALID_OPERATION;
	}
	
	MMAudioFilterInfo filter;
	memset(&filter, 0, sizeof(MMAudioFilterInfo));
	filter.output_mode = _mode;
	int ret = mm_player_apply_sound_filter(handle->mm_handle, &filter);
	if(ret != MM_ERROR_NONE)
	{
		return __convert_error_code(ret,(char*)__FUNCTION__);
	}
	else
	{
		return PLAYER_ERROR_NONE;
	}
}

int player_set_audio_effect_3d(player_h player, audio_effect_3d_e effect)
{
	PLAYER_INSTANCE_CHECK(player);
	player_s * handle = (player_s *) player;

	MMAudioFilterInfo filter;
	memset(&filter, 0, sizeof(MMAudioFilterInfo));
	filter.filter_type = MM_AUDIO_FILTER_3D;

	switch(effect)
	{
	case AUDIO_EFFECT_3D_WIDE:
		 filter.sound_3d.mode = MM_3DSOUND_WIDE;
		break;
	case AUDIO_EFFECT_3D_DYNAMIC:
		 filter.sound_3d.mode = MM_3DSOUND_DYNAMIC;
		break;
	case AUDIO_EFFECT_3D_SURROUND:
		 filter.sound_3d.mode = MM_3DSOUND_SURROUND;
		break;
	}

	int ret = mm_player_apply_sound_filter(handle->mm_handle, &filter);
	if(ret != MM_ERROR_NONE)
	{
		return __convert_error_code(ret,(char*)__FUNCTION__);
	}
	else
	{
		return PLAYER_ERROR_NONE;
	}
}

int player_set_audio_effect_equalizer(player_h player, audio_effect_equalizer_e effect)
{
	PLAYER_INSTANCE_CHECK(player);
	player_s * handle = (player_s *) player;

	MMAudioFilterInfo filter;
	memset(&filter, 0, sizeof(MMAudioFilterInfo));
	filter.filter_type = MM_AUDIO_FILTER_EQUALIZER;
	
	switch(effect)
	{
	case AUDIO_EFFECT_EQ_ROCK:
		filter.equalizer.mode = MM_EQ_ROCK;
		break;
	case AUDIO_EFFECT_EQ_JAZZ:
		filter.equalizer.mode = MM_EQ_JAZZ;
		break;
	case AUDIO_EFFECT_EQ_LIVE:
		filter.equalizer.mode = MM_EQ_LIVE;
		break;
	case AUDIO_EFFECT_EQ_CLASSIC:
		 filter.equalizer.mode = MM_EQ_CLASSIC;
		break;
	case AUDIO_EFFECT_EQ_FULL_BASS:
		 filter.equalizer.mode = MM_EQ_FULL_BASS;
		break;
	case AUDIO_EFFECT_EQ_FULL_BASS_AND_TREBLE:
		 filter.equalizer.mode = MM_EQ_FULL_BASS_AND_TREBLE;
		break;
	case AUDIO_EFFECT_EQ_DANCE:
		 filter.equalizer.mode = MM_EQ_DANCE;
		break;
	case AUDIO_EFFECT_EQ_POP:
		filter.equalizer.mode = MM_EQ_POP;
		break;
	case AUDIO_EFFECT_EQ_FULL_TREBLE:
		 filter.equalizer.mode = MM_EQ_FULL_TREBLE;
		break;
	case AUDIO_EFFECT_EQ_CLUB:
		 filter.equalizer.mode = MM_EQ_CLUB;
		break;
	case AUDIO_EFFECT_EQ_PARTY:
		 filter.equalizer.mode = MM_EQ_PARTY;
		break;
	case AUDIO_EFFECT_EQ_LARGE_HALL:
		filter.equalizer.mode = MM_EQ_LARGE_HALL;
		break;
	case AUDIO_EFFECT_EQ_SOFT:
		 filter.equalizer.mode = MM_EQ_SOFT;
		break;
	case AUDIO_EFFECT_EQ_SOFT_ROCK:
		 filter.equalizer.mode = MM_EQ_SOFT_ROCK;
		break;
	}

	int ret = mm_player_apply_sound_filter(handle->mm_handle, &filter);
	if(ret != MM_ERROR_NONE)
	{
		return __convert_error_code(ret,(char*)__FUNCTION__);
	}
	else
	{
		return PLAYER_ERROR_NONE;
	}
}

int player_set_audio_effect_reverb(player_h player, audio_effect_reverb_e effect)
{
	PLAYER_INSTANCE_CHECK(player);
	player_s * handle = (player_s *) player;

	MMAudioFilterInfo filter;
	memset(&filter, 0, sizeof(MMAudioFilterInfo));
	filter.filter_type = MM_AUDIO_FILTER_REVERB;
	
	switch(effect)
	{
	case AUDIO_EFFECT_REVERB_JAZZ_CLUB:
		filter.reverb.mode = MM_REVERB_JAZZ_CLUB;
		break;
	case AUDIO_EFFECT_REVERB_CONCERT_HALL:
		 filter.reverb.mode = MM_REVERB_CONCERT_HALL;
		break;
	case AUDIO_EFFECT_REVERB_STADIUM:
		 filter.reverb.mode = MM_REVERB_STADIUM;
		break;
	}

	int ret = mm_player_apply_sound_filter(handle->mm_handle, &filter);
	if(ret != MM_ERROR_NONE)
	{
		return __convert_error_code(ret,(char*)__FUNCTION__);
	}
	else
	{
		return PLAYER_ERROR_NONE;
	}
}

int player_set_audio_effect_extra(player_h player, audio_effect_extra_e effect)
{
	PLAYER_INSTANCE_CHECK(player);
	player_s * handle = (player_s *) player;

	MMAudioFilterInfo filter;
	memset(&filter, 0, sizeof(MMAudioFilterInfo));
	
	switch(effect)
	{
	case AUDIO_EFFECT_EX_BASE:
		 filter.filter_type = MM_AUDIO_FILTER_BE;
		break;
	case AUDIO_EFFECT_EX_MUSIC_CLARITY:
		 filter.filter_type = MM_AUDIO_FILTER_MC;
		break;
	case AUDIO_EFFECT_EX_SURROUND:
		 filter.filter_type = MM_AUDIO_FILTER_SRSCSHP;
		break;
	case AUDIO_EFFECT_EX_SOUND_EXTERNAL:
		 filter.filter_type = MM_AUDIO_FILTER_SOUND_EX;
		break;
	}

	int ret = mm_player_apply_sound_filter(handle->mm_handle, &filter);
	if(ret != MM_ERROR_NONE)
	{
		return __convert_error_code(ret,(char*)__FUNCTION__);
	}
	else
	{
		return PLAYER_ERROR_NONE;
	}
}

int player_clear_audio_effect(player_h player)
{
	PLAYER_INSTANCE_CHECK(player);
	player_s * handle = (player_s *) player;
	MMAudioFilterInfo filter;
	memset(&filter, 0, sizeof(MMAudioFilterInfo));
	filter.filter_type = MM_AUDIO_FILTER_NONE;
	int ret = mm_player_apply_sound_filter(handle->mm_handle, &filter);
	if(ret != MM_ERROR_NONE)
	{
		return __convert_error_code(ret,(char*)__FUNCTION__);
	}
	else
	{
		return PLAYER_ERROR_NONE;
	}
}

int 	player_get_video_size (player_h player, int *width, int *height)
{
	PLAYER_INSTANCE_CHECK(player);
	PLAYER_NULL_ARG_CHECK(width);
	PLAYER_NULL_ARG_CHECK(height);
	player_s * handle = (player_s *) player;
	if (!__player_state_validate(handle, PLAYER_STATE_READY))
	{
		LOGE("[%s] PLAYER_ERROR_INVALID_STATE (current state : %d)" ,__FUNCTION__, handle->state);
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
		return PLAYER_ERROR_NONE;
	}
}

int player_set_subtitle_path(player_h player, char* path)
{
	PLAYER_INSTANCE_CHECK(player);
	PLAYER_NULL_ARG_CHECK(path);
	player_s * handle = (player_s *) player;
	PLAYER_STATE_CHECK(handle,PLAYER_STATE_IDLE);

	int ret = mm_player_set_attribute(handle->mm_handle, NULL,"subtitle_uri" , path, strlen(path),"subtitle_silent", 0, (char*)NULL);
	if(ret != MM_ERROR_NONE)
	{
		return __convert_error_code(ret,(char*)__FUNCTION__);
	}
	else
		return PLAYER_ERROR_NONE;
}

int player_capture_video(player_h player, player_video_captured_cb callback, void *user_data)
{
	int ret =__set_callback(_PLAYER_EVENT_TYPE_CAPTURE,player,callback,user_data);
	if(ret != PLAYER_ERROR_NONE)
		return ret;
	
	player_s * handle = (player_s *) player;
	if(handle->state == PLAYER_STATE_PAUSED || handle->state == PLAYER_STATE_PLAYING )
	{
		ret = mm_player_do_video_capture(handle->mm_handle);
		if(ret != MM_ERROR_NONE)
		{
			return __convert_error_code(ret,(char*)__FUNCTION__);
		}
		else
			return PLAYER_ERROR_NONE;
	}
	else
	{
		LOGE("[%s] PLAYER_ERROR_INVALID_STATE (current state : %d)" ,__FUNCTION__, handle->state);
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

int 	player_set_started_cb (player_h player, player_started_cb callback, void *user_data)
{
	return __set_callback(_PLAYER_EVENT_TYPE_BEGIN,player,callback,user_data);
}

int 	player_unset_started_cb (player_h player)
{
	return __unset_callback(_PLAYER_EVENT_TYPE_BEGIN,player);
}

int 	player_set_completed_cb (player_h player, player_completed_cb callback, void *user_data)
{
	return __set_callback(_PLAYER_EVENT_TYPE_COMPLETE,player,callback,user_data);
}

int 	player_unset_completed_cb (player_h player)
{
	return __unset_callback(_PLAYER_EVENT_TYPE_COMPLETE,player);
}

int 	player_set_paused_cb (player_h player, player_paused_cb callback, void *user_data)
{
	return __set_callback(_PLAYER_EVENT_TYPE_PAUSE,player,callback,user_data);
}

int 	player_unset_paused_cb (player_h player)
{
	return __unset_callback(_PLAYER_EVENT_TYPE_PAUSE,player);
}

int 	player_set_interrupted_cb (player_h player, player_interrupted_cb callback, void *user_data)
{
	return __set_callback(_PLAYER_EVENT_TYPE_INTERRUPT,player,callback,user_data);
}

int 	player_unset_interrupted_cb (player_h player)
{
	return __unset_callback(_PLAYER_EVENT_TYPE_INTERRUPT,player);
}

int 	player_set_error_cb (player_h player, player_error_cb callback, void *user_data)
{
	return __set_callback(_PLAYER_EVENT_TYPE_ERROR,player,callback,user_data);
}

int 	player_unset_error_cb (player_h player)
{
	return __unset_callback(_PLAYER_EVENT_TYPE_ERROR,player);
}

int 	player_set_buffering_cb (player_h player, player_buffering_cb callback, void *user_data)
{
	return __set_callback(_PLAYER_EVENT_TYPE_BUFFERING,player,callback,user_data);
}

int 	player_unset_buffering_cb (player_h player)
{
	return __unset_callback(_PLAYER_EVENT_TYPE_BUFFERING,player);
}

int 	player_set_subtitle_updated_cb(player_h player, player_subtitle_updated_cb callback, void* user_data )
{
	return __set_callback(_PLAYER_EVENT_TYPE_SUBTITLE,player,callback,user_data);
}

int 	player_unset_subtitle_updated_cb (player_h player)
{
	return __unset_callback(_PLAYER_EVENT_TYPE_SUBTITLE,player);
}

int player_set_video_frame_decoded_cb(player_h player, player_video_frame_decoded_cb callback, void *user_data)
{
	PLAYER_INSTANCE_CHECK(player);
	PLAYER_NULL_ARG_CHECK(callback);
	player_s * handle = (player_s *) player;
	if (handle->state != PLAYER_STATE_IDLE  &&  handle->state != PLAYER_STATE_READY)
	{
		LOGE("[%s] PLAYER_ERROR_INVALID_STATE (current state : %d)" ,__FUNCTION__, handle->state);
		return PLAYER_ERROR_INVALID_STATE;
	}
	
	handle->user_cb[_PLAYER_EVENT_TYPE_VIDEO_FRAME] = callback;
	handle->user_data[_PLAYER_EVENT_TYPE_VIDEO_FRAME] = user_data;
	LOGI("[%s] Event type : %d ",__FUNCTION__, _PLAYER_EVENT_TYPE_VIDEO_FRAME);
	int ret = mm_player_set_video_stream_callback(handle->mm_handle, __video_stream_callback, (void*)handle);
	if(ret != MM_ERROR_NONE)
		return __convert_error_code(ret,(char*)__FUNCTION__);
	else
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
	PLAYER_CHECK_CONDITION(end>=1 ,PLAYER_ERROR_INVALID_PARAMETER,"PLAYER_ERROR_INVALID_PARAMETER" );
	PLAYER_CHECK_CONDITION((end-start)>0 ,PLAYER_ERROR_INVALID_PARAMETER,"PLAYER_ERROR_INVALID_PARAMETER" );
	player_s * handle = (player_s *) player;
	if (handle->state != PLAYER_STATE_IDLE  &&  handle->state != PLAYER_STATE_READY)
	{
		LOGE("[%s] PLAYER_ERROR_INVALID_STATE (current state : %d)" ,__FUNCTION__, handle->state);
		return PLAYER_ERROR_INVALID_STATE;
	}
	
	handle->user_cb[_PLAYER_EVENT_TYPE_AUDIO_FRAME] = callback;
	handle->user_data[_PLAYER_EVENT_TYPE_AUDIO_FRAME] = user_data;
	LOGI("[%s] Event type : %d ",__FUNCTION__, _PLAYER_EVENT_TYPE_AUDIO_FRAME);

	int ret = mm_player_set_attribute(handle->mm_handle, NULL, "pcm_extraction",TRUE, "pcm_extraction_start_msec", start, "pcm_extraction_end_msec", end, NULL);	
	if(ret != MM_ERROR_NONE)
		return __convert_error_code(ret,(char*)__FUNCTION__);
	
	ret = mm_player_set_audio_buffer_callback(handle->mm_handle, __audio_stream_callback, (void*)handle);
	if(ret != MM_ERROR_NONE)
		return __convert_error_code(ret,(char*)__FUNCTION__);
	else
		return PLAYER_ERROR_NONE;
}

int player_unset_audio_frame_decoded_cb(player_h player)
{
	PLAYER_INSTANCE_CHECK(player);
	player_s * handle = (player_s *) player;

	int ret = mm_player_set_attribute(handle->mm_handle, NULL, "pcm_extraction",FALSE, NULL);
	if(ret != MM_ERROR_NONE)
		return __convert_error_code(ret,(char*)__FUNCTION__);
	
	handle->user_cb[_PLAYER_EVENT_TYPE_AUDIO_FRAME] = NULL;
	handle->user_data[_PLAYER_EVENT_TYPE_AUDIO_FRAME] = NULL;
	LOGI("[%s] Event type : %d ",__FUNCTION__, _PLAYER_EVENT_TYPE_AUDIO_FRAME);

	ret = mm_player_set_audio_buffer_callback(handle->mm_handle, NULL, NULL);
	if(ret != MM_ERROR_NONE)
		return __convert_error_code(ret,(char*)__FUNCTION__);
	else
		return PLAYER_ERROR_NONE;
}
