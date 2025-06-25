#pragma once
#ifndef _SDK_UTILS_H__
#define _SDK_UTILS_H__
#include <stdint.h>
#include <vector>
#include <string>
#include <functional>
#include <winnt.h>
#include <intsafe.h>
#include <utility>
#include <array>
//#define NEW_M2_SUPPORT
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
    #ifdef NEW_M2_SUPPORT
		uint64_t recog;
	#else
		uint32_t recog;
	#endif
		uint16_t ident;
		uint16_t param;
		uint16_t tag;
		uint16_t series;
		TDefaultMessage() = default;

    #ifdef NEW_M2_SUPPORT
		TDefaultMessage(uint64_t recog, uint16_t ident, uint16_t param, uint16_t tag, uint16_t series)
    #else
		TDefaultMessage(uint32_t recog, uint16_t ident, uint16_t param, uint16_t tag, uint16_t series)
    #endif
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
		typedef BOOL(__stdcall* TRecvSocketFunc)(uint32_t, void*, size_t*);
		typedef BOOL(__stdcall* TGetDllParam)(void*, size_t*);
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

		typedef BOOL(__stdcall* TClientInitFunc)(PAppFuncDef AppFunc);
		typedef void(__stdcall* TClientUnInitFunc)();
		typedef void(__stdcall* TClientHookRecvFunc)(PTDefaultMessage DefMsg, char* lpData, int InDataLen);
	}

	namespace rungate {
        inline enum _Action_ID : int
        {
            ID_Move = 0,                   // 移动
            ID_Attack = 1,                 // 攻击
            ID_Magic = 2,                  // 魔法
            ID_Sitdown = 3,                // 挖肉
            ID_Turn = 4,                   // 转向

            ID_AttackToMove = 5,           // 攻击到移动
            ID_AttackToMagic = 6,          // 攻击到魔法
            ID_AttackToSitdown = 7,        // 攻击到挖肉
            ID_AttackToTurn = 8,           // 攻击到转向

            ID_MoveToAttack = 9,           // 移动到攻击
            ID_MoveToMagic = 10,           // 移动到魔法
            ID_MoveToSitdown = 11,         // 移动到挖肉
            ID_MoveToTurn = 12,            // 移动到转向

            ID_MagicToMove = 13,           // 魔法到移动
            ID_MagicToAttack = 14,         // 魔法到攻击
            ID_MagicToSitdown = 15,        // 魔法到挖肉
            ID_MagicToTurn = 16,           // 魔法到转向

            ID_SitdownToMove = 17,         // 挖肉到移动
            ID_SitdownToAttack = 18,       // 挖肉到攻击
            ID_SitdownToMagic = 19,        // 挖肉到魔法
            ID_SitdownToTurn = 20,         // 挖肉到转向

            ID_TurnToMove = 21,            // 转向到移动
            ID_TurnToAttack = 22,          // 转向到攻击
            ID_TurnToMagic = 23,           // 转向到魔法
            ID_TurnToSitdown = 24,         // 转向到挖肉
        }Action_ID;
        const int CUSTOM_MAGIC_COUNT = 300;
        enum _Skill_ID : int
        {
            CM_SPELL = 3017,                            // 施魔法
            CM_HORSERUN = 3009,                         // 骑马
            CM_TURN = 3010,                             // 转身(方向改变)
            CM_WALK = 3011,                             // 走
            CM_SITDOWN = 3012,                          // 挖(蹲下)
            CM_RUN = 3013,                              // 跑
            CM_HIT = 3014,                              // 普通物理近身攻击
            CM_HEAVYHIT = 3015,                         // 跳起来打的动作
            CM_BIGHIT = 3016,                           // 强攻
            CM_POWERHIT = 3018,                         // 攻杀
            CM_LONGHIT = 3019,                          // 刺杀
            CM_WIDEHIT = 3024,                          // 半月
            CM_FIREHIT = 3025,                          // 烈火
            CM_CRSHIT = 3036,                           // 抱月刀 双龙斩 ID=40
            CM_TWNHIT = 3037,                           // 龙影剑法      ID=42
            CM_43HIT = 3043,                            // 雷霆剑法     ID=43
            CM_SWORDHIT = 3056,                         // 逐日剑法     ID=56
            CM_113HIT = 3113,                           // 断空斩 chongchong 2018-01-29
            CM_115HIT = 3115,                           // 血魄一击(战) chongchong 2018-01-30
            CM_60HIT = 3060,                            //破魂斩
            CM_61HIT = 3061,                            //劈星斩
            CM_62HIT = 3062,                            //雷霆一击
            CM_66HIT = 3066,                            // 开天斩
            CM_66HIT1 = 3166,                           // 开天斩轻击
            CM_100HIT = 3100,                           // 追心刺
            CM_101HIT = 3101,                           // 三绝杀
            CM_102HIT = 3102,                           // 断岳斩
            CM_103HIT = 3103,                           // 横扫千军
            CM_CUSTOM_HIT001 = 6000,                    // 自定义技能物理攻击1
            CM_CUSTOM_HIT300 = 6299,
        };

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
			BOOL     IsActive; // 是否为活动连接
			BOOL     IsLoginNotice; // 客户端点击公告确定
			BOOL     IsPlayGame; // 背包，技能等均初始化完成，进入游戏中
			void* DataAdd; // 附加数据
			uint32_t DataLen; // 附加数据长度
			uint32_t Reseved2[30];
		};
		typedef struct TRunGatePlugClientInfo* PRunGatePlugClientInfo;
    #pragma pack(push, 1) 
		typedef BOOL(__stdcall* TRunGatePlugGetClientInfoFunc)(int ClientID, PRunGatePlugClientInfo ClientInfo);
		typedef void(__stdcall* TRunGatePlugAddMainLogMsgFunc)(const char* lpData, int nLevel);
		typedef void(__stdcall* TRunGatePlugSendSocketFunc)(int ClientID, PTDefaultMessage DefMsg, const char* lpData, int DataLen);
		typedef BOOL(__stdcall* TRunGatePlugEncodeDecodeFunc)(char* lpInData, int InDataLen, char* lpOutData, int* OutDataLen);
		typedef void(__stdcall* TRunGatePlugCloseClientFunc)(int ClientID, int DelayTime, uint32_t Code);
		typedef void(__stdcall* TRunGatePlugSetClientPlugLoad)(int ClientID);
		typedef void(__stdcall* TRunGatePlugLockClientFunc)(int ClientID, int LockTime);
        //* 得到当前的动作信息 2024-06-15
        // RealTime: 当前动作的实际间隔; LimiteTime: 当前动作的限制间隔; ActionMode: 当前的动作类型，参考ACTION_ID_DEF
        //TRungatePlugGetActionInfo = function(ClientID: Integer; var RealTime, LimiteTime: DWORD; var ActionMode : Integer) : BOOL; stdcall;

        // 检查是否为技能开关 2024-06-15
        //TRungatePlugCheckIsSpellSwitch = function(MagicID: Integer) : BOOL; stdcall;
		//typedef BOOL(__stdcall* TRungatePlugGetActionInfo)(int ClientID, uint32_t& real_time, uint32_t& limite_time, int& action_mode);
		//typedef BOOL(__stdcall* TRungatePlugCheckIsSpellSwitch)(int magic_id);
        typedef BOOL(__stdcall* TRungatePlugGetActionInfo)(int ClientID, DWORD* real_time, DWORD* limite_time, int* action_mode);
        typedef BOOL(__stdcall* TRungatePlugCheckIsSpellSwitch)(int magic_id);

        /*
          PInitRecord = ^TInitRecord;
          TInitRecord = record
            MainFormHandle: HWND;

            GetClientInfo: TRunGatePlugGetClientInfoFunc;             // 获取客户端连接的信息
            AddShowLog: TRunGatePlugAddMainLogMsgFunc;                // 在网关上显示日志

            SendDataToClient: TRunGatePlugSendSocketFunc;             // 发送数据到客户端

            EncodeBuffer: TRunGatePlugEncodeDecodeFunc;               // 将非可视字符编码为可视字符
            DecodeBuffer: TRunGatePlugEncodeDecodeFunc;               // 将非可视字符编码为可视字符

            CloseClient: TRunGatePlugCloseClientFunc;                 // +++++++++++++++关闭内核连接
            LockClient: TRunGatePlugLockClient;                       // 锁定用户，时间秒

            SetClientPlugLoad: TRunGatePlugSetClientPlugLoad;         // 通知客端反外挂模块加载成功

            //* 得到当前的动作信息 2024-06-16
            GetActionInfo: TRungatePlugGetActionInfo;  // ++++++++++++++++++++++++++++++ 2024-06-16

            // 检查是否为技能开关 if DefMsg.Ident = CM_SPELL then MagicID := DefMsg.Tag, CM_SPELL为判断魔法间隔；if CheckIsSpellSwitch(MagicID) then 开关技能请求不算到魔法中，不计算魔法间隔
            CheckIsSpellSwitch: TRungatePlugCheckIsSpellSwitch;       // ++++++++++++++++++++++++++++++ 2024-06-15

            Reseved: array[0..29] of Pointer;                         // ++++++++++++++++++++++++++++++ 2024-06-15
          end;
        */
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
            //* 得到当前的动作信息 2024-06-16
            //TRungatePlugGetActionInfo     GetActionInfo;  // ++++++++++++++++++++++++++++++ 2024-06-16
            // 检查是否为技能开关 
            // if DefMsg.Ident = CM_SPELL then MagicID := DefMsg.Tag, CM_SPELL为判断魔法间隔；
            // if CheckIsSpellSwitch(MagicID) then 开关技能请求不算到魔法中，不计算魔法间隔
            //TRungatePlugCheckIsSpellSwitch CheckIsSpellSwitch;       // ++++++++++++++++++++++++++++++ 2024-06-15
            void* Reseved2[30] = { 0 };                   // ++++++++++++++++++++++++++++++ 2024-06-15
		};

    #pragma pack(pop) 
		typedef struct TInitRecord* PInitRecord;
		typedef struct TAppFuncDef* PAppFuncDef;

		typedef BOOL(__stdcall* TInitFunc)(PInitRecord InitRecord, BOOL IsReload);
		typedef void(__stdcall* TUnInitFunc)();
		typedef void(__stdcall* THookClientStartFunc)(int ClientID);
		typedef void(__stdcall* THookClientEndFunc)(int ClientID);
		typedef void(__stdcall* THookClientRunFunc)(int ClientID);
		typedef int(__stdcall* THookRecvClientPacketFunc)(
			int ClientID, PTDefaultMessage DefMsg, char* lpData, int DataLen, BOOL& IsStopSend);
		typedef int(__stdcall* THookRecvM2PacketFunc)(
			int ClientID, PTDefaultMessage DefMsg, char* lpData, int DataLen, BOOL& IsStopSend);
		typedef void(__stdcall* TShowPlugConfigFunc)();

        inline std::string GetClientActionName(int action) {
            // 使用constexpr数组映射动作名称（C++17支持constexpr字符串）
            static constexpr std::array<const char*, 25> ACTION_NAMES = {
                "移动",
                "攻击",
                "魔法",
                "挖肉",
                "转向",

                "攻击→移动",
                "攻击→魔法",
                "攻击→挖肉",
                "攻击→转向",

                "移动→攻击",
                "移动→魔法",
                "移动→挖肉",
                "移动→转向",

                "魔法→移动",
                "魔法→攻击",
                "魔法→挖肉",
                "魔法→转向",

                "挖肉→移动",
                "挖肉→攻击",
                "挖肉→魔法",
                "挖肉→转向",

                "转向→移动",
                "转向→攻击",
                "转向→魔法",
                "转向→挖肉"
            };

            // 边界检查（使用枚举值范围）
            if (action >= ID_Move && action <= ID_TurnToSitdown) {
                return ACTION_NAMES[action];
            }
            return "";  // 越界返回空字符串
        }
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
	WRAP(TRungatePlugGetActionInfo) GetActionInfo;
	WRAP(TRungatePlugCheckIsSpellSwitch) CheckIsSpellSwitch;
} // namespace lfengine::rungate

#endif // _SDK_UTILS_H__
