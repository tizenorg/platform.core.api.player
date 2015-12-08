/*
 * Copyright (c) 2011 Samsung Electronics Co., Ltd All Rights Reserved
 *
 * Licensed under the Apache License, Version 2.0 (the License);
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


#ifndef __TIZEN_MEDIA_PLAYER_DOC_H__
#define __TIZEN_MEDIA_PLAYER_DOC_H__


/**
 * @file player_doc.h
 * @brief This file contains high level documentation on the Player API.
 */

/**
 * @ingroup CAPI_MEDIA_FRAMEWORK
 * @defgroup CAPI_MEDIA_PLAYER_MODULE Player
 * @brief The @ref CAPI_MEDIA_PLAYER_MODULE API provides functions for media playback and controlling media playback attributes.
 *
 * @section CAPI_MEDIA_PLAYER_MODULE_HEADER Required Header
 *    \#include <player.h>
 *
 * @section CAPI_MEDIA_PLAYER_OVERVIEW Overview
 * The Player API provides a way to play multimedia content. Content can be played from a file, from the network, or from memory.
 * It gives the ability to start/stop/pause/mute, set the playback position (that is, seek), perform various status queries, and control the display.
 *
 * Additional functions allow registering notifications via callback functions for various state change events.
 *
 * This API also enables collaboration with the GUI service to present a video.
 *
 * @subsection CAPI_MEDIA_PLAYER_LIFE_CYCLE_STATE_DIAGRAM State Diagram
 * Playback of multimedia content is controlled by a state machine.
 * The following diagram shows the life cycle and states of the Player.
 *
 * @image html capi_media_player_state_diagram.png
 *
 * @subsection CAPI_MEDIA_PLAYER_LIFE_CYCLE_STATE_TRANSITIONS State Transitions
 * <div><table class="doxtable" >
 *     <tr>
 *        <th><b>FUNCTION</b></th>
 *        <th><b>PRE-STATE</b></th>
 *        <th><b>POST-STATE</b></th>
 *        <th><b>SYNC TYPE</b></th>
 *     </tr>
 *     <tr>
 *        <td>player_create()</td>
 *        <td>NONE</td>
 *        <td>IDLE</td>
 *        <td>SYNC</td>
 *     </tr>
 *     <tr>
 *         <td>player_destroy()</td>
 *         <td>IDLE</td>
 *         <td>NONE</td>
 *         <td>SYNC</td>
 *     </tr>
 *     <tr>
 *        <td>player_prepare()</td>
 *        <td>IDLE</td>
 *        <td>READY</td>
 *        <td>SYNC</td>
 *     </tr>
 *     <tr>
 *        <td>player_prepare_async()</td>
 *        <td>IDLE</td>
 *        <td>READY</td>
 *        <td>ASYNC</td>
 *     </tr>
 *     <tr>
 *        <td>player_unprepare()</td>
 *        <td>READY, PLAYING or PAUSED</td>
 *        <td>IDLE</td>
 *        <td>SYNC</td>
 *     </tr>
 *     <tr>
 *        <td>player_start()</td>
 *        <td>READY or PAUSED</td>
 *        <td>PLAYING</td>
 *        <td>SYNC</td>
 *     </tr>
 *     <tr>
 *        <td>player_stop()</td>
 *        <td>PLAYING</td>
 *        <td>READY</td>
 *        <td>SYNC</td>
 *     </tr>
 *     <tr>
 *        <td>player_pause()</td>
 *        <td>PLAYING</td>
 *        <td>PAUSED</td>
 *        <td>SYNC</td>
 *     </tr>
 * </table></div>
 *
 * @subsection CAPI_MEDIA_PLAYER_LIFE_CYCLE_STATE_DEPENDENT_FUNCTIONS State Dependent Function Calls
 * The following table shows state-dependent function calls.
 * It is forbidden to call the functions listed below in wrong states.
 * Violation of this rule may result in unpredictable behavior.
 * <div><table class="doxtable" >
 * <tr>
 *    <th><b>FUNCTION</b></th>
 *    <th><b>VALID STATES</b></th>
 *    <th><b>DESCRIPTION</b></th>
 * </tr>
 * <tr>
 *    <td>player_create()</td>
 *    <td>ANY</td>
 *    <td>-</td>
 * </tr>
 * <tr>
 *    <td>player_destroy()</td>
 *    <td>ANY</td>
 *    <td>-</td>
 * </tr>
 * <tr>
 *    <td>player_prepare()</td>
 *    <td>IDLE</td>
 *    <td>This function must be called after player_create()</td>
 * </tr>
 * <tr>
 *    <td>player_unprepare()</td>
 *    <td>READY/PAUSED/PLAYING</td>
 *    <td>This function must be called after player_stop() or player_start() or player_pause()</td>
 * </tr>
 * <tr>
 *    <td>player_start()</td>
 *    <td>READY/ PAUSED</td>
 *    <td>This function must be called after player_prepare()</td>
 * </tr>
 * <tr>
 *    <td>player_stop()</td>
 *    <td>PLAYING/ PAUSED</td>
 *    <td>This function must be called after player_start() or player_pause()</td>
 * </tr>
 * <tr>
 *    <td>player_pause()</td>
 *    <td>PLAYING</td>
 *    <td>This function must be called after player_start()</td>
 * </tr>
 * <tr>
 *    <td>player_set_completed_cb()<BR> player_set_interrupted_cb()<BR> player_set_error_cb()<BR> player_set_buffering_cb()<BR> player_set_subtitle_updated_cb()</td>
 *    <td>IDLE/ READY/ PLAYING/ PAUSED</td>
 *    <td>This function must be called after player_create()</td>
 * </tr>
 * <tr>
 *    <td>player_unset_completed_cb()<BR> player_unset_interrupted_cb()<BR> player_unset_error_cb()<BR> player_unset_buffering_cb()<BR> player_unset_subtitle_updated_cb()</td>
 *    <td>IDLE/ READY/ PLAYING/ PAUSED</td>
 *    <td>This function must be called after register callback functions such as player_set_completed_cb()</td>
 * </tr>
 * <tr>
 *    <td>player_get_state()</td>
 *    <td>ANY</td>
 *    <td>-</td>
 * </tr>
 * <tr>
 *    <td>player_set_uri()</td>
 *    <td>IDLE</td>
 *    <td>This function must be called before player_prepare() </td>
 * </tr>
 * <tr>
 *    <td>player_set_memory_buffer()</td>
 *    <td>IDLE</td>
 *    <td>This function must be called before player_prepare() </td>
 * </tr>
 * <tr>
 *    <td>player_set_subtitle_path()</td>
 *    <td>IDLE</td>
 *    <td>This function must be called before player_prepare() </td>
 * </tr>
 * <tr>
 *    <td>player_set_volume()</td>
 *    <td>IDLE/ READY/ PLAYING/ PAUSED</td>
 *    <td>This function must be called after player_create()</td>
 * </tr>
 * <tr>
 *    <td>player_get_volume()</td>
 *    <td>IDLE/ READY/ PLAYING/ PAUSED</td>
 *    <td>This function must be called after player_create()</td>
 * </tr>
 * <tr>
 *    <td>player_set_audio_policy_info()</td>
 *    <td>IDLE</td>
 *    <td>This function must be called after player_create()</td>
 * </tr>
 * <tr>
 *    <td>player_set_sound_type()</td>
 *    <td>IDLE</td>
 *    <td>This function must be called after player_create()</td>
 * </tr>
 * <tr>
 *    <td>player_set_mute()</td>
 *    <td>IDLE/ READY/ PLAYING/ PAUSED</td>
 *    <td>This function must be called after player_create()</td>
 * </tr>
 * <tr>
 *    <td>player_is_muted()</td>
 *    <td>IDLE/ READY/ PLAYING/ PAUSED</td>
 *    <td>This function must be called after player_create()</td>
 * </tr>
 * <tr>
 *    <td>player_set_looping()</td>
 *    <td>IDLE/ READY/ PLAYING/ PAUSED</td>
 *    <td>This function must be called after player_create()</td>
 * </tr>
 * <tr>
 *    <td>player_is_looping()</td>
 *    <td>IDLE/ READY/ PLAYING/ PAUSED</td>
 *    <td>This function must be called after player_create()</td>
 * </tr>
 * <tr>
 *    <td>player_get_duration()</td>
 *    <td>PLAYING/ PAUSED</td>
 *    <td>This function must be called after player_start()</td>
 * </tr>
 * <tr>
 *    <td>player_set_display()</td>
 *    <td>IDLE</td>
 *    <td>This function must be called before player_prepare()</td>
 * </tr>
 * <tr>
 *    <td>player_set_display_mode() <BR> player_set_display_visible()</td>
 *    <td>IDLE/ READY/ PLAYING/ PAUSED</td>
 *    <td>This function must be called after player_create()</td>
 * </tr>
  * <tr>
 *    <td>player_get_display_rotation() <BR> player_is_display_visible()</td>
 *    <td>IDLE/ READY/ PLAYING/ PAUSED</td>
 *    <td>This function must be called after player_create()</td>
 * </tr>
 * <tr>
 *    <td>player_get_video_size()</td>
 *    <td>READY/ PLAYING/ PAUSED</td>
 *    <td>This function must be called after player_prepare()</td>
 * </tr>
 * </table></div>
 *
 * @subsection CAPI_MEDIA_PLAYER_LIFE_CYCLE_ASYNCHRONOUS_OPERATIONS Asynchronous Operations
 * All functions that change the player state are synchronous except player_prepare_async(), player_set_play_position(), and player_capture_video().
 * Thus the result is passed to the application via the callback mechanism.
 *
 * @subsection CAPI_MEDIA_PLAYER_LIFE_CYCLE_CALLBACK_OPERATIONS Callback(Event) Operations
 * <div><table class="doxtable" >
 *     <tr>
 *        <th><b>REGISTER</b></th>
 *        <th><b>UNREGISTER</b></th>
 *        <th><b>CALLBACK</b></th>
 *        <th><b>DESCRIPTION</b></th>
 *     </tr>
 *		 <tr>
 *        <td>player_set_completed_cb()</td>
 *        <td>player_unset_completed_cb()</td>
 *        <td>player_completed_cb()</td>
 *        <td>called when playback is completed </td>
 *     </tr>
 *		 <tr>
 *        <td>player_set_interrupted_cb()</td>
 *        <td>player_unset_interrupted_cb()</td>
 *        <td>player_interrupted_cb()</td>
 *        <td>called when playback is interrupted by #player_interrupted_code_e </td>
 *     </tr>
 *		 <tr>
 *        <td>player_set_error_cb()</td>
 *        <td>player_unset_error_cb()</td>
 *        <td>player_error_cb()</td>
 *        <td>called when an error has occurred</td>
 *     </tr>
 *		 <tr>
 *        <td>player_set_buffering_cb()</td>
 *        <td>player_unset_buffering_cb()</td>
 *        <td>player_buffering_cb()</td>
 *        <td>called during content buffering </td>
 *     </tr>
 *		 <tr>
 *        <td>player_set_progressive_download_message_cb()</td>
 *        <td>player_unset_progressive_download_message_cb()</td>
 *        <td>player_pd_message_cb()</td>
 *        <td>called when a progressive download starts or completes</td>
 *     </tr>
 *		 <tr>
 *        <td>player_set_subtitle_updated_cb()</td>
 *        <td>player_unset_subtitle_updated_cb()</td>
 *        <td>player_subtitle_updated_cb()</td>
 *        <td>called when a subtitle updates </td>
 *     </tr>
 *		 <tr>
 *        <td>player_set_media_packet_video_frame_decoded_cb()</td>
 *        <td>player_unset_media_packet_video_frame_decoded_cb()</td>
 *        <td>player_media_packet_video_decoded_cb()</td>
 *        <td>called when a video frame is decoded </td>
 *     </tr>

 *</table></div>
 *
 * @if WEARABLE
 * @section CAPI_MEDIA_PLAYER_MODULE_FEATURE Related Features
 * This API is related with the following features:
 *  - http://tizen.org/feature/network.internet
 *
 * It is recommended to design feature related codes in your application for reliability.
 * You can check if a device supports the related features for this API by using @ref CAPI_SYSTEM_SYSTEM_INFO_MODULE, thereby controlling the procedure of your application.
 * To ensure your application is only running on the device with specific features, please define the features in your manifest file using the manifest editor in the SDK.
 * More details on featuring your application can be found from <a href="../org.tizen.mobile.native.appprogramming/html/ide_sdk_tools/feature_element.htm"><b>Feature Element</b>.</a>
 * @endif
 */


