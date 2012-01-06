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

#ifndef __TIZEN_MEDIA_PLAYER_H__
#define __TIZEN_MEDIA_PLAYER_H__

#include <tizen.h>
#include <sound_manager.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PLAYER_ERROR_CLASS          TIZEN_ERROR_MULTIMEDIA_CLASS | 0x20

/**
 * @file player.h
 * @brief This file contains the media player API.
 */

/**
 * @addtogroup CAPI_MEDIA_PLAYER_MODULE
 * @{
 */

/**
 * @brief Media player handle type.
 */
typedef struct player_s *player_h;

/**
 * @brief Enumerations of media player state
 */
typedef enum
{
	PLAYER_STATE_NONE,			/**< Player is not created */
	PLAYER_STATE_IDLE,				/**< Player is created, but not prepared */
	PLAYER_STATE_READY,			/**< Player is ready to play media */
	PLAYER_STATE_PLAYING,		/**< Player is playing media */
	PLAYER_STATE_PAUSED,		/**< Player is paused while playing media */
} player_state_e;



/**
 * @brief Error codes for media player
 */
typedef enum
{
		PLAYER_ERROR_NONE		        = TIZEN_ERROR_NONE,				           						 /**< Successful */
		PLAYER_ERROR_OUT_OF_MEMORY	    = TIZEN_ERROR_OUT_OF_MEMORY,			    /**< Out of memory */
		PLAYER_ERROR_INVALID_PARAMETER  = TIZEN_ERROR_INVALID_PARAMETER,		/**< Invalid parameter */
		PLAYER_ERROR_NO_SUCH_FILE	    = TIZEN_ERROR_NO_SUCH_FILE,			    		/**< No such file or directory */
		PLAYER_ERROR_INVALID_OPERATION	= TIZEN_ERROR_INVALID_OPERATION,		/**< Invalid operation */
		PLAYER_ERROR_SEEK_FAILED	    = PLAYER_ERROR_CLASS | 0x01	,		    		/**< Seek operation failure */
		PLAYER_ERROR_INVALID_STATE	    = PLAYER_ERROR_CLASS | 0x02	,		    		/**< Invalid state */
		PLAYER_ERROR_NOT_SUPPORTED_FILE	= PLAYER_ERROR_CLASS | 0x03	,   		/**< Not supported file format */
		PLAYER_ERROR_INVALID_URI	    = PLAYER_ERROR_CLASS | 0x04	,		    			/**< Invalid URI */
		PLAYER_ERROR_SOUND_POLICY	    = PLAYER_ERROR_CLASS | 0x05	,		    		/**< Sound policy error */
		PLAYER_ERROR_CONNECTION_FAILED	= PLAYER_ERROR_CLASS | 0x06    /**< Streaming connection failed */
} player_error_e;

/**
 * @brief Enumerations of player interrupted type
 */
typedef enum
{
       PLAYER_INTERRUPTED_BY_OTHER_APP = 0, 				/**< Interrupted by another application*/
       PLAYER_INTERRUPTED_BY_CALL_START,						/**< Interrupted by incoming call*/
       PLAYER_INTERRUPTED_BY_CALL_END,							/**< Interrupted by call ending*/
       PLAYER_INTERRUPTED_BY_EARJACK_UNPLUG,			/**< Interrupted by unplugging headphone*/
       PLAYER_INTERRUPTED_BY_RESOURCE_CONFLICT,		/**< Interrupted by resource conflict*/
       PLAYER_INTERRUPTED_BY_ALARM_START,					/**< Interrupted by alarm starting*/
       PLAYER_INTERRUPTED_BY_ALARM_END,						/**< Interrupted by alarm ending*/
} player_interrupted_code_e;

/**
 * @brief
 * Enumerations of display type
 */ 
typedef enum
{
  PLAYER_DISPLAY_TYPE_X11 = 0,			/**< X surface display */
  PLAYER_DISPLAY_TYPE_EVAS = 1,		/**< Evas image object surface display */
  PLAYER_DISPLAY_TYPE_NONE = 3		/**< This just disposes of buffers */
} player_display_type_e;

/**
 * @brief Player display handle
 *
 */
typedef void* player_display_h;

#ifndef GET_DISPLAY
/**
 * @brief Gets a display handle from x window id or evas object
 */
#define GET_DISPLAY(x) (void*)(x)
#endif

/**
 * @}
 */

/**
 * @addtogroup CAPI_MEDIA_PLAYER_X11_DISPLAY_MODULE
 * @{
 */

/**
 * @brief Enumerations of x surface display rotation type.
 */
typedef enum
{
	PLAYER_DISPLAY_ROTATION_NONE,	/**< Display is not rotated */
	PLAYER_DISPLAY_ROTATION_90,		/**< Display is rotated 90 degrees */
	PLAYER_DISPLAY_ROTATION_180,	/**< Display is rotated 180 degrees */
	PLAYER_DISPLAY_ROTATION_270,	/**< Display is rotated 270 degrees */
	PLAYER_DISPLAY_ROTATION_FLIP_HORZ,	/**< flip display horizontally */
	PLAYER_DISPLAY_ROTATION_FLIP_VERT,	/**< flip display vertically */
} player_display_rotation_e;

