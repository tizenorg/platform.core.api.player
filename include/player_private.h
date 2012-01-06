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

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	_PLAYER_EVENT_TYPE_BEGIN,	
	_PLAYER_EVENT_TYPE_COMPLETE,
	_PLAYER_EVENT_TYPE_PAUSE,
	_PLAYER_EVENT_TYPE_INTERRUPT,
	_PLAYER_EVENT_TYPE_ERROR,	
	_PLAYER_EVENT_TYPE_BUFFERING,
	_PLAYER_EVENT_TYPE_SUBTITLE,
	_PLAYER_EVENT_TYPE_CAPTURE,
	_PLAYER_EVENT_TYPE_NUM
}_player_event_e;

typedef struct _player_s{
	MMHandleType mm_handle;
	const void* user_cb[_PLAYER_EVENT_TYPE_NUM];
	void* user_data[_PLAYER_EVENT_TYPE_NUM];
	void* display_handle;
	player_display_type_e display_type;
	int state;
} player_s;

int mm_player_msg_callback(int message, void *param, void *user_data);


#ifdef __cplusplus
}
#endif

#endif //__TIZEN_MEDIA_PLAYER_PRIVATE_H__