/**
 * @ingroup CAPI_MEDIA_PLAYER_MODULE
 * @defgroup CAPI_MEDIA_PLAYER_DISPLAY_MODULE Display
 * @brief The @ref CAPI_MEDIA_PLAYER_DISPLAY_MODULE API provides functions to control the display.
 * @section CAPI_MEDIA_PLAYER_DISPLAY_MODULE_HEADER Required Header
 *    \#include <player.h>
 *
 * @section CAPI_MEDIA_PLAYER_DISPLAY_MODULE_OVERVIEW Overview
 * The API allows you to manage the display of the player.
 * This API provides functions to set and get various display properties:
 * - mode
 * - rotation
 * - visibility
 *
*/

/**
 * @ingroup CAPI_MEDIA_PLAYER_MODULE
 * @defgroup CAPI_MEDIA_PLAYER_STREAM_INFO_MODULE Stream Information
 * @brief The @ref CAPI_MEDIA_PLAYER_STREAM_INFO_MODULE API provides functions to get audio and video stream information, such as codec type, video width or height, bit rate, and so on.
 * @section CAPI_MEDIA_PLAYER_AUDIO_EFFECT_MODULE_HEADER Required Header
 *    \#include <player.h>
 *
 * @section CAPI_MEDIA_PLAYER_STREAM_INFO_MODULE_OVERVIEW Overview
 * The Player stream information API allows you to get media stream information, including:
 * - Content metadata, such as the tile, artist, album title and genre.
 * - Audio stream information, such as audio codec type, sample rate, channels, and bit rate.
 * - Video stream information, such as video codec type, video width and height.
 *
*/