/**
 * @brief  Enumerations of x surface display aspect ratio
 */ 
typedef enum
{
	PLAYER_DISPLAY_MODE_LETTER_BOX = 0,	/**< Letter box*/
	PLAYER_DISPLAY_MODE_ORIGIN_SIZE,		/**< Origin size*/
	PLAYER_DISPLAY_MODE_FULL_SCREEN,		/**< full-screen*/
	PLAYER_DISPLAY_MODE_CROPPED_FULL,	/**< Cropped full-screen*/
} player_display_mode_e;


/**
 * @}
 */

/**
 * @addtogroup CAPI_MEDIA_PLAYER_MODULE
 * @{
 */

/**
 * @brief  Called when the media player is started.
 * @param[in]   user_data  The user data passed from the callback registration function
 * @pre player_start() will cause this callback if you register this callback using player_set_started_cb()
 * @see player_start()
 * @see player_set_started_cb()
 * @see player_unset_started_cb()
 */
typedef void (*player_started_cb)(void *user_data);

/**
 * @brief  Called when the media player is completed.
 * @param[in]   user_data  The user data passed from the callback registration function
 * @pre It will be invoked when playback completed if you register this callback using player_set_completed_cb()
 * @see player_set_completed_cb()
 * @see player_unset_completed_cb()
 */
typedef void (*player_completed_cb)(void *user_data);


/**
 * @brief  Called when the media player is paused.
 * @param[in]   user_data  The user data passed from the callback registration function
 * @pre player_pause() will cause this callback if you register this callback using player_set_paused_cb()
 * @see player_pause()
 * @see player_set_paused_cb()
 * @see player_unset_paused_cb()
 */
typedef void (*player_paused_cb)(void *user_data);

/**
 * @brief  Called when the media player is interrupted.
 * @param[in]	error_code	The interrupted error code
 * @param[in]	user_data	The user data passed from the callback registration function
 * @see player_set_interrupted_cb()
 * @see player_unset_interrupted_cb()
 */
typedef void (*player_interrupted_cb)(player_interrupted_code_e code, void *user_data);

/**
 * @brief  Called when the media player occured error.
 * @param[in]	error_code  Error code
 * @param[in]	user_data	The user data passed from the callback registration function
 * @see player_set_error_cb()
 * @see player_unset_error_cb()
 * @see #player_error_e
 */
typedef void (*player_error_cb)(int error_code, void *user_data);

/**
 * @brief  Called when the buffering percentage of media playback is updated.
 * @details If the buffer is full, it will return 100%. 
 * @param[in]   percent	The percentage of buffering completed (0~100)
 * @param[in]   user_data	The user data passed from the callback registration function
 * @see player_set_buffering_cb()
 * @see player_unset_buffering_cb()
 */
typedef void (*player_buffering_cb)(int percent, void *user_data);


/**
 * @brief  Called when the subtitle is updated.
 * @param[in]   duration	The duration of subtitle updated
 * @param[in]   text	The text of subtitle updated
 * @param[in]   user_data	The user data passed from the callback registration function
 * @see player_set_subtitle_updated_cb()
 * @see player_unset_subtitle_updated_cb()
 */
typedef void (*player_subtitle_updated_cb)(unsigned long duration, char *text, void *user_data);

/**
 * @brief  Called when the video is captured.
 * @remarks The color space format of the captured image is #IMAGE_UTIL_COLORSPACE_RGB888.
 * @remarks @a data must be released with @c free() by you.
 * @param[in]   data	The captured image buffer 
 * @param[in]   width	The width of captured image
 * @param[in]   height The height of captured image
 * @param[in]   size	The size of captured image
 * @param[in]   user_data	The user data passed from the callback registration function
 * @see player_capture_video()
 */
typedef void (*player_video_captured_cb)(unsigned char *data, int width, int height, unsigned int size, void *user_data);

/**
 * @brief Creates a player handle for playing multimedia content.
 * @remarks @a player must be released player_destroy() by you.
 * @remarks  
 *  Although you can create multiple player handles at the same time,
 *  the player cannot guarantee proper operation because of limited resources, such as
 *  audio or display device.
 *
 * @param[out]  player  A new handle to media player
 * @retval #PLAYER_ERROR_NONE Successful
 * @retval #PLAYER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #PLAYER_ERROR_OUT_OF_MEMORY Out of memory
 * @retval #PLAYER_ERROR_INVALID_OPERATION Invalid operation
 * @post The player state will be #PLAYER_STATE_IDLE.
 * @see player_destroy()
 */
int player_create(player_h *player);


/**
 * @brief Destroys the media player handle and releases all its resources.
 *
 * @remarks To completely shutdown player operation, call this function with a valid player handle from any player state.
 *
 * @param[in]		player The handle to media player to be destroyed
 * @return 0 on success, otherwise a negative error value.
 * @retval #PLAYER_ERROR_NONE Successful
 * @retval #PLAYER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #PLAYER_ERROR_INVALID_OPERATION Invalid operation
 * @pre  The player state should be #PLAYER_STATE_IDLE
 * @post The player state will be #PLAYER_STATE_NONE.
 * @see player_create()
 */
int player_destroy(player_h player);


