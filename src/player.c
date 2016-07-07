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
#include <sys/types.h>
#include <unistd.h>
#include <tbm_bufmgr.h>
#include <tbm_surface.h>
#include <tbm_surface_internal.h>
#include <Evas.h>
#include <Elementary.h>
#include <Ecore.h>
#include <Ecore_Wayland.h>
#include <dlog.h>
#include <mm_error.h>
#include <muse_core.h>
#include <muse_core_msg_json.h>
#include <muse_core_module.h>
#include <muse_core_ipc.h>
#include <muse_player.h>
#include <muse_player_msg.h>
#include <sound_manager.h>
#include <sound_manager_internal.h>
#include "player_internal.h"
#include "player_private.h"
#include "player_display.h"
#include "player_msg.h"
#ifdef TIZEN_FEATURE_EVAS_RENDERER
#include <mm_evas_renderer.h>
#endif
#define INVALID_SOCKET -1

typedef struct {
	int int_data;
	char *buf;
	callback_cb_info_s *cb_info;
} _player_cb_data;

typedef struct {
	intptr_t remote_pkt;
	gint fd;
	bool use_tsurf_pool;
} _media_pkt_fin_data;

/*
 Global varialbe
*/

/*
* Internal Implementation
*/
static int _player_deinit_memory_buffer(player_cli_s * pc);

int _player_media_packet_finalize(media_packet_h pkt, int error_code, void *user_data)
{
	int ret = MEDIA_PACKET_FINALIZE;
	muse_player_api_e api = MUSE_PLAYER_API_MEDIA_PACKET_FINALIZE_CB;
	_media_pkt_fin_data *fin_data = (_media_pkt_fin_data *) user_data;
	intptr_t packet;
	char *snd_msg = NULL;
	int snd_len = 0;

	if (!pkt) {
		LOGE("invalid parameter buffer %p, user_data %p", pkt, user_data);
		return ret;
	}

	if (!fin_data || fin_data->fd <= INVALID_SOCKET) {
		LOGE("invalid parameter, fd: %d", (fin_data) ? (fin_data->fd) : (-1));
		goto EXIT;
	}

	if (!fin_data->use_tsurf_pool) {
		tbm_surface_h tsurf = NULL;
		if (media_packet_get_tbm_surface(pkt, &tsurf) != MEDIA_PACKET_ERROR_NONE) {
			LOGE("media_packet_get_tbm_surface failed");
			/* continue the remained job */
		}
		if (tsurf) {
			LOGD("_player_media_packet_finalize tsurf destroy %p", tsurf);
			tbm_surface_destroy(tsurf);
			tsurf = NULL;
		}
	} else {
		/* Do not destroy tbm surface here to reuse during playback          *
		 * they will be destroyed at player_unprepare() or player_destroy(). *
		 * ref: __player_remove_tsurf_list()                                 */

		tbm_surface_h tsurf = NULL;

		if (media_packet_get_tbm_surface(pkt, &tsurf) == MEDIA_PACKET_ERROR_NONE) {
			LOGD("tsurf set to null %p", tsurf);
			tsurf = NULL;
		}
	}

	packet = fin_data->remote_pkt;
	snd_msg = muse_core_msg_json_factory_new(api, MUSE_TYPE_POINTER, "packet", packet, 0);
	snd_len = muse_core_ipc_send_msg(fin_data->fd, snd_msg);
	muse_core_msg_json_factory_free(snd_msg);

	if (snd_len <= 0) {
		LOGE("fail to send msg.");
	}

EXIT:
	if (fin_data) {
		g_free(fin_data);
		fin_data = NULL;
	}

	return ret;
}

static int __player_convert_error_code(int code, char *func_name)
{
	int ret = PLAYER_ERROR_INVALID_OPERATION;
	char *msg = "PLAYER_ERROR_INVALID_OPERATION";
	switch (code) {
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
		ret = PLAYER_ERROR_INVALID_OPERATION;
		msg = "PLAYER_ERROR_INVALID_OPERATION";
		break;
	case MM_ERROR_PLAYER_SOUND_EFFECT_INVALID_STATUS:
	case MM_ERROR_NOT_SUPPORT_API:
	case MM_ERROR_PLAYER_SOUND_EFFECT_NOT_SUPPORTED_FILTER:
		ret = PLAYER_ERROR_FEATURE_NOT_SUPPORTED_ON_DEVICE;
		msg = "PLAYER_ERROR_FEATURE_NOT_SUPPORTED_ON_DEVICE";
		break;
	case MM_ERROR_PLAYER_NO_FREE_SPACE:
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
	case MM_ERROR_PLAYER_STREAMING_DNS_FAIL:
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
	case MM_ERROR_PLAYER_DRM_OUTPUT_PROTECTION:
		ret = PLAYER_ERROR_DRM_NOT_PERMITTED;
		msg = "PLAYER_ERROR_DRM_NOT_PERMITTED";
		break;
	case MM_ERROR_PLAYER_RESOURCE_LIMIT:
		ret = PLAYER_ERROR_RESOURCE_LIMIT;
		msg = "PLAYER_ERROR_RESOURCE_LIMIT";
		break;
	case MM_ERROR_PLAYER_PERMISSION_DENIED:
		ret = PLAYER_ERROR_PERMISSION_DENIED;
		msg = "PLAYER_ERROR_PERMISSION_DENIED";
	}
	LOGE("[%s] %s(0x%08x) : core fw error(0x%x)", func_name, msg, ret, code);
	return ret;
}

static void *_get_mem(player_cli_s * player, int size)
{
	player_data_s *mem = g_new(player_data_s, sizeof(player_data_s));
	if (mem) {
		mem->data = g_new(void, size);
		mem->next = player->head;
		player->head = mem;
		return mem->data;
	}
	return NULL;
}

static void _del_mem(player_cli_s * player)
{
	player_data_s *mem;
	while (player->head) {
		mem = player->head->next;
		g_free(player->head->data);
		g_free(player->head);
		player->head = mem;
	}
}

static int player_recv_msg(callback_cb_info_s * cb_info)
{
	int recvLen = 0;
	msg_buff_s *buff = &cb_info->buff;

	memset(buff->recvMsg, 0x00, sizeof(char)*buff->bufLen);
	recvLen = muse_core_ipc_recv_msg(cb_info->fd, buff->recvMsg);

	/* check the first msg */
	if (buff->part_of_msg && buff->recvMsg[0] != '{')
	{
		gchar *tmp = strndup(buff->recvMsg, recvLen);
		if (!tmp) {
			LOGE("failed to copy msg.");
			return 0;
		}

		LOGD("get remained part of msg %d + %d, %d", recvLen, strlen(buff->part_of_msg), buff->bufLen);

		/* realloc buffer */
		if (recvLen+strlen(buff->part_of_msg) >= buff->bufLen) {
			LOGD("realloc Buffer %d -> %d", buff->bufLen, recvLen+strlen(buff->part_of_msg)+1);
			buff->bufLen = recvLen+strlen(buff->part_of_msg)+1;
			buff->recvMsg = g_renew(char, buff->recvMsg, buff->bufLen);
			if (!buff->recvMsg) {
				LOGE("failed renew buffer.");
				if (tmp)
					free(tmp);
				return 0;
			}
			memset(buff->recvMsg, 0x00, sizeof(char)*buff->bufLen);
		}
		snprintf(buff->recvMsg, buff->bufLen, "%s%s", buff->part_of_msg, tmp);
		recvLen += strlen(buff->part_of_msg);

		free(buff->part_of_msg);
		buff->part_of_msg = NULL;
		free(tmp);
		tmp = NULL;
	}

	/* check the last msg */
	if (buff->recvMsg[recvLen-1] != '}') {
		char *part_pos = strrchr(buff->recvMsg, '}');
		int part_len = ((part_pos) ? (strlen(part_pos+1)) : (0));

		if (part_len > 0) {
			buff->part_of_msg = strndup(part_pos+1, part_len);
			if (!buff->part_of_msg) {
				LOGE("failed to alloc buffer for part of msg.");
				return 0;
			}
			LOGD("get part of msg: %d, %s", strlen(buff->part_of_msg), buff->part_of_msg);
			recvLen -= part_len;
		}
	}

	return recvLen;
}

static void set_null_user_cb(callback_cb_info_s * cb_info, muse_player_event_e event)
{
	if (cb_info && event < MUSE_PLAYER_EVENT_TYPE_NUM) {
		cb_info->user_cb[event] = NULL;
		cb_info->user_data[event] = NULL;
	}
}

static void set_null_user_cb_lock(callback_cb_info_s * cb_info, muse_player_event_e event)
{
	bool lock = g_thread_self() != cb_info->event_queue.thread;

	if (lock)
		g_mutex_lock(&cb_info->event_queue.mutex);

	set_null_user_cb(cb_info, event);

	if (lock)
		g_mutex_unlock(&cb_info->event_queue.mutex);
}

static int __set_callback(muse_player_event_e type, player_h player, void *callback, void *user_data)
{
	PLAYER_INSTANCE_CHECK(player);
	PLAYER_NULL_ARG_CHECK(callback);
	int ret = PLAYER_ERROR_NONE;
	player_cli_s *pc = (player_cli_s *) player;
	muse_player_api_e api = MUSE_PLAYER_API_SET_CALLBACK;
	int set = 1;

	LOGI("Event type : %d ", type);
	player_msg_set_callback(api, pc, ret, type, set);

	if (ret == PLAYER_ERROR_NONE) {
		pc->cb_info->user_cb[type] = callback;
		pc->cb_info->user_data[type] = user_data;
	}
	return ret;
}

static int __unset_callback(muse_player_event_e type, player_h player)
{
	PLAYER_INSTANCE_CHECK(player);
	int ret = PLAYER_ERROR_NONE;
	player_cli_s *pc = (player_cli_s *) player;
	muse_player_api_e api = MUSE_PLAYER_API_SET_CALLBACK;
	int set = 0;

	LOGI("Event type : %d ", type);

	PLAYER_NULL_ARG_CHECK(CALLBACK_INFO(pc));
	set_null_user_cb_lock(CALLBACK_INFO(pc), type);

	player_msg_set_callback(api, pc, ret, type, set);
	ret = PLAYER_ERROR_NONE;

	return ret;
}

static void __prepare_cb_handler(callback_cb_info_s * cb_info, char *recvMsg)
{
	muse_player_event_e ev = MUSE_PLAYER_EVENT_TYPE_PREPARE;

	((player_prepared_cb) cb_info->user_cb[ev]) (cb_info->user_data[ev]);

	set_null_user_cb(cb_info, ev);
}

static void __complete_cb_handler(callback_cb_info_s * cb_info, char *recvMsg)
{
	muse_player_event_e ev = MUSE_PLAYER_EVENT_TYPE_COMPLETE;
	((player_completed_cb) cb_info->user_cb[ev]) (cb_info->user_data[ev]);
}

static void __interrupt_cb_handler(callback_cb_info_s * cb_info, char *recvMsg)
{
	int code;
	muse_player_event_e ev = MUSE_PLAYER_EVENT_TYPE_INTERRUPT;

	if (player_msg_get(code, recvMsg))
		((player_interrupted_cb) cb_info->user_cb[ev]) (code, cb_info->user_data[ev]);
}

static void __error_cb_handler(callback_cb_info_s * cb_info, char *recvMsg)
{
	int code;
	muse_player_event_e ev = MUSE_PLAYER_EVENT_TYPE_ERROR;

	if (player_msg_get(code, recvMsg))
		((player_error_cb) cb_info->user_cb[ev]) (code, cb_info->user_data[ev]);
}

static void __buffering_cb_handler(callback_cb_info_s * cb_info, char *recvMsg)
{
	int percent;
	muse_player_event_e ev = MUSE_PLAYER_EVENT_TYPE_BUFFERING;

	if (player_msg_get(percent, recvMsg))
		((player_buffering_cb) cb_info->user_cb[ev]) (percent, cb_info->user_data[ev]);
}

static void __subtitle_cb_handler(callback_cb_info_s * cb_info, char *recvMsg)
{
	int duration = 0;
	char text[MUSE_URI_MAX_LENGTH] = { 0, };
	muse_player_event_e ev = MUSE_PLAYER_EVENT_TYPE_SUBTITLE;
	bool ret = TRUE;

	player_msg_get1_string(recvMsg, duration, INT, text, ret);
	if (ret)
		((player_subtitle_updated_cb) cb_info->user_cb[ev]) (duration, text, cb_info->user_data[ev]);
}

static void __capture_cb_handler(callback_cb_info_s * cb_info, char *recvMsg)
{
	unsigned char *data = NULL;
	int width = 0;
	int height = 0;
	unsigned int size = 0;
	tbm_bo bo = NULL;
	tbm_bo_handle thandle;
	tbm_key key = 0;
	bool ret_val = TRUE;

	player_msg_get3(recvMsg, width, INT, height, INT, size, INT, ret_val);
	if (ret_val) {
		if (!player_msg_get(key, recvMsg)) {
			LOGE("There is no tbm_key value. %d", key);
			goto EXIT;
		}

		bo = tbm_bo_import(cb_info->bufmgr, key);
		if (bo == NULL) {
			LOGE("TBM get error : bo is NULL");
			goto EXIT;
		}
		thandle = tbm_bo_map(bo, TBM_DEVICE_CPU, TBM_OPTION_WRITE | TBM_OPTION_READ);
		if (thandle.ptr == NULL) {
			LOGE("TBM get error : handle pointer is NULL");
			goto EXIT;
		}
		data = g_new(unsigned char, size);
		if (data) {
			memcpy(data, thandle.ptr, size);
			((player_video_captured_cb) cb_info->user_cb[MUSE_PLAYER_EVENT_TYPE_CAPTURE]) (data, width, height, size, cb_info->user_data[MUSE_PLAYER_EVENT_TYPE_CAPTURE]);
			g_free(data);
		} else
			LOGE("g_new failure");

		tbm_bo_unmap(bo);
	}

EXIT:
	if (bo)
		tbm_bo_unref(bo);

	/* return buffer */
	if (key != 0) {
		LOGD("send msg to release buffer. key:%d", key);
		player_msg_send1_no_return(MUSE_PLAYER_API_RETURN_BUFFER, cb_info->fd, INT, key);
	}

	set_null_user_cb(cb_info, MUSE_PLAYER_EVENT_TYPE_CAPTURE);
}

static void __seek_cb_handler(callback_cb_info_s * cb_info, char *recvMsg)
{
	muse_player_event_e ev = MUSE_PLAYER_EVENT_TYPE_SEEK;

	g_mutex_lock(&cb_info->seek_cb_mutex);
	if (cb_info->user_cb[ev] && cb_info->block_seek_cb == FALSE) {
		LOGD("call seek cb");
		((player_seek_completed_cb) cb_info->user_cb[ev]) (cb_info->user_data[ev]);
		set_null_user_cb(cb_info, ev);
	} else {
		LOGW("ignored. seek cb %p, block %d", cb_info->user_cb[ev], cb_info->block_seek_cb);
	}
	g_mutex_unlock(&cb_info->seek_cb_mutex);
}

static void __player_remove_tsurf_list(player_cli_s * pc)
{
	GList *l = NULL;

	g_mutex_lock(&pc->cb_info->data_mutex);
	if (pc->cb_info->tsurf_list) {
		LOGD("total num of tsurf list = %d", g_list_length(pc->cb_info->tsurf_list));

		for (l = g_list_first(pc->cb_info->tsurf_list); l; l = g_list_next(l)) {
			player_tsurf_info_t* tmp = (player_tsurf_info_t *)l->data;

			LOGD("%p will be removed", tmp);
			if (tmp) {
				if (tmp->tsurf) {
					tbm_surface_destroy(tmp->tsurf);
					tmp->tsurf = NULL;
				}
				g_free(tmp);
			}
		}
		g_list_free(pc->cb_info->tsurf_list);
		pc->cb_info->tsurf_list = NULL;
	}
	g_mutex_unlock(&pc->cb_info->data_mutex);
	return;
}

