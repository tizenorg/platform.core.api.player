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

#ifndef __TIZEN_MEDIA_PLAYER_INTERNAL_H__
#define	__TIZEN_MEDIA_PLAYER_INTERNAL_H__
#include <player.h>
#ifdef TIZEN_TV
#include <glib.h>
#endif
#include <Ecore_Wayland.h>
#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file player_internal.h
 */

/**
 * @addtogroup CAPI_MEDIA_PLAYER_MODULE
 * @{
 */

/**
 * @brief This file contains the media player API for custom features.
 * @since_tizen 2.4
 */
typedef struct {
	void *data;          /**< PCM data */
	int size;            /**< Data Size */
	int channel;         /**< Channel */
	int rate;            /**<  Samplerate */
	int depth;           /**< Depth */
	bool little_endian;  /**< Endianness */
	unsigned long long channel_mask;	/**< channel_mask */
} player_audio_raw_data_s;

#ifdef TIZEN_TV
/**
 * @brief Information about packet.
 * @since_tizen 3.0
 */
typedef struct {
	guint32 len;
	guint64 pts;
	player_stream_type_e streamtype;
} player_submit_packet_t;


/**
 * @brief Video stream info in external demux case
 * @since_tizen 3.0
 */
typedef struct _video_stream_info_s {
	const char *mime;
	unsigned int framerate_num;
	unsigned int framerate_den;
	int format3D;
	unsigned int width;
	unsigned int height;
	unsigned int pixelAspectRatioX;
	unsigned int pixelAspectRatioY;
	unsigned char *codec_extradata;
	unsigned int extradata_size;
	unsigned int version;
	unsigned int colordepth;
	unsigned int max_width;
	unsigned int max_height;
	unsigned int drm_type;
	unsigned int buffer_type;
} player_video_stream_info_s;

/**
 * @brief Audio stream info in external demux case
 * @since_tizen 3.0
 */
typedef struct _audio_stream_info_s {
	const char *mime;
	unsigned int channels;
	unsigned int sample_rate;
	unsigned int bit_rate;
	unsigned int block_align;
	unsigned char *codec_extradata;
	unsigned int extradata_size;
	unsigned int version;
	unsigned int user_info;

	/* for pcm, eg. CODEC_ID_PCM_S16LE, width=16,depth=16,endianness=1234,signedness=true */
	unsigned int width;
	unsigned int depth;
	unsigned int endianness;	/* LE:1234, BE:4321 */
	bool signedness;			/* S:true, U:false */
	unsigned int drm_type;
	unsigned int buffer_type;
} player_audio_stream_info_s;

#endif

/**
 * @brief Called when the audio frame is decoded.
 * @since_tizen 2.4
 * @param[in] audio_raw_frame The decoded audio frame data type
 * @param[in] user_data          The user data passed from the callback registration function
 * @see player_set_audio_frame_decoded_cb()
 * @see player_unset_audio_frame_decoded_cb()
 */
typedef void (*player_audio_pcm_extraction_cb)(player_audio_raw_data_s *audio_raw_frame, void *user_data);

/**
 * @brief Called when the buffer level drops below the min size or exceeds the max size.
 * @since_tizen 3.0
 * @remarks This API is used for media stream playback only.
 * @param[in] status The buffer status
 * @param[in] bytes The current buffer level bytes
 * @param[in] user_data The user data passed from the callback registration function
 * @see player_set_media_stream_buffer_status_cb_ex()
 * @see player_set_media_stream_buffer_max_size()
 * @see player_set_media_stream_buffer_min_threshold()
 */
typedef void (*player_media_stream_buffer_status_cb_ex) (player_media_stream_buffer_status_e status, unsigned long long bytes, void *user_data);