/**
 * @brief Prepares the media player for playback.
 * @param[in]	player The handle to media player
 * @return 0 on success, otherwise a negative error value.
 * @retval #PLAYER_ERROR_NONE Successful
 * @retval #PLAYER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #PLAYER_ERROR_INVALID_URI Invalid URI
 * @retval #PLAYER_ERROR_NO_SUCH_FILE File not found
 * @retval #PLAYER_ERROR_NOT_SUPPORTED_FILE Not supported file
 * @retval #PLAYER_ERROR_INVALID_OPERATION Invalid operation
 * @retval #PLAYER_ERROR_INVALID_STATE Invalid player state
 * @pre	The player state should be #PLAYER_STATE_IDLE by player_create() or player_unprepare().. After that, call player_set_uri() to load the media content you want to play.
 * @post The player state will be #PLAYER_STATE_READY.
 * @see player_unprepare()
 * @see player_set_uri()
 */
int player_prepare(player_h player);

/**
 * @brief Reset the media player.
 * @details
 *	The most recently used media is reset and no longer associated with the player.
 * Playback is no longer possible. If you want to use the player again, you will have to set the data URI and call
 * player_prepare() again. 
 * @param[in]		player The handle to media player
 * @return 0 on success, otherwise a negative error value.
 * @retval #PLAYER_ERROR_NONE Successful
 * @retval #PLAYER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #PLAYER_ERROR_INVALID_OPERATION Invalid operation
 * @retval #PLAYER_ERROR_INVALID_STATE Invalid player state
 * @pre	The player state should be #PLAYER_STATE_READY by player_prepare() or player_stop(). 
 * @post The player state will be #PLAYER_STATE_IDLE.
 * @see player_prepare()
 */
int player_unprepare(player_h player);

/**
 * @brief Sets the data source (file-path, http or rtsp URI) to use.
 *
 * @details
 *	Associates media contents, referred to by the URI, with the player. 
 * If the function call is successful, subsequent calls to player_prepare() and player_start() will start playing the media.
 *
 * @remarks 
 * If you use http or rtsp, URI should start with "http://" or "rtsp://". The default protocol is "file://".
 * If you provide an invalid URI, you won't receive an error message until you call player_start().
 *
 * @param[in]   player The handle to media player
 * @param[in]   uri Specifies the content location, such as the file path, the URI of the http or rtsp stream you want to play  
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #PLAYER_ERROR_NONE Successful
 * @retval #PLAYER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #PLAYER_ERROR_INVALID_OPERATION Invalid operation
 * @retval #PLAYER_ERROR_INVALID_STATE Invalid player state
 * @pre The player state should be #PLAYER_STATE_IDLE by player_prepare() or player_unprepare().
 * @see player_set_memory_buffer()
 */
int player_set_uri(player_h player, const char * uri);

/**
 * @brief Sets memory as the data source.
 *
 * @details
 * Associates media content, cached in memory, with the player. Unlike the case of player_set_uri(), the media resides in memory.
 * If the function call is successful, subsequent calls to player_prepare() and player_start() will start playing the media.
 *
 * @remarks 
 * If you provide an invalid data, you won't receive an error message until you call player_start().
 *
 * 
 * @param[in]   player The handle to media player  
 * @param[in]   data The memory pointer of media data
 * @param[in]   size The size of media data
 * @return 0 on success, otherwise a negative error value.
 * @retval #PLAYER_ERROR_NONE Successful
 * @retval #PLAYER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #PLAYER_ERROR_INVALID_OPERATION Invalid operation
 * @retval #PLAYER_ERROR_INVALID_STATE Invalid player state
 * @pre	The player state should be #PLAYER_STATE_IDLE by player_prepare() or player_unprepare().
 * @see player_set_uri()
 */
int player_set_memory_buffer(player_h player, const void * data, int size);

/**
 * @brief Gets the player's current state.
 * @param[in]   player	The handle to media player
 * @param[out]  state	The current state of the player
 * @return 0 on success, otherwise a negative error value.
 * @retval #PLAYER_ERROR_NONE Successful
 * @retval #PLAYER_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #player_state_e
 */
int  player_get_state(player_h player, player_state_e *state);


/**
 * @brief Sets the player's volume.
 *
 * @details
 *	The range of @a left and @c right is from 0 to 1.0, inclusive (1.0 = 100%). Default value is 1.0.
 *	Setting this volume adjusts the player volume, not the system volume. 
 * To change system volume, use the @ref CAPI_MEDIA_SOUND_MANAGER_MODULE API.
 *
 * @param[in]   player The handle to media player
 * @param[in]   left Left volume scalar
 * @param[in]   right Right volume scalar
 * @return 0 on success, otherwise a negative error value.
 * @retval #PLAYER_ERROR_NONE Successful
 * @retval #PLAYER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #PLAYER_ERROR_INVALID_OPERATION Invalid operation
 * @see player_get_volume()
 */
int player_set_volume(player_h player, float left, float right);


/**
 * @brief Gets the player's current volume factor.
 *
 * @details The range of @a left and @a right is from 0 to 1.0, inclusive (1.0 = 100%). This function gets the player volume, not the system volume. 
 * To get the system volume, use the @ref CAPI_MEDIA_SOUND_MANAGER_MODULE API.
 *
 * @param[in]   player The handle to media player
 * @param[out]  left The current left volume scalar
 * @param[out]  right The current right volume scalar
 * @return 0 on success, otherwise a negative error value.
 * @retval #PLAYER_ERROR_NONE Successful
 * @retval #PLAYER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #PLAYER_ERROR_INVALID_OPERATION Invalid operation
 * @see player_set_volume()
 */
