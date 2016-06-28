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

#include <glib.h>
#include <string.h>
#include <dlog.h>
#include <mm_error.h>
#include <wayland-client.h>
#include <tizen-extension-client-protocol.h>
#include <muse_player_msg.h>
#include <Evas.h>
#include <Ecore_Evas.h>
#include "player_msg.h"
#include "player_display.h"
#include "player_private.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "TIZEN_N_PLAYER"

#define goto_if_fail(expr, label)	\
{	\
	if (!(expr)) {	\
		debug_error(" failed [%s]\n", #expr);	\
		goto label;	\
	}	\
}

void handle_resource_id(void *data, struct tizen_resource *tizen_resource, uint32_t id)
{
	unsigned int *wl_surface_id = data;

	*wl_surface_id = id;

	LOGD("[CLIENT] got wl_surface_id(%d) from server\n", id);
}

static const struct tizen_resource_listener tz_resource_listener = {
	handle_resource_id,
};

static void handle_global(void *data, struct wl_registry *registry, uint32_t name, const char *interface, uint32_t version)
{
	return_if_fail(data != NULL);
	wl_client *wlclient = data;

	if (strcmp(interface, "tizen_surface") == 0) {
		LOGD("binding tizen_surface");
		wlclient->tz_surface = wl_registry_bind(registry, name, &tizen_surface_interface, version);
		return_if_fail(wlclient->tz_surface != NULL);
	}
}

static const struct wl_registry_listener registry_listener = {
	handle_global,
};

int _wl_client_create(wl_client ** wlclient)
{
	wl_client *ptr = NULL;

	ptr = g_malloc0(sizeof(wl_client));
	if (!ptr) {
		LOGE("Cannot allocate memory for wlclient\n");
		goto ERROR;
	} else {
		*wlclient = ptr;
		LOGD("Success create wlclient(%p)", *wlclient);
	}
	return MM_ERROR_NONE;

 ERROR:
	*wlclient = NULL;
	return MM_ERROR_PLAYER_NO_FREE_SPACE;
}

int _wl_client_get_wl_window_wl_surface_id(wl_client * wlclient, struct wl_surface *surface, struct wl_display *display)
{
	goto_if_fail(wlclient != NULL, failed);
	goto_if_fail(surface != NULL, failed);
	goto_if_fail(display != NULL, failed);

	unsigned int wl_surface_id = 0;

	wlclient->display = display;
	goto_if_fail(wlclient->display != NULL, failed);

	wlclient->registry = wl_display_get_registry(wlclient->display);
	goto_if_fail(wlclient->registry != NULL, failed);

	wl_registry_add_listener(wlclient->registry, &registry_listener, wlclient);
	wl_display_dispatch(wlclient->display);
	wl_display_roundtrip(wlclient->display);

	/* check global objects */
	goto_if_fail(wlclient->tz_surface != NULL, failed);

	/* Get wl_surface_id which is unique in a entire systemw. */
	wlclient->tz_resource = tizen_surface_get_tizen_resource(wlclient->tz_surface, surface);
	goto_if_fail(wlclient->tz_resource != NULL, failed);

	tizen_resource_add_listener(wlclient->tz_resource, &tz_resource_listener, &wl_surface_id);
	wl_display_roundtrip(wlclient->display);
	goto_if_fail(wl_surface_id > 0, failed);

	_wl_client_finalize(wlclient);

	return wl_surface_id;

 failed:
	LOGE("Failed to get wl_surface_id");

	return 0;
}

void _wl_client_finalize(wl_client * wlclient)
{
	LOGD("start finalize wlclient");
	return_if_fail(wlclient != NULL)

		if (wlclient->tz_surface)
		tizen_surface_destroy(wlclient->tz_surface);

	if (wlclient->tz_resource)
		tizen_resource_destroy(wlclient->tz_resource);

	/* destroy registry */
	if (wlclient->registry)
		wl_registry_destroy(wlclient->registry);
	return;
}

static void __evas_resize_cb(void *data, Evas * e, Evas_Object * eo, void *event_info)
{
	player_cli_s *pc = (player_cli_s *) data;
	wl_win_msg_type wl_win;
	char *wl_win_msg = (char *)&wl_win;
	char *ret_buf = NULL;
	muse_player_api_e api = MUSE_PLAYER_API_RESIZE_VIDEO_RENDER_RECT;
	int ret = PLAYER_ERROR_NONE;
	LOGD("ret =%d", ret);

	ret = _wl_window_geometry_get(eo, e, &wl_win.wl_window_x, &wl_win.wl_window_y,
				&wl_win.wl_window_width, &wl_win.wl_window_height);
	if (ret != PLAYER_ERROR_NONE) {
		LOGE("Fail to get window geometry");
		return;
	}
	wl_win.type = 0;			/*init  but not use */
	wl_win.wl_surface_id = 0;	/*init  but not use */

	player_msg_send_array(api, pc, ret_buf, ret, wl_win_msg, sizeof(wl_win_msg_type), sizeof(char));

	g_free(ret_buf);
	return;

}

static void __evas_del_cb(void *data, Evas * e, Evas_Object * eo, void *event_info)
{

	player_cli_s *pc = (player_cli_s *) data;

	evas_object_event_callback_del(eo, EVAS_CALLBACK_RESIZE, __evas_resize_cb);
	evas_object_event_callback_del(eo, EVAS_CALLBACK_DEL, __evas_del_cb);

	LOGD("evas callback del %p", eo);
	pc->have_evas_callback = FALSE;

	return;
}

int _wl_window_geometry_get(Evas_Object *eo, Evas *e, int *x, int *y, int *width, int *height)
{
	Ecore_Evas *ecore_evas;
	int rotation;
	return_val_if_fail(eo != NULL || e != NULL || x != NULL || y != NULL || width != NULL || height != NULL,
			PLAYER_ERROR_INVALID_OPERATION);

	evas_object_geometry_get(eo, x, y, width, height);
	LOGD("get window geometroy : x(%d) y(%d) width(%d) height(%d)", *x, *y, *width, *height);
	/* get rotaion */
	ecore_evas = ecore_evas_ecore_evas_get(e);
	return_val_if_fail(ecore_evas != NULL, PLAYER_ERROR_INVALID_OPERATION);
	rotation = ecore_evas_rotation_get(ecore_evas);

	LOGD("rotation(%d)", rotation);
	/* swap */
	if (rotation == 270 || rotation == 90) {
		LOGD("swap width with height");
		int temp;
		temp = *width;
		*width = *height;
		*height = temp;
		LOGD("get window geometroy : x(%d) y(%d) width(%d) height(%d)", *x, *y, *width, *height);
	}

	return MM_ERROR_NONE;
}

int _wl_window_evas_object_cb_add(player_h player, Evas_Object *eo)
{

	PLAYER_INSTANCE_CHECK(player);
	return_val_if_fail(eo != NULL, MM_ERROR_INVALID_ARGUMENT);

	player_cli_s *pc = (player_cli_s *) player;

	if (pc->have_evas_callback && pc->eo == eo) {
		LOGW("evas object had callback already %p", pc->eo);
		return MM_ERROR_UNKNOWN;
	}
	pc->eo = eo;
	evas_object_event_callback_add(eo, EVAS_CALLBACK_RESIZE, __evas_resize_cb, player);
	evas_object_event_callback_add(eo, EVAS_CALLBACK_DEL, __evas_del_cb, player);
	LOGD("evas callback add %p", pc->eo);
	pc->have_evas_callback = TRUE;

	return MM_ERROR_NONE;
}

int _wl_window_evas_object_cb_del(player_h player)
{
	PLAYER_INSTANCE_CHECK(player);
	player_cli_s *pc = (player_cli_s *) player;
	return_val_if_fail(pc->eo != NULL, MM_ERROR_INVALID_ARGUMENT);

	evas_object_event_callback_del(pc->eo, EVAS_CALLBACK_RESIZE, __evas_resize_cb);
	evas_object_event_callback_del(pc->eo, EVAS_CALLBACK_DEL, __evas_del_cb);
	LOGD("evas callback del %p", pc->eo);
	pc->eo = NULL;
	pc->have_evas_callback = FALSE;

	return MM_ERROR_NONE;
}