/**
 * @brief Registers a callback function to be invoked when audio frame is decoded. Audio only contents is possible. If included video, error happens.
 * @since_tizen 2.4
 * @param[in] player    The handle to the media player
 * @param[in] sync Sync on the clock
 * @param[in] format Audio format of output pcm
 * @param[in] callback The callback function to register
 * @param[in] user_data The user data to be passed to the callback function
 * @return @c 0 on success,
 *         otherwise a negative error value
 * @retval #PLAYER_ERROR_NONE Successful
 * @retval #PLAYER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #PLAYER_ERROR_INVALID_OPERATION Invalid operation
 * @retval #PLAYER_ERROR_INVALID_STATE Invalid player state
 * @retval #PLAYER_ERROR_NOT_SUPPORTD Not supported
 * @pre The player state must be #PLAYER_STATE_IDLE by player_create() or player_unprepare().
 * @post player_audio_pcm_extraction_cb() will be invoked.
 */
int player_set_pcm_extraction_mode(player_h player, bool sync, player_audio_pcm_extraction_cb callback, void *user_data);

/**
 * @brief Set pcm mode spec. Samplerate, channel is needed.
 * @since_tizen 2.4
 * @param[in] player    The handle to the media player
 * @param[in] samplerate Samplerate
 * @param[in] channel Channel
 * @return @c 0 on success,
 *         otherwise a negative error value
 * @retval #PLAYER_ERROR_NONE Successful
 * @retval #PLAYER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #PLAYER_ERROR_INVALID_OPERATION Invalid operation
 * @retval #PLAYER_ERROR_INVALID_STATE Invalid player state
 * @retval #PLAYER_ERROR_NOT_SUPPORTD Not supported
 * @pre The player state must be #PLAYER_STATE_IDLE by player_create() or player_unprepare().
 * @see player_set_pcm_extraction_mode()
 */
int player_set_pcm_spec(player_h player, const char *format, int samplerate, int channel);

/**
 * @brief Sets the playback rate include streaming mode.
 * @since_tizen 3.0
 * @details The default value is @c 1.0.
 * @remarks No operation is performed, if @a rate is @c 0.
 * @remarks The sound is muted, when playback rate is under @c 0.0 and over @c 2.0.
 * @param[in]   player The handle to the media player
 * @param[in]   rate The playback rate
 * @retval #PLAYER_ERROR_INVALID_OPERATION Invalid operation
 * @retval #PLAYER_ERROR_INVALID_STATE Invalid player state
 * @pre The player state must be set to #PLAYER_STATE_PLAYING by calling player_start().
 * @pre The player state must be set to #PLAYER_STATE_READY by calling player_prepare() or set to #PLAYER_STATE_PLAYING by calling player_start() or set to #PLAYER_STATE_PAUSED by calling player_pause().
 */
int player_set_streaming_playback_rate(player_h player, float rate);

/**
 * @brief Registers a callback function to be invoked when buffer underrun or overflow is occurred.
 * @since_tizen 3.0
 * @remarks This API is used for media stream playback only.
 * @param[in] player   The handle to the media player
 * @param[in] type     The type of target stream
 * @param[in] callback The buffer status callback function to register
 * @param[in] user_data The user data to be passed to the callback function
 * @return @c 0 on success,
 *         otherwise a negative error value
 * @retval #PLAYER_ERROR_NONE Successful
 * @retval #PLAYER_ERROR_INVALID_STATE Invalid player state
 * @retval #PLAYER_ERROR_INVALID_PARAMETER Invalid parameter
 * @pre The player state must be set to #PLAYER_STATE_IDLE by calling player_create() or player_unprepare().
 * @post player_media_stream_buffer_status_cb_ex() will be invoked.
 * @see player_unset_media_stream_buffer_status_cb_ex()
 * @see player_media_stream_buffer_status_cb_ex()
 */
int player_set_media_stream_buffer_status_cb_ex(player_h player, player_stream_type_e type, player_media_stream_buffer_status_cb_ex callback, void *user_data);

