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

#ifndef __TIZEN_MEDIA_PLAYER_MSG_PRIVATE_H__
#define __TIZEN_MEDIA_PLAYER_MSG_PRIVATE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <muse_player_msg.h>

/**
 * @brief Create and send address of server side module infomation structure.
 * @remarks Does NOT guarantee thread safe.
 * @param[in] module The server side module infomation.
 * @param[in] fd socket fd
 */
#define muse_core_send_module_addr(module, fd) \
	do {\
		char *__sndMsg__; \
		int __len__; \
		__sndMsg__ = muse_core_msg_json_factory_new(0, \
				MUSE_TYPE_POINTER, #module, module, \
				0); \
		__len__ = muse_core_ipc_send_msg(fd, __sndMsg__); \
		muse_core_msg_json_factory_free(__sndMsg__); \
		if (__len__ <= 0) { \
			LOGE("sending message failed"); \
			return PLAYER_ERROR_INVALID_OPERATION; \
		} \
	} while (0)

/**
 * @brief Create and send message. Wait for server result.
 * @remarks Does NOT guarantee thread safe.
 * @param[in] api The enum of module API.
 * @param[in] player The handle of capi media player.
 * @param[out] retbuf The buffer of return message. Must be char pointer.
 * @param[out] ret The return value from server.
 */
#define player_msg_send(api, player, retbuf, ret) \
	do{	\
		char *__sndMsg__; \
		int __len__; \
		int __fd__; \
		int __timeout__ = client_get_api_timeout(player, api); \
		if(CALLBACK_INFO(player)) __fd__ = MSG_FD(player); \
		else {ret = PLAYER_ERROR_INVALID_STATE;break;} \
		__sndMsg__ = muse_core_msg_json_factory_new(api, \
				0); \
		__len__ = muse_core_ipc_send_msg(__fd__, __sndMsg__); \
		if (__len__ <= 0) { \
			LOGE("sending message failed"); \
			ret = PLAYER_ERROR_INVALID_OPERATION; \
		} else \
			ret = client_wait_for_cb_return(api, CALLBACK_INFO(player), &retbuf, __timeout__); \
		muse_core_msg_json_factory_free(__sndMsg__); \
	}while(0)

/**
 * @brief Create and send message. Wait for server result.
 * @remarks Does NOT guarantee thread safe.
 * @param[in] api The enum of module API.
 * @param[in] player The handle of capi media player.
 * @param[out] retbuf The buffer of return message. Must be char pointer.Must free after use.
 * @param[out] ret The return value from server.
 * @param[in] type The enum of parameter type. Muse be one of thease(INT, INT64, POINTER, DOUBLE, STRING, ARRAY)
 * @param[in] param the name of param is key, must be local variable. never be pointer.
 */
#define player_msg_send1(api, player, retbuf, ret, type, param) \
	do{	\
		char *__sndMsg__; \
		int __len__; \
		int __fd__; \
		int __timeout__ = client_get_api_timeout(player, api); \
		type __value__ = (type)param; \
		if(CALLBACK_INFO(player)) __fd__ = MSG_FD(player); \
		else {ret = PLAYER_ERROR_INVALID_STATE;break;} \
		__sndMsg__ = muse_core_msg_json_factory_new(api, \
				MUSE_TYPE_##type, #param, __value__, \
				0); \
		__len__ = muse_core_ipc_send_msg(__fd__, __sndMsg__); \
		if (__len__ <= 0) { \
			LOGE("sending message failed"); \
			ret = PLAYER_ERROR_INVALID_OPERATION; \
		} else \
			ret = client_wait_for_cb_return(api, CALLBACK_INFO(player), &retbuf, __timeout__); \
		muse_core_msg_json_factory_free(__sndMsg__); \
	}while(0)

/**
 * @brief Create and send message. Wait for server result.
 * @remarks Does NOT guarantee thread safe.
 * @param[in] api The enum of module API.
 * @param[in] player The handle of capi media player.
 * @param[out] retbuf The buffer of return message. Must be char pointer.Must free after use.
 * @param[out] ret The return value from server.
 * @param[in] type The enum of parameter type. Muse be one of thease(INT, INT64, POINTER, DOUBLE, STRING, ARRAY)
 * @param[in] param# the name of param is key, must be local variable. never be pointer.
 */