int player_get_volume(player_h player, float *left, float *right);

/**
 * @brief Sets the player's sound type.
 *
 * @remarks
 *	The default sound type of player is #SOUND_TYPE_MEDIA.
 * To get current sound type, use the sound_manager_get_current_sound_type().
 *
 * @param[in]   player The handle to media player
 * @param[in]   type The sound type
 * @return 0 on success, otherwise a negative error value.
 * @retval #PLAYER_ERROR_NONE Successful
 * @retval #PLAYER_ERROR_INVALID_PARAMETER Invalid parameter
 * @see sound_manager_get_current_sound_type()
 */
int player_set_sound_type(player_h player, sound_type_e type);

/**
 * @brief Starts or resumes playback, asynchronously.
 *
 * @details Plays current media content, or resumes play if paused.
 *
 * @param[in]   player The handle to media player
 * @return 0 on success, otherwise a negative error value.
 * @retval #PLAYER_ERROR_NONE Successful
 * @retval #PLAYER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #PLAYER_ERROR_INVALID_OPERATION Invalid operation
 * @retval #PLAYER_ERROR_INVALID_STATE Invalid player state
 * @retval #PLAYER_ERROR_CONNECTION_FAILED Network connection failed
 * @retval #PLAYER_ERROR_SOUND_POLICY Sound policy error
 * @pre Call player_prepare() before calling this function.
 * @pre The player state must be #PLAYER_STATE_READY by player_prepare() or #PLAYER_STATE_PAUSED by player_pause().
 * @post The player state will be #PLAYER_STATE_PLAYING.
 * @post It invokes player_started_cb() when playback starts, if you set a callback with player_set_started_cb().
 * @post It invokes player_completed_cb() when playback completes, if you set a callback with player_set_completed_cb().
 * @see player_prepare()
 * @see player_stop()
 * @see player_pause()
 * @see player_set_started_cb()
 * @see player_started_cb()
 * @see player_set_completed_cb()
 * @see player_completed_cb()
 */
int player_start(player_h player);


/**
 * @brief Stops playing media content.
 * @param[in]   player The handle to media player
 * @return 0 on success, otherwise a negative error value.
 * @retval #PLAYER_ERROR_NONE Successful
 * @retval #PLAYER_ERROR_INVALID_PARAMETER Invalid state
 * @retval #PLAYER_ERROR_INVALID_OPERATION Invalid operation
 * @retval #PLAYER_ERROR_INVALID_STATE Invalid player state
 * @retval #PLAYER_ERROR_SOUND_POLICY Sound policy error
 * @pre The player state must be either #PLAYER_STATE_PLAYING by player_start() or #PLAYER_STATE_PAUSED by player_pause().
 * @post The player state will be #PLAYER_STATE_READY.
 * @see player_start()
 * @see player_pause()
 */
int player_stop(player_h player);

/**
 * @brief Pauses the player, asynchronously.
 *
 * @remarks	Playback can be resumed with player_start().
 *
 * @param[in]   player The handle to media player
 * @return 0 on success, otherwise a negative error value.
 * @retval #PLAYER_ERROR_NONE Successful
 * @retval #PLAYER_ERROR_INVALID_PARAMETER Invalid state
 * @retval #PLAYER_ERROR_INVALID_OPERATION Invalid operation
 * @retval #PLAYER_ERROR_INVALID_STATE Invalid player state
 * @retval #PLAYER_ERROR_SOUND_POLICY Sound policy error
 * @pre The player state must be #PLAYER_STATE_PLAYING.
 * @post The player state will be #PLAYER_STATE_READY.
 * @post It invokes player_paused_cb() when playback is pauses, if you set a callback with player_set_paused_cb().
 * @see player_start()
 * @see player_set_paused_cb()
 * @see player_unset_paused_cb()
 * @see player_paused_cb()
 */
int player_pause(player_h player);

/**
 * @brief Sets the seek position for playback.
 * @param[in]   player The handle to media player
 * @param[in]   millisecond The position in milliseconds from the start to seek to 
 * @return 0 on success, otherwise a negative error value.
 * @retval #PLAYER_ERROR_NONE Successful
 * @retval #PLAYER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #PLAYER_ERROR_INVALID_OPERATION Invalid operation
 * @retval #PLAYER_ERROR_SEEK_FAILED Seek operation failure
 * @pre The player state must be one of these: #PLAYER_STATE_READY, #PLAYER_STATE_PLAYING, or #PLAYER_STATE_PAUSED.
 * @see player_get_position()
 * @see player_get_position_ratio()
 * @see player_set_position_ratio()
 */
int player_set_position(player_h player, int millisecond);