static player_tsurf_info_t* __player_get_tsurf_from_list(callback_cb_info_s * cb_info, tbm_key key, tbm_surface_info_s sinfo)
{
	GList *l = NULL;

	g_mutex_lock(&cb_info->data_mutex);
	for (l = g_list_first(cb_info->tsurf_list); l; l = g_list_next(l)) {
		player_tsurf_info_t *tmp = (player_tsurf_info_t *)l->data;
		if (tmp && (tmp->key == key)) {
			LOGD("found tsurf_data of tbm_key %d", key);

			/* need to check tsuf info to support DRC */
			if ((tbm_surface_get_height(tmp->tsurf) != sinfo.height) ||
				(tbm_surface_get_width(tmp->tsurf) != sinfo.width)) {

				cb_info->tsurf_list = g_list_remove(cb_info->tsurf_list, tmp);
				LOGW("tsurf info is changed. need to create new tsurf.");
				tbm_surface_destroy(tmp->tsurf);
				g_free(tmp);

				g_mutex_unlock(&cb_info->data_mutex);
				return NULL;

			} else {
				g_mutex_unlock(&cb_info->data_mutex);
				return tmp;
			}
		}
	}
	g_mutex_unlock(&cb_info->data_mutex);
	LOGD("there is no tsurf_data for tbm_key:%d", key);
	return NULL;
}

static void __media_packet_video_frame_cb_handler(callback_cb_info_s * cb_info, char *recvMsg)
{
	tbm_bo bo[4] = { NULL, };
	tbm_key key[4] = { 0, };
	tbm_surface_info_s sinfo;
	char *surface_info = (char *)&sinfo;
	media_packet_h pkt = NULL;
	tbm_surface_h tsurf = NULL;
	player_tsurf_info_t *tsurf_data = NULL;
	int bo_num = 0;
	media_format_mimetype_e mimetype = MEDIA_FORMAT_NV12;
	bool make_pkt_fmt = false;
	int ret = MEDIA_FORMAT_ERROR_NONE;
	_media_pkt_fin_data *fin_data = NULL;
	intptr_t packet;
	uint64_t pts = 0;
	int i = 0;
	muse_core_msg_parse_err_e err = MUSE_MSG_PARSE_ERROR_NONE;

	void *jobj = muse_core_msg_json_object_new(recvMsg, NULL, &err);
	if (!jobj ||
		!muse_core_msg_json_object_get_value("key[0]", jobj, &key[0], MUSE_TYPE_ANY) ||
		!muse_core_msg_json_object_get_value("key[1]", jobj, &key[1], MUSE_TYPE_ANY) ||
		!muse_core_msg_json_object_get_value("key[2]", jobj, &key[2], MUSE_TYPE_ANY) ||
		!muse_core_msg_json_object_get_value("key[3]", jobj, &key[3], MUSE_TYPE_ANY) ||
		!muse_core_msg_json_object_get_value("packet", jobj, &packet, MUSE_TYPE_POINTER) ||
		!muse_core_msg_json_object_get_value("mimetype", jobj, &mimetype, MUSE_TYPE_ANY) ||
		!muse_core_msg_json_object_get_value("pts", jobj, &pts, MUSE_TYPE_INT64) ||
		!muse_core_msg_json_object_get_value("surface_info", jobj, surface_info, MUSE_TYPE_ARRAY)) {

		LOGE("failed to get value from msg. jobj:%p, err:%d", jobj, err);
		if (jobj)
			muse_core_msg_json_object_free(jobj);
		return;
	}
	muse_core_msg_json_object_free(jobj);

	LOGD("width %d, height %d", sinfo.width, sinfo.height);

	if (!cb_info) {
		LOGE("cb_info is null");
		return;
	}

	if (!cb_info->user_cb[MUSE_PLAYER_EVENT_TYPE_MEDIA_PACKET_VIDEO_FRAME]) {
		/* send msg to release packet. */
		LOGE("_video_decoded_cb is not set");
		player_msg_send1_no_return(MUSE_PLAYER_API_MEDIA_PACKET_FINALIZE_CB, cb_info->fd, POINTER, packet);
		return;
	}

	for (i = 0; i < 4; i++) {
		if (key[i]) {
			bo_num++;
			bo[i] = tbm_bo_import(cb_info->bufmgr, key[i]);
		}
	}

	tsurf_data = __player_get_tsurf_from_list(cb_info, key[0], sinfo);
	if (!tsurf_data) {
		tsurf_data = g_new(player_tsurf_info_t, 1);
		if (!tsurf_data) {
			LOGE("failed to alloc tsurf info");
			goto ERROR;
		}

		tsurf = tbm_surface_internal_create_with_bos(&sinfo, bo, bo_num);
		if (!tsurf) {
			LOGE("failed to create tbm surface");
			g_free(tsurf_data);
			goto ERROR;
		}
		tsurf_data->key = key[0];
		tsurf_data->tsurf = tsurf;
		if (cb_info->use_tsurf_pool) {
			g_mutex_lock(&cb_info->data_mutex);
			cb_info->tsurf_list = g_list_append(cb_info->tsurf_list, tsurf_data);
			LOGD("key %d is added to the pool", key[0]);
			if (cb_info->video_frame_pool_size < g_list_length(cb_info->tsurf_list))
				LOGE("need to check the pool size: %d < %d", cb_info->video_frame_pool_size, g_list_length(cb_info->tsurf_list));
			g_mutex_unlock(&cb_info->data_mutex);
		}
	} else {
		if (tsurf_data->tsurf) {
			tsurf = tsurf_data->tsurf;
		} else {
			LOGE("tsurf_data->tsurf is null (never enter here)");
			goto ERROR;
		}
	}

	/* check media packet format */
	if (cb_info->pkt_fmt) {
		int pkt_fmt_width = 0;
		int pkt_fmt_height = 0;
		media_format_mimetype_e pkt_fmt_mimetype = MEDIA_FORMAT_NV12;

		media_format_get_video_info(cb_info->pkt_fmt, &pkt_fmt_mimetype, &pkt_fmt_width, &pkt_fmt_height, NULL, NULL);
		if (pkt_fmt_mimetype != mimetype || pkt_fmt_width != sinfo.width || pkt_fmt_height != sinfo.height) {
			LOGW("different format. current 0x%x, %dx%d, new 0x%x, %dx%d", pkt_fmt_mimetype, pkt_fmt_width, pkt_fmt_height, mimetype, sinfo.width, sinfo.height);
			media_format_unref(cb_info->pkt_fmt);
			cb_info->pkt_fmt = NULL;
			make_pkt_fmt = true;
		}
	} else {
		make_pkt_fmt = true;
	}
	/* create packet format */
	if (make_pkt_fmt) {
		LOGI("make new pkt_fmt - mimetype 0x%x, %dx%d", mimetype, sinfo.width, sinfo.height);
		ret = media_format_create(&cb_info->pkt_fmt);
		if (ret == MEDIA_FORMAT_ERROR_NONE) {
			ret = media_format_set_video_mime(cb_info->pkt_fmt, mimetype);
			ret |= media_format_set_video_width(cb_info->pkt_fmt, sinfo.width);
			ret |= media_format_set_video_height(cb_info->pkt_fmt, sinfo.height);
			LOGI("media_format_set_video_mime,width,height ret : 0x%x", ret);
		} else {
			LOGE("media_format_create failed");
		}
	}

	fin_data = g_new(_media_pkt_fin_data, 1);
	if (!fin_data) {
		LOGE("failed to alloc fin_data");
		goto ERROR;
	}
	fin_data->remote_pkt = packet;
	fin_data->fd = cb_info->fd;
	fin_data->use_tsurf_pool = cb_info->use_tsurf_pool;
	ret = media_packet_create_from_tbm_surface(cb_info->pkt_fmt, tsurf, (media_packet_finalize_cb) _player_media_packet_finalize, (void *)fin_data, &pkt);
	if (ret != MEDIA_PACKET_ERROR_NONE || !pkt) {
		LOGE("media_packet_create_from_tbm_surface failed %d %p", ret, pkt);
		goto ERROR;
	}

	if (pts != 0) {
		ret = media_packet_set_pts(pkt, (uint64_t) pts);
		if (ret != MEDIA_PACKET_ERROR_NONE)
			LOGE("media_packet_set_pts failed");
	}
	if (cb_info->user_cb[MUSE_PLAYER_EVENT_TYPE_MEDIA_PACKET_VIDEO_FRAME]) {
		/* call media packet callback */
		((player_media_packet_video_decoded_cb) cb_info->user_cb[MUSE_PLAYER_EVENT_TYPE_MEDIA_PACKET_VIDEO_FRAME]) (pkt, cb_info->user_data[MUSE_PLAYER_EVENT_TYPE_MEDIA_PACKET_VIDEO_FRAME]);
	} else {
		LOGE("_video_decoded_cb is not set");
		media_packet_destroy(pkt);
	}

	for (i = 0; i < bo_num; i++) {
		if (bo[i])
			tbm_bo_unref(bo[i]);
	}
	return;

ERROR:
	if (pkt)
		media_packet_destroy(pkt);

	if (fin_data)
		g_free(fin_data);

	for (i = 0; i < bo_num; i++) {
		if (bo[i])
			tbm_bo_unref(bo[i]);
	}

	player_msg_send1_no_return(MUSE_PLAYER_API_MEDIA_PACKET_FINALIZE_CB, cb_info->fd, POINTER, packet);
	return;
}

static void __audio_frame_cb_handler(callback_cb_info_s * cb_info, char *recvMsg)
{
	tbm_bo bo = NULL;
	tbm_bo_handle thandle;
	tbm_key key = 0;
	player_audio_raw_data_s audio;

	if (!player_msg_get(key, recvMsg)) {
		LOGE("failed to get value from msg.");
		return;
	}

	bo = tbm_bo_import(cb_info->bufmgr, key);
	if (bo == NULL) {
		LOGE("TBM get error : bo is NULL, key:%d", key);
		goto EXIT;
	}

	thandle = tbm_bo_map(bo, TBM_DEVICE_CPU, TBM_OPTION_WRITE | TBM_OPTION_READ);
	if (thandle.ptr == NULL) {
		LOGE("TBM get error : handle pointer is NULL");
		goto EXIT;
	}

	memcpy(&audio, thandle.ptr, sizeof(player_audio_raw_data_s));
	audio.data = thandle.ptr + sizeof(player_audio_raw_data_s);

	LOGD("user callback data %p, size %d", audio.data, audio.size);
	((player_audio_pcm_extraction_cb) cb_info->user_cb[MUSE_PLAYER_EVENT_TYPE_AUDIO_FRAME]) (&audio, cb_info->user_data[MUSE_PLAYER_EVENT_TYPE_AUDIO_FRAME]);
	tbm_bo_unmap(bo);

EXIT:
	if (bo)
		tbm_bo_unref(bo);

	/* return buffer */
	if (key != 0) {
		LOGD("send msg to release buffer. key:%d", key);
		player_msg_send1_no_return(MUSE_PLAYER_API_RETURN_BUFFER, cb_info->fd, INT, key);
	}
}

static void __video_frame_render_error_cb_handler(callback_cb_info_s * cb_info, char *recvMsg)
{
}

static void __pd_cb_handler(callback_cb_info_s * cb_info, char *recvMsg)
{
	int type;
	muse_player_event_e ev = MUSE_PLAYER_EVENT_TYPE_PD;

	if (player_msg_get(type, recvMsg))
		((player_pd_message_cb) cb_info->user_cb[ev]) (type, cb_info->user_data[ev]);
}

static void __supported_audio_effect_cb_handler(callback_cb_info_s * cb_info, char *recvMsg)
{
}

static void __supported_audio_effect_freset_cb_handler(callback_cb_info_s * cb_info, char *recvMsg)
{
}

static void __missed_plugin_cb_handler(callback_cb_info_s * cb_info, char *recvMsg)
{
}

static void __media_stream_video_buffer_cb_handler(callback_cb_info_s * cb_info, char *recvMsg)
{
	/* player_media_stream_buffer_status_e status; */
	int status;

	if (player_msg_get(status, recvMsg)) {
		((player_media_stream_buffer_status_cb)
		 cb_info->user_cb[MUSE_PLAYER_EVENT_TYPE_MEDIA_STREAM_VIDEO_BUFFER_STATUS]) ((player_media_stream_buffer_status_e) status, cb_info->user_data[MUSE_PLAYER_EVENT_TYPE_MEDIA_STREAM_VIDEO_BUFFER_STATUS]);
	}
}

static void __media_stream_audio_buffer_cb_handler(callback_cb_info_s * cb_info, char *recvMsg)
{
	/* player_media_stream_buffer_status_e status; */
	int status;

	if (player_msg_get(status, recvMsg)) {
		((player_media_stream_buffer_status_cb)
			cb_info->user_cb[MUSE_PLAYER_EVENT_TYPE_MEDIA_STREAM_AUDIO_BUFFER_STATUS])
			((player_media_stream_buffer_status_e) status, cb_info->user_data[MUSE_PLAYER_EVENT_TYPE_MEDIA_STREAM_AUDIO_BUFFER_STATUS]);
	}

}

static void __media_stream_video_buffer_cb_handler_ex(callback_cb_info_s * cb_info, char *recvMsg)
{
	/* player_media_stream_buffer_status_e status; */
	int status;
	unsigned long long bytes;
	bool ret = TRUE;

	player_msg_get2(recvMsg, status, INT, bytes, INT64, ret);
	if (ret == TRUE) {
		((player_media_stream_buffer_status_cb_ex)
			cb_info->user_cb[MUSE_PLAYER_EVENT_TYPE_MEDIA_STREAM_VIDEO_BUFFER_STATUS_WITH_INFO])
			((player_media_stream_buffer_status_e) status, bytes, cb_info->user_data[MUSE_PLAYER_EVENT_TYPE_MEDIA_STREAM_VIDEO_BUFFER_STATUS_WITH_INFO]);
	}
}

static void __media_stream_audio_buffer_cb_handler_ex(callback_cb_info_s * cb_info, char *recvMsg)
{
	/* player_media_stream_buffer_status_e status; */
	int status;
	unsigned long long bytes;
	bool ret = TRUE;

	player_msg_get2(recvMsg, status, INT, bytes, INT64, ret);
	if (ret == TRUE) {
		((player_media_stream_buffer_status_cb_ex)
			cb_info->user_cb[MUSE_PLAYER_EVENT_TYPE_MEDIA_STREAM_AUDIO_BUFFER_STATUS_WITH_INFO])
			((player_media_stream_buffer_status_e) status, bytes, cb_info->user_data[MUSE_PLAYER_EVENT_TYPE_MEDIA_STREAM_AUDIO_BUFFER_STATUS_WITH_INFO]);
	}

}

static void __media_stream_video_seek_cb_handler(callback_cb_info_s * cb_info, char *recvMsg)
{
	unsigned long long offset;

	if (player_msg_get_type(offset, recvMsg, INT64)) {
		((player_media_stream_seek_cb)
			cb_info->user_cb[MUSE_PLAYER_EVENT_TYPE_MEDIA_STREAM_VIDEO_SEEK])
			(offset, cb_info->user_data[MUSE_PLAYER_EVENT_TYPE_MEDIA_STREAM_VIDEO_SEEK]);
	}
}

static void __media_stream_audio_seek_cb_handler(callback_cb_info_s * cb_info, char *recvMsg)
{
	unsigned long long offset;

	if (player_msg_get_type(offset, recvMsg, INT64)) {
		((player_media_stream_seek_cb)
			cb_info->user_cb[MUSE_PLAYER_EVENT_TYPE_MEDIA_STREAM_AUDIO_SEEK])
			(offset, cb_info->user_data[MUSE_PLAYER_EVENT_TYPE_MEDIA_STREAM_AUDIO_SEEK]);
	}
}

static void __video_stream_changed_cb_handler(callback_cb_info_s * cb_info, char *recvMsg)
{
	int width;
	int height;
	int fps;
	int bit_rate;
	bool ret_val = TRUE;

	player_msg_get4(recvMsg, width, INT, height, INT, fps, INT, bit_rate, INT, ret_val);
	if (ret_val) {
		((player_video_stream_changed_cb) cb_info->user_cb[MUSE_PLAYER_EVENT_TYPE_VIDEO_STREAM_CHANGED])
			(width, height, fps, bit_rate, cb_info->user_data[MUSE_PLAYER_EVENT_TYPE_VIDEO_STREAM_CHANGED]);
	}
}