#define player_msg_send2(api, player, retbuf, ret, type1, param1, type2, param2) \
	do{	\
		char *__sndMsg__; \
		int __len__; \
		int __fd__; \
		int __timeout__ = client_get_api_timeout(player, api); \
		type1 __value1__ = (type1)param1; \
		type2 __value2__ = (type2)param2; \
		if(CALLBACK_INFO(player)) __fd__ = MSG_FD(player); \
		else {ret = PLAYER_ERROR_INVALID_STATE;break;} \
		__sndMsg__ = muse_core_msg_json_factory_new(api, \
				MUSE_TYPE_##type1, #param1, __value1__, \
				MUSE_TYPE_##type2, #param2, __value2__, \
				0); \
		__len__ = muse_core_ipc_send_msg(__fd__, __sndMsg__); \
		if (__len__ <= 0) { \
			LOGE("sending message failed"); \
			ret = PLAYER_ERROR_INVALID_OPERATION; \
		} else \
			ret = client_wait_for_cb_return(api, CALLBACK_INFO(player), &retbuf, __timeout__); \
		muse_core_msg_json_factory_free(__sndMsg__); \
	}while(0)

/**
 * @brief Create and send message. Wait for server result.
 * @remarks Does NOT guarantee thread safe.
 * @param[in] api The enum of module API.
 * @param[in] player The handle of capi media player.
 * @param[out] retbuf The buffer of return message. Must be char pointer.Must free after use.
 * @param[out] ret The return value from server.
 * @param[in] type The enum of parameter type. Muse be one of thease(INT, INT64, POINTER, DOUBLE, STRING, ARRAY)
 * @param[in] param# the name of param is key, must be local variable. never be pointer.
 */
#define player_msg_send3(api, player, retbuf, ret, type1, param1, type2, param2, type3, param3) \
	do{	\
		char *__sndMsg__; \
		int __len__; \
		int __fd__; \
		int __timeout__ = client_get_api_timeout(player, api); \
		type1 __value1__ = (type1)param1; \
		type2 __value2__ = (type2)param2; \
		type3 __value3__ = (type3)param3; \
		if(CALLBACK_INFO(player)) __fd__ = MSG_FD(player); \
		else {ret = PLAYER_ERROR_INVALID_STATE;break;} \
		__sndMsg__ = muse_core_msg_json_factory_new(api, \
				MUSE_TYPE_##type1, #param1, __value1__, \
				MUSE_TYPE_##type2, #param2, __value2__, \
				MUSE_TYPE_##type3, #param3, __value3__, \
				0); \
		__len__ = muse_core_ipc_send_msg(__fd__, __sndMsg__); \
		if (__len__ <= 0) { \
			LOGE("sending message failed"); \
			ret = PLAYER_ERROR_INVALID_OPERATION; \
		} else \
			ret = client_wait_for_cb_return(api, CALLBACK_INFO(player), &retbuf, __timeout__); \
		muse_core_msg_json_factory_free(__sndMsg__); \
	}while(0)

/**
 * @brief Create and send message. Wait for server result.
 * @remarks Does NOT guarantee thread safe.
 * @param[in] api The enum of module API.
 * @param[in] player The handle of capi media player.
 * @param[out] retbuf The buffer of return message. Must be char pointer.Must free after use.
 * @param[out] ret The return value from server.
 * @param[in] type The enum of parameter type. Muse be one of thease(INT, INT64, POINTER, DOUBLE, STRING, ARRAY)
 * @param[in] param# the name of param is key, must be local variable. never be pointer.
 */