/**
 * @brief Sets the playback position specified by percent of media content played.
 * @param[in]   player The handle to media player
 * @param[in]   percent The position in percentage from the start to seek to. \n The position is relative to content. (length, 0 = beginning, 100 = end)
 * @return 0 on success, otherwise a negative error value.
 * @retval #PLAYER_ERROR_NONE Successful
 * @retval #PLAYER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #PLAYER_ERROR_INVALID_OPERATION Invalid operation
 * @retval #PLAYER_ERROR_SEEK_FAILED Seek operation failure
 * @pre The player state must be one of these: #PLAYER_STATE_READY, #PLAYER_STATE_PLAYING, or #PLAYER_STATE_PAUSED.
 * @see player_get_position()
 * @see player_get_position_ratio()
 * @see player_set_position()
 */
int player_set_position_ratio(player_h player, int percent);

/**
 * @brief Gets current position in milliseconds. 
 * @param[in]   player The handle to media player
 * @param[out]  millisecond The current position in milliseconds
 * @return 0 on success, otherwise a negative error value.
 * @retval #PLAYER_ERROR_NONE Successful
 * @retval #PLAYER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #PLAYER_ERROR_INVALID_OPERATION Invalid operation
 * @retval #PLAYER_ERROR_SEEK_FAILED Seek operation failure
 * @pre The player state must be one of these: #PLAYER_STATE_IDLE, #PLAYER_STATE_READY, #PLAYER_STATE_PLAYING, or #PLAYER_STATE_PAUSED.
 * @see player_get_position_ratio()
 * @see player_set_position()
 * @see player_set_position_ratio()
 */

int player_get_position(player_h player, int *millisecond);

/**
 * @brief Gets the playback position specified by percent of media content played.
 * @param[in]   player The handle to media player
 * @param[out]  percent The current position in percentage [0, 100]
 * @return 0 on success, otherwise a negative error value.
 * @retval #PLAYER_ERROR_NONE Successful
 * @retval #PLAYER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #PLAYER_ERROR_INVALID_OPERATION Invalid operation
 * @retval #PLAYER_ERROR_SEEK_FAILED Seek operation failure
 * @pre The player state must be one of these: #PLAYER_STATE_IDLE, #PLAYER_STATE_READY, #PLAYER_STATE_PLAYING, or #PLAYER_STATE_PAUSED.
 * @see player_get_position()
 * @see player_set_position()
 * @see player_set_position_ratio()
 */
int player_get_position_ratio(player_h player, int *percent);

/**
 * @brief Sets the player's mute status.
 *
 * @details  If the mute status is @c true, no sounds will be played. If @c false, sounds will be played at the previously set volume level. Until this function is called, by default the player is not muted.
 *
 * @param[in]   player The handle to media player
 * @param[in]   muted New mute status: (@c true = mute, @c false = not muted)
 * @return 0 on success, otherwise a negative error value.
 * @retval #PLAYER_ERROR_NONE Successful
 * @retval #PLAYER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #PLAYER_ERROR_INVALID_OPERATION Invalid operation
 * @pre The player state must be one of these: #PLAYER_STATE_IDLE, #PLAYER_STATE_READY, #PLAYER_STATE_PLAYING, or #PLAYER_STATE_PAUSED.
 * @see player_is_muted()
 */
int player_set_mute(player_h player, bool muted);

/**
 * @brief Gets the player's mute status.
 *
 * @details If the mute status is @c true, no sounds are played. If @c false, sounds are played at previously set volume level.
 *
 * @param[in]   player The handle to media player
 * @param[out]  muted  The current mute status: (@c true = mute, @c false = not muted)
 * @return 0 on success, otherwise a negative error value.
 * @retval #PLAYER_ERROR_NONE Successful
 * @retval #PLAYER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #PLAYER_ERROR_INVALID_OPERATION Invalid operation
 * @pre The player state must be one of these: #PLAYER_STATE_IDLE, #PLAYER_STATE_READY, #PLAYER_STATE_PLAYING, or #PLAYER_STATE_PAUSED.
 * @see player_set_mute()
 */
int player_is_muted(player_h player, bool *muted);

/**
 * @brief Sets the player's looping status.
 *
 * @details	If the looping status is @c true, playback will automatically restart upon finishing. If @c false, it won't. The default value is false.
 *
 * @param[in]   player The handle to media player
 * @param[in]   looping New looping status: (@c true = looping, @c false = non-looping )
 * @return 0 on success, otherwise a negative error value.
 * @retval #PLAYER_ERROR_NONE Successful
 * @retval #PLAYER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #PLAYER_ERROR_INVALID_OPERATION Invalid operation
 * @retval #PLAYER_ERROR_INVALID_STATE Invalid player state
 * @pre The player state must be one of these: #PLAYER_STATE_IDLE, #PLAYER_STATE_READY, #PLAYER_STATE_PLAYING, or #PLAYER_STATE_PAUSED.
 * @see player_is_looping()
 */
int player_set_looping(player_h player, bool looping);


/**
 * @brief Gets the player's looping status.
 *
 * @details  If the looping status is @c true, playback will automatically restart upon finishing. If @c false, it won't.
 * 
 * @param[in]   player The handle to media player
 * @param[out]  looping The looping status: (@c true = looping, @c false = non-looping )
 * @return 0 on success, otherwise a negative error value.
 * @retval #PLAYER_ERROR_NONE Successful
 * @retval #PLAYER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #PLAYER_ERROR_INVALID_OPERATION Invalid operation
 * @retval #PLAYER_ERROR_INVALID_STATE Invalid player state
 * @pre The player state must be one of these: #PLAYER_STATE_IDLE, #PLAYER_STATE_READY, #PLAYER_STATE_PLAYING, or #PLAYER_STATE_PAUSED.
 * @see player_set_looping()
 */
