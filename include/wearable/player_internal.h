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
#include <mm_player.h>
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
typedef struct
{
	void *data;          /**< PCM data */
	int size;            /**< Data Size */
	int channel;         /**< Channel */
	int rate;            /**<  Samplerate */
	int depth;           /**< Depth */
	bool little_endian;  /**< Endianness */
	guint64 channel_mask;	  /**< channel_mask */
} player_audio_raw_data_s;

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
 * @}
 */


#ifdef __cplusplus
}
#endif

#endif //__TIZEN_MEDIA_PLAYER_PRODUCT_H__