static void (*_user_callbacks[MUSE_PLAYER_EVENT_TYPE_NUM]) (callback_cb_info_s * cb_info, char *recvMsg) = {
	__prepare_cb_handler,		/* MUSE_PLAYER_EVENT_TYPE_PREPARE */
	__complete_cb_handler,	/* MUSE_PLAYER_EVENT_TYPE_COMPLETE */
	__interrupt_cb_handler,	/* MUSE_PLAYER_EVENT_TYPE_INTERRUPT */
	__error_cb_handler,		/* MUSE_PLAYER_EVENT_TYPE_ERROR */
	__buffering_cb_handler,	/* MUSE_PLAYER_EVENT_TYPE_BUFFERING */
	__subtitle_cb_handler,	/* MUSE_PLAYER_EVENT_TYPE_SUBTITLE */
	__capture_cb_handler,	/* MUSE_PLAYER_EVENT_TYPE_CAPTURE */
	__seek_cb_handler,		/* MUSE_PLAYER_EVENT_TYPE_SEEK */
	__media_packet_video_frame_cb_handler,	/* MUSE_PLAYER_EVENT_TYPE_MEDIA_PACKET_VIDEO_FRAME */
	__audio_frame_cb_handler,	/* MUSE_PLAYER_EVENT_TYPE_AUDIO_FRAME */
	__video_frame_render_error_cb_handler,	/* MUSE_PLAYER_EVENT_TYPE_VIDEO_FRAME_RENDER_ERROR */
	__pd_cb_handler,			/* MUSE_PLAYER_EVENT_TYPE_PD */
	__supported_audio_effect_cb_handler,	/* MUSE_PLAYER_EVENT_TYPE_SUPPORTED_AUDIO_EFFECT */
	__supported_audio_effect_freset_cb_handler,	/* MUSE_PLAYER_EVENT_TYPE_SUPPORTED_AUDIO_EFFECT_PRESET */
	__missed_plugin_cb_handler,	/* MUSE_PLAYER_EVENT_TYPE_MISSED_PLUGIN */
#ifdef _PLAYER_FOR_PRODUCT
	NULL,	/* MUSE_PLAYER_EVENT_TYPE_IMAGE_BUFFER */
	NULL,	/* MUSE_PLAYER_EVENT_TYPE_SELECTED_SUBTITLE_LANGUAGE */
#endif
	__media_stream_video_buffer_cb_handler,	/* MUSE_PLAYER_EVENT_TYPE_MEDIA_STREAM_VIDEO_BUFFER_STATUS */
	__media_stream_audio_buffer_cb_handler,	/* MUSE_PLAYER_EVENT_TYPE_MEDIA_STREAM_AUDIO_BUFFER_STATUS */
	__media_stream_video_buffer_cb_handler_ex,	/* MUSE_PLAYER_EVENT_TYPE_MEDIA_STREAM_VIDEO_BUFFER_STATUS_WITH_INFO */
	__media_stream_audio_buffer_cb_handler_ex,	/* MUSE_PLAYER_EVENT_TYPE_MEDIA_STREAM_AUDIO_BUFFER_STATUS_WITH_INFO */
	__media_stream_video_seek_cb_handler,	/* MUSE_PLAYER_EVENT_TYPE_MEDIA_STREAM_VIDEO_SEEK */
	__media_stream_audio_seek_cb_handler,	/* MUSE_PLAYER_EVENT_TYPE_MEDIA_STREAM_AUDIO_SEEK */
	NULL,	/* MUSE_PLAYER_EVENT_TYPE_AUDIO_STREAM_CHANGED */
	__video_stream_changed_cb_handler,		/* MUSE_PLAYER_EVENT_TYPE_VIDEO_STREAM_CHANGED */
};

static void _player_event_job_function(_player_cb_data * data)
{
	muse_player_event_e ev = data->int_data;
	if (data->cb_info->user_cb[ev])
		_user_callbacks[ev] (data->cb_info, data->buf);
	else
		LOGW("user callback is unset. type : %d", ev);

	g_free(data->buf);
	g_free(data);
}

static void *_player_event_queue_loop(void *param)
{
	if (!param) {
		LOGE("NULL parameter");
		return NULL;
	}
	callback_cb_info_s *cb_info = param;
	player_event_queue *ev = &cb_info->event_queue;
	_player_cb_data *event_data;

	g_mutex_lock(&ev->mutex);
	while (ev->running) {
		g_mutex_lock(&ev->qlock);
		if (g_queue_is_empty(ev->queue)) {
			g_mutex_unlock(&ev->qlock);
			g_cond_wait(&ev->cond, &ev->mutex);
			if (!ev->running)
				break;
		} else
			g_mutex_unlock(&ev->qlock);

		while (1) {
			g_mutex_lock(&ev->qlock);
			event_data = (_player_cb_data *) g_queue_pop_head(ev->queue);
			g_mutex_unlock(&ev->qlock);
			if (event_data)
				_player_event_job_function(event_data);
			else
				break;
		}
	}
	g_mutex_unlock(&ev->mutex);
	LOGI("Exit event loop");
	return NULL;
}

static gboolean _player_event_queue_new(callback_cb_info_s * cb_info)
{
	g_return_val_if_fail(cb_info, FALSE);
	player_event_queue *ev = &cb_info->event_queue;

	ev->queue = g_queue_new();
	g_return_val_if_fail(ev->queue, FALSE);
	g_mutex_init(&ev->qlock);

	g_mutex_init(&ev->mutex);
	g_cond_init(&ev->cond);
	ev->running = TRUE;
	ev->thread = g_thread_new("cb_event_thread", _player_event_queue_loop, (gpointer) cb_info);
	g_return_val_if_fail(ev->thread, FALSE);
	LOGI("event queue thread %p", ev->thread);

	return TRUE;

}

static void _player_event_queue_destroy(callback_cb_info_s * cb_info)
{
	g_return_if_fail(cb_info);
	player_event_queue *ev = &cb_info->event_queue;
	_player_cb_data *event_data;

	LOGI("event queue thread %p", ev->thread);

	g_mutex_lock(&ev->mutex);
	ev->running = FALSE;
	g_cond_broadcast(&ev->cond);
	g_mutex_unlock(&ev->mutex);

	g_thread_join(ev->thread);
	g_thread_unref(ev->thread);

	while (!g_queue_is_empty(ev->queue)) {
		event_data = (_player_cb_data *) g_queue_pop_head(ev->queue);
		if (event_data) {
			g_free(event_data->buf);
			g_free(event_data);
		}
	}
	g_queue_free(ev->queue);
	g_mutex_clear(&ev->qlock);
	g_mutex_clear(&ev->mutex);
	g_cond_clear(&ev->cond);

}

static void _player_event_queue_remove(player_event_queue * ev_queue, int ev)
{
	GList *item;

	g_mutex_lock(&ev_queue->qlock);

	item = g_queue_peek_head_link(ev_queue->queue);
	while (item) {
		GList *next = item->next;
		_player_cb_data *cb_data = (_player_cb_data *)item->data;

		if (cb_data && cb_data->int_data == ev) {
			LOGD("removing '%p (ev:%d)' from event queue", cb_data, cb_data->int_data);
			g_free(cb_data->buf);
			g_free(cb_data);

			g_queue_delete_link(ev_queue->queue, item);
		}
		item = next;
	}
	g_mutex_unlock(&ev_queue->qlock);
}

static void _player_event_queue_add(player_event_queue * ev, _player_cb_data * data)
{
	if (ev->running) {
		g_mutex_lock(&ev->qlock);
		g_queue_push_tail(ev->queue, (gpointer) data);
		g_mutex_unlock(&ev->qlock);
		g_cond_signal(&ev->cond);
	}
}

static void _user_callback_handler(callback_cb_info_s * cb_info, muse_player_event_e event, char *buffer)
{
	LOGD("get event %d", event);

	if (event < MUSE_PLAYER_EVENT_TYPE_NUM) {
		if (cb_info->user_cb[event] && _user_callbacks[event]) {
			_player_cb_data *data = NULL;
			data = g_new(_player_cb_data, 1);
			if (!data) {
				LOGE("fail to alloc mem");
				return;
			}
			data->int_data = (int)event;
			data->cb_info = cb_info;
			data->buf = buffer;
			_player_event_queue_add(&cb_info->event_queue, data);
		}
	}
}

static void _add_ret_msg(muse_player_api_e api, callback_cb_info_s * cb_info, int offset, int parse_len)
{
	ret_msg_s *msg = NULL;
	ret_msg_s *last = cb_info->buff.retMsgHead;

	msg = g_new(ret_msg_s, 1);
	if (msg) {
		msg->api = api;
		msg->msg = strndup(cb_info->buff.recvMsg + offset, parse_len);
		msg->next = NULL;
		if (last == NULL)
			cb_info->buff.retMsgHead = msg;
		else {
			while (last->next)
				last = last->next;
			last->next = msg;
		}
	} else
		LOGE("g_new failure");
}

static ret_msg_s *_get_ret_msg(muse_player_api_e api, callback_cb_info_s * cb_info)
{
	ret_msg_s *msg = cb_info->buff.retMsgHead;
	ret_msg_s *prev = NULL;
	while (msg) {
		if (msg->api == api) {
			if (!prev)
				cb_info->buff.retMsgHead = msg->next;
			else
				prev->next = msg->next;
			return msg;
		}
		prev = msg;
		msg = msg->next;
	}
	return NULL;
}

static void _notify_disconnected(callback_cb_info_s * cb_info)
{
	int code = PLAYER_ERROR_SERVICE_DISCONNECTED;
	muse_player_event_e ev = MUSE_PLAYER_EVENT_TYPE_ERROR;
	if (!cb_info || !cb_info->user_cb[ev])
		return;
	((player_error_cb) cb_info->user_cb[ev]) (code, cb_info->user_data[ev]);
}

static void *client_cb_handler(gpointer data)
{
	int api;
	int len = 0;
	int parse_len = 0;
	int offset = 0;
	callback_cb_info_s *cb_info = data;
	char *recvMsg = NULL;
	muse_core_msg_parse_err_e err;

	while (g_atomic_int_get(&cb_info->running)) {
		len = 0;
		err = MUSE_MSG_PARSE_ERROR_NONE;

		len = player_recv_msg(cb_info);
		if (len <= 0)
			break;

		recvMsg = cb_info->buff.recvMsg;
		recvMsg[len] = '\0';

		parse_len = len;
		offset = 0;
		while (offset < len) {
			api = MUSE_PLAYER_API_MAX;
//			LOGD(">>>>>> [%d/%d] %p, %s", offset, len, recvMsg + offset, recvMsg + offset);
//			usleep(10*1000);

			void *jobj = muse_core_msg_json_object_new(recvMsg + offset, &parse_len, &err);
			if (jobj) {
				if (muse_core_msg_json_object_get_value("api", jobj, &api, MUSE_TYPE_INT)) {
					if (api < MUSE_PLAYER_API_MAX) {
						g_mutex_lock(&cb_info->player_mutex);
						cb_info->buff.recved++;
						_add_ret_msg(api, cb_info, offset, parse_len);
						g_cond_signal(&cb_info->player_cond[api]);
						g_mutex_unlock(&cb_info->player_mutex);
						if (api == MUSE_PLAYER_API_DESTROY)
							g_atomic_int_set(&cb_info->running, 0);
					} else if (api == MUSE_PLAYER_CB_EVENT) {
						int event;
						char *buffer;
						g_mutex_lock(&cb_info->player_mutex);
						buffer = strndup(recvMsg + offset, parse_len);
						g_mutex_unlock(&cb_info->player_mutex);
						if (muse_core_msg_json_object_get_value("event", jobj, &event, MUSE_TYPE_INT))
							_user_callback_handler(cb_info, event, buffer);
					}
				} else {
					LOGE("Failed to get value. offset:%d/%d, [msg][ %s ]", offset, len, (recvMsg+offset));
				}
				muse_core_msg_json_object_free(jobj);
			} else {
				LOGE("Failed to get msg obj. err:%d", err);
			}

			if (parse_len == 0 || err != MUSE_MSG_PARSE_ERROR_NONE)
				break;

			offset += parse_len;
			parse_len = len - offset;
		}
	}
	if (g_atomic_int_get(&cb_info->running))
		_notify_disconnected(cb_info);
	LOGD("client cb exit");

	return NULL;
}

static callback_cb_info_s *callback_new(gint sockfd)
{
	callback_cb_info_s *cb_info;
	msg_buff_s *buff;
	int i;

	g_return_val_if_fail(sockfd > 0, NULL);

	cb_info = g_new(callback_cb_info_s, 1);
	if (!cb_info)
		return NULL;
	memset(cb_info, 0, sizeof(callback_cb_info_s));

	g_mutex_init(&cb_info->player_mutex);
	for (i = 0; i < MUSE_PLAYER_API_MAX; i++)
		g_cond_init(&cb_info->player_cond[i]);

	g_mutex_init(&cb_info->data_mutex);
	g_mutex_init(&cb_info->seek_cb_mutex);

	buff = &cb_info->buff;
	buff->recvMsg = g_new(char, MUSE_MSG_MAX_LENGTH + 1);
	buff->bufLen = MUSE_MSG_MAX_LENGTH + 1;
	buff->recved = 0;
	buff->retMsgHead = NULL;
	buff->part_of_msg = NULL;

	g_atomic_int_set(&cb_info->running, 1);
	cb_info->fd = sockfd;
	cb_info->thread = g_thread_new("callback_thread", client_cb_handler, (gpointer) cb_info);

	return cb_info;
}

static void callback_destroy(callback_cb_info_s * cb_info)
{
	int i;
	g_return_if_fail(cb_info);

	if (cb_info->fd > INVALID_SOCKET)
		muse_core_connection_close(cb_info->fd);
	if (cb_info->data_fd > INVALID_SOCKET)
		muse_core_connection_close(cb_info->data_fd);
	cb_info->fd = cb_info->data_fd = INVALID_SOCKET;

	g_thread_join(cb_info->thread);
	g_thread_unref(cb_info->thread);
	cb_info->thread = NULL;

	LOGI("%p Callback destroyed", cb_info);

	g_mutex_clear(&cb_info->player_mutex);
	for (i = 0; i < MUSE_PLAYER_API_MAX; i++)
		g_cond_clear(&cb_info->player_cond[i]);

	g_mutex_clear(&cb_info->data_mutex);
	g_mutex_clear(&cb_info->seek_cb_mutex);

	g_free(cb_info->buff.recvMsg);
	if (cb_info->buff.part_of_msg)
		g_free(cb_info->buff.part_of_msg);
	g_free(cb_info);
}

int client_get_api_timeout(player_cli_s * pc, muse_player_api_e api)
{
	int timeout = 0;

	switch (api) {
	case MUSE_PLAYER_API_PREPARE:
	case MUSE_PLAYER_API_PREPARE_ASYNC:
	case MUSE_PLAYER_API_UNPREPARE:
	case MUSE_PLAYER_API_START:
	case MUSE_PLAYER_API_STOP:
	case MUSE_PLAYER_API_PAUSE:
		timeout += SERVER_TIMEOUT(pc) * G_TIME_SPAN_MILLISECOND;
		break;
	default:
		/* check prepare async is done */
		if (pc && CALLBACK_INFO(pc) && CALLBACK_INFO(pc)->user_cb[MUSE_PLAYER_EVENT_TYPE_PREPARE])
			timeout += SERVER_TIMEOUT(pc) * G_TIME_SPAN_MILLISECOND;
		break;
	}
	timeout += CALLBACK_TIME_OUT;
	return timeout;	/* ms */
}

int client_wait_for_cb_return(muse_player_api_e api, callback_cb_info_s * cb_info, char **ret_buf, int time_out)
{
	int ret = PLAYER_ERROR_NONE;
	gint64 end_time = g_get_monotonic_time() + time_out * G_TIME_SPAN_MILLISECOND;
	msg_buff_s *buff = &cb_info->buff;
	ret_msg_s *msg = NULL;

	g_mutex_lock(&cb_info->player_mutex);

	msg = _get_ret_msg(api, cb_info);
	if (!buff->recved || !msg) {
		if (!g_cond_wait_until(&cb_info->player_cond[api], &cb_info->player_mutex, end_time)) {
			LOGW("api %d return msg does not received %dms", api, time_out);
			g_mutex_unlock(&cb_info->player_mutex);
			return PLAYER_ERROR_INVALID_OPERATION;
		}
	}
	if (!msg)
		msg = _get_ret_msg(api, cb_info);
	if (msg) {
		*ret_buf = msg->msg;
		g_free(msg);
		if (!player_msg_get(ret, *ret_buf))
			ret = PLAYER_ERROR_INVALID_OPERATION;
	} else {
		LOGE("api %d return msg is not exist", api);
		ret = PLAYER_ERROR_INVALID_OPERATION;
	}
	buff->recved--;

	g_mutex_unlock(&cb_info->player_mutex);

	return ret;
}