#define player_msg_send4(api, player, retbuf, ret, type1, param1, type2, param2, type3, param3, type4, param4) \
	do{	\
		char *__sndMsg__; \
		int __len__; \
		int __fd__; \
		int __timeout__ = client_get_api_timeout(player, api); \
		type1 __value1__ = (type1)param1; \
		type2 __value2__ = (type2)param2; \
		type3 __value3__ = (type3)param3; \
		type4 __value4__ = (type4)param4; \
		if(CALLBACK_INFO(player)) __fd__ = MSG_FD(player); \
		else {ret = PLAYER_ERROR_INVALID_STATE;break;} \
		__sndMsg__ = muse_core_msg_json_factory_new(api, \
				MUSE_TYPE_##type1, #param1, __value1__, \
				MUSE_TYPE_##type2, #param2, __value2__, \
				MUSE_TYPE_##type3, #param3, __value3__, \
				MUSE_TYPE_##type4, #param4, __value4__, \
				0); \
		__len__ = muse_core_ipc_send_msg(__fd__, __sndMsg__); \
		if (__len__ <= 0) { \
			LOGE("sending message failed"); \
			ret = PLAYER_ERROR_INVALID_OPERATION; \
		} else \
			ret = client_wait_for_cb_return(api, CALLBACK_INFO(player), &retbuf, __timeout__); \
		muse_core_msg_json_factory_free(__sndMsg__); \
	}while(0)

/**
 * @brief Create and send message. Wait for server result.
 * @remarks Does NOT guarantee thread safe.
 * @param[in] api The enum of module API.
 * @param[in] player The handle of capi media player.
 * @param[out] retbuf The buffer of return message. Must be char pointer.Must free after use.
 * @param[out] ret The return value from server.
 * @param[in] type The enum of parameter type. Muse be one of thease(INT, INT64, POINTER, DOUBLE, STRING, ARRAY)
 * @param[in] param# the name of param is key, must be local variable. never be pointer.
 */
#define player_msg_send5(api, player, retbuf, ret, type1, param1, type2, param2, type3, param3, type4, param4, type5, param5) \
    do{ \
      char *__sndMsg__; \
      int __len__; \
      int __fd__; \
      int __timeout__ = client_get_api_timeout(player, api); \
      type1 __value1__ = (type1)param1; \
      type2 __value2__ = (type2)param2; \
      type3 __value3__ = (type3)param3; \
      type4 __value4__ = (type4)param4; \
      type5 __value5__ = (type5)param5; \
      if(CALLBACK_INFO(player)) __fd__ = MSG_FD(player); \
      else {ret = PLAYER_ERROR_INVALID_STATE;break;} \
      __sndMsg__ = muse_core_msg_json_factory_new(api, \
          MUSE_TYPE_##type1, #param1, __value1__, \
          MUSE_TYPE_##type2, #param2, __value2__, \
          MUSE_TYPE_##type3, #param3, __value3__, \
          MUSE_TYPE_##type4, #param4, __value4__, \
          MUSE_TYPE_##type5, #param5, __value5__, \
          0); \
      __len__ = muse_core_ipc_send_msg(__fd__, __sndMsg__); \
      if (__len__ <= 0) { \
        LOGE("sending message failed"); \
        ret = PLAYER_ERROR_INVALID_OPERATION; \
      } else \
        ret = client_wait_for_cb_return(api, CALLBACK_INFO(player), &retbuf, __timeout__); \
      muse_core_msg_json_factory_free(__sndMsg__); \
    }while(0)

/**
 * @brief Create and send message. Wait for server result.
 * @remarks Does NOT guarantee thread safe.
 * @param[in] api The enum of module API.
 * @param[in] player The handle of capi media player.
 * @param[out] retbuf The buffer of return message. Must be char pointer.Must free after use.
 * @param[out] ret The return value from server.
 * @param[in] type The enum of parameter type. Muse be one of thease(INT, INT64, POINTER, DOUBLE, STRING, ARRAY)
 * @param[in] param# the name of param is key, must be local variable. never be pointer.
 */