/**
 * @brief Unregisters the buffer status callback function.
 * @since_tizen 3.0
 * @remarks This API is used for media stream playback only.
 * @param[in] player The handle to the media player
 * @param[in] type   The type of target stream
 * @return @c 0 on success,
 *         otherwise a negative error value
 * @retval #PLAYER_ERROR_NONE Successful
 * @retval #PLAYER_ERROR_INVALID_STATE Invalid player state
 * @retval #PLAYER_ERROR_INVALID_PARAMETER Invalid parameter
 * @see player_set_media_stream_buffer_status_cb()
 */
int player_unset_media_stream_buffer_status_cb_ex(player_h player, player_stream_type_e type);

/**
 * @brief Sets DRC(dynamic resolution change) information of media stream data.
 * @since_tizen 3.0
 * @remarks This API is used for media stream playback only.
 *          If the hw codec can not support drc, app have to set DRC information before calling player_prepare().
 * @param[in] player The handle to the media player
 * @param[in] drc The drc status: (@c true = drc, @c false = non drc)
 * @return @c 0 on success,
 *         otherwise a negative error value
 * @retval #PLAYER_ERROR_NONE Successful
 * @retval #PLAYER_ERROR_INVALID_STATE Invalid player state
 * @retval #PLAYER_ERROR_INVALID_PARAMETER Invalid parameter
 * @pre The player state must be set to #PLAYER_STATE_IDLE by calling player_create() or player_unprepare().
 */
int player_set_media_stream_dynamic_resolution(player_h player, bool drc);

/**
 * @brief Sets the ecore wayland video display.
 * @since_tizen 3.0
 * @remarks This API support PLAYER_DISPLAY_TYPE_OVERLAY type only.
 * @param[in]   player The handle to the media player
 * @param[in]   type The display type
 * @param[in]   ecore_wl_window The ecore wayland window handle
 * @param[in]   x the x coordinate of window
 * @param[in]   y the y coordinate of window
 * @param[in]   width the width of window
 * @param[in]   height the height of window
 * @return @c 0 on success,
 *         otherwise a negative error value
 * @retval #PLAYER_ERROR_NONE Successful
 * @retval #PLAYER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #PLAYER_ERROR_INVALID_OPERATION Invalid operation
 * @retval #PLAYER_ERROR_INVALID_STATE Invalid player state
 * @pre The player state must be one of these: #PLAYER_STATE_IDLE, #PLAYER_STATE_READY, #PLAYER_STATE_PLAYING, or #PLAYER_STATE_PAUSED.
 * @see player_set_display_rotation
 */
int player_set_ecore_wl_display(player_h player, player_display_type_e type, Ecore_Wl_Window *ecore_wl_window, int x, int y, int  width, int height);

#ifdef TIZEN_TV
/**
 * @brief Called when no free space in buffer.
 * @since_tizen 3.0
 * @param[in] user_data    The user data passed from the callback registration function
 * @see player_set_buffer_enough_video_data_cb()
 * @see player_set_buffer_enough_audio_data_cb()
 */
typedef void (*player_buffer_enough_data_cb) (void *user_data);

/**
 * @brief Called when the buffer level drops below the threshold.
 * @since_tizen 3.0
 * @param[in] size         Required data size
 * @param[in] user_data    The user data passed from the callback registration function
 * @see player_set_buffer_need_video_data_cb()
 * @see player_set_buffer_need_audio_data_cb()
 */
typedef void (*player_buffer_need_data_cb) (unsigned int size, void *user_data);

/**
 * @brief Called to notify the resource conflict
 * @since_tizen 3.0
 * @param[in] user_data    The user data passed from the callback registration function
 * @see player_set_resourceconflict_cb()
 */
typedef void (*player_resourceconflict_cb)(void *user_data);

/**
 * @brief Submits packet to appsrc, used in external demux mode
 * @since_tizen 3.0
 * @param[in]  player   The handle to media player
 * @param[in]  buf      the buf want to submit to appsrc, usually one buffer one frame
 * @param[in]  len      the size of buf
 * @param[in]  pts      pts of the submit buf(frame)
 * @param[in]  streamtype   true for video, false for audio
 * @return 0 on success, otherwise a negative error value.
 */