/*
* Public Implementation
*/

int player_create(player_h * player)
{
	PLAYER_INSTANCE_CHECK(player);

	int ret = PLAYER_ERROR_NONE;
	int sock_fd = INVALID_SOCKET;
	int pid = getpid();

	muse_player_api_e api = MUSE_PLAYER_API_CREATE;
	muse_core_api_module_e module = MUSE_PLAYER;
	player_cli_s *pc = NULL;
	char *ret_buf = NULL;

	LOGD("ENTER");

	sock_fd = muse_core_client_new();
	if (sock_fd <= INVALID_SOCKET) {
		LOGE("connection failure %d", errno);
		ret = PLAYER_ERROR_INVALID_OPERATION;
		goto ERROR;
	}
	player_msg_create_handle(api, sock_fd, INT, module, INT, pid);

	pc = g_new0(player_cli_s, 1);
	if (pc == NULL) {
		ret = PLAYER_ERROR_OUT_OF_MEMORY;
		goto ERROR;
	}

	pc->cb_info = callback_new(sock_fd);
	if (!pc->cb_info) {
		LOGE("fail to create callback");
		ret = PLAYER_ERROR_INVALID_OPERATION;
		goto ERROR;
	}
	if (!_player_event_queue_new(pc->cb_info)) {
		LOGE("fail to create event queue");
		ret = PLAYER_ERROR_INVALID_OPERATION;
		goto ERROR;
	}

	ret = client_wait_for_cb_return(api, pc->cb_info, &ret_buf, CALLBACK_TIME_OUT);
	if (ret == PLAYER_ERROR_NONE) {
		intptr_t module_addr;
		*player = (player_h) pc;
		if (player_msg_get_type(module_addr, ret_buf, POINTER)) {
			pc->cb_info->data_fd = muse_core_client_new_data_ch();
			muse_core_send_module_addr(module_addr, pc->cb_info->data_fd);
			LOGD("Data channel fd %d, muse module addr %p", pc->cb_info->data_fd, module_addr);
		}
		SERVER_TIMEOUT(pc) = MAX_SERVER_TIME_OUT;	/* will be update after prepare phase. */
	} else
		goto ERROR;

	pc->cb_info->bufmgr = tbm_bufmgr_init(-1);
	pc->push_media_stream = FALSE;

	g_free(ret_buf);
	return ret;

 ERROR:
	if (pc && pc->cb_info) {
		if (pc->cb_info->event_queue.running)
			_player_event_queue_destroy(pc->cb_info);
		callback_destroy(pc->cb_info);
		pc->cb_info = NULL;
	}
	g_free(pc);
	g_free(ret_buf);
	LOGD("ret value : %d", ret);
	return ret;
}

int player_destroy(player_h player)
{
	PLAYER_INSTANCE_CHECK(player);

	int ret = PLAYER_ERROR_NONE;
	muse_player_api_e api = MUSE_PLAYER_API_DESTROY;
	player_cli_s *pc = (player_cli_s *) player;
	char *ret_buf = NULL;

	LOGD("ENTER");

	player_msg_send(api, pc, ret_buf, ret);
#ifdef TIZEN_FEATURE_EVAS_RENDERER
	if (CALLBACK_INFO(pc) && EVAS_HANDLE(pc)) {
		player_unset_media_packet_video_frame_decoded_cb(player);
		if (mm_evas_renderer_destroy(&EVAS_HANDLE(pc)) != MM_ERROR_NONE)
			LOGW("fail to unset evas client");
	}
#endif

	if (_wl_window_evas_object_cb_del(player) != MM_ERROR_NONE)
		LOGW("fail to unset evas object callback");

	if (CALLBACK_INFO(pc)) {
		__player_remove_tsurf_list(pc);
		_player_event_queue_destroy(CALLBACK_INFO(pc));
		tbm_bufmgr_deinit(TBM_BUFMGR(pc));

		callback_destroy(CALLBACK_INFO(pc));
	}

	g_free(pc);
	pc = NULL;

	g_free(ret_buf);
	return ret;
}

int player_prepare_async(player_h player, player_prepared_cb callback, void *user_data)
{
	PLAYER_INSTANCE_CHECK(player);
	int ret = PLAYER_ERROR_NONE;
	muse_player_api_e api = MUSE_PLAYER_API_PREPARE_ASYNC;
	player_cli_s *pc = (player_cli_s *) player;
	char *ret_buf = NULL;

	LOGD("ENTER");

	if (pc->cb_info->user_cb[MUSE_PLAYER_EVENT_TYPE_PREPARE]) {
		LOGE("PLAYER_ERROR_INVALID_OPERATION (0x%08x) : preparing...", PLAYER_ERROR_INVALID_OPERATION);
		return PLAYER_ERROR_INVALID_OPERATION;
	} else {
		pc->cb_info->user_cb[MUSE_PLAYER_EVENT_TYPE_PREPARE] = callback;
		pc->cb_info->user_data[MUSE_PLAYER_EVENT_TYPE_PREPARE] = user_data;
	}
	player_msg_send(api, pc, ret_buf, ret);
	if (ret == PLAYER_ERROR_NONE) {
		int timeout = 0;
		player_msg_get_type(timeout, ret_buf, INT);

		LOGD("server timeout will be %d", timeout);
		SERVER_TIMEOUT(pc) = timeout;
	}

	g_free(ret_buf);
	return ret;
}

int player_prepare(player_h player)
{
	PLAYER_INSTANCE_CHECK(player);
	int ret = PLAYER_ERROR_NONE;
	muse_player_api_e api = MUSE_PLAYER_API_PREPARE;
	player_cli_s *pc = (player_cli_s *) player;
	char *ret_buf = NULL;

	LOGD("ENTER");

	player_msg_send(api, pc, ret_buf, ret);
	if (ret == PLAYER_ERROR_NONE) {
		int timeout = 0;
		player_msg_get_type(timeout, ret_buf, INT);

		LOGD("server timeout will be %d", timeout);
		SERVER_TIMEOUT(pc) = timeout;
	}

	g_free(ret_buf);
	return ret;
}

int player_unprepare(player_h player)
{
	PLAYER_INSTANCE_CHECK(player);
	int ret = PLAYER_ERROR_NONE;
	muse_player_api_e api = MUSE_PLAYER_API_UNPREPARE;
	player_cli_s *pc = (player_cli_s *) player;
	char *ret_buf = NULL;

	LOGD("ENTER");

	if (!CALLBACK_INFO(pc))
		return PLAYER_ERROR_INVALID_STATE;
#ifdef TIZEN_FEATURE_EVAS_RENDERER
	if (EVAS_HANDLE(pc)) {
		player_unset_media_packet_video_frame_decoded_cb(player);
		if (mm_evas_renderer_destroy(&EVAS_HANDLE(pc)) != MM_ERROR_NONE)
			LOGW("fail to unset evas client");
	}
#endif
	player_msg_send(api, pc, ret_buf, ret);
	if (ret == PLAYER_ERROR_NONE) {
		set_null_user_cb_lock(pc->cb_info, MUSE_PLAYER_EVENT_TYPE_SEEK);
		set_null_user_cb_lock(pc->cb_info, MUSE_PLAYER_EVENT_TYPE_PREPARE);
		_del_mem(pc);
		_player_deinit_memory_buffer(pc);
	}

	pc->cb_info->video_frame_pool_size = 0;
	__player_remove_tsurf_list(pc);

	g_free(ret_buf);
	return ret;
}

int player_set_uri(player_h player, const char *uri)
{
	PLAYER_INSTANCE_CHECK(player);
	PLAYER_NULL_ARG_CHECK(uri);
	int ret = PLAYER_ERROR_NONE;
	muse_player_api_e api = MUSE_PLAYER_API_SET_URI;
	player_cli_s *pc = (player_cli_s *) player;
	char *ret_buf = NULL;

	LOGD("ENTER");

	player_msg_send1(api, pc, ret_buf, ret, STRING, uri);
	pc->push_media_stream = FALSE;

	g_free(ret_buf);
	return ret;
}

int player_set_memory_buffer(player_h player, const void *data, int size)
{
	PLAYER_INSTANCE_CHECK(player);
	PLAYER_NULL_ARG_CHECK(data);
	int ret = PLAYER_ERROR_NONE;
	muse_player_api_e api = MUSE_PLAYER_API_SET_MEMORY_BUFFER;
	player_cli_s *pc = (player_cli_s *) player;
	char *ret_buf = NULL;
	tbm_bo bo;
	tbm_bo_handle thandle;
	tbm_key key;

	if (SERVER_TBM_BO(pc)) {
		LOGE("Already set the memory buffer. unprepare please");
		return PLAYER_ERROR_INVALID_OPERATION;
	}

	bo = tbm_bo_alloc(pc->cb_info->bufmgr, size, TBM_BO_DEFAULT);
	if (bo == NULL) {
		LOGE("TBM get error : bo is NULL");
		return PLAYER_ERROR_INVALID_OPERATION;
	}
	thandle = tbm_bo_map(bo, TBM_DEVICE_CPU, TBM_OPTION_WRITE);
	if (thandle.ptr == NULL) {
		LOGE("TBM get error : handle pointer is NULL");
		ret = PLAYER_ERROR_INVALID_OPERATION;
		goto EXIT;
	}
	memcpy(thandle.ptr, data, size);
	tbm_bo_unmap(bo);

	key = tbm_bo_export(bo);
	if (key == 0) {
		LOGE("TBM get error : key is 0");
		ret = PLAYER_ERROR_INVALID_OPERATION;
		goto EXIT;
	}

	player_msg_send2(api, pc, ret_buf, ret, INT, key, INT, size);
	pc->push_media_stream = FALSE;

 EXIT:
	tbm_bo_unref(bo);

	if (ret == PLAYER_ERROR_NONE) {
		intptr_t bo_addr = 0;
		if (player_msg_get_type(bo_addr, ret_buf, POINTER))
			SERVER_TBM_BO(pc) = (intptr_t) bo_addr;
	}

	g_free(ret_buf);
	return ret;
}

static int _player_deinit_memory_buffer(player_cli_s * pc)
{
	PLAYER_INSTANCE_CHECK(pc);
	int ret = PLAYER_ERROR_NONE;
	muse_player_api_e api = MUSE_PLAYER_API_DEINIT_MEMORY_BUFFER;
	intptr_t bo_addr = SERVER_TBM_BO(pc);

	if (!bo_addr)
		return ret;

	player_msg_send1_async(api, pc, POINTER, bo_addr);

	SERVER_TBM_BO(pc) = 0;

	return ret;
}

int player_get_state(player_h player, player_state_e * pstate)
{
	PLAYER_INSTANCE_CHECK(player);
	PLAYER_NULL_ARG_CHECK(pstate);
	int ret = PLAYER_ERROR_NONE;
	muse_player_api_e api = MUSE_PLAYER_API_GET_STATE;
	player_cli_s *pc = (player_cli_s *) player;
	int state;
	char *ret_buf = NULL;

	LOGD("ENTER");

	player_msg_send(api, pc, ret_buf, ret);

	if (ret == PLAYER_ERROR_NONE) {
		player_msg_get(state, ret_buf);
		*pstate = state;
	}

	g_free(ret_buf);
	return ret;
}

int player_set_volume(player_h player, float left, float right)
{
	PLAYER_INSTANCE_CHECK(player);
	PLAYER_CHECK_CONDITION(left >= 0 && left <= 1.0, PLAYER_ERROR_INVALID_PARAMETER, "PLAYER_ERROR_INVALID_PARAMETER");
	PLAYER_CHECK_CONDITION(right >= 0 && right <= 1.0, PLAYER_ERROR_INVALID_PARAMETER, "PLAYER_ERROR_INVALID_PARAMETER");
	int ret = PLAYER_ERROR_NONE;
	muse_player_api_e api = MUSE_PLAYER_API_SET_VOLUME;
	player_cli_s *pc = (player_cli_s *) player;
	char *ret_buf = NULL;

	LOGD("ENTER");

	player_msg_send2(api, pc, ret_buf, ret, DOUBLE, right, DOUBLE, left);
	g_free(ret_buf);
	return ret;
}

int player_get_volume(player_h player, float *pleft, float *pright)
{
	PLAYER_INSTANCE_CHECK(player);
	PLAYER_NULL_ARG_CHECK(pleft);
	PLAYER_NULL_ARG_CHECK(pright);
	int ret = PLAYER_ERROR_NONE;
	muse_player_api_e api = MUSE_PLAYER_API_GET_VOLUME;
	player_cli_s *pc = (player_cli_s *) player;
	double left = -1;
	double right = -1;
	char *ret_buf = NULL;

	LOGD("ENTER");

	player_msg_send(api, pc, ret_buf, ret);

	if (ret == PLAYER_ERROR_NONE) {
		bool ret_val = TRUE;
		player_msg_get2(ret_buf, left, DOUBLE, right, DOUBLE, ret_val);
		if (ret_val) {
			*pleft = (float)left;
			*pright = (float)right;
		} else {
			LOGE("failed to get value from msg");
			ret = PLAYER_ERROR_INVALID_OPERATION;
		}
	}

	g_free(ret_buf);
	return ret;
}

int player_set_sound_type(player_h player, sound_type_e type)
{
	PLAYER_INSTANCE_CHECK(player);
	int ret = PLAYER_ERROR_NONE;
	muse_player_api_e api = MUSE_PLAYER_API_SET_SOUND_TYPE;
	player_cli_s *pc = (player_cli_s *) player;
	char *ret_buf = NULL;

	LOGD("ENTER");

	player_msg_send1(api, pc, ret_buf, ret, INT, type);
	g_free(ret_buf);
	return ret;
}

int player_set_audio_policy_info(player_h player, sound_stream_info_h stream_info)
{
	PLAYER_INSTANCE_CHECK(player);

	muse_player_api_e api = MUSE_PLAYER_API_SET_AUDIO_POLICY_INFO;
	player_cli_s *pc = (player_cli_s *) player;
	char *ret_buf = NULL;

	LOGD("ENTER");

	bool is_available = false;

	/* check if stream_info is valid */
	int ret = __player_convert_error_code(
					sound_manager_is_available_stream_information(
					stream_info, NATIVE_API_PLAYER, &is_available), (char *)__FUNCTION__);

	if (ret == PLAYER_ERROR_NONE) {
		if (is_available == false)
			ret = PLAYER_ERROR_FEATURE_NOT_SUPPORTED_ON_DEVICE;
		else {
			char *stream_type = NULL;
			int stream_index = 0;
			ret = sound_manager_get_type_from_stream_information(stream_info, &stream_type);
			ret = sound_manager_get_index_from_stream_information(stream_info, &stream_index);
			if (ret == SOUND_MANAGER_ERROR_NONE)
				player_msg_send2(api, pc, ret_buf, ret, STRING, stream_type, INT, stream_index);
			else
				ret = PLAYER_ERROR_INVALID_OPERATION;
		}
	}

	g_free(ret_buf);
	return ret;

}

int player_set_audio_latency_mode(player_h player, audio_latency_mode_e latency_mode)
{
	PLAYER_INSTANCE_CHECK(player);
	int ret = PLAYER_ERROR_NONE;
	muse_player_api_e api = MUSE_PLAYER_API_SET_AUDIO_LATENCY_MODE;
	player_cli_s *pc = (player_cli_s *) player;
	char *ret_buf = NULL;

	LOGD("ENTER");

	player_msg_send1(api, pc, ret_buf, ret, INT, latency_mode);
	g_free(ret_buf);
	return ret;
}