int player_is_looping(player_h player, bool *looping);

/**
 * @brief Gets the total running time of the associated media.
 * @remarks The media source is associated with the player, using either player_set_uri() or player_set_memory_buffer().
 * @param[in]   player The handle to media player
 * @param[out]  duration The duration is in milliseconds
 * @return 0 on success, otherwise a negative error value.
 * @retval #PLAYER_ERROR_NONE Successful
 * @retval #PLAYER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #PLAYER_ERROR_INVALID_OPERATION Invalid operation
 * @retval #PLAYER_ERROR_INVALID_STATE Invalid player state
 * @pre The player state must be one of: #PLAYER_STATE_READY, #PLAYER_STATE_PLAYING, or #PLAYER_STATE_PAUSED.
 * @see player_get_position()
 * @see player_set_uri()
 * @see player_set_memory_buffer()
 */
int player_get_duration(player_h player, int *duration);

/**
 * @brief Sets the video display.
 * @param[in]   player The handle to media player
 * @param[in]   type The display type
 * @param[in]   display The handle to display
 * @return 0 on success, otherwise a negative error value.
 * @retval #PLAYER_ERROR_NONE Successful
 * @retval #PLAYER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #PLAYER_ERROR_INVALID_OPERATION Invalid operation
 * @retval #PLAYER_ERROR_INVALID_STATE Invalid player state
 * @pre The player state must be #PLAYER_STATE_IDLE by player_prepare() or player_unprepare().
 */
int player_set_display(player_h player, player_display_type_e type, player_display_h display);

/**
 * @}
 */

/**
 * @addtogroup CAPI_MEDIA_PLAYER_X11_DISPLAY_MODULE
 * @{
 */

/**
 * @brief Sets the rotation settings of the x surface video display
 * @details Use this function to change the video orientation to portrait mode.
 * @remarks If current display type is not #PLAYER_DISPLAY_TYPE_X11, no operation is performed.
 * @param[in] player   The handle to media player
 * @param[in] rotation The rotation of display
 * @return 0 on success, otherwise a negative error value.
 * @retval #PLAYER_ERROR_NONE Successful
 * @retval #PLAYER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #PLAYER_ERROR_INVALID_STATE Invalid state
 * @see  player_get_x11_display_rotation()
 */
int player_set_x11_display_rotation(player_h player, player_display_rotation_e rotation);

/**
 * @brief Gets a rotation of the x surface video display
 * @remarks If current display type is not #PLAYER_DISPLAY_TYPE_X11, no operation is performed.
 * @param[in] player The handle to media player
 * @param[out] rotation The current rotation of display
 * @return 0 on success, otherwise a negative error value.
 * @retval  #PLAYER_ERROR_NONE Successful
 * @retval  #PLAYER_ERROR_INVALID_PARAMETER Invalid parameter
 * @see	player_set_x11_display_rotation()
 */
int player_get_x11_display_rotation( player_h player, player_display_rotation_e *rotation);

/**
 * @brief Sets the visibility of the x surface video display
 * @remarks If current display type is not #PLAYER_DISPLAY_TYPE_X11, no operation is performed.
 * @param[in] player   The handle to media player
 * @param[in] rotation The visibility of display (@c true = visible, @c false = non-visible )
 * @return 0 on success, otherwise a negative error value.
 * @retval #PLAYER_ERROR_NONE Successful
 * @retval #PLAYER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #PLAYER_ERROR_INVALID_STATE Invalid state
 * @see	player_is_x11_display_visible()
 */
int player_set_x11_display_visible(player_h player, bool visible);

/**
 * @brief Gets a visibility of the x surface video display
 * @remarks If current display type is not #PLAYER_DISPLAY_TYPE_X11, no operation is performed.
 * @param[in] player The handle to media player
 * @param[out] visible The current visibility of display (@c true = visible, @c false = non-visible )
 * @return 0 on success, otherwise a negative error value.
 * @retval  #PLAYER_ERROR_NONE Successful
 * @retval  #PLAYER_ERROR_INVALID_PARAMETER Invalid parameter
 * @see	player_set_x11_display_visible()
 */
int player_is_x11_display_visible(player_h player, bool* visible);

/**
 * @brief Sets a x surface video display mode
 * @remarks If current display type is not #PLAYER_DISPLAY_TYPE_X11, no operation is performed.
 * @param[in] player   The handle to media player
 * @param[in] mode The display mode
 * @return 0 on success, otherwise a negative error value.
 * @retval #PLAYER_ERROR_NONE Successful
 * @retval #PLAYER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #PLAYER_ERROR_INVALID_STATE Invalid state
 * @see	player_get_x11_display_mode()
 */
int player_set_x11_display_mode(player_h player, player_display_mode_e mode);