int player_submit_packet(player_h player, guint8 *buf, guint32 len, guint64 pts, player_stream_type_e streamtype);

/**
 * @brief Sets video stream info, escepically used in external demux mode
 * @since_tizen 3.0
 * @param[in]  player   The handle to media player
 * @param[in]  info     the info of video stream set by external demux
 * @return 0 on success, otherwise a negative error value.
 */
int player_set_video_stream_info(player_h player, player_video_stream_info_s *info);

/**
 * @brief Sets audio stream info, escepically used in external demux mode
 * @since_tizen 3.0
 * @param[in]  player   The handle to media player
 * @param[in]  info     the info of audio stream set by external demux
 * @return 0 on success, otherwise a negative error value.
 */
int player_set_audio_stream_info(player_h player, player_audio_stream_info_s *info);

/**
 * @brief Registers the callback function.
 * @since_tizen 3.0
 * @param[in] player The handle to media player
 * @param[in] callback  The callback function to register
 * @param[in] user_data The user data to be passed to the callback function
 * @return 0 on success, otherwise a negative error value.
 * @retval #PLAYER_ERROR_NONE Successful
 * @retval #PLAYER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #PLAYER_ERROR_INVALID_OPERATION Invalid operation
 */
int player_set_buffer_enough_video_data_cb(player_h player, player_buffer_enough_data_cb callback, void *user_data);

/**
 * @brief Registers the callback function.
 * @since_tizen 3.0
 * @param[in] player The handle to media player
 * @param[in] callback  The callback function to register
 * @param[in] user_data The user data to be passed to the callback function
 * @return 0 on success, otherwise a negative error value.
 * @retval #PLAYER_ERROR_NONE Successful
 * @retval #PLAYER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #PLAYER_ERROR_INVALID_OPERATION Invalid operation
 */
int player_set_buffer_enough_audio_data_cb(player_h player, player_buffer_enough_data_cb callback, void *user_data);

/**
 * @brief Registers the callback function.
 * @since_tizen 3.0
 * @param[in] player The handle to media player
 * @param[in] callback  The callback function to register
 * @param[in] user_data The user data to be passed to the callback function
 * @return 0 on success, otherwise a negative error value.
 * @retval #PLAYER_ERROR_NONE Successful
 * @retval #PLAYER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #PLAYER_ERROR_INVALID_OPERATION Invalid operation
 */
int player_set_buffer_need_video_data_cb(player_h player, player_buffer_need_data_cb callback, void *user_data);

/**
 * @brief Registers the callback function.
 * @since_tizen 3.0
 * @param[in] player The handle to media player
 * @param[in] callback  The callback function to register
 * @param[in] user_data The user data to be passed to the callback function
 * @return 0 on success, otherwise a negative error value.
 * @retval #PLAYER_ERROR_NONE Successful
 * @retval #PLAYER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #PLAYER_ERROR_INVALID_OPERATION Invalid operation
 */
int player_set_buffer_need_audio_data_cb(player_h player, player_buffer_need_data_cb callback, void *user_data);

/**
 * @brief Registers a callback function to be invoked when the resource conflict is detected.
 * @since_tizen 3.0
 * @param[in] player	The handle to the media player
 * @param[in] callback	The callback function to register
 * @param[in] user_data	The user data to be passed to the callback function
 * @return @c 0 on success,
 *         otherwise a negative error value
 * @retval #PLAYER_ERROR_NONE Successful
 * @retval #PLAYER_ERROR_INVALID_PARAMETER Invalid parameter
 * @post player_resourceconflict_cb() will be invoked.
 * @see player_unset_resourceconflict_cb()
 */
int player_set_resourceconflict_cb(player_h player, player_resourceconflict_cb callback, void *user_data);

/**
 * @brief Unregisters the callback function.
 * @since_tizen 3.0
 * @param[in] player	The handle to the media player
 * @return @c 0 on success,
 *         otherwise a negative error value
 * @retval #PLAYER_ERROR_NONE Successful
 * @retval #PLAYER_ERROR_INVALID_PARAMETER Invalid parameter
 * @see player_set_resourceconflict_cb()
 */
