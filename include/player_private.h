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

#ifndef __TIZEN_MEDIA_PLAYER_PRIVATE_H__
#define	__TIZEN_MEDIA_PLAYER_PRIVATE_H__
#include <player.h>
#include <mm_player.h>
#include <system_info.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "TIZEN_N_PLAYER"
//#define USE_ECORE_FUNCTIONS

#define PLAYER_CHECK_CONDITION(condition,error,msg)     \
                if(condition) {} else \
                { LOGE("[%s] %s(0x%08x)",__FUNCTION__, msg,error); return error;}; \

#define PLAYER_INSTANCE_CHECK(player)   \
        PLAYER_CHECK_CONDITION(player != NULL, PLAYER_ERROR_INVALID_PARAMETER,"PLAYER_ERROR_INVALID_PARAMETER")

#define PLAYER_STATE_CHECK(player,expected_state)       \
        PLAYER_CHECK_CONDITION(player->state == expected_state,PLAYER_ERROR_INVALID_STATE,"PLAYER_ERROR_INVALID_STATE")

#define PLAYER_NULL_ARG_CHECK(arg)      \
        PLAYER_CHECK_CONDITION(arg != NULL,PLAYER_ERROR_INVALID_PARAMETER,"PLAYER_ERROR_INVALID_PARAMETER")

#define PLAYER_RANGE_ARG_CHECK(arg, min, max)      \
        PLAYER_CHECK_CONDITION(arg <= max,PLAYER_ERROR_INVALID_PARAMETER,"PLAYER_ERROR_INVALID_PARAMETER")		\
        PLAYER_CHECK_CONDITION(arg >= min,PLAYER_ERROR_INVALID_PARAMETER,"PLAYER_ERROR_INVALID_PARAMETER")

#ifdef TIZEN_TTRACE
#include <ttrace.h>
#define PLAYER_TRACE_BEGIN(NAME) traceBegin(TTRACE_TAG_VIDEO, NAME)
#define PLAYER_TRACE_END() traceEnd(TTRACE_TAG_VIDEO)
#define PLAYER_TRACE_ASYNC_BEGIN(NAME, COOKIE) traceAsyncBegin (TTRACE_TAG_VIDEO, COOKIE, NAME);
#define PLAYER_TRACE_ASYNC_END(NAME, COOKIE) traceAsyncEnd(TTRACE_TAG_VIDEO, COOKIE, NAME);
#else
#define PLAYER_TRACE_BEGIN(NAME)
#define PLAYER_TRACE_END()
#define PLAYER_TRACE_ASYNC_BEGIN(NAME, KEY)
#define PLAYER_TRACE_ASYNC_END(NAME, KEY)
#endif

typedef enum {
	_PLAYER_EVENT_TYPE_PREPARE,
	_PLAYER_EVENT_TYPE_COMPLETE,
	_PLAYER_EVENT_TYPE_INTERRUPT,
	_PLAYER_EVENT_TYPE_ERROR,
	_PLAYER_EVENT_TYPE_BUFFERING,
	_PLAYER_EVENT_TYPE_SUBTITLE,
	_PLAYER_EVENT_TYPE_CAPTURE,
	_PLAYER_EVENT_TYPE_SEEK,
	_PLAYER_EVENT_TYPE_MEDIA_PACKET_VIDEO_FRAME,
	_PLAYER_EVENT_TYPE_AUDIO_FRAME,
	_PLAYER_EVENT_TYPE_VIDEO_FRAME_RENDER_ERROR,
	_PLAYER_EVENT_TYPE_PD,
	_PLAYER_EVENT_TYPE_SUPPORTED_AUDIO_EFFECT,
	_PLAYER_EVENT_TYPE_SUPPORTED_AUDIO_EFFECT_PRESET,
	_PLAYER_EVENT_TYPE_MISSED_PLUGIN,
#ifdef _PLAYER_FOR_PRODUCT
	_PLAYER_EVENT_TYPE_IMAGE_BUFFER,
	_PLAYER_EVENT_TYPE_SELECTED_SUBTITLE_LANGUAGE,
#endif
	_PLAYER_EVENT_TYPE_MEDIA_STREAM_VIDEO_BUFFER_STATUS,
	_PLAYER_EVENT_TYPE_MEDIA_STREAM_AUDIO_BUFFER_STATUS,
	_PLAYER_EVENT_TYPE_MEDIA_STREAM_VIDEO_SEEK,
	_PLAYER_EVENT_TYPE_MEDIA_STREAM_AUDIO_SEEK,
	_PLAYER_EVENT_TYPE_AUDIO_STREAM_CHANGED,
	_PLAYER_EVENT_TYPE_VIDEO_STREAM_CHANGED,
	_PLAYER_EVENT_TYPE_VIDEO_BIN_CREATED,
	_PLAYER_EVENT_TYPE_NUM
}_player_event_e;