/**
 * @brief Gets the x surface video display mode
 * @remarks If current display type is not #PLAYER_DISPLAY_TYPE_X11, no operation is performed.
 * @param[in] player The handle to media player
 * @param[out] mode The current display mode
 * @return 0 on success, otherwise a negative error value.
 * @retval  #PLAYER_ERROR_NONE Successful
 * @retval  #PLAYER_ERROR_INVALID_PARAMETER Invalid parameter
 * @see	player_set_x11_display_mode()
 */
int player_get_x11_display_mode(player_h player, player_display_mode_e *mode);

/**
 * @}
 */

/**
 * @addtogroup CAPI_MEDIA_PLAYER_MODULE
 * @{
 */

/**
 * @brief Gets the video display's height and width.
 * @param[in] player The handle to media player
 * @param[out] width The width of video. It can be invalid value if there is no video, no display was set.
 * @param[out] height The height of video. It can be invalid value if there is no video, no display was set.
 * @return 0 on success, otherwise a negative error value.
 * @retval #PLAYER_ERROR_NONE Successful
 * @retval #PLAYER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #PLAYER_ERROR_INVALID_OPERATION Invalid operation
 * @retval #PLAYER_ERROR_INVALID_STATE Invalid player state
 * @pre The player state must be #PLAYER_STATE_READY by player_prepare() .
 */
int player_get_video_size(player_h player, int *width, int *height);

/**
 * @brief Sets a subtitle path.
 * @remarks Only MicroDVD/SubViewer(*.sub), SAMI(*.smi), and SubRip(*.srt) subtitle formats are supported.
 * @param[in]   player The handle to media player
 * @param[in]   path The absolute path of the subtitle file
 * @return 0 on success, otherwise a negative error value.
 * @retval #PLAYER_ERROR_NONE Successful
 * @retval #PLAYER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #PLAYER_ERROR_INVALID_OPERATION Invalid operation
 * @retval #PLAYER_ERROR_INVALID_STATE Invalid player state
 * @pre The player state should be #PLAYER_STATE_IDLE by player_prepare() or player_unprepare().
 */
int player_set_subtitle_path(player_h player, char* path);

/**
 * @brief Captures the video frame, asynchronously.
 * @param[in] player The handle to media player
 * @param[in] callback	The callback function to register
 * @param[in] user_data	The user data to be passed to the callback function
 * @return 0 on success, otherwise a negative error value.
 * @retval #PLAYER_ERROR_NONE Successful
 * @retval #PLAYER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #PLAYER_ERROR_INVALID_OPERATION Invalid operation
 * @retval #PLAYER_ERROR_INVALID_STATE Invalid player state
 * @pre The player state must be #PLAYER_STATE_PLAYING by player_start() or #PLAYER_STATE_PAUSED by player_pause().
 * @post It invokes player_video_captured_cb() when capture completes, if you set a callback.
 * @see player_video_captured_cb()
 */
int player_capture_video(player_h player, player_video_captured_cb callback, void *user_data); 

/** 
 * @brief Registers a callback function to be invoked when the playback starts.
 * @param[in] player	The handle to media player
 * @param[in] callback	The callback function to register
 * @param[in] user_data	The user data to be passed to the callback function
 * @return 0 on success, otherwise a negative error value.
 * @retval #PLAYER_ERROR_NONE Successful
 * @retval #PLAYER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #PLAYER_ERROR_INVALID_OPERATION Invalid operation
 * @post  player_started_cb() will be invoked
 * @see player_unset_started_cb()
 * @see player_started_cb()
 * @see player_start()
 */
int player_set_started_cb(player_h player, player_started_cb callback, void *user_data);

/**
 * @brief	Unregisters the callback function.
 * @param[in] player The handle to media player
 * @return 0 on success, otherwise a negative error value.
 * @retval #PLAYER_ERROR_NONE Successful
 * @retval #PLAYER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #PLAYER_ERROR_INVALID_OPERATION Invalid operation
 * @see player_set_started_cb()
 */
int player_unset_started_cb(player_h player);

/**
 * @brief Registers a callback function to be invoked when the playback finishes.
 * @param[in] player	The handle to media player
 * @param[in] callback	The callback function to register
 * @param[in] user_data	The user data to be passed to the callback function
 * @return 0 on success, otherwise a negative error value.
 * @retval #PLAYER_ERROR_NONE Successful
 * @retval #PLAYER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #PLAYER_ERROR_INVALID_OPERATION Invalid operation
 * @post  player_completed_cb() will be invoked
 * @see player_unset_completed_cb()
 * @see player_completed_cb()
 * @see player_start()
 */
int player_set_completed_cb(player_h player, player_completed_cb callback, void *user_data);

/**
 * @brief	Unregisters the callback function.
 * @param[in] player The handle to media player
 * @return 0 on success, otherwise a negative error value.
 * @retval #PLAYER_ERROR_NONE Successful
 * @retval #PLAYER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #PLAYER_ERROR_INVALID_OPERATION Invalid operation
 * @see player_set_completed_cb()
 */
int player_unset_completed_cb(player_h player);

/**
 * @brief Registers a callback function to be invoked when the playback is paused.
 * @param[in] player	The handle to media player
 * @param[in] callback	The callback function to register
 * @param[in] user_data	The user data to be passed to the callback function
 * @return 0 on success, otherwise a negative error value.
 * @retval #PLAYER_ERROR_NONE Successful
 * @retval #PLAYER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #PLAYER_ERROR_INVALID_OPERATION Invalid operation
 * @post  player_paused_cb() will be invoked
 * @see player_unset_paused_cb()
 * @see player_paused_cb()
 * @see player_pause()
 */