int player_unset_resourceconflict_cb(player_h player);

/**
 * @brief Display the first video frame at the player_prepare() function to improve the playback performance.
 * @since_tizen 3.0
 * @details This is only availble when you want to start playback from 0 position.
 *	        1. Normal play : player_create() -> player_display_video_at_paused_state() -> player_prepare()[unmute] -> player_start()
 *			2. Resume play : player_create() -> player_prepare() -> player_set_position() -> player_start()[unmute]
 * @param[in] player    The handle to media player
 * @param[in] visible   The visibility of the display (@c true = visible, @c false = non-visible )
 * @pre The player state must be : #PLAYER_STATE_IDLE
*/
int player_display_video_at_paused_state(player_h player, bool visible);

/**
 * @brief Sets the win_ind to player
 * @since_tizen 3.0
 * @param[in] player    The handle to media player
 * @param[in] win_id    windows handle
 * @pre The player state must be : #PLAYER_STATE_IDLE
 */
int player_set_display_parent_win_id(player_h player, int win_id);

#else /* TIZEN_TV */

/**
 * @brief Sets the data source (http or local file) to transite.
 * @since_tizen 3.0
 *
 * @details Player will transite the media source as seamlessly as possible after finishing current playback.
 *          If the function call is successful, the uri will be added at the end of uri list.
 *          You can add several uris and the set of uris will be played repeatedly if you call the player_set_looping().
 *          The first uri in the list is the original uri which is set by player_set_uri().
 *
 * @remarks Uri list will be cleared except first one when the player_unprepare() is called.
 *          If you call the player_set_uri(), you can change the first uri too.
 *
 * @param[in]   player The handle to the media player
 * @param[in]   uri Specifies the data location.
 *
 * @return @c 0 on success,
 *         otherwise a negative error value
 * @retval #PLAYER_ERROR_NONE Successful
 * @retval #PLAYER_ERROR_INVALID_OPERATION Invalid operation
 * @retval #PLAYER_ERROR_INVALID_STATE Invalid player state
 * @retval #PLAYER_ERROR_NOT_SUPPORTD Not supported
 * @pre The original uri must be set first by player_set_uri().
 * @pre The player state must be one of these: #PLAYER_STATE_IDLE, #PLAYER_STATE_READY, #PLAYER_STATE_PLAYING, or #PLAYER_STATE_PAUSED.
 * @see player_set_uri(), player_set_looping()
 */
int player_set_next_uri(player_h player, const char *uri);

/**
 * @brief Gets the data source path.
 * @since_tizen 3.0
 *
 * @param[in]   player The handle to the media player
 * @param[out]  uri Specifies the next data location.
 *
 * @return @c 0 on success,
 *         otherwise a negative error value
 * @retval #PLAYER_ERROR_NONE Successful
 * @retval #PLAYER_ERROR_INVALID_OPERATION Invalid operation
 * @retval #PLAYER_ERROR_INVALID_STATE Invalid player state
 * @retval #PLAYER_ERROR_NOT_SUPPORTD Not supported
 * @pre The player state must be one of these: #PLAYER_STATE_IDLE, #PLAYER_STATE_READY, #PLAYER_STATE_PLAYING, or #PLAYER_STATE_PAUSED.
 * @see player_set_uri()
 */
int player_get_next_uri(player_h player, char **uri);

/**
 * @brief Sets the gapless playback status
 * @since_tizen 3.0
 * @details If it is @c true, player will play next content without(or with minimized) gap.
 *          If it is @c false, it won't. The default value is @c false.
 * @param[in] player   The handle to the media player
 * @param[in] gapless  The new gapless status : (@c true = gapless, @c false = non-gapless )
 * @return @c 0 on success,
 *         otherwise a negative error value
 * @retval #PLAYER_ERROR_NONE Successful
 * @retval #PLAYER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #PLAYER_ERROR_INVALID_STATE Invalid state
 * @pre The player state must be one of these: #PLAYER_STATE_IDLE, #PLAYER_STATE_READY, #PLAYER_STATE_PLAYING, or #PLAYER_STATE_PAUSED.
 * @see	player_is_gapless()
 */