#define player_msg_send6(api, player, retbuf, ret, type1, param1, type2, param2, type3, param3, type4, param4, type5, param5, type6, param6) \
	do{	\
		char *__sndMsg__; \
		int __len__; \
		int __fd__; \
		int __timeout__ = client_get_api_timeout(player, api); \
		type1 __value1__ = (type1)param1; \
		type2 __value2__ = (type2)param2; \
		type3 __value3__ = (type3)param3; \
		type4 __value4__ = (type4)param4; \
		type5 __value5__ = (type5)param5; \
		type6 __value6__ = (type6)param6; \
		if(CALLBACK_INFO(player)) __fd__ = MSG_FD(player); \
		else {ret = PLAYER_ERROR_INVALID_STATE;break;} \
		__sndMsg__ = muse_core_msg_json_factory_new(api, \
				MUSE_TYPE_##type1, #param1, __value1__, \
				MUSE_TYPE_##type2, #param2, __value2__, \
				MUSE_TYPE_##type3, #param3, __value3__, \
				MUSE_TYPE_##type4, #param4, __value4__, \
				MUSE_TYPE_##type5, #param5, __value5__, \
				MUSE_TYPE_##type6, #param6, __value6__, \
				0); \
		__len__ = muse_core_ipc_send_msg(__fd__, __sndMsg__); \
		if (__len__ <= 0) { \
			LOGE("sending message failed"); \
			ret = PLAYER_ERROR_INVALID_OPERATION; \
		} else \
			ret = client_wait_for_cb_return(api, CALLBACK_INFO(player), &retbuf, __timeout__); \
		muse_core_msg_json_factory_free(__sndMsg__); \
	}while(0)

/**
 * @brief Create and send message. Wait for server result.
 * @remarks Does NOT guarantee thread safe.
 * @param[in] api The enum of module API.
 * @param[in] player The handle of capi media player.
 * @param[out] retbuf The buffer of return message. Must be char pointer.Must free after use.
 * @param[out] ret The return value from server.
 * @param[in] param the name of param is key, must be local array/pointer variable.
 * @param[in] length The size of array.
 * @param[in] datum_size The size of a array's datum.
 */
#define player_msg_send_array(api, player, retbuf, ret, param, length, datum_size) \
	do{	\
		char *__sndMsg__; \
		int __len__; \
		int __fd__; \
		int __timeout__ = client_get_api_timeout(player, api); \
		int *__value__ = (int *)param; \
		if(CALLBACK_INFO(player)) __fd__ = MSG_FD(player); \
		else {ret = PLAYER_ERROR_INVALID_STATE;break;} \
		__sndMsg__ = muse_core_msg_json_factory_new(api, \
				MUSE_TYPE_INT, #length, length, \
				MUSE_TYPE_ARRAY, #param, \
					datum_size == sizeof(int)? length :  \
					length / sizeof(int) + (length % sizeof(int)?1:0), \
					__value__, \
				0); \
		__len__ = muse_core_ipc_send_msg(__fd__, __sndMsg__); \
		if (__len__ <= 0) { \
			LOGE("sending message failed"); \
			ret = PLAYER_ERROR_INVALID_OPERATION; \
		} else \
			ret = client_wait_for_cb_return(api, CALLBACK_INFO(player), &retbuf, __timeout__); \
		muse_core_msg_json_factory_free(__sndMsg__); \
	}while(0)

/**
 * @brief Create and send message. Wait for server result.
 * @remarks Does NOT guarantee thread safe.
 * @param[in] api The enum of module API.
 * @param[in] player The handle of capi media player.
 * @param[out] retbuf The buffer of return message. Must be char pointer.Must free after use.
 * @param[out] ret The return value from server.
 * @param[in] param# the name of param is key, must be local array/pointer variable.
 * @param[in] length# The size of array.
 * @param[in] datum_size# The size of a array's datum.
 */
#define player_msg_send_array2(api, player, retbuf, ret, param1, length1, datum_size1, param2, length2, datum_size2) \
	do{	\
		char *__sndMsg__; \
		int __len__; \
		int __fd__; \
		int __timeout__ = client_get_api_timeout(player, api); \
		int *__value1__ = (int *)param1; \
		int *__value2__ = (int *)param2; \
		if(CALLBACK_INFO(player)) __fd__ = MSG_FD(player); \
		else {ret = PLAYER_ERROR_INVALID_STATE;break;} \
		__sndMsg__ = muse_core_msg_json_factory_new(api, \
				MUSE_TYPE_INT, #length1, length1, \
				MUSE_TYPE_ARRAY, #param1, \
					datum_size1 == sizeof(int)? length1 :  \
					length1 / sizeof(int) + (length1 % sizeof(int)?1:0), \
					__value1__, \
				MUSE_TYPE_INT, #length2, length2, \
				MUSE_TYPE_ARRAY, #param2, \
					datum_size2 == sizeof(int)? length2 :  \
					length2 / sizeof(int) + (length2 % sizeof(int)?1:0), \
					__value2__, \
				0); \
		__len__ = muse_core_ipc_send_msg(__fd__, __sndMsg__); \
		if (__len__ <= 0) { \
			LOGE("sending message failed"); \
			ret = PLAYER_ERROR_INVALID_OPERATION; \
		} else \
			ret = client_wait_for_cb_return(api, CALLBACK_INFO(player), &retbuf, __timeout__); \
		muse_core_msg_json_factory_free(__sndMsg__); \
	}while(0)