int player_set_paused_cb(player_h player, player_paused_cb callback, void *user_data);

/**
 * @brief Unregisters the callback function.
 * @param[in] player The handle to media player
 * @return 0 on success, otherwise a negative error value.
 * @retval #PLAYER_ERROR_NONE Successful
 * @retval #PLAYER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #PLAYER_ERROR_INVALID_OPERATION Invalid operation
 * @see player_set_paused_cb()
 */
int player_unset_paused_cb(player_h player);


/**
 * @brief Registers a callback function to be invoked when the playback is interrupted.
 * @param[in] player	The handle to media player
 * @param[in] callback	The callback function to register
 * @param[in] user_data	The user data to be passed to the callback function
 * @return 0 on success, otherwise a negative error value.
 * @retval #PLAYER_ERROR_NONE Successful
 * @retval #PLAYER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #PLAYER_ERROR_INVALID_OPERATION Invalid operation
 * @post  player_interrupted_cb() will be invoked
 * @see player_unset_interrupted_cb()
 * @see #player_interrupted_code_e
 * @see player_interrupted_cb()
 */
int player_set_interrupted_cb(player_h player, player_interrupted_cb callback, void *user_data);

/**
 * @brief Unregisters the callback function.
 * @param[in] player The handle to media player
 * @return 0 on success, otherwise a negative error value.
 * @retval #PLAYER_ERROR_NONE Successful
 * @retval #PLAYER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #PLAYER_ERROR_INVALID_OPERATION Invalid operation
 * @see player_set_interrupted_cb()
 */
int player_unset_interrupted_cb(player_h player);

/**
 * @brief Registers a callback function to be invoked when error occured.
 * @param[in] player	The handle to media player
 * @param[in] callback	The callback function to register
 * @param[in] user_data	The user data to be passed to the callback function
 * @return 0 on success, otherwise a negative error value.
 * @retval #PLAYER_ERROR_NONE Successful
 * @retval #PLAYER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #PLAYER_ERROR_INVALID_OPERATION Invalid operation
 * @post  player_error_cb() will be invoked
 * @see player_unset_error_cb()
 * @see player_error_cb()
 */
int player_set_error_cb(player_h player, player_error_cb callback, void *user_data);

/**
 * @brief Unregisters the callback function.
 *
 * @param[in] player The handle to media player
 * @return 0 on success, otherwise a negative error value.
 * @retval #PLAYER_ERROR_NONE Successful
 * @retval #PLAYER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #PLAYER_ERROR_INVALID_OPERATION Invalid operation
 * @see player_set_error_cb()
 */
int player_unset_error_cb(player_h player);

/**
 * @brief Registers a callback function to be invoked when there is a change in the buffering status of a media stream.
 * @remarks  The media resource should be streamed over the network.
 * @param[in] player	The handle to media player
 * @param[in] callback	The callback function to register
 * @param[in] user_data	The user data to be passed to the callback function
 * @return 0 on success, otherwise a negative error value.
 * @retval #PLAYER_ERROR_NONE Successful
 * @retval #PLAYER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #PLAYER_ERROR_INVALID_OPERATION Invalid operation
 * @post  player_buffering_cb() will be invoked
 * @see player_unset_buffering_cb()
 * @see player_set_uri()
 * @see player_buffering_cb() 
 */
int player_set_buffering_cb(player_h player, player_buffering_cb callback, void *user_data);

/**
 * @brief Unregisters the callback function.
 * @param[in] player The handle to media player
 * @return 0 on success, otherwise a negative error value.
 * @retval #PLAYER_ERROR_NONE Successful
 * @retval #PLAYER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #PLAYER_ERROR_INVALID_OPERATION Invalid operation
 * @see player_set_buffering_cb()
 */
int player_unset_buffering_cb(player_h player);

/**
 * @brief Registers a callback function to be invoked when subtitle updates.
 * @param[in] player	The handle to media player
 * @param[in] callback	The callback function to register
 * @param[in] user_data	The user data to be passed to the callback function
 * @return 0 on success, otherwise a negative error value.
 * @retval #PLAYER_ERROR_NONE Successful
 * @retval #PLAYER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #PLAYER_ERROR_INVALID_OPERATION Invalid operation
 * @pre The subtile must be set by player_set_subtitle_path().
 * @post  player_subtitle_updated_cb() will be invoked
 * @see player_unset_subtitle_updated_cb()
 * @see player_subtitle_updated_cb()
 * @see player_set_subtitle_path()
 */
int player_set_subtilte_updated_cb(player_h player, player_subtitle_updated_cb callback, void *user_data);

/**
 * @brief Unregisters the callback function.
 * @param[in] player The handle to media player
 * @return 0 on success, otherwise a negative error value.
 * @retval #PLAYER_ERROR_NONE Successful
 * @retval #PLAYER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #PLAYER_ERROR_INVALID_OPERATION Invalid operation
 * @see player_set_subtitle_updated_cb()
 */
int player_unset_subtilte_updated_cb(player_h player);


/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* __TIZEN_MEDIA_PLAYER_H__ */
