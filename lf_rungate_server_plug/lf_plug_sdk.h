#ifndef _SDK_UTILS_H__
#define _SDK_UTILS_H__
#include <stdint.h>
#include <vector>
#include <string>
#include <functional>
#include <winnt.h>
#include <intsafe.h>

namespace sdk::utils {
	template<class F>
	struct safe_api_warpper;

	template<class R, class... Args>
	struct safe_api_warpper<R(__stdcall*)(Args...)> : public safe_api_warpper<R(Args...)> {
	public:
		using super = safe_api_warpper<R(Args...)>;
		void operator=(void* value) { super::function = reinterpret_cast<super::type_t>(value); }
	};

	template<class R, class... Args>
	struct safe_api_warpper<R(Args...)> {
	public:
		using type_t = R(__stdcall*)(Args...);
		safe_api_warpper() : function(nullptr) {};
		safe_api_warpper(type_t value) : function(value) {};
		operator bool() { return function != nullptr; };

		R operator()(Args... args)
		{
			if (!function) {
				return R();
			}
			return function(args...);
		}
		type_t function;
	};
} // namespace sdk::utils

namespace lfengine {
	struct TDefaultMessage {
		uint32_t recog;
		uint16_t ident;
		uint16_t param;
		uint16_t tag;
		uint16_t series;
		TDefaultMessage() = default;
		TDefaultMessage(uint32_t recog, uint16_t ident, uint16_t param, uint16_t tag, uint16_t series)
		{
			this->recog = recog;
			this->ident = ident;
			this->param = param;
			this->tag = tag;
			this->series = series;
		}
	};
	typedef struct TDefaultMessage* PTDefaultMessage;
	namespace client {

		typedef void(__stdcall* TAddChatText)(const char* src, int ForeColor, int BackColor);
		typedef void(__stdcall* TSendSocketFunc)(PTDefaultMessage Msg, char* AddData, int AddDataLen);
		typedef bool(__stdcall* TRecvSocketFunc)(uint32_t, void*, size_t*);
		typedef bool(__stdcall* TGetDllParam)(void*, size_t*);
		typedef int(__stdcall* TCompress)(void* dest, size_t* destLen, void* source, size_t sourceLen);
		typedef int(__stdcall* TUnCompress)(void* dest, size_t* destLen, void* source, size_t sourceLen);
		struct TAppFuncDef {
			TAddChatText    AddChatText;
			TSendSocketFunc SendSocket;
		};

		struct TAppFuncDefExt {
			TAddChatText    AddChatText = NULL;
			TSendSocketFunc SendSocket = NULL;
			TRecvSocketFunc RecvSocket = NULL;
			TGetDllParam    GetDllParam = NULL;
			TCompress       Compress = NULL;
			TUnCompress     UnCompress = NULL;
		};

		typedef struct TAppFuncDef* PAppFuncDef;
		typedef struct TAppFuncDefExt* PAppFuncDefExt;

		typedef bool(__stdcall* TClientInitFunc)(PAppFuncDef AppFunc);
		typedef void(__stdcall* TClientUnInitFunc)();
		typedef void(__stdcall* TClientHookRecvFunc)(PTDefaultMessage DefMsg, char* lpData, int InDataLen);
	}

	namespace rungate {

