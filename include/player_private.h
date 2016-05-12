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
#include <tbm_bufmgr.h>
#include <Evas.h>
#include <media_format.h>
#include <muse_player.h>
#include "player.h"
#include "player_wayland.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "TIZEN_N_PLAYER"

#define PLAYER_CHECK_CONDITION(condition, error, msg)     \
		if (condition) {} else \
		{ LOGE("[%s] %s(0x%08x)", __FUNCTION__, msg, error); return error; }; \

#define PLAYER_INSTANCE_CHECK(player)   \
		PLAYER_CHECK_CONDITION(player != NULL, PLAYER_ERROR_INVALID_PARAMETER, "PLAYER_ERROR_INVALID_PARAMETER")

#define PLAYER_STATE_CHECK(player, expected_state)       \
		PLAYER_CHECK_CONDITION(player->state == expected_state, PLAYER_ERROR_INVALID_STATE, "PLAYER_ERROR_INVALID_STATE")

#define PLAYER_NULL_ARG_CHECK(arg)      \
		PLAYER_CHECK_CONDITION(arg != NULL, PLAYER_ERROR_INVALID_PARAMETER, "PLAYER_ERROR_INVALID_PARAMETER")

#define CALLBACK_TIME_OUT (5*1000) /* ms */
#define MAX_SERVER_TIME_OUT 35     /* sec */

typedef struct _ret_msg_s {
	gint api;
	gchar *msg;
	struct _ret_msg_s *next;
} ret_msg_s;

typedef struct {
	gint bufLen;
	gchar *recvMsg;
	gint recved;
	ret_msg_s *retMsgHead;
} msg_buff_s;

typedef struct _player_data {
	void *data;
	struct _player_data *next;
} player_data_s;

typedef struct {
	GThread *thread;
	GQueue *queue;
	GMutex qlock;
	GMutex mutex;
	GCond cond;
	gboolean running;
} player_event_queue;

typedef struct _callback_cb_info {
	GThread *thread;
	gint running;
	gint fd;
	gint data_fd;
	gpointer user_cb[MUSE_PLAYER_EVENT_TYPE_NUM];
	gpointer user_data[MUSE_PLAYER_EVENT_TYPE_NUM];
	GMutex player_mutex;
	GCond player_cond[MUSE_PLAYER_API_MAX];
	GList *packet_list;
	GMutex packet_list_mutex;
	msg_buff_s buff;
	player_event_queue event_queue;
	media_format_h pkt_fmt;
	void *evas_info;
	tbm_bufmgr bufmgr;
} callback_cb_info_s;

typedef struct {
	intptr_t bo;
	gint timeout; /* sec */
} server_info_s;

typedef struct _player_cli_s {
	callback_cb_info_s *cb_info;
	player_data_s *head;
	server_info_s server;
	wl_client *wlclient;
	Evas_Object *eo;
	gboolean have_evas_callback;
} player_cli_s;

/* player callback information */
#define CALLBACK_INFO(h)	((h)->cb_info)
/* MSG Channel socket fd */
#define MSG_FD(h)			(CALLBACK_INFO(h)->fd)
/* Data Channel socket fd */
#define DATA_FD(h)			(CALLBACK_INFO(h)->data_fd)
/* TBM buffer manager */
#define TBM_BUFMGR(h)		(CALLBACK_INFO(h)->bufmgr)
/* evas display handle */
#define EVAS_HANDLE(h)		((h)->cb_info->evas_info)

/* server tbm bo */
#define SERVER_TBM_BO(h)	((h)->server.bo)
/* server state change timeout (sec) */
#define SERVER_TIMEOUT(h)		((h)->server.timeout)

int player_set_evas_object_cb(player_h player, Evas_Object * eo);
int player_unset_evas_object_cb(player_h player);
int client_get_api_timeout(player_cli_s * pc, muse_player_api_e api);
int client_wait_for_cb_return(muse_player_api_e api, callback_cb_info_s * cb_info, char **ret_buf, int time_out);

#ifdef __cplusplus
}
#endif
#endif	/*__TIZEN_MEDIA_PLAYER_PRIVATE_H__*/
