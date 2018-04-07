#pragma once

#include <stdint.h>

#undef ERROR
namespace details
{
	enum SocketDataType : int32_t
	{
		ERROR, //�쳣ֵ
		CONNECT_CONFIRM,//data=��ǰ����Ŀ¼u16string
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
	};
};
typedef details::SocketDataType SocketDataType;