int player_get_audio_latency_mode(player_h player, audio_latency_mode_e * platency_mode)
{
	PLAYER_INSTANCE_CHECK(player);
	PLAYER_NULL_ARG_CHECK(platency_mode);
	int ret = PLAYER_ERROR_NONE;
	muse_player_api_e api = MUSE_PLAYER_API_GET_AUDIO_LATENCY_MODE;
	player_cli_s *pc = (player_cli_s *) player;
	char *ret_buf = NULL;
	int latency_mode = -1;

	LOGD("ENTER");

	player_msg_send(api, pc, ret_buf, ret);

	if (ret == PLAYER_ERROR_NONE) {
		player_msg_get(latency_mode, ret_buf);
		*platency_mode = latency_mode;
	}

	g_free(ret_buf);
	return ret;

}

int player_start(player_h player)
{
	PLAYER_INSTANCE_CHECK(player);
	int ret = PLAYER_ERROR_NONE;
	muse_player_api_e api = MUSE_PLAYER_API_START;
	player_cli_s *pc = (player_cli_s *) player;
	char *ret_buf = NULL;

	LOGD("ENTER");
#ifdef TIZEN_FEATURE_EVAS_RENDERER
	if (EVAS_HANDLE(pc)) {
		ret = mm_evas_renderer_update_param(EVAS_HANDLE(pc));
		if (ret != PLAYER_ERROR_NONE)
			return ret;
	}
#endif
	player_msg_send(api, pc, ret_buf, ret);

	g_free(ret_buf);
	return ret;
}

int player_stop(player_h player)
{
	PLAYER_INSTANCE_CHECK(player);
	int ret = PLAYER_ERROR_NONE;
	muse_player_api_e api = MUSE_PLAYER_API_STOP;
	player_cli_s *pc = (player_cli_s *) player;
	char *ret_buf = NULL;

	LOGD("ENTER");

	player_msg_send(api, pc, ret_buf, ret);
	if (ret == PLAYER_ERROR_NONE)
		set_null_user_cb_lock(pc->cb_info, MUSE_PLAYER_EVENT_TYPE_SEEK);

	g_free(ret_buf);
	return ret;
}

int player_pause(player_h player)
{
	PLAYER_INSTANCE_CHECK(player);
	int ret = PLAYER_ERROR_NONE;
	muse_player_api_e api = MUSE_PLAYER_API_PAUSE;
	player_cli_s *pc = (player_cli_s *) player;
	char *ret_buf = NULL;

	LOGD("ENTER");

	player_msg_send(api, pc, ret_buf, ret);
	g_free(ret_buf);
	return ret;
}

int player_set_play_position(player_h player, int millisecond, bool accurate, player_seek_completed_cb callback, void *user_data)
{
	PLAYER_INSTANCE_CHECK(player);
	PLAYER_CHECK_CONDITION(millisecond >= 0, PLAYER_ERROR_INVALID_PARAMETER, "PLAYER_ERROR_INVALID_PARAMETER");

	int ret = PLAYER_ERROR_NONE;
	muse_player_api_e api = MUSE_PLAYER_API_SET_PLAY_POSITION;
	player_cli_s *pc = (player_cli_s *) player;
	char *ret_buf = NULL;
	int pos = millisecond;

	LOGD("ENTER");
	if (!pc->cb_info) {
		LOGE("cb_info is null");
		return PLAYER_ERROR_INVALID_OPERATION;
	}

	if ((pc->push_media_stream == FALSE) &&
		(pc->cb_info->user_cb[MUSE_PLAYER_EVENT_TYPE_SEEK])) {
		LOGE("PLAYER_ERROR_SEEK_FAILED (0x%08x) : seeking...", PLAYER_ERROR_SEEK_FAILED);
		return PLAYER_ERROR_SEEK_FAILED;
	} else {
		g_mutex_lock(&pc->cb_info->seek_cb_mutex);
		if (pc->push_media_stream == TRUE)
			pc->cb_info->block_seek_cb = TRUE;
		LOGI("Event type : %d, pos : %d ", MUSE_PLAYER_EVENT_TYPE_SEEK, millisecond);
		pc->cb_info->user_cb[MUSE_PLAYER_EVENT_TYPE_SEEK] = callback;
		pc->cb_info->user_data[MUSE_PLAYER_EVENT_TYPE_SEEK] = user_data;
		g_mutex_unlock(&pc->cb_info->seek_cb_mutex);
	}

	player_msg_send2(api, pc, ret_buf, ret, INT, pos, INT, accurate);

	if (ret != PLAYER_ERROR_NONE) {
		g_mutex_lock(&pc->cb_info->seek_cb_mutex);
		set_null_user_cb(pc->cb_info, MUSE_PLAYER_EVENT_TYPE_SEEK);
		g_mutex_unlock(&pc->cb_info->seek_cb_mutex);
	}

	if (pc->push_media_stream == TRUE)
		_player_event_queue_remove(&pc->cb_info->event_queue, MUSE_PLAYER_EVENT_TYPE_SEEK);

	pc->cb_info->block_seek_cb = FALSE;
	g_free(ret_buf);
	return ret;
}

int player_get_play_position(player_h player, int *millisecond)
{
	PLAYER_INSTANCE_CHECK(player);
	PLAYER_NULL_ARG_CHECK(millisecond);

	int ret = PLAYER_ERROR_NONE;
	muse_player_api_e api = MUSE_PLAYER_API_GET_PLAY_POSITION;
	player_cli_s *pc = (player_cli_s *) player;
	int pos;
	char *ret_buf = NULL;

	LOGD("ENTER");

	player_msg_send(api, pc, ret_buf, ret);

	if (ret == PLAYER_ERROR_NONE) {
		player_msg_get(pos, ret_buf);
		*millisecond = pos;
	}

	g_free(ret_buf);
	return ret;
}

int player_set_mute(player_h player, bool muted)
{
	PLAYER_INSTANCE_CHECK(player);
	int ret = PLAYER_ERROR_NONE;
	muse_player_api_e api = MUSE_PLAYER_API_SET_MUTE;
	player_cli_s *pc = (player_cli_s *) player;
	char *ret_buf = NULL;
	int mute = (int)muted;

	LOGD("ENTER");

	player_msg_send1(api, pc, ret_buf, ret, INT, mute);
	g_free(ret_buf);
	return ret;
}

int player_is_muted(player_h player, bool * muted)
{
	PLAYER_INSTANCE_CHECK(player);
	PLAYER_NULL_ARG_CHECK(muted);
	int ret = PLAYER_ERROR_NONE;
	muse_player_api_e api = MUSE_PLAYER_API_IS_MUTED;
	player_cli_s *pc = (player_cli_s *) player;
	char *ret_buf = NULL;
	int mute = -1;

	LOGD("ENTER");

	player_msg_send(api, pc, ret_buf, ret);
	if (ret == PLAYER_ERROR_NONE) {
		player_msg_get(mute, ret_buf);
		*muted = (bool) mute;
	}

	g_free(ret_buf);
	return ret;
}

int player_set_looping(player_h player, bool looping)
{
	PLAYER_INSTANCE_CHECK(player);
	int ret = PLAYER_ERROR_NONE;
	muse_player_api_e api = MUSE_PLAYER_API_SET_LOOPING;
	player_cli_s *pc = (player_cli_s *) player;
	char *ret_buf = NULL;

	LOGD("ENTER");

	player_msg_send1(api, pc, ret_buf, ret, INT, looping);
	g_free(ret_buf);
	return ret;
}

int player_is_looping(player_h player, bool * plooping)
{
	PLAYER_INSTANCE_CHECK(player);
	PLAYER_NULL_ARG_CHECK(plooping);
	int ret = PLAYER_ERROR_NONE;
	muse_player_api_e api = MUSE_PLAYER_API_IS_LOOPING;
	player_cli_s *pc = (player_cli_s *) player;
	char *ret_buf = NULL;
	int looping = 0;

	LOGD("ENTER");

	player_msg_send(api, pc, ret_buf, ret);
	if (ret == PLAYER_ERROR_NONE) {
		player_msg_get(looping, ret_buf);
		*plooping = looping;
	}
	g_free(ret_buf);
	return ret;
}

int player_get_duration(player_h player, int *pduration)
{
	PLAYER_INSTANCE_CHECK(player);
	PLAYER_NULL_ARG_CHECK(pduration);
	int ret = PLAYER_ERROR_NONE;
	muse_player_api_e api = MUSE_PLAYER_API_GET_DURATION;
	player_cli_s *pc = (player_cli_s *) player;
	char *ret_buf = NULL;
	int duration = 0;

	LOGD("ENTER");

	player_msg_send(api, pc, ret_buf, ret);
	if (ret == PLAYER_ERROR_NONE) {
		player_msg_get(duration, ret_buf);
		*pduration = duration;
	}

	g_free(ret_buf);
	return ret;
}

int player_set_display(player_h player, player_display_type_e type, player_display_h display)
{
	PLAYER_INSTANCE_CHECK(player);
	int ret = PLAYER_ERROR_NONE;
	muse_player_api_e api = MUSE_PLAYER_API_SET_DISPLAY;
	player_cli_s *pc = (player_cli_s *) player;
	char *ret_buf = NULL;
	Evas_Object *obj = NULL;
	const char *object_type = NULL;
	wl_win_msg_type wl_win;
	char *wl_win_msg = (char *)&wl_win;
	unsigned int wl_surface_id;
	struct wl_surface *wl_surface;
	struct wl_display *wl_display;
	Ecore_Wl_Window *wl_window = NULL;
	Evas *e;

	LOGD("ENTER type: %d", type);

	if (type != PLAYER_DISPLAY_TYPE_NONE) {
		obj = (Evas_Object *) display;
		if (!obj)
			return PLAYER_ERROR_INVALID_PARAMETER;

		object_type = evas_object_type_get(obj);
		if (object_type) {
			if (type == PLAYER_DISPLAY_TYPE_OVERLAY && !strcmp(object_type, "elm_win")) {
				/* wayland overlay surface */
				LOGI("Wayland overlay surface type");
				wl_win.type = type;

				e = evas_object_evas_get(obj);
				return_val_if_fail(e != NULL, PLAYER_ERROR_INVALID_OPERATION);

				ret = _wl_window_geometry_get(obj, e, &wl_win.wl_window_x, &wl_win.wl_window_y,
							&wl_win.wl_window_width, &wl_win.wl_window_height);
				if (ret != MM_ERROR_NONE) {
					LOGE("Fail to get window geometry");
					return ret;
				}
				if (_wl_window_evas_object_cb_add(player, obj) != MM_ERROR_NONE)
					LOGW("fail to set evas object callback");

				wl_window = elm_win_wl_window_get(obj);
				return_val_if_fail(wl_window != NULL, PLAYER_ERROR_INVALID_OPERATION);

				wl_surface = (struct wl_surface *)ecore_wl_window_surface_get(wl_window);
				return_val_if_fail(wl_surface != NULL, PLAYER_ERROR_INVALID_OPERATION);

				/* get wl_display */
				wl_display = (struct wl_display *)ecore_wl_display_get();
				return_val_if_fail(wl_display != NULL, PLAYER_ERROR_INVALID_OPERATION);

				LOGD("surface = %p, wl_display = %p", wl_surface, wl_display);

				ret = _wl_client_create(&pc->wlclient);
				if (ret != MM_ERROR_NONE) {
					LOGE("Wayland client create failure");
					return ret;
				}

				wl_surface_id = _wl_client_get_wl_window_wl_surface_id(pc->wlclient, wl_surface, wl_display);
				LOGD("wl_surface_id = %d", wl_surface_id);

				/* need to free always */
				if (pc->wlclient) {
					g_free(pc->wlclient);
					pc->wlclient = NULL;
				}
				return_val_if_fail(wl_surface_id > 0, PLAYER_ERROR_INVALID_OPERATION);
				wl_win.wl_surface_id = wl_surface_id;

			}
#ifdef TIZEN_FEATURE_EVAS_RENDERER
			else if (type == PLAYER_DISPLAY_TYPE_EVAS && !strcmp(object_type, "image")) {
				/* evas object surface */
				LOGI("evas surface type");
				wl_win.type = type;

				evas_object_geometry_get(obj, &wl_win.wl_window_x, &wl_win.wl_window_y,
					&wl_win.wl_window_width, &wl_win.wl_window_height);

				if (EVAS_HANDLE(pc)) {
					LOGW("evas client already exists");
					if (mm_evas_renderer_destroy(&EVAS_HANDLE(pc)) != MM_ERROR_NONE)
						LOGW("fail to unset evas client");
				}
				if (mm_evas_renderer_create(&EVAS_HANDLE(pc), obj) != MM_ERROR_NONE)
					LOGW("fail to set evas client");

				if (player_set_media_packet_video_frame_decoded_cb(player, mm_evas_renderer_write, (void *)EVAS_HANDLE(pc)) != PLAYER_ERROR_NONE)
					LOGW("fail to set decoded callback");
			}
#endif
			else
				return PLAYER_ERROR_INVALID_PARAMETER;
		} else
			return PLAYER_ERROR_INVALID_PARAMETER;
	} else {	/* PLAYER_DISPLAY_TYPE_NONE */
		LOGI("Wayland surface type is NONE");
		wl_win.type = type;
		wl_win.wl_window_x = 0;
		wl_win.wl_window_y = 0;
		wl_win.wl_window_width = 0;
		wl_win.wl_window_height = 0;
	}
	player_msg_send_array(api, pc, ret_buf, ret, wl_win_msg, sizeof(wl_win_msg_type), sizeof(char));
	g_free(ret_buf);
	return ret;
}

int player_set_display_mode(player_h player, player_display_mode_e mode)
{
	PLAYER_INSTANCE_CHECK(player);
	PLAYER_CHECK_CONDITION(PLAYER_DISPLAY_MODE_LETTER_BOX <= mode && mode < PLAYER_DISPLAY_MODE_NUM, PLAYER_ERROR_INVALID_PARAMETER, "PLAYER_ERROR_INVALID_PARAMETER");
	int ret = PLAYER_ERROR_NONE;
	muse_player_api_e api = MUSE_PLAYER_API_SET_DISPLAY_MODE;
	player_cli_s *pc = (player_cli_s *) player;
	char *ret_buf = NULL;

	LOGD("ENTER");
#ifdef TIZEN_FEATURE_EVAS_RENDERER
	if (EVAS_HANDLE(pc)) {
		ret = mm_evas_renderer_set_geometry(EVAS_HANDLE(pc), mode);
		return ret;
	}
#endif
	player_msg_send1(api, pc, ret_buf, ret, INT, mode);
	g_free(ret_buf);
	return ret;
}

int player_get_display_mode(player_h player, player_display_mode_e * pmode)
{
	PLAYER_INSTANCE_CHECK(player);
	PLAYER_NULL_ARG_CHECK(pmode);
	int ret = PLAYER_ERROR_NONE;
	muse_player_api_e api = MUSE_PLAYER_API_GET_DISPLAY_MODE;
	player_cli_s *pc = (player_cli_s *) player;
	char *ret_buf = NULL;
	int mode = -1;

	LOGD("ENTER");
#ifdef TIZEN_FEATURE_EVAS_RENDERER
	if (EVAS_HANDLE(pc)) {
		ret = mm_evas_renderer_get_geometry(EVAS_HANDLE(pc), &mode);
		*pmode = (player_display_mode_e) mode;
		return ret;
	}
#endif
	player_msg_send(api, pc, ret_buf, ret);
	if (ret == PLAYER_ERROR_NONE) {
		player_msg_get_type(mode, ret_buf, INT);
		*pmode = mode;
	}

	g_free(ret_buf);
	return ret;
}

int player_set_playback_rate(player_h player, float rate)
{
	PLAYER_INSTANCE_CHECK(player);
	PLAYER_CHECK_CONDITION(rate >= -5.0 && rate <= 5.0, PLAYER_ERROR_INVALID_PARAMETER, "PLAYER_ERROR_INVALID_PARAMETER");
	int ret = PLAYER_ERROR_NONE;
	muse_player_api_e api = MUSE_PLAYER_API_SET_PLAYBACK_RATE;
	player_cli_s *pc = (player_cli_s *) player;
	char *ret_buf = NULL;

	LOGD("ENTER");

	player_msg_send1(api, pc, ret_buf, ret, DOUBLE, rate);
	g_free(ret_buf);
	return ret;
}