#ifndef USE_ECORE_FUNCTIONS
typedef enum {
	PLAYER_MESSAGE_NONE,
	PLAYER_MESSAGE_PREPARED,
	PLAYER_MESSAGE_ERROR,
	PLAYER_MESSAGE_SEEK_DONE,
	PLAYER_MESSAGE_EOS,
	PLAYER_MESSAGE_LOOP_EXIT,
	PLAYER_MESSAGE_MAX
}_player_message_e;
#endif

typedef struct _player_s{
	MMHandleType mm_handle;
	const void* user_cb[_PLAYER_EVENT_TYPE_NUM];
	void* user_data[_PLAYER_EVENT_TYPE_NUM];
#ifdef HAVE_WAYLAND
	void* wl_display;
#endif
	void* display_handle;
	player_display_type_e display_type;
	int state;
	bool is_set_pixmap_cb;
	bool is_stopped;
	bool is_display_visible;
	bool is_progressive_download;
	pthread_t prepare_async_thread;
#ifdef USE_ECORE_FUNCTIONS
	GHashTable *ecore_jobs;
#else
	pthread_t message_thread;
	GQueue *message_queue;
	GMutex message_queue_lock;
	GCond message_queue_cond;
	int current_message;
#endif
	player_error_e error_code;
	bool is_doing_jobs;
	media_format_h pkt_fmt;
} player_s;

int __player_convert_error_code(int code, char* func_name);
bool __player_state_validate(player_s * handle, player_state_e threshold);
int player_sound_register(player_h player, int pid);

/**
 * @brief Called when the video sink bin is crated.
 * @since_tizen 3.0
 * @param[out] caps			video sink current caps
 * @param[out ] user_data	The user data passed from the callback registration function
 * @see player_set_vidoe_bin_created_cb()
 * @see player_unset_vidoe_bin_created_cb()
 */
typedef void (*player_video_bin_created_cb)(const char *caps, void *user_data);

/**
 * @brief Registers a callback function to be invoked when video sink bin is created.
 * @since_tizen 3.0
 * @param[in] player    The handle to the media player
 * @param[in] callback The callback function to register
 * @param[in] user_data The user data to be passed to the callback function
 * @return @c 0 on success,
 *         otherwise a negative error value
 * @retval #PLAYER_ERROR_NONE Successful
 * @retval #PLAYER_ERROR_INVALID_PARAMETER Invalid parameter
 * @post player_video_bin_created_cb() will be invoked.
 * @see player_unset_vidoe_bin_created_cb()
 */
int player_set_video_bin_created_cb(player_h player, player_video_bin_created_cb callback, void *user_data);

/**
 * @brief Unregisters a callback function to be invoked when video sink bin is created.
 * @since_tizen 3.0
 * @param[in] player    The handle to the media player
 * @return @c 0 on success,
 *         otherwise a negative error value
 * @retval #PLAYER_ERROR_NONE Successful
 * @retval #PLAYER_ERROR_INVALID_PARAMETER Invalid parameter
 * @post player_video_bin_created_cb() will be invoked.
 * @see player_unset_vidoe_bin_created_cb()
 */
int player_unset_video_bin_created_cb(player_h player);


#ifdef __cplusplus
}
#endif

#endif //__TIZEN_MEDIA_PLAYER_PRIVATE_H__