int player_set_gapless(player_h player, bool gapless);

/**
 * @brief Gets the player's gapless status.
 * @since_tizen 3.0
 * @details If it is @c true, player will play next content without(or with minimized) gap.
 *          If it is @c false, it won't. The default value is @c false.
 * @param[in]   player The handle to the media player
 * @param[out]  gapless The gapless status: (@c true = gapless, @c false = non-gapless )
 * @return @c 0 on success,
 *         otherwise a negative error value
 * @retval #PLAYER_ERROR_NONE Successful
 * @retval #PLAYER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #PLAYER_ERROR_INVALID_OPERATION Invalid operation
 * @retval #PLAYER_ERROR_INVALID_STATE Invalid player state
 * @pre The player state must be one of these: #PLAYER_STATE_IDLE, #PLAYER_STATE_READY, #PLAYER_STATE_PLAYING, or #PLAYER_STATE_PAUSED.
 * @see player_set_gapless()
 */
int player_is_gapless(player_h player, bool *gapless);

/**
 * @brief Enables the tbm surface pool.
 * @since_tizen 3.0
 * @details If it is @c true, tbm surface will be enabled to share the video frame.
 *          If it is @c false, it won't. The default value is @c false.
 * @param[in] player  The handle to the media player
 * @param[in] enable  The new tbm surface status
 * @return @c 0 on success,
 *         otherwise a negative error value
 * @retval #PLAYER_ERROR_NONE Successful
 * @retval #PLAYER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #PLAYER_ERROR_INVALID_STATE Invalid state
 * @pre The player state must be #PLAYER_STATE_IDLE.
 * @see	player_is_enabled_tsurf_pool()
 */
int player_enable_tsurf_pool(player_h player, bool enable);

/**
 * @brief Gets the player's tbm surface pool enable status.
 * @since_tizen 3.0
 * @details If it is @c true, tbm surface pool is enabled to share the video frame.
 *          If it is @c false, it won't. The default value is @c false.
 * @param[in]   player The handle to the media player
 * @param[out]  enabled The tbm surface enable status
 * @return @c 0 on success,
 *         otherwise a negative error value
 * @retval #PLAYER_ERROR_NONE Successful
 * @retval #PLAYER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #PLAYER_ERROR_INVALID_OPERATION Invalid operation
 * @retval #PLAYER_ERROR_INVALID_STATE Invalid player state
 * @pre The player state must be one of these: #PLAYER_STATE_IDLE, #PLAYER_STATE_READY, #PLAYER_STATE_PLAYING, or #PLAYER_STATE_PAUSED.
 * @see player_enable_tsurf_pool()
 */
int player_is_enabled_tsurf_pool(player_h player, bool *enabled);

/**
 * @brief Gets the size of video frame pool.
 * @since_tizen 3.0
 * @details App gets the video frame pool size which will be reused during playback.
 * @param[in] player The handle to the media player
 * @param[out] size The size of surface pool
 * @return @c 0 on success,
 *         otherwise a negative error value
 * @retval #PLAYER_ERROR_NONE Successful
 * @retval #PLAYER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #PLAYER_ERROR_INVALID_OPERATION Invalid operation
 * @retval #PLAYER_ERROR_INVALID_STATE Invalid player state
 * @pre The player state must be one of these: #PLAYER_STATE_READY, #PLAYER_STATE_PLAYING, or #PLAYER_STATE_PAUSED
 * @see player_set_media_packet_video_frame_decoded_cb()
 */
int player_get_media_packet_video_frame_pool_size(player_h player, int *size);

#endif /* TIZEN_TV */

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /*__TIZEN_MEDIA_PLAYER_INTERNAL_H__*/