int player_set_display_rotation(player_h player, player_display_rotation_e rotation)
{
	PLAYER_INSTANCE_CHECK(player);
	int ret = PLAYER_ERROR_NONE;
	muse_player_api_e api = MUSE_PLAYER_API_SET_DISPLAY_ROTATION;
	player_cli_s *pc = (player_cli_s *) player;
	char *ret_buf = NULL;

	LOGD("ENTER");
#ifdef TIZEN_FEATURE_EVAS_RENDERER
	if (EVAS_HANDLE(pc)) {
		ret = mm_evas_renderer_set_rotation(EVAS_HANDLE(pc), rotation);
		return ret;
	}
#endif
	player_msg_send1(api, pc, ret_buf, ret, INT, rotation);
	g_free(ret_buf);
	return ret;
}

int player_get_display_rotation(player_h player, player_display_rotation_e * protation)
{
	PLAYER_INSTANCE_CHECK(player);
	PLAYER_NULL_ARG_CHECK(protation);
	int ret = PLAYER_ERROR_NONE;
	player_cli_s *pc = (player_cli_s *) player;
	muse_player_api_e api = MUSE_PLAYER_API_GET_DISPLAY_ROTATION;
	char *ret_buf = NULL;
	int rotation = -1;

	LOGD("ENTER");
#ifdef TIZEN_FEATURE_EVAS_RENDERER
	if (EVAS_HANDLE(pc)) {
		ret = mm_evas_renderer_get_rotation(EVAS_HANDLE(pc), &rotation);
		*protation = (player_display_rotation_e) rotation;
		return ret;
	}
#endif
	player_msg_send(api, pc, ret_buf, ret);
	if (ret == PLAYER_ERROR_NONE) {
		player_msg_get_type(rotation, ret_buf, INT);
		*protation = rotation;
	}

	g_free(ret_buf);
	return ret;
}

int player_set_display_visible(player_h player, bool visible)
{
	PLAYER_INSTANCE_CHECK(player);
	int ret = PLAYER_ERROR_NONE;
	player_cli_s *pc = (player_cli_s *) player;
	muse_player_api_e api = MUSE_PLAYER_API_SET_DISPLAY_VISIBLE;
	char *ret_buf = NULL;

	LOGD("ENTER");
#ifdef TIZEN_FEATURE_EVAS_RENDERER
	if (EVAS_HANDLE(pc)) {
		ret = mm_evas_renderer_set_visible(EVAS_HANDLE(pc), visible);
		return ret;
	}
#endif
	player_msg_send1(api, pc, ret_buf, ret, INT, visible);
	g_free(ret_buf);
	return ret;
}

int player_is_display_visible(player_h player, bool * pvisible)
{
	PLAYER_INSTANCE_CHECK(player);
	PLAYER_NULL_ARG_CHECK(pvisible);
	int ret = PLAYER_ERROR_NONE;
	player_cli_s *pc = (player_cli_s *) player;
	muse_player_api_e api = MUSE_PLAYER_API_IS_DISPLAY_VISIBLE;
	char *ret_buf = NULL;
	int value = -1;
#ifdef TIZEN_FEATURE_EVAS_RENDERER
	bool visible = 0;
#endif
	LOGD("ENTER");
#ifdef TIZEN_FEATURE_EVAS_RENDERER
	if (EVAS_HANDLE(pc)) {
		ret = mm_evas_renderer_get_visible(EVAS_HANDLE(pc), &visible);
		if (visible)
			*pvisible = TRUE;
		else
			*pvisible = FALSE;
		return ret;
	}
#endif
	player_msg_send(api, pc, ret_buf, ret);
	if (ret == PLAYER_ERROR_NONE) {
		player_msg_get_type(value, ret_buf, INT);

		if (value)
			*pvisible = TRUE;
		else
			*pvisible = FALSE;
	}

	g_free(ret_buf);
	return ret;
}

int player_get_content_info(player_h player, player_content_info_e key, char **pvalue)
{
	PLAYER_INSTANCE_CHECK(player);
	PLAYER_NULL_ARG_CHECK(pvalue);
	int ret = PLAYER_ERROR_NONE;
	muse_player_api_e api = MUSE_PLAYER_API_GET_CONTENT_INFO;
	player_cli_s *pc = (player_cli_s *) player;
	char *ret_buf = NULL;
	char value[MUSE_MSG_MAX_LENGTH] = { 0, };

	LOGD("ENTER");

	player_msg_send1(api, pc, ret_buf, ret, INT, key);
	if (ret == PLAYER_ERROR_NONE) {
		player_msg_get_string(value, ret_buf);
		*pvalue = strndup(value, MUSE_MSG_MAX_LENGTH);
	}
	g_free(ret_buf);
	return ret;
}

int player_get_codec_info(player_h player, char **paudio_codec, char **pvideo_codec)
{
	PLAYER_INSTANCE_CHECK(player);
	PLAYER_NULL_ARG_CHECK(paudio_codec);
	PLAYER_NULL_ARG_CHECK(pvideo_codec);
	int ret = PLAYER_ERROR_NONE;
	muse_player_api_e api = MUSE_PLAYER_API_GET_CODEC_INFO;
	player_cli_s *pc = (player_cli_s *) player;
	char *ret_buf = NULL;
	char video_codec[MUSE_MSG_MAX_LENGTH] = { 0, };
	char audio_codec[MUSE_MSG_MAX_LENGTH] = { 0, };
	bool ret_val = TRUE;

	LOGD("ENTER");

	player_msg_send(api, pc, ret_buf, ret);
	if (ret == PLAYER_ERROR_NONE) {
		player_msg_get_string2(ret_buf, video_codec, audio_codec, ret_val);
		if (ret_val) {
			*pvideo_codec = strndup(video_codec, MUSE_MSG_MAX_LENGTH);
			*paudio_codec = strndup(audio_codec, MUSE_MSG_MAX_LENGTH);
		} else {
			ret = PLAYER_ERROR_INVALID_OPERATION;
		}
	}
	g_free(ret_buf);
	return ret;
}

int player_get_audio_stream_info(player_h player, int *psample_rate, int *pchannel, int *pbit_rate)
{
	PLAYER_INSTANCE_CHECK(player);
	PLAYER_NULL_ARG_CHECK(psample_rate);
	PLAYER_NULL_ARG_CHECK(pchannel);
	PLAYER_NULL_ARG_CHECK(pbit_rate);
	int ret = PLAYER_ERROR_NONE;
	muse_player_api_e api = MUSE_PLAYER_API_GET_AUDIO_STREAM_INFO;
	player_cli_s *pc = (player_cli_s *) player;
	char *ret_buf = NULL;
	int sample_rate = 0;
	int channel = 0;
	int bit_rate = 0;

	LOGD("ENTER");

	player_msg_send(api, pc, ret_buf, ret);
	if (ret == PLAYER_ERROR_NONE) {
		bool ret_val = TRUE;
		player_msg_get3(ret_buf, sample_rate, INT, channel, INT, bit_rate, INT, ret_val);
		if (ret_val) {
			*psample_rate = sample_rate;
			*pchannel = channel;
			*pbit_rate = bit_rate;
		} else {
			ret = PLAYER_ERROR_INVALID_OPERATION;
		}
	}
	g_free(ret_buf);
	return ret;
}

int player_get_video_stream_info(player_h player, int *pfps, int *pbit_rate)
{
	PLAYER_INSTANCE_CHECK(player);
	PLAYER_NULL_ARG_CHECK(pfps);
	PLAYER_NULL_ARG_CHECK(pbit_rate);
	int ret = PLAYER_ERROR_NONE;
	muse_player_api_e api = MUSE_PLAYER_API_GET_VIDEO_STREAM_INFO;
	player_cli_s *pc = (player_cli_s *) player;
	char *ret_buf = NULL;
	int fps = 0;
	int bit_rate = 0;

	LOGD("ENTER");

	player_msg_send(api, pc, ret_buf, ret);
	if (ret == PLAYER_ERROR_NONE) {
		bool ret_val = TRUE;
		player_msg_get2(ret_buf, fps, INT, bit_rate, INT, ret_val);
		if (ret_val) {
			*pfps = fps;
			*pbit_rate = bit_rate;
		} else {
			ret = PLAYER_ERROR_INVALID_OPERATION;
		}
	}
	g_free(ret_buf);
	return ret;
}

int player_get_video_size(player_h player, int *pwidth, int *pheight)
{
	PLAYER_INSTANCE_CHECK(player);
	PLAYER_NULL_ARG_CHECK(pwidth);
	PLAYER_NULL_ARG_CHECK(pheight);
	int ret = PLAYER_ERROR_NONE;
	muse_player_api_e api = MUSE_PLAYER_API_GET_VIDEO_SIZE;
	player_cli_s *pc = (player_cli_s *) player;
	char *ret_buf = NULL;
	int width = 0;
	int height = 0;

	LOGD("ENTER");

	player_msg_send(api, pc, ret_buf, ret);
	if (ret == PLAYER_ERROR_NONE) {
		bool ret_val = TRUE;
		player_msg_get2(ret_buf, width, INT, height, INT, ret_val);
		if (ret_val) {
			*pwidth = width;
			*pheight = height;
		} else {
			ret = PLAYER_ERROR_INVALID_OPERATION;
		}
	}
	g_free(ret_buf);
	return ret;
}

int player_get_album_art(player_h player, void **palbum_art, int *psize)
{
	PLAYER_INSTANCE_CHECK(player);
	PLAYER_NULL_ARG_CHECK(palbum_art);
	PLAYER_NULL_ARG_CHECK(psize);
	int ret = PLAYER_ERROR_NONE;
	muse_player_api_e api = MUSE_PLAYER_API_GET_ALBUM_ART;
	player_cli_s *pc = (player_cli_s *) player;
	char *ret_buf = NULL;
	char *album_art;
	int size = 0;
	tbm_bo bo = NULL;
	tbm_bo_handle thandle;
	tbm_key key = 0;
	void *jobj = NULL;

	LOGD("ENTER");

	player_msg_send(api, pc, ret_buf, ret);
	if (ret == PLAYER_ERROR_NONE) {
		muse_core_msg_parse_err_e err = MUSE_MSG_PARSE_ERROR_NONE;
		jobj = muse_core_msg_json_object_new(ret_buf, NULL, &err);
		if (!jobj) {
			LOGE("failed to get msg obj, err:%d", err);
			ret = PLAYER_ERROR_INVALID_OPERATION;
			goto EXIT;
		}

		if (muse_core_msg_json_object_get_value("size", jobj, &size, MUSE_TYPE_INT) && (size > 0)) {
			LOGD("size : %d", size);
			if (!muse_core_msg_json_object_get_value("key", jobj, &key, MUSE_TYPE_INT)) {
				ret = PLAYER_ERROR_INVALID_OPERATION;
				goto EXIT;
			}

			bo = tbm_bo_import(pc->cb_info->bufmgr, key);
			if (bo == NULL) {
				LOGE("TBM get error : bo is NULL");
				ret = PLAYER_ERROR_INVALID_OPERATION;
				goto EXIT;
			}
			thandle = tbm_bo_map(bo, TBM_DEVICE_CPU, TBM_OPTION_WRITE | TBM_OPTION_READ);
			if (thandle.ptr == NULL) {
				LOGE("TBM get error : handle pointer is NULL");
				ret = PLAYER_ERROR_INVALID_OPERATION;
				goto EXIT;
			}
			album_art = _get_mem(pc, size);
			if (album_art) {
				memcpy(album_art, thandle.ptr, size);
				*palbum_art = album_art;
			} else {
				LOGE("g_new failure");
				ret = PLAYER_ERROR_INVALID_OPERATION;
			}
			tbm_bo_unmap(bo);
		} else {
			*palbum_art = NULL;
		}
		*psize = size;
	}

EXIT:
	if (jobj)
		muse_core_msg_json_object_free(jobj);

	if (ret_buf)
		g_free(ret_buf);

	if (bo)
		tbm_bo_unref(bo);

	/* return buffer */
	if (key != 0) {
		LOGD("send msg to release buffer. key:%d", key);
		player_msg_send1_async(MUSE_PLAYER_API_RETURN_BUFFER, pc, INT, key);
	}

	return ret;
}

int player_audio_effect_get_equalizer_bands_count(player_h player, int *pcount)
{
	PLAYER_INSTANCE_CHECK(player);
	PLAYER_NULL_ARG_CHECK(pcount);
	int ret = PLAYER_ERROR_NONE;
	muse_player_api_e api = MUSE_PLAYER_API_AUDIO_EFFECT_GET_EQUALIZER_BANDS_COUNT;
	player_cli_s *pc = (player_cli_s *) player;
	char *ret_buf = NULL;
	int count;

	LOGD("ENTER");

	player_msg_send(api, pc, ret_buf, ret);
	if (ret == PLAYER_ERROR_NONE) {
		player_msg_get(count, ret_buf);
		*pcount = count;
	}
	g_free(ret_buf);
	return ret;
}

int player_audio_effect_set_equalizer_all_bands(player_h player, int *band_levels, int length)
{
	PLAYER_INSTANCE_CHECK(player);
	PLAYER_NULL_ARG_CHECK(band_levels);
	int ret = PLAYER_ERROR_NONE;
	muse_player_api_e api = MUSE_PLAYER_API_AUDIO_EFFECT_SET_EQUALIZER_ALL_BANDS;
	player_cli_s *pc = (player_cli_s *) player;
	char *ret_buf = NULL;

	LOGD("ENTER");

	player_msg_send_array(api, pc, ret_buf, ret, band_levels, length, sizeof(int));

	g_free(ret_buf);
	return ret;

}

int player_audio_effect_set_equalizer_band_level(player_h player, int index, int level)
{
	PLAYER_INSTANCE_CHECK(player);
	int ret = PLAYER_ERROR_NONE;
	muse_player_api_e api = MUSE_PLAYER_API_AUDIO_EFFECT_SET_EQUALIZER_BAND_LEVEL;
	player_cli_s *pc = (player_cli_s *) player;
	char *ret_buf = NULL;

	LOGD("ENTER");

	player_msg_send2(api, pc, ret_buf, ret, INT, index, INT, level);

	g_free(ret_buf);
	return ret;
}

int player_audio_effect_get_equalizer_band_level(player_h player, int index, int *plevel)
{
	PLAYER_INSTANCE_CHECK(player);
	PLAYER_NULL_ARG_CHECK(plevel);
	int ret = PLAYER_ERROR_NONE;
	muse_player_api_e api = MUSE_PLAYER_API_AUDIO_EFFECT_GET_EQUALIZER_BAND_LEVEL;
	player_cli_s *pc = (player_cli_s *) player;
	char *ret_buf = NULL;
	int level;

	LOGD("ENTER");

	player_msg_send1(api, pc, ret_buf, ret, INT, index);
	if (ret == PLAYER_ERROR_NONE) {
		player_msg_get(level, ret_buf);
		*plevel = level;
	}
	g_free(ret_buf);
	return ret;
}

int player_audio_effect_get_equalizer_level_range(player_h player, int *pmin, int *pmax)
{
	PLAYER_INSTANCE_CHECK(player);
	PLAYER_NULL_ARG_CHECK(pmin);
	PLAYER_NULL_ARG_CHECK(pmax);
	int ret = PLAYER_ERROR_NONE;
	muse_player_api_e api = MUSE_PLAYER_API_AUDIO_EFFECT_GET_EQUALIZER_LEVEL_RANGE;
	player_cli_s *pc = (player_cli_s *) player;
	char *ret_buf = NULL;
	int min = 0, max = 0;

	LOGD("ENTER");

	player_msg_send(api, pc, ret_buf, ret);
	if (ret == PLAYER_ERROR_NONE) {
		bool ret_val = TRUE;
		player_msg_get2(ret_buf, min, INT, max, INT, ret_val);
		if (ret_val) {
			*pmin = min;
			*pmax = max;
		} else {
			ret = PLAYER_ERROR_INVALID_OPERATION;
		}
	}
	g_free(ret_buf);
	return ret;
}

int player_audio_effect_get_equalizer_band_frequency(player_h player, int index, int *pfrequency)
{
	PLAYER_INSTANCE_CHECK(player);
	PLAYER_NULL_ARG_CHECK(pfrequency);
	int ret = PLAYER_ERROR_NONE;
	muse_player_api_e api = MUSE_PLAYER_API_AUDIO_EFFECT_GET_EQUALIZER_BAND_FREQUENCY;
	player_cli_s *pc = (player_cli_s *) player;
	char *ret_buf = NULL;
	int frequency;

	LOGD("ENTER");

	player_msg_send1(api, pc, ret_buf, ret, INT, index);
	if (ret == PLAYER_ERROR_NONE) {
		player_msg_get(frequency, ret_buf);
		*pfrequency = frequency;
	}
	g_free(ret_buf);
	return ret;
}