/**
 * @brief Create and send message. Wait for server result.
 * @remarks Does NOT guarantee thread safe.
 * @param[in] api The enum of module API.
 * @param[in] player The handle of capi media player.
 * @param[out] retbuf The buffer of return message. Must be char pointer.Must free after use.
 * @param[out] ret The return value from server.
 * @param[in] param# the name of param is key, must be local array/pointer variable.
 * @param[in] length# The size of array.
 * @param[in] datum_size# The size of a array's datum.
 */
#define player_msg_send1_array2(api, player, retbuf, ret, type1, param1, param2, length2, datum_size2, param3, length3, datum_size3) \
    do{ \
        char *__sndMsg__; \
        int __len__; \
        int __fd__; \
        int __timeout__ = client_get_api_timeout(player, api); \
        type1 __value1__ = (type1)param1; \
        int *__value2__ = (int *)param2; \
        int *__value3__ = (int *)param3; \
        if(CALLBACK_INFO(player)) __fd__ = MSG_FD(player); \
        else {ret = PLAYER_ERROR_INVALID_STATE;break;} \
        __sndMsg__ = muse_core_msg_json_factory_new(api, \
                MUSE_TYPE_##type1, #param1, __value1__, \
                MUSE_TYPE_INT, #length2, length2, \
                MUSE_TYPE_ARRAY, #param2, \
                    datum_size2 == sizeof(int)? length2 :  \
                    length2 / sizeof(int) + (length2 % sizeof(int)?1:0), \
                    __value2__, \
                MUSE_TYPE_INT, #length3, length3, \
                MUSE_TYPE_ARRAY, #param3, \
                    datum_size3 == sizeof(int)? length3 :  \
                    length3 / sizeof(int) + (length3 % sizeof(int)?1:0), \
                    __value3__, \
                0); \
        __len__ = muse_core_ipc_send_msg(__fd__, __sndMsg__); \
        if (__len__ <= 0) { \
            LOGE("sending message failed"); \
            ret = PLAYER_ERROR_INVALID_OPERATION; \
        } else \
            ret = client_wait_for_cb_return(api, CALLBACK_INFO(player), &retbuf, __timeout__); \
        muse_core_msg_json_factory_free(__sndMsg__); \
    }while(0)

/**
 * @brief Create and send message. Wait for server result.
 * @remarks Does NOT guarantee thread safe.
 * @param[in] api The enum of module API.
 * @param[in] player The handle of capi media player.
 * @param[out] retbuf The buffer of return message. Must be char pointer.Must free after use.
 * @param[out] ret The return value from server.
 * @param[in] type The enum of parameter type. Muse be one of thease(INT, INT64, POINTER, DOUBLE, STRING, ARRAY)
 * @param[in] param# the name of param is key, must be local variable. never be pointer.
 * @param[in] length# The size of array.
 * @param[in] datum_size# The size of a array's datum.
 */
