#pragma once

#include <stdint.h>

#undef ERROR
namespace details
{
	enum SocketDataType : int32_t
	{
		ERROR, //异常值
		CONNECT_CONFIRM,//data=当前工作目录u16string
		RETURN_SUCCESS,	//datalen=0
		RETURN_FAIL,	//datalen=0
		NEW_BREAKPOINT,	//data=fullfilename:lineNo
		DEL_BREAKPOINT,	//data=fullfilename:lineNo
		NEW_BREAKPOINTONCE,	//data=fullfilename:lineNo
		QUERY_VAR,		//data=variable name expression
		QUERY_SP,		//datalen=4,data=(int32_t)spIndex
		QUERY_SCREEN,	//datalen=0
		QUERY_AUDIO,	//datalen=4, data=(int32_t)audio_index
		STEP_NEXT,		//datalen=0
		STEP_INTO,		//datalen=0
		STEP_OUT,		//datalen=0
		RUN,			//datalen=0
		PAUSE,			//datalen=0
		EXECUTE_BAGEL,	//data=bagel expression
		RETURN_NOTFOUND,//datalen=0
		RETURN_BAGEL,	//data=serialized bagel data
		RETURN_EXCEPT,	//data=serialized bagel data
		LOG,			//data=[int32]level [u16string]msg
		QUERY_SPRITE_TREE, //datalen=0
		QUERY_SPRITE_IMAGE, //datalen=6, data=index:int32_t,witheffect:bool,withchildren:bool
		RETURN_SPRITE_IMAGE, //datalen=8+?, data=width:uint16,height:uint16,decompressed:int32,lz4data
	};
};
typedef details::SocketDataType SocketDataType;