int player_audio_effect_get_equalizer_band_frequency_range(player_h player, int index, int *prange)
{
	PLAYER_INSTANCE_CHECK(player);
	PLAYER_NULL_ARG_CHECK(prange);
	int ret = PLAYER_ERROR_NONE;
	muse_player_api_e api = MUSE_PLAYER_API_AUDIO_EFFECT_GET_EQUALIZER_BAND_FREQUENCY_RANGE;
	player_cli_s *pc = (player_cli_s *) player;
	char *ret_buf = NULL;
	int range;

	LOGD("ENTER");

	player_msg_send1(api, pc, ret_buf, ret, INT, index);
	if (ret == PLAYER_ERROR_NONE) {
		player_msg_get(range, ret_buf);
		*prange = range;
	}
	g_free(ret_buf);
	return ret;
}

int player_audio_effect_equalizer_clear(player_h player)
{
	PLAYER_INSTANCE_CHECK(player);
	int ret = PLAYER_ERROR_NONE;
	muse_player_api_e api = MUSE_PLAYER_API_AUDIO_EFFECT_EQUALIZER_CLEAR;
	player_cli_s *pc = (player_cli_s *) player;
	char *ret_buf = NULL;

	LOGD("ENTER");

	player_msg_send(api, pc, ret_buf, ret);
	g_free(ret_buf);
	return ret;
}

int player_audio_effect_equalizer_is_available(player_h player, bool * pavailable)
{
	PLAYER_INSTANCE_CHECK(player);
	PLAYER_NULL_ARG_CHECK(pavailable);
	int ret = PLAYER_ERROR_NONE;
	muse_player_api_e api = MUSE_PLAYER_API_AUDIO_EFFECT_EQUALIZER_IS_AVAILABLE;
	player_cli_s *pc = (player_cli_s *) player;
	char *ret_buf = NULL;
	int available;

	LOGD("ENTER");

	player_msg_send(api, pc, ret_buf, ret);
	if (ret == PLAYER_ERROR_NONE) {
		player_msg_get(available, ret_buf);
		*pavailable = available;
	}
	g_free(ret_buf);
	return ret;
}

int player_set_subtitle_path(player_h player, const char *path)
{
	PLAYER_INSTANCE_CHECK(player);
	int ret = PLAYER_ERROR_NONE;
	muse_player_api_e api = MUSE_PLAYER_API_SET_SUBTITLE_PATH;
	player_cli_s *pc = (player_cli_s *) player;
	char *ret_buf = NULL;

	LOGD("ENTER");

	player_msg_send1(api, pc, ret_buf, ret, STRING, path);
	g_free(ret_buf);
	return ret;
}

int player_set_subtitle_position_offset(player_h player, int millisecond)
{
	PLAYER_INSTANCE_CHECK(player);
	int ret = PLAYER_ERROR_NONE;
	muse_player_api_e api = MUSE_PLAYER_API_SET_SUBTITLE_POSITION_OFFSET;
	player_cli_s *pc = (player_cli_s *) player;
	char *ret_buf = NULL;

	LOGD("ENTER");

	player_msg_send1(api, pc, ret_buf, ret, INT, millisecond);

	g_free(ret_buf);
	return ret;
}

int player_set_progressive_download_path(player_h player, const char *path)
{
	PLAYER_INSTANCE_CHECK(player);
	PLAYER_NULL_ARG_CHECK(path);
	int ret = PLAYER_ERROR_NONE;
	muse_player_api_e api = MUSE_PLAYER_API_SET_PROGRESSIVE_DOWNLOAD_PATH;
	player_cli_s *pc = (player_cli_s *) player;
	char *ret_buf = NULL;

	LOGD("ENTER");

	player_msg_send1(api, pc, ret_buf, ret, STRING, path);
	g_free(ret_buf);
	return ret;
}

int player_get_progressive_download_status(player_h player, unsigned long *pcurrent, unsigned long *ptotal_size)
{
	PLAYER_INSTANCE_CHECK(player);
	PLAYER_NULL_ARG_CHECK(pcurrent);
	PLAYER_NULL_ARG_CHECK(ptotal_size);
	int ret = PLAYER_ERROR_NONE;
	muse_player_api_e api = MUSE_PLAYER_API_GET_PROGRESSIVE_DOWNLOAD_STATUS;
	player_cli_s *pc = (player_cli_s *) player;
	char *ret_buf = NULL;
	unsigned long current, total_size;

	LOGD("ENTER");

	player_msg_send(api, pc, ret_buf, ret);
	if (ret == PLAYER_ERROR_NONE) {
		bool ret_val = TRUE;
		player_msg_get2(ret_buf, current, POINTER, total_size, POINTER, ret_val);
		if (ret_val) {
			*pcurrent = current;
			*ptotal_size = total_size;
		} else {
			ret = PLAYER_ERROR_INVALID_OPERATION;
		}
	}
	g_free(ret_buf);
	return ret;

}

int player_capture_video(player_h player, player_video_captured_cb callback, void *user_data)
{
	PLAYER_INSTANCE_CHECK(player);
	PLAYER_NULL_ARG_CHECK(callback);
	int ret = PLAYER_ERROR_NONE;
	muse_player_api_e api = MUSE_PLAYER_API_CAPTURE_VIDEO;
	player_cli_s *pc = (player_cli_s *) player;
	char *ret_buf = NULL;

	LOGD("ENTER");
	if (pc->cb_info->user_cb[MUSE_PLAYER_EVENT_TYPE_CAPTURE]) {
		LOGE("PLAYER_ERROR_VIDEO_CAPTURE_FAILED (0x%08x) : capturing...", PLAYER_ERROR_VIDEO_CAPTURE_FAILED);
		return PLAYER_ERROR_VIDEO_CAPTURE_FAILED;
	} else {
		LOGI("Event type : %d ", MUSE_PLAYER_EVENT_TYPE_CAPTURE);
		pc->cb_info->user_cb[MUSE_PLAYER_EVENT_TYPE_CAPTURE] = callback;
		pc->cb_info->user_data[MUSE_PLAYER_EVENT_TYPE_CAPTURE] = user_data;
	}

	player_msg_send(api, pc, ret_buf, ret);

	if (ret != PLAYER_ERROR_NONE)
		set_null_user_cb(pc->cb_info, MUSE_PLAYER_EVENT_TYPE_CAPTURE);

	g_free(ret_buf);
	return ret;
}

int player_set_streaming_cookie(player_h player, const char *cookie, int size)
{
	PLAYER_INSTANCE_CHECK(player);
	PLAYER_NULL_ARG_CHECK(cookie);
	PLAYER_CHECK_CONDITION(size >= 0, PLAYER_ERROR_INVALID_PARAMETER, "PLAYER_ERROR_INVALID_PARAMETER");
	int ret = PLAYER_ERROR_NONE;
	muse_player_api_e api = MUSE_PLAYER_API_SET_STREAMING_COOKIE;
	player_cli_s *pc = (player_cli_s *) player;
	char *ret_buf = NULL;

	LOGD("ENTER");

	player_msg_send2(api, pc, ret_buf, ret, STRING, cookie, INT, size);
	g_free(ret_buf);
	return ret;
}

int player_set_streaming_user_agent(player_h player, const char *user_agent, int size)
{
	PLAYER_INSTANCE_CHECK(player);
	PLAYER_NULL_ARG_CHECK(user_agent);
	PLAYER_CHECK_CONDITION(size >= 0, PLAYER_ERROR_INVALID_PARAMETER, "PLAYER_ERROR_INVALID_PARAMETER");
	int ret = PLAYER_ERROR_NONE;
	muse_player_api_e api = MUSE_PLAYER_API_SET_STREAMING_USER_AGENT;
	player_cli_s *pc = (player_cli_s *) player;
	char *ret_buf = NULL;

	LOGD("ENTER");

	player_msg_send2(api, pc, ret_buf, ret, STRING, user_agent, INT, size);
	g_free(ret_buf);
	return ret;
}

int player_get_streaming_download_progress(player_h player, int *pstart, int *pcurrent)
{
	PLAYER_INSTANCE_CHECK(player);
	PLAYER_NULL_ARG_CHECK(pstart);
	PLAYER_NULL_ARG_CHECK(pcurrent);
	int ret = PLAYER_ERROR_NONE;
	muse_player_api_e api = MUSE_PLAYER_API_GET_STREAMING_DOWNLOAD_PROGRESS;
	player_cli_s *pc = (player_cli_s *) player;
	char *ret_buf = NULL;
	int start = 0, current = 0;

	LOGD("ENTER");

	player_msg_send(api, pc, ret_buf, ret);
	if (ret == PLAYER_ERROR_NONE) {
		bool ret_val = TRUE;
		player_msg_get2(ret_buf, start, INT, current, INT, ret_val);
		if (ret_val) {
			*pstart = start;
			*pcurrent = current;
		} else {
			ret = PLAYER_ERROR_INVALID_OPERATION;
		}
	}
	g_free(ret_buf);
	return ret;

}

int player_set_completed_cb(player_h player, player_completed_cb callback, void *user_data)
{
	return __set_callback(MUSE_PLAYER_EVENT_TYPE_COMPLETE, player, callback, user_data);
}

int player_unset_completed_cb(player_h player)
{
	return __unset_callback(MUSE_PLAYER_EVENT_TYPE_COMPLETE, player);
}

int player_set_interrupted_cb(player_h player, player_interrupted_cb callback, void *user_data)
{
	return __set_callback(MUSE_PLAYER_EVENT_TYPE_INTERRUPT, player, callback, user_data);
}

int player_unset_interrupted_cb(player_h player)
{
	return __unset_callback(MUSE_PLAYER_EVENT_TYPE_INTERRUPT, player);
}

int player_set_error_cb(player_h player, player_error_cb callback, void *user_data)
{
	return __set_callback(MUSE_PLAYER_EVENT_TYPE_ERROR, player, callback, user_data);
}

int player_unset_error_cb(player_h player)
{
	return __unset_callback(MUSE_PLAYER_EVENT_TYPE_ERROR, player);
}

int player_set_buffering_cb(player_h player, player_buffering_cb callback, void *user_data)
{
	return __set_callback(MUSE_PLAYER_EVENT_TYPE_BUFFERING, player, callback, user_data);
}

int player_unset_buffering_cb(player_h player)
{
	return __unset_callback(MUSE_PLAYER_EVENT_TYPE_BUFFERING, player);
}

int player_set_subtitle_updated_cb(player_h player, player_subtitle_updated_cb callback, void *user_data)
{
	return __set_callback(MUSE_PLAYER_EVENT_TYPE_SUBTITLE, player, callback, user_data);
}

int player_unset_subtitle_updated_cb(player_h player)
{
	return __unset_callback(MUSE_PLAYER_EVENT_TYPE_SUBTITLE, player);
}

int player_set_progressive_download_message_cb(player_h player, player_pd_message_cb callback, void *user_data)
{
	PLAYER_INSTANCE_CHECK(player);
	PLAYER_NULL_ARG_CHECK(callback);
	int ret = PLAYER_ERROR_NONE;
	player_cli_s *pc = (player_cli_s *) player;
	muse_player_api_e api = MUSE_PLAYER_API_SET_CALLBACK;
	char *ret_buf = NULL;
	muse_player_event_e type = MUSE_PLAYER_EVENT_TYPE_PD;
	int set = 1;

	LOGD("ENTER");

	player_msg_send2(api, pc, ret_buf, ret, INT, type, INT, set);

	if (ret == PLAYER_ERROR_NONE) {
		pc->cb_info->user_cb[type] = callback;
		pc->cb_info->user_data[type] = user_data;
		LOGI("Event type : %d ", type);
	}

	g_free(ret_buf);
	return ret;
}

int player_unset_progressive_download_message_cb(player_h player)
{
	PLAYER_INSTANCE_CHECK(player);
	int ret = PLAYER_ERROR_NONE;
	player_cli_s *pc = (player_cli_s *) player;
	muse_player_api_e api = MUSE_PLAYER_API_SET_CALLBACK;
	char *ret_buf = NULL;
	muse_player_event_e type = MUSE_PLAYER_EVENT_TYPE_PD;
	int set = 0;

	LOGD("ENTER");

	set_null_user_cb_lock(pc->cb_info, type);

	player_msg_send2(api, pc, ret_buf, ret, INT, type, INT, set);

	g_free(ret_buf);
	return ret;

}

int player_set_media_packet_video_frame_decoded_cb(player_h player, player_media_packet_video_decoded_cb callback, void *user_data)
{
	PLAYER_INSTANCE_CHECK(player);
	PLAYER_NULL_ARG_CHECK(callback);
	int ret = PLAYER_ERROR_NONE;
	player_cli_s *pc = (player_cli_s *) player;
	muse_player_api_e api = MUSE_PLAYER_API_SET_CALLBACK;
	char *ret_buf = NULL;
	muse_player_event_e type = MUSE_PLAYER_EVENT_TYPE_MEDIA_PACKET_VIDEO_FRAME;
	int set = 1;

	LOGD("ENTER");

	player_msg_send2(api, pc, ret_buf, ret, INT, type, INT, set);

	if (ret == PLAYER_ERROR_NONE) {
		pc->cb_info->user_cb[MUSE_PLAYER_EVENT_TYPE_MEDIA_PACKET_VIDEO_FRAME] = callback;
		pc->cb_info->user_data[MUSE_PLAYER_EVENT_TYPE_MEDIA_PACKET_VIDEO_FRAME] = user_data;
		LOGI("Event type : %d ", MUSE_PLAYER_EVENT_TYPE_MEDIA_PACKET_VIDEO_FRAME);
	}

	g_free(ret_buf);
	return ret;
}

int player_unset_media_packet_video_frame_decoded_cb(player_h player)
{
	PLAYER_INSTANCE_CHECK(player);
	int ret = PLAYER_ERROR_NONE;
	player_cli_s *pc = (player_cli_s *) player;
	muse_player_api_e api = MUSE_PLAYER_API_SET_CALLBACK;
	char *ret_buf = NULL;
	muse_player_event_e type = MUSE_PLAYER_EVENT_TYPE_MEDIA_PACKET_VIDEO_FRAME;
	int set = 0;

	LOGD("ENTER");

	set_null_user_cb_lock(pc->cb_info, type);

	player_msg_send2(api, pc, ret_buf, ret, INT, type, INT, set);

	g_free(ret_buf);
	return ret;
}

int player_set_video_stream_changed_cb(player_h player, player_video_stream_changed_cb callback, void *user_data)
{
	PLAYER_INSTANCE_CHECK(player);
	PLAYER_NULL_ARG_CHECK(callback);
	int ret = PLAYER_ERROR_NONE;
	player_cli_s *pc = (player_cli_s *) player;
	muse_player_api_e api = MUSE_PLAYER_API_SET_CALLBACK;
	char *ret_buf = NULL;
	muse_player_event_e type = MUSE_PLAYER_EVENT_TYPE_VIDEO_STREAM_CHANGED;
	int set = 1;

	LOGD("ENTER");

	player_msg_send2(api, pc, ret_buf, ret, INT, type, INT, set);

	if (ret == PLAYER_ERROR_NONE) {
		pc->cb_info->user_cb[type] = callback;
		pc->cb_info->user_data[type] = user_data;
		LOGI("Event type : %d ", type);
	}

	g_free(ret_buf);
	return ret;
}

int player_unset_video_stream_changed_cb(player_h player)
{
	PLAYER_INSTANCE_CHECK(player);
	int ret = PLAYER_ERROR_NONE;
	player_cli_s *pc = (player_cli_s *) player;
	muse_player_api_e api = MUSE_PLAYER_API_SET_CALLBACK;
	char *ret_buf = NULL;
	muse_player_event_e type = MUSE_PLAYER_EVENT_TYPE_VIDEO_STREAM_CHANGED;
	int set = 0;

	LOGD("ENTER");

	set_null_user_cb_lock(pc->cb_info, type);

	player_msg_send2(api, pc, ret_buf, ret, INT, type, INT, set);

	g_free(ret_buf);
	return ret;
}