/**
 * @ingroup CAPI_MEDIA_PLAYER_MODULE
 * @defgroup CAPI_MEDIA_PLAYER_AUDIO_EFFECT_MODULE Audio Effect
 * @brief The @ref CAPI_MEDIA_PLAYER_AUDIO_EFFECT_MODULE API provides functions to control the audio effect.
 * @section CAPI_MEDIA_PLAYER_AUDIO_EFFECT_MODULE_HEADER Required Header
 *    \#include <player.h>
 *
 * @section CAPI_MEDIA_PLAYER_AUDIO_EFFECT_MODULE_MODULE_OVERVIEW Overview
 * The Audio effect API allows you to apply effects to the player:
 * - Equalizer
 *
 *
*/

/**
 * @ingroup CAPI_MEDIA_PLAYER_MODULE
 * @defgroup CAPI_MEDIA_PLAYER_SUBTITLE_MODULE Subtitle
 * @brief The @ref CAPI_MEDIA_PLAYER_SUBTITLE_MODULE API provides functions to control the subtitle.
 * @section CAPI_MEDIA_PLAYER_SUBTITLE_MODULE_HEADER Required Header
 *    \#include <player.h>
 *
 *
*/
/**
*		<div><table class="doxtable">
*				<tr>
*					<th><b>FUNCTION</b></th>
*					<th><b>ATTRIBUTE</b></th>
*					<th><b>PRECONDITION</b></th>
*					<th><b>RELEASE-STATE</b></th>
*				</tr>
*				<tr>
*					<td>player_set_uri</td>
*					<td>"profile_uri"</td>
*					<td>create</td>
*					<td>destroy</td>
*				</tr>
*				<tr>
*					<td>player_set_memory_buffer</td>
*					<td>"profile_uri"</td>
*					<td>create</td>
*					<td>destroy</td>
*				</tr>
*				<tr>
*					<td>player_set_volume
*						<br>player_get_volume
*					</td>
*					<td>player-&gt;sound.volume</td>
*					<td>-
*					</td>
*					<td>-
*					</td>
*				</tr>
*				<tr>
*					<td>player_set_sound_type</td>
*					<td>"sound_volume_type"</td>
*					<td>create</td>
*					<td>destroy</td>
*				</tr>
*				<tr>
*					<td>player_set_audio_policy_info</td>
*					<td>"sound_stream_type"</td>
*					<td>create</td>
*					<td>destroy</td>
*				</tr>
*				<tr>
*					<td>player_set_audio_latency_mode
*						<br>player_get_audio_latency_mode
*					</td>
*					<td>"sound_latency_mode"</td>
*					<td>-</td>
*					<td>-</td>
*				</tr>
*				<tr>
*					<td>player_set_mute
*						<br>player_is_muted
*					</td>
*					<td>player-&gt;sound.mute</td>
*					<td>create
*					</td>
*					<td>destroy
*					</td>
*				</tr>
*				<tr>
*					<td>player_set_looping
*						<br>player_is_looping
*					</td>
*					<td>"profile_play_count"</td>
*					<td>create
*					</td>
*					<td>destroy
*					</td>
*				</tr>
*				<tr>
*					<td>player_set_display</td>
*					<td>"display_surface_type"
*						<br>"display_overlay"(or wl_display etc.)
*					</td>
*					<td>create</td>
*					<td>destroy</td>
*				</tr>
*				<tr>
*					<td>player_set_media_stream_info
*					</td>
*					<td>player-&gt;v_stream_caps
*						<br>player-&gt;a_stream_caps
*					</td>
*					<td>create
*					</td>
*					<td>destroy
*					</td>
*				</tr>
*				<tr>
*					<td>player_set_media_stream_buffer_max_size
*						<br>player_get_media_stream_buffer_max_size
*					</td>
*					<td>player-&gt;media_stream_buffer_max_size[type]</td>
*					<td>create
*					</td>
*					<td>destroy
*					</td>
*				</tr>
*				<tr>
*					<td>player_set_media_stream_buffer_min_threshold
*						<br>player_get_media_stream_buffer_min_threshold
*					</td>
*					<td>player-&gt;media_stream_buffer_min_percent[type]</td>
*					<td>create</td>
*					<td>destroy</td>
*				</tr>
*				<tr>
*					<td>player_set_display_mode
*						<br>player_get_display_mode
*					</td>
*					<td>"display_method"</td>
*					<td>-</td>
*					<td>-</td>
*				</tr>
*				<tr>
*					<td>player_set_display_visible
*						<br>player_is_display_visible
*					</td>
*					<td>"display_visible"</td>
*					<td>-</td>
*					<td>-</td>
*				</tr>
*				<tr>
*					<td>player_set_display_rotation
*						<br>player_get_display_rotation
*					</td>
*					<td>"display_rotation"</td>
*					<td>-</td>
*					<td>-</td>
*				</tr>
*				<tr>
*					<td>player_audio_effect_set_equalizer_band_level
*						<br>player_audio_effect_get_equalizer_band_level
*					</td>
*					<td>player-&gt;audio_effect_info.custom_eq_level[type]
*						<br>player-&gt;audio_effect_info.custom_ext_level[type-1]
*					</td>
*					<td>create
*					</td>
*					<td>destroy
*					</td>
*				</tr>
*				<tr>
*					<td>player_audio_effect_set_equalizer_all_bands</td>
*					<td>player-&gt;audio_effect_info.custom_eq_level[min:length]</td>
*					<td>create
*					</td>
*					<td>destroy
*					</td>
*				</tr>
*				<tr>
*					<td>player_set_streaming_cookie</td>
*					<td>"streaming_cookie"</td>
*					<td>create</td>
*					<td>destroy</td>
*				</tr>
*				<tr>
*					<td>player_set_streaming_user_agent</td>
*					<td>"streaming_user_agent"</td>
*					<td>create</td>
*					<td>destroy</td>
*				</tr>
*				<tr>
*					<td>player_set_progressive_download_path</td>
*					<td>"pd_mode"
*						<br>"pd_location"
*					</td>
*					<td>create</td>
*					<td>destroy</td>
*				</tr>
*				<tr>
*					<td>player_set_subtitle_path</td>
*					<td>"subtitle_uri"</td>
*					<td>create</td>
*					<td>destroy
*					</td>
*				</tr>
*				<tr>
*					<td>player_set_play_position
*						<br>player_get_play_position
*					</td>
*					<td>"accurate_seek"
*						<br>position
*					</td>
*					<td>prepare</td>
*					<td>(temporary)</td>
*				</tr>
*				<tr>
*					<td>player_get_content_info</td>
*					<td>"tag_album"
*						<br>"tag_artist"
*						<br>"tag_author"
*						<br>"tag_genre"
*						<br>"tag_title"
*						<br>"tag_date"
*					</td>
*					<td>prepare</td>
*					<td>unprepare</td>
*				</tr>
*				<tr>
*					<td>player_get_codec_info</td>
*					<td>"content_audio_codec"
*						<br>"content_video_codec"&nbsp;
*					</td>
*					<td>prepare</td>
*					<td>unprepare</td>
*				</tr>
*				<tr>
*					<td>player_get_audio_stream_info</td>
*					<td>"content_audio_samplerate"
*						<br>"content_audio_channels"
*						<br>"content_audio_bitrate"&nbsp;
*					</td>
*					<td>prepare</td>
*					<td>unprepare</td>
*				</tr>
*				<tr>
*					<td>player_get_video_stream_info</td>
*					<td>"content_video_fps"
*						<br>"content_video_bitrate"&nbsp;
*					</td>
*					<td>prepare</td>
*					<td>unprepare</td>
*				</tr>
*				<tr>
*					<td>player_get_video_size</td>
*					<td>"content_video_width"
*						<br>"content_video_height"&nbsp;
*					</td>
*					<td>prepare</td>
*					<td>unprepare</td>
*				</tr>
*				<tr>
*					<td>player_get_album_art</td>
*					<td>"tag_album_cover"</td>
*					<td>prepare</td>
*					<td>unprepare</td>
*				</tr>
*				<tr>
*					<td>player_get_duration</td>
*					<td>"content_duration"</td>
*					<td>prepare</td>
*					<td>unprepare</td>
*				</tr>
*				<tr>
*					<td>player_set_playback_rate</td>
*					<td>player-&gt;playback_rate</td>
*					<td>prepare</td>
*					<td>unprepare</td>
*				</tr>
*				<tr>
*					<td>player_set_subtitle_position_offset</td>
*					<td>player-&gt;adjust_subtitle_pos</td>
*					<td>play</td>
*					<td>(temporary)
*					</td>
*				</tr>
*				<tr>
*					<td>player_select_track</td>
*					<td>"current-language"</td>
*					<td>prepare</td>
*					<td>unprepare</td>
*				</tr>
*	</table></div>
**/
#endif /* __TIZEN_MEDIA_PLAYER_DOC_H__ */