#define player_msg_send2_array(api, player, retbuf, ret, type1, param1, type2, param2, param_arr, length, datum_size) \
	do{	\
		char *__sndMsg__; \
		int __len__; \
		int __fd__; \
		int __timeout__ = client_get_api_timeout(player, api); \
		type1 __value1__ = (type1)param1; \
		type2 __value2__ = (type2)param2; \
        int *__value_arr__ = (int *)param_arr; \
		if(CALLBACK_INFO(player)) __fd__ = MSG_FD(player); \
		else {ret = PLAYER_ERROR_INVALID_STATE;break;} \
		__sndMsg__ = muse_core_msg_json_factory_new(api, \
				MUSE_TYPE_##type1, #param1, __value1__, \
				MUSE_TYPE_##type2, #param2, __value2__, \
                MUSE_TYPE_INT, #length, length, \
				MUSE_TYPE_ARRAY, #param_arr, \
					datum_size == sizeof(int)? length :  \
					length / sizeof(int) + (length % sizeof(int)?1:0), \
					__value_arr__, \
				0); \
		__len__ = muse_core_ipc_send_msg(__fd__, __sndMsg__); \
		if (__len__ <= 0) { \
			LOGE("sending message failed"); \
			ret = PLAYER_ERROR_INVALID_OPERATION; \
		} else \
			ret = client_wait_for_cb_return(api, CALLBACK_INFO(player), &retbuf, __timeout__); \
		muse_core_msg_json_factory_free(__sndMsg__); \
	}while(0)

/**
 * @brief Create and send message. Wait for server result.
 * @remarks Does NOT guarantee thread safe.
 * @param[in] api The enum of module API.
 * @param[in] player The handle of capi media player.
 * @param[out] retbuf The buffer of return message. Must be char pointer.Must free after use.
 * @param[out] ret The return value from server.
 * @param[in] type The enum of parameter type. Muse be one of thease(INT, INT64, POINTER, DOUBLE, STRING, ARRAY)
 * @param[in] param# the name of param is key, must be local variable. never be pointer.
 * @param[in] length# The size of array.
 * @param[in] datum_size# The size of a array's datum.
 */
#define player_msg_send3_array2(api, player, retbuf, ret, type1, param1, type2, param2, type3, param3, param4, length4, datum_size4, param5, length5, datum_size5) \
            do{ \
                char *__sndMsg__; \
                int __len__; \
                int __fd__; \
                int __timeout__ = client_get_api_timeout(player, api); \
                type1 __value1__ = (type1)param1; \
                type2 __value2__ = (type2)param2; \
                type3 __value3__ = (type3)param3; \
                int *__value4__ = (int *)param4; \
                int *__value5__ = (int *)param5; \
                if(CALLBACK_INFO(player)) __fd__ = MSG_FD(player); \
                else {ret = PLAYER_ERROR_INVALID_STATE;break;} \
                __sndMsg__ = muse_core_msg_json_factory_new(api, \
                        MUSE_TYPE_##type1, #param1, __value1__, \
                        MUSE_TYPE_##type2, #param2, __value2__, \
                        MUSE_TYPE_##type3, #param3, __value3__, \
                        MUSE_TYPE_INT, #length4, length4, \
                        MUSE_TYPE_ARRAY, #param4, \
                            datum_size4 == sizeof(int)? length4 :  \
                            length4 / sizeof(int) + (length4 % sizeof(int)?1:0), \
                            __value4__, \
                        MUSE_TYPE_INT, #length5, length5, \
                        MUSE_TYPE_ARRAY, #param5, \
                            datum_size5 == sizeof(int)? length5 :  \
                            length5 / sizeof(int) + (length5 % sizeof(int)?1:0), \
                            __value5__, \
                        0); \
                __len__ = muse_core_ipc_send_msg(__fd__, __sndMsg__); \
                if (__len__ <= 0) { \
                    LOGE("sending message failed"); \
                    ret = PLAYER_ERROR_INVALID_OPERATION; \
                } else \
                    ret = client_wait_for_cb_return(api, CALLBACK_INFO(player), &retbuf, __timeout__); \
                muse_core_msg_json_factory_free(__sndMsg__); \
            }while(0)

/**
 * @brief Create and send message. Does not wait server result.
 * @remarks Does NOT guarantee thread safe.
 * @param[in] api The enum of module API.
 * @param[in] player The handle of capi media player.
 * @param[in] type The enum of parameter type. Muse be one of thease(INT, INT64, POINTER, DOUBLE, STRING, ARRAY)
 * @param[in] param the name of param is key, must be local variable. never be pointer.
 */