int player_set_media_stream_buffer_status_cb(player_h player, player_stream_type_e stream_type, player_media_stream_buffer_status_cb callback, void *user_data)
{
	PLAYER_INSTANCE_CHECK(player);
	PLAYER_NULL_ARG_CHECK(callback);
	int ret = PLAYER_ERROR_NONE;
	player_cli_s *pc = (player_cli_s *) player;
	muse_player_api_e api = MUSE_PLAYER_API_SET_CALLBACK;
	char *ret_buf = NULL;
	muse_player_event_e type;
	int set = 1;

	LOGD("ENTER");

	if (stream_type == PLAYER_STREAM_TYPE_VIDEO)
		type = MUSE_PLAYER_EVENT_TYPE_MEDIA_STREAM_VIDEO_BUFFER_STATUS;
	else if (stream_type == PLAYER_STREAM_TYPE_AUDIO)
		type = MUSE_PLAYER_EVENT_TYPE_MEDIA_STREAM_AUDIO_BUFFER_STATUS;
	else {
		LOGE("PLAYER_ERROR_INVALID_PARAMETER(type : %d)", stream_type);
		return PLAYER_ERROR_INVALID_PARAMETER;
	}

	player_msg_send2(api, pc, ret_buf, ret, INT, type, INT, set);

	if (ret == PLAYER_ERROR_NONE) {
		pc->cb_info->user_cb[type] = callback;
		pc->cb_info->user_data[type] = user_data;
		LOGI("Event type : %d ", type);
	}

	g_free(ret_buf);
	return ret;
}

int player_unset_media_stream_buffer_status_cb(player_h player, player_stream_type_e stream_type)
{
	PLAYER_INSTANCE_CHECK(player);
	int ret = PLAYER_ERROR_NONE;
	player_cli_s *pc = (player_cli_s *) player;
	muse_player_api_e api = MUSE_PLAYER_API_SET_CALLBACK;
	char *ret_buf = NULL;
	muse_player_event_e type;
	int set = 0;

	LOGD("ENTER");

	if (stream_type == PLAYER_STREAM_TYPE_VIDEO)
		type = MUSE_PLAYER_EVENT_TYPE_MEDIA_STREAM_VIDEO_BUFFER_STATUS;
	else if (stream_type == PLAYER_STREAM_TYPE_AUDIO)
		type = MUSE_PLAYER_EVENT_TYPE_MEDIA_STREAM_AUDIO_BUFFER_STATUS;
	else {
		LOGE("PLAYER_ERROR_INVALID_PARAMETER(type : %d)", stream_type);
		return PLAYER_ERROR_INVALID_PARAMETER;
	}

	set_null_user_cb_lock(pc->cb_info, type);

	player_msg_send2(api, pc, ret_buf, ret, INT, type, INT, set);

	g_free(ret_buf);
	return ret;
}

int player_set_media_stream_seek_cb(player_h player, player_stream_type_e stream_type, player_media_stream_seek_cb callback, void *user_data)
{
	PLAYER_INSTANCE_CHECK(player);
	PLAYER_NULL_ARG_CHECK(callback);
	int ret = PLAYER_ERROR_NONE;
	player_cli_s *pc = (player_cli_s *) player;
	muse_player_api_e api = MUSE_PLAYER_API_SET_CALLBACK;
	char *ret_buf = NULL;
	muse_player_event_e type;
	int set = 1;

	LOGD("ENTER");

	if (stream_type == PLAYER_STREAM_TYPE_VIDEO)
		type = MUSE_PLAYER_EVENT_TYPE_MEDIA_STREAM_VIDEO_SEEK;
	else if (stream_type == PLAYER_STREAM_TYPE_AUDIO)
		type = MUSE_PLAYER_EVENT_TYPE_MEDIA_STREAM_AUDIO_SEEK;
	else {
		LOGE("PLAYER_ERROR_INVALID_PARAMETER(type : %d)", stream_type);
		return PLAYER_ERROR_INVALID_PARAMETER;
	}

	player_msg_send2(api, pc, ret_buf, ret, INT, type, INT, set);

	if (ret == PLAYER_ERROR_NONE) {
		pc->cb_info->user_cb[type] = callback;
		pc->cb_info->user_data[type] = user_data;
		LOGI("Event type : %d ", type);
	}

	g_free(ret_buf);
	return ret;
}

int player_unset_media_stream_seek_cb(player_h player, player_stream_type_e stream_type)
{
	PLAYER_INSTANCE_CHECK(player);
	int ret = PLAYER_ERROR_NONE;
	player_cli_s *pc = (player_cli_s *) player;
	muse_player_api_e api = MUSE_PLAYER_API_SET_CALLBACK;
	char *ret_buf = NULL;
	muse_player_event_e type;
	int set = 0;

	LOGD("ENTER");

	if (stream_type == PLAYER_STREAM_TYPE_VIDEO)
		type = MUSE_PLAYER_EVENT_TYPE_MEDIA_STREAM_VIDEO_SEEK;
	else if (stream_type == PLAYER_STREAM_TYPE_AUDIO)
		type = MUSE_PLAYER_EVENT_TYPE_MEDIA_STREAM_AUDIO_SEEK;
	else {
		LOGE("PLAYER_ERROR_INVALID_PARAMETER(type : %d)", stream_type);
		return PLAYER_ERROR_INVALID_PARAMETER;
	}

	set_null_user_cb_lock(pc->cb_info, type);

	player_msg_send2(api, pc, ret_buf, ret, INT, type, INT, set);

	g_free(ret_buf);
	return ret;
}

/* TODO Implement raw data socket channel */
int player_push_media_stream(player_h player, media_packet_h packet)
{
	PLAYER_INSTANCE_CHECK(player);
	PLAYER_NULL_ARG_CHECK(packet);
	int ret = PLAYER_ERROR_NONE;
	player_cli_s *pc = (player_cli_s *) player;
	muse_player_api_e api = MUSE_PLAYER_API_PUSH_MEDIA_STREAM;
	char *ret_buf = NULL;
	player_push_media_msg_type push_media;
	char *push_media_msg = (char *)&push_media;
	int msg_size = sizeof(player_push_media_msg_type);
	int buf_size = 0;
#ifdef __UN_USED
	tbm_bo bo = NULL;
	tbm_bo_handle thandle;
#endif
	char *buf;
	media_format_h format;
	bool is_video;
	bool is_audio;

	LOGD("ENTER");

	media_packet_get_buffer_data_ptr(packet, (void **)&buf);
	media_packet_get_buffer_size(packet, &push_media.size);
	media_packet_get_pts(packet, &push_media.pts);
	media_packet_get_format(packet, &format);
	media_packet_get_flags(packet, &push_media.flags);

	push_media.buf_type = PUSH_MEDIA_BUF_TYPE_RAW;

	media_packet_is_video(packet, &is_video);
	media_packet_is_audio(packet, &is_audio);
	if (is_video)
		media_format_get_video_info(format, &push_media.mimetype, &push_media.width, &push_media.height, NULL, NULL);
	else if (is_audio)
		media_format_get_audio_info(format, &push_media.mimetype, NULL, NULL, NULL, NULL);

	media_format_unref(format);

#ifdef __UN_USED
	if (push_media.buf_type == PUSH_MEDIA_BUF_TYPE_TBM) {
		bo = tbm_bo_alloc(pc->cb_info->bufmgr, push_media.size, TBM_BO_DEFAULT);
		if (bo == NULL) {
			LOGE("TBM get error : bo is NULL");
			return PLAYER_ERROR_INVALID_OPERATION;
		}
		thandle = tbm_bo_map(bo, TBM_DEVICE_CPU, TBM_OPTION_WRITE);
		if (thandle.ptr == NULL) {
			LOGE("TBM get error : handle pointer is NULL");
			ret = PLAYER_ERROR_INVALID_OPERATION;
			goto ERROR;
		}
		memcpy(thandle.ptr, buf, push_media.size);
		tbm_bo_unmap(bo);

		push_media.key = tbm_bo_export(bo);
		if (push_media.key == 0) {
			LOGE("TBM get error : key is 0");
			ret = PLAYER_ERROR_INVALID_OPERATION;
			goto ERROR;
		}

		player_msg_send_array(api, pc, ret_buf, ret, push_media_msg, msg_size, sizeof(char));
	} else if (push_media.buf_type == PUSH_MEDIA_BUF_TYPE_MSG) {
		buf_size = (int)push_media.size;
		player_msg_send_array2(api, pc, ret_buf, ret, push_media_msg, msg_size, sizeof(char), buf, buf_size, sizeof(char));
	} else
#endif
	if (push_media.buf_type == PUSH_MEDIA_BUF_TYPE_RAW) {
		buf_size = (int)push_media.size;
		muse_core_ipc_push_data(pc->cb_info->data_fd, buf, buf_size, push_media.pts);
		player_msg_send_array(api, pc, ret_buf, ret, push_media_msg, msg_size, sizeof(char));
	}

	LOGD("ret_buf %s", ret_buf);

#ifdef __UN_USED
 ERROR:
	if (push_media.buf_type == PUSH_MEDIA_BUF_TYPE_TBM)
		tbm_bo_unref(bo);
#endif

	g_free(ret_buf);
	return ret;
}

int player_set_media_stream_info(player_h player, player_stream_type_e type, media_format_h format)
{
	PLAYER_INSTANCE_CHECK(player);
	g_return_val_if_fail(format, PLAYER_ERROR_INVALID_OPERATION);
	int ret = PLAYER_ERROR_NONE;
	player_cli_s *pc = (player_cli_s *) player;
	muse_player_api_e api = MUSE_PLAYER_API_SET_MEDIA_STREAM_INFO;
	char *ret_buf = NULL;
	media_format_mimetype_e mimetype;
	int width;
	int height;
	int avg_bps;
	int max_bps;
	int channel;
	int samplerate;
	int bit;

	LOGD("ENTER");

	media_format_ref(format);
	if (type == PLAYER_STREAM_TYPE_VIDEO) {
		media_format_get_video_info(format, &mimetype, &width, &height, &avg_bps, &max_bps);
		player_msg_send6(api, pc, ret_buf, ret, INT, type, INT, mimetype, INT, width, INT, height, INT, avg_bps, INT, max_bps);
	} else if (type == PLAYER_STREAM_TYPE_AUDIO) {
		media_format_get_audio_info(format, &mimetype, &channel, &samplerate, &bit, &avg_bps);
		player_msg_send6(api, pc, ret_buf, ret, INT, type, INT, mimetype, INT, channel, INT, samplerate, INT, avg_bps, INT, bit);
	}
	media_format_unref(format);
	pc->push_media_stream = TRUE;

	g_free(ret_buf);
	return ret;
}

int player_set_media_stream_buffer_max_size(player_h player, player_stream_type_e type, unsigned long long max_size)
{
	int ret = PLAYER_ERROR_NONE;
	PLAYER_INSTANCE_CHECK(player);
	player_cli_s *pc = (player_cli_s *) player;
	muse_player_api_e api = MUSE_PLAYER_API_SET_MEDIA_STREAM_BUFFER_MAX_SIZE;
	char *ret_buf = NULL;

	LOGD("ENTER");

	player_msg_send2(api, pc, ret_buf, ret, INT, type, INT64, max_size);

	g_free(ret_buf);
	return ret;
}

int player_get_media_stream_buffer_max_size(player_h player, player_stream_type_e type, unsigned long long *pmax_size)
{
	int ret = PLAYER_ERROR_NONE;
	PLAYER_INSTANCE_CHECK(player);
	PLAYER_NULL_ARG_CHECK(pmax_size);
	player_cli_s *pc = (player_cli_s *) player;
	muse_player_api_e api = MUSE_PLAYER_API_GET_MEDIA_STREAM_BUFFER_MAX_SIZE;
	char *ret_buf = NULL;
	unsigned long long max_size;

	LOGD("ENTER");

	player_msg_send1(api, pc, ret_buf, ret, INT, type);
	if (ret == PLAYER_ERROR_NONE) {
		player_msg_get_type(max_size, ret_buf, INT64);
		*pmax_size = max_size;
	}
	g_free(ret_buf);
	return ret;
}

int player_set_media_stream_buffer_min_threshold(player_h player, player_stream_type_e type, unsigned int percent)
{
	int ret = PLAYER_ERROR_NONE;
	PLAYER_INSTANCE_CHECK(player);
	player_cli_s *pc = (player_cli_s *) player;
	muse_player_api_e api = MUSE_PLAYER_API_SET_MEDIA_STREAM_BUFFER_MIN_THRESHOLD;
	char *ret_buf = NULL;

	LOGD("ENTER");

	player_msg_send2(api, pc, ret_buf, ret, INT, type, INT, percent);

	g_free(ret_buf);
	return ret;
}

int player_get_media_stream_buffer_min_threshold(player_h player, player_stream_type_e type, unsigned int *ppercent)
{
	int ret = PLAYER_ERROR_NONE;
	PLAYER_INSTANCE_CHECK(player);
	PLAYER_NULL_ARG_CHECK(ppercent);
	player_cli_s *pc = (player_cli_s *) player;
	muse_player_api_e api = MUSE_PLAYER_API_GET_MEDIA_STREAM_BUFFER_MIN_THRESHOLD;
	char *ret_buf = NULL;
	uint percent;

	LOGD("ENTER");

	player_msg_send1(api, pc, ret_buf, ret, INT, type);
	if (ret == PLAYER_ERROR_NONE) {
		player_msg_get(percent, ret_buf);
		*ppercent = percent;
	}

	g_free(ret_buf);
	return ret;
}

int player_get_track_count(player_h player, player_stream_type_e type, int *pcount)
{
	PLAYER_INSTANCE_CHECK(player);
	PLAYER_NULL_ARG_CHECK(pcount);
	int ret = PLAYER_ERROR_NONE;
	player_cli_s *pc = (player_cli_s *) player;
	muse_player_api_e api = MUSE_PLAYER_API_GET_TRACK_COUNT;
	char *ret_buf = NULL;
	int count;

	LOGD("ENTER");

	player_msg_send1(api, pc, ret_buf, ret, INT, type);
	if (ret == PLAYER_ERROR_NONE) {
		player_msg_get(count, ret_buf);
		*pcount = count;
	}

	g_free(ret_buf);
	return ret;
}

int player_get_current_track(player_h player, player_stream_type_e type, int *pindex)
{
	PLAYER_INSTANCE_CHECK(player);
	PLAYER_NULL_ARG_CHECK(pindex);
	int ret = PLAYER_ERROR_NONE;
	player_cli_s *pc = (player_cli_s *) player;
	muse_player_api_e api = MUSE_PLAYER_API_GET_CURRENT_TRACK;
	char *ret_buf = NULL;
	int index;

	LOGD("ENTER");

	player_msg_send1(api, pc, ret_buf, ret, INT, type);
	if (ret == PLAYER_ERROR_NONE) {
		player_msg_get(index, ret_buf);
		*pindex = index;
	}

	g_free(ret_buf);
	return ret;
}

int player_select_track(player_h player, player_stream_type_e type, int index)
{
	PLAYER_INSTANCE_CHECK(player);
	int ret = PLAYER_ERROR_NONE;
	player_cli_s *pc = (player_cli_s *) player;
	muse_player_api_e api = MUSE_PLAYER_API_SELECT_TRACK;
	char *ret_buf = NULL;

	LOGD("ENTER");

	player_msg_send2(api, pc, ret_buf, ret, INT, type, INT, index);

	g_free(ret_buf);
	return ret;
}

int player_get_track_language_code(player_h player, player_stream_type_e type, int index, char **pcode)
{
	PLAYER_INSTANCE_CHECK(player);
	PLAYER_NULL_ARG_CHECK(pcode);
	int ret = PLAYER_ERROR_NONE;
	player_cli_s *pc = (player_cli_s *) player;
	muse_player_api_e api = MUSE_PLAYER_API_GET_TRACK_LANGUAGE_CODE;
	char *ret_buf = NULL;
	char code[MUSE_MSG_MAX_LENGTH] = { 0, };
	const int code_len = 2;

	LOGD("ENTER");

	player_msg_send2(api, pc, ret_buf, ret, INT, type, INT, index);
	if (ret == PLAYER_ERROR_NONE) {
		if (player_msg_get_string(code, ret_buf))
			*pcode = strndup(code, code_len);
	}

	g_free(ret_buf);
	return ret;
}