		struct TRunGatePlugClientInfo {
			uint64_t RecogId; // 对象ID
			int      MoveSpeed; // 移动速度
			int      AttackSpeed; // 攻击速度
			int      SpellSpeed; // 魔法速度
			char     Account[16]; // 帐户
			char     ChrName[16]; // 角色名
			int		 IPValue; // IP地址
			char     IpAddr[24]; // 客户端IP
			int      Port; // 客户端端口
			char     MacID[64]; // 机器码
			bool     IsActive; // 是否为活动连接
			bool     IsLoginNotice; // 客户端点击公告确定
			bool     IsPlayGame; // 背包，技能等均初始化完成，进入游戏中
			void* DataAdd; // 附加数据
			uint32_t DataLen; // 附加数据长度
			uint32_t Reseved2[100];
		};
		typedef struct TRunGatePlugClientInfo* PRunGatePlugClientInfo;
		typedef bool(__stdcall* TRunGatePlugGetClientInfoFunc)(int ClientID, PRunGatePlugClientInfo ClientInfo);
		typedef void(__stdcall* TRunGatePlugAddMainLogMsgFunc)(const char* lpData, int nLevel);
		typedef void(__stdcall* TRunGatePlugSendSocketFunc)(int ClientID, PTDefaultMessage DefMsg, const char* lpData, int DataLen);
		typedef bool(__stdcall* TRunGatePlugEncodeDecodeFunc)(char* lpInData, int InDataLen, char* lpOutData, int* OutDataLen);
		typedef void(__stdcall* TRunGatePlugCloseClientFunc)(int ClientID, int DelayTime, uint32_t Code);
		typedef void(__stdcall* TRunGatePlugSetClientPlugLoad)(int ClientID);
		typedef void(__stdcall* TRunGatePlugLockClientFunc)(int ClientID, int LockTime);

		struct TInitRecord {
			uint32_t                      MainFormHandle; // 主窗口句柄
			TRunGatePlugGetClientInfoFunc GetClientInfo; // 获取客户端连接的信息
			TRunGatePlugAddMainLogMsgFunc AddShowLog; // 在网关上显示日志
			TRunGatePlugSendSocketFunc    SendDataToClient; // 发送数据到客户端
			TRunGatePlugEncodeDecodeFunc  EncodeBuffer; // 将非可视字符编码为可视字符
			TRunGatePlugEncodeDecodeFunc  DecodeBuffer; // 可视化字符解码
			TRunGatePlugCloseClientFunc   CloseClient; // 关闭客户端连接
			TRunGatePlugLockClientFunc    LockClient; // 发送数据到M2Server
			TRunGatePlugSetClientPlugLoad SetClientPlugLoad; // 通知客端反外挂模块加载成功
		};

		typedef struct TInitRecord* PInitRecord;
		typedef struct TAppFuncDef* PAppFuncDef;

		typedef bool(__stdcall* TInitFunc)(PInitRecord InitRecord, bool IsReload);
		typedef void(__stdcall* TUnInitFunc)();
		typedef void(__stdcall* THookClientStartFunc)(int ClientID);
		typedef void(__stdcall* THookClientEndFunc)(int ClientID);
		typedef void(__stdcall* THookClientRunFunc)(int ClientID);
		typedef void(__stdcall* THookRecvClientPacketFunc)(
			int ClientID, PTDefaultMessage DefMsg, char* lpData, int DataLen, char* IsStopSend);
		typedef void(__stdcall* THookRecvM2PacketFunc)(
			int ClientID, PTDefaultMessage DefMsg, char* lpData, int DataLen, char* IsStopSend);
		typedef void(__stdcall* TShowPlugConfigFunc)();
	}// namespace rungate
} // namespace lfengine

#define WRAP(def) __declspec(selectany) sdk::utils::safe_api_warpper<def>

namespace lfengine::client {
	WRAP(TAddChatText) AddChatText;
	WRAP(TSendSocketFunc) SendSocket;
} // namespace lfengine::client

namespace lfengine::rungate {
	WRAP(TRunGatePlugAddMainLogMsgFunc) AddShowLog;
	WRAP(TRunGatePlugEncodeDecodeFunc) EncodeBuffer;
	WRAP(TRunGatePlugEncodeDecodeFunc) DecodeBuffer;
	WRAP(TRunGatePlugGetClientInfoFunc) GetClientInfo;
	WRAP(TRunGatePlugCloseClientFunc) CloseClient;
	WRAP(TRunGatePlugSetClientPlugLoad) SetClientPlugLoad;
	WRAP(TRunGatePlugSendSocketFunc) SendDataToClient;
	WRAP(TRunGatePlugLockClientFunc) LockClient;
} // namespace lfengine::rungate

#endif // _SDK_UTILS_H__