#define player_msg_send1_async(api, player, type, param) \
	do{	\
		char *__sndMsg__; \
		int __len__; \
		int __fd__; \
		type __value__ = (type)param; \
		if(CALLBACK_INFO(player)) __fd__ = MSG_FD(player); \
		else {ret = PLAYER_ERROR_INVALID_STATE;break;} \
		__sndMsg__ = muse_core_msg_json_factory_new(api, \
				MUSE_TYPE_##type, #param, __value__, \
				0); \
		__len__ = muse_core_ipc_send_msg(__fd__, __sndMsg__); \
		muse_core_msg_json_factory_free(__sndMsg__); \
		if (__len__ <= 0) { \
			LOGE("sending message failed"); \
			return PLAYER_ERROR_INVALID_OPERATION; \
		} \
	}while(0)

/**
 * @brief Create and send message. Does not wait server result.
 * @remarks Does NOT guarantee thread safe.
 * @param[in] api The enum of module API.
 * @param[in] player The handle of capi media player.
 * @param[in] type The enum of parameter type. Muse be one of thease(INT, INT64, POINTER, DOUBLE, STRING, ARRAY)
 * @param[in] param the name of param is key, must be local variable. never be pointer.
 */
#define player_msg_send1_no_return(api, fd, type, param) \
	do{ \
		char *__sndMsg__; \
		int __len__; \
		type __value__ = (type)param; \
		__sndMsg__ = muse_core_msg_json_factory_new(api, \
				MUSE_TYPE_##type, #param, __value__, \
				0); \
		__len__ = muse_core_ipc_send_msg(fd, __sndMsg__); \
		muse_core_msg_json_factory_free(__sndMsg__); \
		if (__len__ <= 0) { \
			LOGE("sending message failed"); \
		} \
	}while(0)

/**
 * @brief Create and send message. Does not wait server result.
 * @remarks Does NOT guarantee thread safe.
 * @param[in] api The enum of module API.
 * @param[in] player The server side handle of media player.
 * @param[in] fd socket fd
 * @param[in] type The enum of parameter type. Muse be one of thease(INT, INT64, POINTER, DOUBLE, STRING, ARRAY)
 * @param[in] param the name of param is key, must be local variable. never be pointer.
 */
#define player_msg_create_handle(api, fd, type1, param1, type2, param2) \
	do{	\
		char *__sndMsg__; \
		int __len__; \
		type1 __value1__ = (type1)param1; \
		type2 __value2__ = (type2)param2; \
		__sndMsg__ = muse_core_msg_json_factory_new(api, \
				MUSE_TYPE_##type1, #param1, __value1__, \
				MUSE_TYPE_##type2, #param2, __value2__, \
				0); \
		__len__ = muse_core_ipc_send_msg(fd, __sndMsg__); \
		muse_core_msg_json_factory_free(__sndMsg__); \
		if (__len__ <= 0) { \
			LOGE("sending message failed"); \
			return PLAYER_ERROR_INVALID_OPERATION; \
		} \
	}while(0)


/**
 * @brief Create and send message for callback. Does not wait server result.
 * @remarks Does NOT guarantee thread safe.
 * @param[in] api The enum of module API.
 * @param[in] player The handle of capi media player.
 * @param[out] ret set ERROR when fail to send msg.
 * @param[in] event_type type of event (muse_player_event_e).
 * @param[in] set 1 is set the user callback, 0 is unset the user callback.
 */
#define player_msg_set_callback(api, player, ret, event_type, set) \
	do{	\
		char *__sndMsg__; \
		int __len__; \
		int __fd__; \
		int __value1__ = (int)event_type; \
		int __value2__ = (int)set; \
		if(CALLBACK_INFO(player)) __fd__ = MSG_FD(player); \
		else {ret = PLAYER_ERROR_INVALID_STATE;break;} \
		__sndMsg__ = muse_core_msg_json_factory_new(api, \
				MUSE_TYPE_INT, #event_type, __value1__, \
				MUSE_TYPE_INT, #set, __value2__, \
				0); \
		__len__ = muse_core_ipc_send_msg(__fd__, __sndMsg__); \
		muse_core_msg_json_factory_free(__sndMsg__); \
		if (__len__ <= 0) { \
			LOGE("sending message failed"); \
			ret = PLAYER_ERROR_INVALID_OPERATION; \
		} \
	}while(0)



#endif /* __TIZEN_MEDIA_PLAYER_MSG_PRIVATE_H__ */
