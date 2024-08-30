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

namespace lfengine::client1 {
	struct TDefaultMessage {
		uint64_t recog;
		uint16_t ident;
		uint16_t param;
		uint16_t tag;
		uint16_t series;
		TDefaultMessage() = default;
		TDefaultMessage(uint64_t recog, uint16_t ident, uint16_t param, uint16_t tag, uint16_t series)
		{
			this->recog = recog;
			this->ident = ident;
			this->param = param;
			this->tag = tag;
			this->series = series;
		}
	};
	typedef struct TDefaultMessage* PTDefaultMessage;

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
} // namespace lfengine::client



// C++ equivalent of Delphi's Pointer type
typedef void* Pointer;

namespace lfengine::rungate {
	struct TDefaultMessage {
		uint64_t recog;
		uint16_t ident;
		uint16_t param;
		uint16_t tag;
		uint16_t series;
	};
	typedef struct TDefaultMessage* PTDefaultMessage;
	struct TRunGatePlugClientInfo {
		uint64_t RecogId; // 对象ID
		int      MoveSpeed; // 移动速度
		int      AttackSpeed; // 攻击速度
		int      SpellSpeed; // 魔法速度
		char     Account[16]; // 帐户
		char     ChrName[16]; // 角色名
		char     IpAddr[16]; // 客户端IP
		char     MacID[64]; // 机器码
		int      port; // 客户端端口
		bool     IsActive; // 是否为活动连接
		bool     IsLoginNotice; // 客户端点击公告确定
		bool     IsPlayGame; // 背包，技能等均初始化完成，进入游戏中
		void* DataAdd; // 附加数据
		uint32_t DataLen; // 附加数据长度
		uint32_t Reseved2[32];
	};
	typedef struct TRunGatePlugClientInfo* PRunGatePlugClientInfo;
	typedef bool(__stdcall* TRunGatePlugGetClientInfoFunc)(int ClientID, PRunGatePlugClientInfo ClientInfo);
	typedef void(__stdcall* TRunGatePlugAddMainLogMsgFunc)(const char* lpData, int nLevel);
	typedef void(__stdcall* TRunGatePlugSendSocketFunc)(int              ClientID,
		PTDefaultMessage DefMsg,
		char* lpData,
		int              DataLen);
	typedef bool(__stdcall* TRunGatePlugEncodeDecodeFunc)(char* lpInData,
		int   InDataLen,
		char* lpOutData,
		int* OutDataLen);
	typedef void(__stdcall* TRunGatePlugCloseClientFunc)(int ClientID, int DelayTime, uint32_t Code);
	typedef void(__stdcall* TRunGatePlugSetClientPlugLoad)(int ClientID);

	struct TInitRecord {
		uint32_t                      AppHandle; // 应用程序Handle
		uint32_t                      MainFormHandle; // 主窗体Handle
		uint32_t                      Version; // 版本号
		char                          RootDir[256]; // 程序根目录
		TRunGatePlugAddMainLogMsgFunc AddShowLog; // 在网关上显示日志
		TRunGatePlugEncodeDecodeFunc  EncodeBuffer; // 将非可视字符编码为可视字符
		TRunGatePlugEncodeDecodeFunc  DecodeBuffer; // 可视化字符解码
		TRunGatePlugGetClientInfoFunc GetClientInfo; // 获取客户端连接的信息
		TRunGatePlugCloseClientFunc   CloseClient; // 关闭客户端连接
		TRunGatePlugSetClientPlugLoad SetClientPlugLoad; // 通知客端反外挂模块加载成功
		TRunGatePlugSendSocketFunc    SendDataToClient; // 发送数据到客户端
		TRunGatePlugSendSocketFunc    SendDataToM2; // 发送数据到M2Server
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




	typedef int* _TList;
	typedef int* _TStringList;
	typedef int* _TObjectType;
	typedef int* _TMemoryStream;
	typedef int* _TMenuItem;
	typedef int* _TIniFile;
	typedef int* _TMagicACList;
	typedef int* _TEnvironment;
	typedef int* _TBaseObject;
	typedef int* _TSmartObject;
	typedef int* _TPlayObject;
	typedef int* _TDummyObject;
	typedef int* _THeroObject;
	typedef int* _TNormNpc;
	typedef int* _TGuild;
	typedef void* pTUserItem;
	typedef void* pTUserMagic;



	typedef void* (*TMemory_Alloc_Ptr)(std::size_t);
	typedef void (*TMemory_Free_Ptr)(void*);
	typedef void* (*TMemory_Realloc_Ptr)(void*, std::size_t);

	typedef _TList* (*TList_Create_Ptr)();
	typedef void (*TList_Free_Ptr)(_TList*);
	typedef int(*TList_Count_Ptr)(_TList*);
	typedef void (*TList_Clear_Ptr)(_TList*);
	typedef void (*TList_Add_Ptr)(_TList*, void*);
	typedef void (*TList_Insert_Ptr)(_TList*, int, void*);
	typedef void (*TList_Remove_Ptr)(_TList*, void*);
	typedef void (*TList_Delete_Ptr)(_TList*, int);
	typedef void* (*TList_GetItem_Ptr)(_TList*, int);
	typedef void (*TList_SetItem_Ptr)(_TList*, int, void*);
	typedef int(*TList_IndexOf_Ptr)(_TList*, void*);
	typedef void (*TList_Exchange_Ptr)(_TList*, int, int);
	typedef void (*TList_CopyTo_Ptr)(_TList*, _TList*);


	typedef _TStringList* (*TStrList_Create_Ptr)();
	typedef void (*TStrList_Free_Ptr)(_TStringList*);
	typedef bool(*TStrList_GetCaseSensitive_Ptr)(_TStringList*);
	typedef void (*TStrList_SetCaseSensitive_Ptr)(_TStringList*, bool);
	typedef bool(*TStrList_GetSorted_Ptr)(_TStringList*);
	typedef void (*TStrList_SetSorted_Ptr)(_TStringList*, bool);
	typedef bool(*TStrList_GetDuplicates_Ptr)(_TStringList*);
	typedef void (*TStrList_SetDuplicates_Ptr)(_TStringList*, bool);
	typedef int(*TStrList_Count_Ptr)(_TStringList*);
	typedef bool(*TStrList_GetText_Ptr)(_TStringList*, char*, size_t&);
	typedef void (*TStrList_SetText_Ptr)(_TStringList*, char*, size_t);
	typedef void (*TStrList_Add_Ptr)(_TStringList*, char*);
	typedef void (*TStrList_AddObject_Ptr)(_TStringList*, char*, _TObjectType*);
	typedef void (*TStrList_Insert_Ptr)(_TStringList*, int, char*);
	typedef void (*TStrList_InsertObject_Ptr)(_TStringList*, int, char*, _TObjectType*);
	typedef void (*TStrList_Remove_Ptr)(_TStringList*, char*);
	typedef void (*TStrList_Delete_Ptr)(_TStringList*, int);
	typedef bool(*TStrList_GetItem_Ptr)(_TStringList*, int, char*, size_t&);
	typedef void (*TStrList_SetItem_Ptr)(_TStringList*, int, char*);
	typedef _TObjectType* (*TStrList_GetObject_Ptr)(_TStringList*, int);
	typedef void (*TStrList_SetObject_Ptr)(_TStringList*, int, _TObjectType*);
	typedef int(*TStrList_IndexOf_Ptr)(_TStringList*, char*);
	typedef int(*TStrList_IndexOfObject_Ptr)(_TStringList*, _TObjectType*);
	typedef bool(*TStrList_Find_Ptr)(_TStringList*, char*, int&);
	typedef void (*TStrList_Exchange_Ptr)(_TStringList*, int, int);
	typedef void (*TStrList_LoadFromFile_Ptr)(_TStringList*, char*);
	typedef void (*TStrList_SaveToFile_Ptr)(_TStringList*, char*);
	typedef void (*TStrList_CopyTo_Ptr)(_TStringList*, _TStringList*);


	// 创建内存流
	typedef _TMemoryStream* (*TMemStream_Create_Ptr)();
	// 释放内存流
	typedef void (*TMemStream_Free_Ptr)(_TMemoryStream*);
	// 取得大小
	typedef int64_t(*TMemStream_GetSize_Ptr)(_TMemoryStream*);
	// 设置大小
	typedef void (*TMemStream_SetSize_Ptr)(_TMemoryStream*, int);
	// 清空
	typedef void (*TMemStream_Clear_Ptr)(_TMemoryStream*);
	// 把内存流的数据读取到 Buffer 中
	typedef int(*TMemStream_Read_Ptr)(_TMemoryStream*, char*, int);
	// 向内存流写入数据
	typedef int(*TMemStream_Write_Ptr)(_TMemoryStream*, char*, int);
	// 指针定位 SeekOrigin: 0 (从头开始), 1 (当前位置开始), 2 (从尾部开始)
	typedef int(*TMemStream_Seek_Ptr)(_TMemoryStream*, int, uint16_t);
	// 获取流数据的内存指针
	typedef Pointer(*TMemStream_Memory_Ptr)(_TMemoryStream*);
	// 获取指针位置
	typedef int64_t(*TMemStream_GetPosition_Ptr)(_TMemoryStream*);
	// 指针定位 (定到第几个字节)
	typedef void (*TMemStream_SetPosition_Ptr)(_TMemoryStream*, int64_t);
	// 从文件载入
	typedef void (*TMemStream_LoadFromFile_Ptr)(_TMemoryStream*, char*);
	// 保存到文件
	typedef void (*TMemStream_SaveToFile_Ptr)(_TMemoryStream*, char*);




	typedef void (*TNotifyEventEx)(void* sender, void* eventArgs);

	// Function pointer type definitions for TMenu
	// 获取主菜单
	typedef _TMenuItem* (*TMenu_GetMainMenu_Ptr)();
	// 获取控制菜单
	typedef _TMenuItem* (*TMenu_GetControlMenu_Ptr)();
	// 获取查看菜单
	typedef _TMenuItem* (*TMenu_GetViewMenu_Ptr)();
	// 获取选项菜单
	typedef _TMenuItem* (*TMenu_GetOptionMenu_Ptr)();
	// 获取管理菜单
	typedef _TMenuItem* (*TMenu_GetManagerMenu_Ptr)();
	// 获取工具菜单
	typedef _TMenuItem* (*TMenu_GetToolsMenu_Ptr)();
	// 获取帮助菜单
	typedef _TMenuItem* (*TMenu_GetHelpMenu_Ptr)();
	// 获取插件菜单
	typedef _TMenuItem* (*TMenu_GetPluginMenu_Ptr)();
	// 获取子菜单数量
	typedef int(*TMenu_Count_Ptr)(_TMenuItem*);
	// 获取某个子菜单
	typedef _TMenuItem* (*TMenu_GetItems_Ptr)(_TMenuItem*, int);
	// 添加菜单
	typedef _TMenuItem* (*TMenu_Add_Ptr)(int, _TMenuItem*, char*, int, TNotifyEventEx);
	// 插入菜单
	typedef _TMenuItem* (*TMenu_Insert_Ptr)(int, _TMenuItem*, int, char*, int, TNotifyEventEx);
	// 获取菜单标题
	typedef bool(*TMenu_GetCaption_Ptr)(_TMenuItem*, char*, int&);
	// 设置菜单标题
	typedef void (*TMenu_SetCaption_Ptr)(_TMenuItem*, char*);
	// 获取菜单可用
	typedef bool(*TMenu_GetEnabled_Ptr)(_TMenuItem*);
	// 设置菜单可用
	typedef void (*TMenu_SetEnabled_Ptr)(_TMenuItem*, bool);
	// 获取菜单可见
	typedef bool(*TMenu_GetVisible_Ptr)(_TMenuItem*);
	// 设置菜单可见
	typedef void (*TMenu_SetVisible_Ptr)(_TMenuItem*, bool);
	// 获取菜单选中状态
	typedef bool(*TMenu_GetChecked_Ptr)(_TMenuItem*);
	// 设置菜单选中状态
	typedef void (*TMenu_SetChecked_Ptr)(_TMenuItem*, bool);
	// 获取菜单是否为单选
	typedef bool(*TMenu_GetRadioItem_Ptr)(_TMenuItem*);
	// 设置菜单是否为单选
	typedef void (*TMenu_SetRadioItem_Ptr)(_TMenuItem*, bool);
	// 获取菜单单选分组
	typedef int(*TMenu_GetGroupIndex_Ptr)(_TMenuItem*);
	// 设置菜单单选分组
	typedef void (*TMenu_SetGroupIndex_Ptr)(_TMenuItem*, int);
	// 获取附加数据
	typedef int(*TMenu_GetTag_Ptr)(_TMenuItem*);
	// 设置附加数据
	typedef void (*TMenu_SetTag_Ptr)(_TMenuItem*, int);

	// Function pointer type definitions for TIniFile
	// 创建Ini对象
	typedef _TIniFile* (*TIniFile_Create_Ptr)(char*);
	// 释放ini对象
	typedef void (*TIniFile_Free_Ptr)(_TIniFile*);
	// 判断区段是否存在
	typedef bool(*TIniFile_SectionExists_Ptr)(_TIniFile*, char*);
	// 判断键是否存在
	typedef bool(*TIniFile_ValueExists_Ptr)(_TIniFile*, char*, char*);
	// 读取文本
	typedef bool(*TIniFile_ReadString_Ptr)(_TIniFile*, char*, char*, char*, char*, int&);
	// 写入文本
	typedef void (*TIniFile_WriteString_Ptr)(_TIniFile*, char*, char*, char*);
	// 读取整数
	typedef int(*TIniFile_ReadInteger_Ptr)(_TIniFile*, char*, char*, int);
	// 写入整数
	typedef void (*TIniFile_WriteInteger_Ptr)(_TIniFile*, char*, char*, int);
	// 读取布尔值
	typedef bool(*TIniFile_ReadBool_Ptr)(_TIniFile*, char*, char*, bool);
	// 写入布尔值
	typedef void (*TIniFile_WriteBool_Ptr)(_TIniFile*, char*, char*, bool);



	// 技能破防百分比列表数量
	typedef int(*TMagicACList_Count_Ptr)(_TMagicACList*);
	// 取单个元素
	typedef _TMagicACList(*TMagicACList_GetItem_Ptr)(_TMagicACList*, int);
	// 根据技能得到破防百分比信息
	typedef _TMagicACList(*TMagicACList_FindByMagIdx_Ptr)(_TMagicACList*, int);

	// Function pointer type definitions for TMapManager
	// 根据地图名得到地图对象
	typedef _TEnvironment* (*TMapManager_FindMap_Ptr)(char*);
	// 得到地图列表; 返回值中每个元素为：_TEnvironment
	typedef _TList* (*TMapManager_GetMapList_Ptr)();

	// Function pointer type definitions for _TEnvironment
	// 地图名称
	typedef bool(*TEnvir_GetMapName_Ptr)(_TEnvironment*, char*, int&);
	// 地图描述
	typedef bool(*TEnvir_GetMapDesc_Ptr)(_TEnvironment*, char*, int&);
	// 地图宽度
	typedef int(*TEnvir_GetWidth_Ptr)(_TEnvironment*);
	// 地图高度
	typedef int(*TEnvir_GetHeight_Ptr)(_TEnvironment*);
	// 小地图
	typedef int(*TEnvir_GetMinMap_Ptr)(_TEnvironment*);
	// 是否主地图
	typedef bool(*TEnvir_IsMainMap_Ptr)(_TEnvironment*);
	// 主地图名
	typedef bool(*TEnvir_GetMainMapName_Ptr)(_TEnvironment*, char*, int&);
	// 是否动态镜像地图
	typedef bool(*TEnvir_IsMirrMap_Ptr)(_TEnvironment*);
	// 动态镜像地图创建时间
	typedef int (*TEnvir_GetMirrMapCreateTick_Ptr)(_TEnvironment*);
	// 动态镜像地图存活时间
	typedef int (*TEnvir_GetMirrMapSurvivalTime_Ptr)(_TEnvironment*);
	// 动态地图退到哪个地图
	typedef bool(*TEnvir_GetMirrMapExitToMap_Ptr)(_TEnvironment*, char*, int&);
	// 动态镜像地图小地图编号
	typedef int(*TEnvir_GetMirrMapMinMap_Ptr)(_TEnvironment*);
	// 动态镜像地图是否一直显示时间
	typedef bool(*TEnvir_GetAlwaysShowTime_Ptr)(_TEnvironment*);
	// 是否副本地图
	typedef bool(*TEnvir_IsFBMap_Ptr)(_TEnvironment*);
	// 副本地图名
	typedef bool(*TEnvir_GetFBMapName_Ptr)(_TEnvironment*, char*, int&);
	// 副本进入限制 (0:队友必须有三职业; 1:不限制职业，队友均可进; 2:只允许自己; 3:允许行会进入)
	typedef int(*TEnvir_GetFBEnterLimit_Ptr)(_TEnvironment*);
	// 副本地图是否创建
	typedef bool(*TEnvir_GetFBCreated_Ptr)(_TEnvironment*);
	// 副本地图创建时间
	typedef int (*TEnvir_GetFBCreateTime_Ptr)(_TEnvironment*);
	// 获取地图是否设置某参数
	// 如地图有参数: FIGHT4，调用 GetMapParam(Envir, 'FIGHT4')，则返回True
	// 如地图有参数：INCGAMEGOLD(1/10)，调用 GetMapParam(Envir, 'INCGAMEGOLD')，则返回True
	typedef bool(*TEnvir_GetMapParam_Ptr)(_TEnvironment*, char*);
	// 获取地图设置某参数值
	// 如地图有参数：INCGAMEGOLD(1/10)，调用 GetMapParam(Envir, 'INCGAMEGOLD')，则返回1/10
	typedef bool(*TEnvir_GetMapParamValue_Ptr)(_TEnvironment*, char*, char*, int&);
	// 地图点是否可达，boFlag = False时，会判断该坐标点是否有角色占据
	typedef bool(*TEnvir_CheckCanMove_Ptr)(_TEnvironment*, int, int, bool);
	// 判断地图上以坐标(nX, nY)为空中，以nRange为半径的矩形范围内，是否有Obj对象
	typedef bool(*TEnvir_IsValidObject_Ptr)(_TEnvironment*, int, int, int, _TObjectType*);
	// 获取地图上某坐标的物品列表
	typedef int(*TEnvir_GetItemObjects_Ptr)(_TEnvironment*, int, int, _TList*);
	// 获取地图上某坐标的角色列表
	typedef int(*TEnvir_GetBaseObjects_Ptr)(_TEnvironment*, int, int, bool, _TList*);
	// 获取地图上某坐标的人物列表
	typedef int(*TEnvir_GetPlayObjects_Ptr)(_TEnvironment*, int, int, bool, _TList*);


	//-------------------------------------------------------------------------------------------------------------------------------------------------------
	//---------------------------------------------- M2引擎相关函数 ----------------------------------------------------------------------------------------
	//-------------------------------------------------------------------------------------------------------------------------------------------------------
  // 取M2版本号
	typedef bool(*TM2Engine_GetVersion)(char*, DWORD&);
	// 取版本号
	typedef int(*TM2Engine_GetVersionInt)();
	// 取主窗体主柄
	typedef HANDLE(*TM2Engine_GetMainFormHandle)();
	// 设置主窗体标题
	typedef void (*TM2Engine_SetMainFormCaption)(char*);
	// 主程序所在目录
	typedef bool(*TM2Engine_GetAppDir)(char*, DWORD&);
	// 获取服务器的Ini对象 (0:!Setup.txt; 1:String.ini}
	typedef _TIniFile* (*TM2Engine_GetGlobalIniFile)(int);
	// 获得其他文件或目录
	typedef bool(*TM2Engine_GetOtherFileDir)(int, char*, DWORD&);
	// M2输出信息
	typedef void (*TM2Engine_MainOutMessage)(char*, bool);
	// 读取全局I变量
	typedef int(*TM2Engine_GetGlobalVarI)(int);
	// 设置全局I变量
	typedef bool(*TM2Engine_SetGlobalVarI)(int, int);
	// 读取全局G变量
	typedef int(*TM2Engine_GetGlobalVarG)(int);
	// 设置全局G变量
	typedef bool(*TM2Engine_SetGlobalVarG)(int, int);
	// 读取全局A变量
	typedef bool(*TM2Engine_GetGlobalVarA)(int, char*, DWORD&);
	// 设置全局A变量
	typedef bool(*TM2Engine_SetGlobalVarA)(int, char*);
	// 编码
	typedef bool(*TM2Engine_EncodeBuffer)(char*, DWORD, char*, DWORD&);
	// 解码
	typedef bool(*TM2Engine_DecodeBuffer)(char*, DWORD, char*, DWORD&);
	// 压缩编码
	typedef bool(*TM2Engine_ZLibEncodeBuffer)(char*, DWORD, char*, DWORD&);
	// 压缩解码
	typedef bool(*TM2Engine_ZLibDecodeBuffer)(char*, DWORD, char*, DWORD&);
	// 加密
	typedef bool(*TM2Engine_EncryptBuffer)(char*, DWORD, char*, DWORD&);
	// 解密
	typedef bool(*TM2Engine_DecryptBuffer)(char*, DWORD, char*, DWORD&);
	// 密码加密，不同的电脑得到不同的结果
	typedef bool(*TM2Engine_EncryptPassword)(char*, char*, DWORD&);
	// 密码解密
	typedef bool(*TM2Engine_DecryptPassword)(char*, char*, DWORD&);
	// 根据物品StdMode得到物品装备位置
	typedef int(*TM2Engine_GetTakeOnPosition)(int);
	// 检查物品是否有某个绑定类型 (TUserItem.btBindOption)
	typedef bool(*TM2Engine_CheckBindType)(uint8_t, uint8_t);
	// 设置物品某个绑定类型
	typedef void (*TM2Engine_SetBindValue)(uint8_t&, uint8_t, bool);
	// 根据单字节颜色得到RGB颜色
	typedef DWORD(*TM2Engine_GetRGB)(uint8_t);



	//-------------------------------------------------------------------------------------------------------------------------------------------------------
	//---------------------------------------------- TBaseObject对象 ----------------------------------------------------------------------------------------
	//-------------------------------------------------------------------------------------------------------------------------------------------------------

  // 名称
	typedef bool(*TBaseObject_GetChrName_Ptr)(_TBaseObject*, char*, DWORD&);
	// 设置名称(不能设置人物、英雄)
	typedef bool(*TBaseObject_SetChrName_Ptr)(_TBaseObject*, char*);
	// 刷新名称到客户端
	typedef void (*TBaseObject_RefShowName_Ptr)(_TBaseObject*);
	// 刷新名称颜色 PKPoint等改变时
	typedef void (*TBaseObject_RefNameColor_Ptr)(_TBaseObject*);
	// 获取性别
	typedef uint8_t(*TBaseObject_GetGender_Ptr)(_TBaseObject*);
	// 设置性别
	typedef bool(*TBaseObject_SetGender_Ptr)(_TBaseObject*, uint8_t);
	// 获取职业
	typedef uint8_t(*TBaseObject_GetJob_Ptr)(_TBaseObject*);
	// 设置职业
	typedef bool(*TBaseObject_SetJob_Ptr)(_TBaseObject*, uint8_t);
	// 获取发型
	typedef uint8_t(*TBaseObject_GetHair_Ptr)(_TBaseObject*);
	// 设置发型
	typedef void (*TBaseObject_SetHair_Ptr)(_TBaseObject*, uint8_t);
	// 所在地图
	typedef _TEnvironment* (*TBaseObject_GetEnvir_Ptr)(_TBaseObject*);
	// 所在地图
	typedef bool(*TBaseObject_GetMapName_Ptr)(_TBaseObject*, char*, DWORD&);
	// 坐标X
	typedef int(*TBaseObject_GetCurrX_Ptr)(_TBaseObject*);
	// 坐标Y
	typedef int(*TBaseObject_GetCurrY_Ptr)(_TBaseObject*);
	// 当前方向
	typedef uint8_t(*TBaseObject_GetDirection_Ptr)(_TBaseObject*);
	// 回城地图
	typedef bool(*TBaseObject_GetHomeMap_Ptr)(_TBaseObject*, char*, DWORD&);
	// 回城坐标X
	typedef int(*TBaseObject_GetHomeX_Ptr)(_TBaseObject*);
	// 回城坐标Y
	typedef int(*TBaseObject_GetHomeY_Ptr)(_TBaseObject*);
	// 权限等级
	typedef uint8_t(*TBaseObject_GetPermission_Ptr)(_TBaseObject*);
	// 设置权限等级
	typedef void (*TBaseObject_SetPermission_Ptr)(_TBaseObject*, uint8_t);
	// 是否死亡
	typedef bool(*TBaseObject_GetDeath_Ptr)(_TBaseObject*);
	// 死亡时间
	typedef DWORD(*TBaseObject_GetDeathTick_Ptr)(_TBaseObject*);
	// 是否死亡并清理
	typedef bool(*TBaseObject_GetGhost_Ptr)(_TBaseObject*);
	// 清理时间
	typedef DWORD(*TBaseObject_GetGhostTick_Ptr)(_TBaseObject*);
	// 杀死并清掉
	typedef void (*TBaseObject_MakeGhost_Ptr)(_TBaseObject*);
	// 复活
	typedef void (*TBaseObject_ReAlive_Ptr)(_TBaseObject*);
	// 类型
	typedef uint8_t(*TBaseObject_GetRaceServer_Ptr)(_TBaseObject*);
	// Appr
	typedef uint16_t(*TBaseObject_GetAppr_Ptr)(_TBaseObject*);
	// RaceImg
	typedef uint8_t(*TBaseObject_GetRaceImg_Ptr)(_TBaseObject*);
	// 状态
	typedef int(*TBaseObject_GetCharStatus_Ptr)(_TBaseObject*);
	// 状态改变
	typedef void (*TBaseObject_SetCharStatus_Ptr)(_TBaseObject*, int);
	// 发送状态改变
	typedef void (*TBaseObject_StatusChanged_Ptr)(_TBaseObject*);
	// 获取饥饿点
	typedef int(*TBaseObject_GetHungerPoint_Ptr)(_TBaseObject*);
	// 设置饥饿点
	typedef void (*TBaseObject_SetHungerPoint_Ptr)(_TBaseObject*, int);
	// 是否为内功怪
	typedef bool(*TBaseobject_IsNGMonster_Ptr)(_TBaseObject*);
	// 是否假人
	typedef bool(*TBaseObject_IsDummyObject_Ptr)(_TBaseObject*);
	// 获取视觉范围
	typedef int(*TBaseObject_GetViewRange_Ptr)(_TBaseObject*);
	// 设置视觉范围
	typedef void (*TBaseObject_SetViewRange_Ptr)(_TBaseObject*, int);
	// 原始属性
	typedef bool(*TBaseObject_GetAbility_Ptr)(_TBaseObject*, void*);
	// 最终属性
	typedef bool(*TBaseObject_GetWAbility_Ptr)(_TBaseObject*, void*);
	// 设置属性
	typedef void (*TBaseObject_SetWAbility_Ptr)(_TBaseObject*, void*);
	// 宝宝列表
	typedef _TList* (*TBaseObject_GetSlaveList_Ptr)(_TBaseObject*);
	// 主人
	typedef _TBaseObject* (*TBaseObject_GetMaster_Ptr)(_TBaseObject*);
	// 最上层主人
	typedef _TBaseObject* (*TBaseObject_GetMasterEx_Ptr)(_TBaseObject*);
	// 是否无敌模式
	typedef bool(*TBaseObject_GetSuperManMode_Ptr)(_TBaseObject*);
	// 设置无敌模式
	typedef void (*TBaseObject_SetSuperManMode_Ptr)(_TBaseObject*, bool);
	// 是否管理模式
	typedef bool(*TBaseObject_GetAdminMode_Ptr)(_TBaseObject*);
	// 设置管理模式
	typedef void (*TBaseObject_SetAdminMode_Ptr)(_TBaseObject*, bool);
	// 魔法隐身
	typedef bool(*TBaseObject_GetTransparent_Ptr)(_TBaseObject*);
	// 设置魔法隐身
	typedef void (*TBaseObject_SetTransparent_Ptr)(_TBaseObject*, bool);
	// 隐身模式
	typedef bool(*TBaseObject_GetObMode_Ptr)(_TBaseObject*);
	// 设置隐身模式
	typedef void (*TBaseObject_SetObMode_Ptr)(_TBaseObject*, bool);
	// 石像化模式
	typedef bool(*TBaseObject_GetStoneMode_Ptr)(_TBaseObject*);
	// 设置石像化模式
	typedef void (*TBaseObject_SetStoneMode_Ptr)(_TBaseObject*, bool);
	// 是否能推动
	typedef bool(*TBaseObject_GetStickMode_Ptr)(_TBaseObject*);
	// 设置不可推动模式
	typedef void (*TBaseObject_SetStickMode_Ptr)(_TBaseObject*, bool);
	// 怪物是否可挖
	typedef bool(*TBaseObject_GetIsAnimal_Ptr)(_TBaseObject*);
	// 设置怪物是否可挖
	typedef void (*TBaseObject_SetIsAnimal_Ptr)(_TBaseObject*, bool);
	// 死亡是否不掉装备
	typedef bool(*TBaseObject_GetIsNoItem_Ptr)(_TBaseObject*);
	// 设置死亡是否不掉装备
	typedef void (*TBaseObject_SetIsNoItem_Ptr)(_TBaseObject*, bool);
	// 隐身免疫
	typedef bool(*TBaseObject_GetCoolEye_Ptr)(_TBaseObject*);
	// 设置隐身免疫
	typedef void (*TBaseObject_SetCoolEye_Ptr)(_TBaseObject*, bool);
	// 命中
	typedef uint16_t(*TBaseObject_GetHitPoint_Ptr)(_TBaseObject*);
	// 设置命中
	typedef void (*TBaseObject_SetHitPoint_Ptr)(_TBaseObject*, uint16_t);
	// 敏捷
	typedef uint16_t(*TBaseObject_GetSpeedPoint_Ptr)(_TBaseObject*);
	// 设置敏捷
	typedef void (*TBaseObject_SetSpeedPoint_Ptr)(_TBaseObject*, uint16_t);
	// 攻击速度
	typedef short (*TBaseObject_GetHitSpeed_Ptr)(_TBaseObject*);
	// 设置攻击速度
	typedef void (*TBaseObject_SetHitSpeed_Ptr)(_TBaseObject*, short);
	// 移动速度
	typedef int(*TBaseObject_GetWalkSpeed_Ptr)(_TBaseObject*);
	// 设置移动速度
	typedef void (*TBaseObject_SetWalkSpeed_Ptr)(_TBaseObject*, int);
	// HP恢复速度
	typedef short (*TBaseObject_GetHPRecover_Ptr)(_TBaseObject*);
	// 设置HP恢复速度
	typedef void (*TBaseObject_SetHPRecover_Ptr)(_TBaseObject*, short);
	// MP恢复速度
	typedef short (*TBaseObject_GetMPRecover_Ptr)(_TBaseObject*);
	// 设置MP恢复速度
	typedef void (*TBaseObject_SetMPRecover_Ptr)(_TBaseObject*, short);
	// 中毒恢复
	typedef short (*TBaseObject_GetPoisonRecover_Ptr)(_TBaseObject*);
	// 设置中毒恢复
	typedef void (*TBaseObject_SetPoisonRecover_Ptr)(_TBaseObject*, short);
	// 毒躲避
	typedef uint8_t(*TBaseObject_GetAntiPoison_Ptr)(_TBaseObject*);
	// 设置毒躲避
	typedef void (*TBaseObject_SetAntiPoison_Ptr)(_TBaseObject*, uint8_t);
	// 魔法躲避
	typedef short (*TBaseObject_GetAntiMagic_Ptr)(_TBaseObject*);
	// 设置魔法躲避
	typedef void (*TBaseObject_SetAntiMagic_Ptr)(_TBaseObject*, short);
	// 幸运
	typedef int(*TBaseObject_GetLuck_Ptr)(_TBaseObject*);
	// 设置幸运
	typedef void (*TBaseObject_SetLuck_Ptr)(_TBaseObject*, int);
	// 攻击模式
	typedef uint8_t(*TBaseObject_GetAttatckMode_Ptr)(_TBaseObject*);
	// 设置攻击模式
	typedef void (*TBaseObject_SetAttatckMode_Ptr)(_TBaseObject*, uint8_t);
	// 获取所属国家
	typedef uint8_t(*TBaseObject_GetNation_Ptr)(_TBaseObject*);
	// 设置所属国家
	typedef bool(*TBaseObject_SetNation_Ptr)(_TBaseObject*, uint8_t);
	// 获取国家名字
	typedef bool(*TBaseObject_GetNationaName_Ptr)(_TBaseObject*, char*, DWORD&);
	// 行会
	typedef _TGuild* (*TBaseObject_GetGuild_Ptr)(_TBaseObject*);
	// 人物所在行会中的分组编号
	typedef int(*TBaseobject_GetGuildRankNo_Ptr)(_TBaseObject*);
	// 人物所在行会中的分组名称
	typedef bool(*TBaseobject_GetGuildRankName_Ptr)(_TBaseObject*, char*, DWORD&);
	// 是否为行会老大
	typedef bool(*TBaseObject_IsGuildMaster_Ptr)(_TBaseObject*);
	// 隐身戒指 特殊物品:111
	typedef bool(*TBaseObject_GetHideMode_Ptr)(_TBaseObject*);
	typedef void (*TBaseObject_SetHideMode_Ptr)(_TBaseObject*, bool);
	// 麻痹戒指  特殊物品:113
	typedef bool(*TBaseObject_GetIsParalysis_Ptr)(_TBaseObject*);
	typedef void (*TBaseObject_SetIsParalysis_Ptr)(_TBaseObject*, bool);
	// 麻痹几率
	typedef DWORD(*TBaseObject_GetParalysisRate_Ptr)(_TBaseObject*);

	// 继续使用之前定义的类型

	// 麻痹戒指 特殊物品:113
	typedef void (*TBaseObject_SetParalysisRate_Ptr)(_TBaseObject*, DWORD);
	// 魔道麻痹戒指 特殊物品:204
	typedef bool(*TBaseObject_GetIsMDParalysis_Ptr)(_TBaseObject*);
	typedef void (*TBaseObject_SetIsMDParalysis_Ptr)(_TBaseObject*, bool);
	typedef DWORD(*TBaseObject_GetMDParalysisRate_Ptr)(_TBaseObject*);
	typedef void (*TBaseObject_SetMDParalysisRate_Ptr)(_TBaseObject*, DWORD);
	// 冰冻戒指 特殊物品:205
	typedef bool(*TBaseObject_GetIsFrozen_Ptr)(_TBaseObject*);
	typedef void (*TBaseObject_SetIsFrozen_Ptr)(_TBaseObject*, bool);
	typedef DWORD(*TBaseObject_GetFrozenRate_Ptr)(_TBaseObject*);
	typedef void (*TBaseObject_SetFrozenRate_Ptr)(_TBaseObject*, DWORD);
	// 蛛网戒指 特殊物品:206
	typedef bool(*TBaseObject_GetIsCobwebWinding_Ptr)(_TBaseObject*);
	typedef void (*TBaseObject_SetIsCobwebWinding_Ptr)(_TBaseObject*, bool);
	typedef DWORD(*TBaseObject_GetCobwebWindingRate_Ptr)(_TBaseObject*);
	typedef void (*TBaseObject_SetCobwebWindingRate_Ptr)(_TBaseObject*, DWORD);
	// 防麻几率
	typedef DWORD(*TBaseObject_GetUnParalysisValue_Ptr)(_TBaseObject*);
	typedef void (*TBaseObject_SetUnParalysisValue_Ptr)(_TBaseObject*, DWORD);
	typedef bool(*TBaseObject_GetIsUnParalysis_Ptr)(_TBaseObject*);
	// 防护身几率
	typedef DWORD(*TBaseObject_GetUnMagicShieldValue_Ptr)(_TBaseObject*);
	typedef void (*TBaseObject_SetUnMagicShieldValue_Ptr)(_TBaseObject*, DWORD);
	typedef bool(*TBaseObject_GetIsUnMagicShield_Ptr)(_TBaseObject*);
	// 防复活几率
	typedef DWORD(*TBaseObject_GetUnRevivalValue_Ptr)(_TBaseObject*);
	typedef void (*TBaseObject_SetUnRevivalValue_Ptr)(_TBaseObject*, DWORD);
	typedef bool(*TBaseObject_GetIsUnRevival_Ptr)(_TBaseObject*);
	// 防毒几率
	typedef DWORD(*TBaseObject_GetUnPosionValue_Ptr)(_TBaseObject*);
	typedef void (*TBaseObject_SetUnPosionValue_Ptr)(_TBaseObject*, DWORD);
	typedef bool(*TBaseObject_GetIsUnPosion_Ptr)(_TBaseObject*);
	// 防诱惑几率
	typedef DWORD(*TBaseObject_GetUnTammingValue_Ptr)(_TBaseObject*);
	typedef void (*TBaseObject_SetUnTammingValue_Ptr)(_TBaseObject*, DWORD);
	typedef bool(*TBaseObject_GetIsUnTamming_Ptr)(_TBaseObject*);
	// 防火墙几率
	typedef DWORD(*TBaseObject_GetUnFireCrossValue_Ptr)(_TBaseObject*);
	typedef void (*TBaseObject_SetUnFireCrossValue_Ptr)(_TBaseObject*, DWORD);
	typedef bool(*TBaseObject_GetIsUnFireCross_Ptr)(_TBaseObject*);
	// 防冰冻几率
	typedef DWORD(*TBaseObject_GetUnFrozenValue_Ptr)(_TBaseObject*);
	typedef void (*TBaseObject_SetUnFrozenValue_Ptr)(_TBaseObject*, DWORD);
	typedef bool(*TBaseObject_GetIsUnFrozen_Ptr)(_TBaseObject*);
	// 防蛛网几率
	typedef DWORD(*TBaseObject_GetUnCobwebWindingValue_Ptr)(_TBaseObject*);
	typedef void (*TBaseObject_SetUnCobwebWindingValue_Ptr)(_TBaseObject*, DWORD);
	typedef bool(*TBaseObject_GetIsUnCobwebWinding_Ptr)(_TBaseObject*);
	// 获取当前攻击目标
	typedef _TBaseObject* (*TBaseObject_GetTargetCret_Ptr)(_TBaseObject*);
	// 设置当前攻击目标
	typedef void (*TBaseObject_SetTargetCret_Ptr)(_TBaseObject*, _TBaseObject*);
	// 删除当前攻击目标
	typedef void (*TBaseObject_DelTargetCreat_Ptr)(_TBaseObject*);
	// 被谁攻击
	typedef _TBaseObject* (*TBaseObject_GetLastHiter_Ptr)(_TBaseObject*);
	// 谁得经验
	typedef _TBaseObject* (*TBaseObject_GetExpHitter_Ptr)(_TBaseObject*);
	// 施毒者
	typedef _TBaseObject* (*TBaseObject_GetPoisonHitter_Ptr)(_TBaseObject*);
	// 面前的对象是谁
	typedef _TBaseObject* (*TBaseObject_GetPoseCreate_Ptr)(_TBaseObject*);
	// 是否为攻击目标
	typedef bool(*TBaseObject_IsProperTarget_Ptr)(_TBaseObject*, _TBaseObject*);
	// 是否为朋友
	typedef bool(*TBaseObject_IsProperFriend_Ptr)(_TBaseObject*, _TBaseObject*);
	// 判断对象在指定范围内
	typedef bool(*TBaseObject_TargetInRange_Ptr)(_TBaseObject*, _TBaseObject*, int, int, int);
	// 发消息 int64_t 2021-01-06
	typedef void (*TBaseObject_SendMsg_Ptr)(_TBaseObject*, _TBaseObject*, uint16_t, int, int, int, int, char*);
	// 发延时消息 int64_t 2021-01-06
	typedef void (*TBaseObject_SendDelayMsg_Ptr)(_TBaseObject*, _TBaseObject*, uint16_t, int, int, int, int, char*, DWORD);
	// 向全屏玩家发消息 int64_t 2021-01-06
	typedef void (*TBaseObject_SendRefMsg_Ptr)(_TBaseObject*, uint16_t, int, int, int, int, char*, DWORD);
	// 更新发消息 int64_t 2021-01-06
	typedef void (*TBaseObject_SendUpdateMsg_Ptr)(_TBaseObject*, _TBaseObject*, uint16_t, int, int, int, int, char*);
	// 发聊天信息
	typedef bool(*TBaseObject_SysMsg_Ptr)(_TBaseObject*, char*, uint8_t, uint8_t, int);

	// 继续使用之前定义的类型

	// 背包物品
	typedef _TList* (*TBaseObject_GetBagItemList_Ptr)(_TBaseObject*);
	// 检测背包是否满
	typedef bool(*TBaseObject_IsEnoughBag_Ptr)(_TBaseObject*);
	// 背包是否还有足够的空间放指定数量的物品
	typedef bool(*TBaseObject_IsEnoughBagEx_Ptr)(_TBaseObject*, int);
	// 加物品到背包
	typedef bool(*TBaseObject_AddItemToBag_Ptr)(_TBaseObject*, pTUserItem);
	// 删除背包第几个物品
	typedef bool(*TBaseObject_DelBagItemByIndex_Ptr)(_TBaseObject*, int);
	// 根据makeIndex删除背包物品
	typedef bool(*TBaseObject_DelBagItemByMakeIdx_Ptr)(_TBaseObject*, int, char*);
	// 根据UserItem删除背包物品
	typedef bool(*TBaseObject_DelBagItemByUserItem_Ptr)(_TBaseObject*, pTUserItem);
	// 检查角色是否在安全区
	typedef bool(*TBaseObject_IsInSafeZone_Ptr)(_TBaseObject*);
	// 检查坐标点是否在安全区内
	typedef bool(*TBaseObject_IsPtInSafeZone_Ptr)(_TBaseObject*, _TEnvironment*, int, int);
	// 重算等级属性
	typedef void (*TBaseObject_RecalcLevelAbil_Ptr)(_TBaseObject*, bool);
	// 重算属性
	typedef void (*TBaseObject_RecalcAbil_Ptr)(_TBaseObject*);
	// 重算背包重量
	typedef int(*TBaseObject_RecalcBagWeight_Ptr)(_TBaseObject*);
	// 指定等级升到下级所要经验值
	typedef DWORD(*TBaseObject_GetLevelExp_Ptr)(_TBaseObject*, int);
	// 升级
	typedef void (*TBaseObject_HasLevelUp_Ptr)(_TBaseObject*, int);
	// 加技能熟练点
	typedef bool(*TBaseObject_TrainSkill_Ptr)(_TBaseObject*, pTUserMagic*, int, bool);
	// 检查技能是否升级
	typedef bool(*TBaseObject_CheckMagicLevelup_Ptr)(_TBaseObject*, pTUserMagic);
	// 技能点改点
	typedef void (*TBaseObject_MagicTranPointChanged_Ptr)(_TBaseObject*, pTUserMagic);
	// 掉血
	typedef void (*TBaseObject_DamageHealth_Ptr)(_TBaseObject*, int, _TBaseObject*);
	// 消耗MP
	typedef void (*TBaseObject_DamageSpell_Ptr)(_TBaseObject*, int);
	// 增加HP/MP
	typedef void (*TBaseObject_IncHealthSpell_Ptr)(_TBaseObject*, int, int, bool);
	// 通知客户端HP/MP改变
	typedef void (*TBaseObject_HealthSpellChanged_Ptr)(_TBaseObject*, DWORD);
	// 通知客户端外观改变
	typedef void (*TBaseObject_FeatureChanged_Ptr)(_TBaseObject*);
	// 通知客户端负重改变
	typedef void (*TBaseObject_WeightChanged_Ptr)(_TBaseObject*);
	// 减防后物理伤害
	typedef int(*TBaseObject_GetHitStruckDamage_Ptr)(_TBaseObject*, _TBaseObject*, int, /*PMagicACInfo*/void*, int);
	// 减防后魔法伤害
	typedef int(*TBaseObject_GetMagStruckDamage_Ptr)(_TBaseObject*, _TBaseObject*, int);
	// 顶戴花花翎
	typedef bool(*TBaseObject_GetActorIcon_Ptr)(_TBaseObject*, int, /*pTActorIcon*/void*);
	// 设置顶戴花花翎
	typedef bool(*TBaseObject_SetActorIcon_Ptr)(_TBaseObject*, int, /*pTActorIcon*/void*);
	// 刷新顶戴花花翎
	typedef void (*TBaseObject_RefUseIcons_Ptr)(_TBaseObject*);
	// 刷新效果
	typedef void (*TBaseObject_RefUseEffects_Ptr)(_TBaseObject*);
	// 飞到指定地图及坐标
	typedef void (*TBaseObject_SpaceMove_Ptr)(_TBaseObject*, char*, int, int, int);
	// 飞到指定地图随机坐标
	typedef void (*TBaseObject_MapRandomMove_Ptr)(_TBaseObject*, char*, int);
	// 对象是否可移动
	typedef bool(*TBaseObject_CanMove_Ptr)(_TBaseObject*);
	// 是否可以从一个点跑往另一个点
	typedef bool(*TBaseObject_CanRun_Ptr)(_TBaseObject*, int, int, int, int);
	// 转向
	typedef void (*TBaseObject_TurnTo_Ptr)(_TBaseObject*, uint8_t);
	// 指定方向走一步
	typedef bool(*TBaseObject_WalkTo_Ptr)(_TBaseObject*, uint8_t, bool);
	// 指定方向跑一步
	typedef bool(*TBaseObject_RunTo_Ptr)(_TBaseObject*, uint8_t, bool);
	// 预留给插件用，其他地方未使用
	typedef _TList* (*TBaseObject_PluginList_Ptr)(_TBaseObject*);

	//-------------------------------------------------------------------------------------------------------------------------------------------------------
	//---------------------------------------------- _TSmartObject 对象，继承_TBaseObject ---------------------------------------------------------------------
	//-------------------------------------------------------------------------------------------------------------------------------------------------------
  // 继续使用之前定义的类型

  // 获取技能列表
	typedef _TList* (*TSmartObject_GetMagicList_Ptr)(_TSmartObject*);
	// 取某个位置的装备; Index: 0-29
	typedef bool(*TSmartObject_GetUseItem_Ptr)(_TSmartObject*, int, pTUserItem);
	// 首饰盒状态 0:未激活; 1:激活; 2:开启
	typedef int(*TSmartObject_GetJewelryBoxStatus_Ptr)(_TSmartObject*);
	// 设置首饰盒状态 0:未激活; 1:激活; 2:开启
	typedef void (*TSmartObject_SetJewelryBoxStatus_Ptr)(_TSmartObject*, int);
	// 取首饰盒物品; Index: 0-5
	typedef bool(*TSmartObject_GetJewelryItem_Ptr)(_TSmartObject*, int, pTUserItem);
	// 是否显示神佑袋
	typedef bool(*TSmartObject_GetIsShowGodBless_Ptr)(_TSmartObject*);
	// 设置显示神佑袋
	typedef void (*TSmartObject_SetIsShowGodBless_Ptr)(_TSmartObject*, bool);
	// 取某个神佑格子的开关状态
	typedef bool(*TSmartObject_GetGodBlessItemsState_Ptr)(_TSmartObject*, int);
	// 设置神佑格子的开关状态
	typedef void (*TSmartObject_SetGodBlessItemsState_Ptr)(_TSmartObject*, int, bool);
	// 取神佑袋物品; Index: 0-11
	typedef bool(*TSmartObject_GetGodBlessItem_Ptr)(_TSmartObject*, int, pTUserItem);
	// 封号列表
	typedef _TList* (*TSmartObject_GetFengHaoItems_Ptr)(_TSmartObject*);
	// 激活的封号
	typedef int(*TSmartObject_GetActiveFengHao_Ptr)(_TSmartObject*);
	// 设置当前激活封号; -1清封号
	typedef void (*TSmartObject_SetActiveFengHao_Ptr)(_TSmartObject*, int);
	// 刷新活动封号到客户端
	typedef void (*TSmartObject_ActiveFengHaoChanged_Ptr)(_TSmartObject*);
	// 删除封号
	typedef void (*TSmartObject_DeleteFengHao_Ptr)(_TSmartObject*, int);
	// 清空封号
	typedef void (*TSmartObject_ClearFengHao_Ptr)(_TSmartObject*);
	// 移动速度
	typedef int16_t(*TSmartObject_GetMoveSpeed_Ptr)(_TSmartObject*);
	typedef void (*TSmartObject_SetMoveSpeed_Ptr)(_TSmartObject*, int16_t);
	// 攻击速度
	typedef int16_t(*TSmartObject_GetAttackSpeed_Ptr)(_TSmartObject*);
	typedef void (*TSmartObject_SetAttackSpeed_Ptr)(_TSmartObject*, int16_t);
	// 施法速度
	typedef int16_t(*TSmartObject_GetSpellSpeed_Ptr)(_TSmartObject*);
	typedef void (*TSmartObject_SetSpellSpeed_Ptr)(_TSmartObject*, int16_t);
	// 刷新游戏速度
	typedef void (*TSmartObject_RefGameSpeed_Ptr)(_TSmartObject*);
	// 是否可挖
	typedef bool(*TSmartObject_GetIsButch_Ptr)(_TSmartObject*);
	typedef void (*TSmartObject_SetIsButch_Ptr)(_TSmartObject*, bool);
	// 是否学内功
	typedef bool(*TSmartObject_GetIsTrainingNG_Ptr)(_TSmartObject*);
	typedef void (*TSmartObject_SetIsTrainingNG_Ptr)(_TSmartObject*, bool);
	// 是否学心法
	typedef bool(*TSmartObject_GetIsTrainingXF_Ptr)(_TSmartObject*);
	typedef void (*TSmartObject_SetIsTrainingXF_Ptr)(_TSmartObject*, bool);
	// 第4个连击是否开启
	typedef bool(*TSmartObject_GetIsOpenLastContinuous_Ptr)(_TSmartObject*);
	typedef void (*TSmartObject_SetIsOpenLastContinuous_Ptr)(_TSmartObject*, bool);
	// 连击顺序 Index:0-3
	typedef uint8_t(*TSmartObject_GetContinuousMagicOrder_Ptr)(_TSmartObject*, int);
	typedef void (*TSmartObject_SetContinuousMagicOrder_Ptr)(_TSmartObject*, int, uint8_t);
	// PK 死亡掉经验，不够经验就掉等级
	typedef DWORD(*TSmartObject_GetPKDieLostExp_Ptr)(_TSmartObject*);
	typedef void (*TSmartObject_SetPKDieLostExp_Ptr)(_TSmartObject*, DWORD);
	// PK 死亡掉等级
	typedef int(*TSmartObject_GetPKDieLostLevel_Ptr)(_TSmartObject*);
	typedef void (*TSmartObject_SetPKDieLostLevel_Ptr)(_TSmartObject*, int);
	// PK点数
	typedef int(*TSmartObject_GetPKPoint_Ptr)(_TSmartObject*);
	typedef void (*TSmartObject_SetPKPoint_Ptr)(_TSmartObject*, int);
	// 增加PK值
	typedef void (*TSmartObject_IncPKPoint_Ptr)(_TSmartObject*, int);
	// 减少PK值
	typedef void (*TSmartObject_DecPKPoint_Ptr)(_TSmartObject*, int);
	// PK等级
	typedef int(*TSmartObject_GetPKLevel_Ptr)(_TSmartObject*);
	typedef void (*TSmartObject_SetPKLevel_Ptr)(_TSmartObject*, int);
	// 传送戒指 特殊物品:112
	typedef bool(*TSmartObject_GetIsTeleport_Ptr)(_TSmartObject*);
	typedef void (*TSmartObject_SetIsTeleport_Ptr)(_TSmartObject*, bool);
	// 复活戒指 特殊物品:114
	typedef bool(*TSmartObject_GetIsRevival_Ptr)(_TSmartObject*);
	typedef void (*TSmartObject_SetIsRevival_Ptr)(_TSmartObject*, bool);
	// 复活戒指: 复活间隔
	typedef int(*TSmartObject_GetRevivalTime_Ptr)(_TSmartObject*);
	typedef void (*TSmartObject_SetRevivalTime_Ptr)(_TSmartObject*, int);
	// 火焰戒指 特殊物品:115
	typedef bool(*TSmartObject_GetIsFlameRing_Ptr)(_TSmartObject*);
	typedef void (*TSmartObject_SetIsFlameRing_Ptr)(_TSmartObject*, bool);
	// 治愈戒指 特殊物品:116
	typedef bool(*TSmartObject_GetIsRecoveryRing_Ptr)(_TSmartObject*);
	typedef void (*TSmartObject_SetIsRecoveryRing_Ptr)(_TSmartObject*, bool);
	// 护身戒指 特殊物品:118
	typedef bool(*TSmartObject_GetIsMagicShield_Ptr)(_TSmartObject*);
	typedef void (*TSmartObject_SetIsMagicShield_Ptr)(_TSmartObject*, bool);
	// 活力戒指(超负载) 特殊物品:119
	typedef bool(*TSmartObject_GetIsMuscleRing_Ptr)(_TSmartObject*);
	typedef void (*TSmartObject_SetIsMuscleRing_Ptr)(_TSmartObject*, bool);
	// 技巧项链 特殊物品:120
	typedef bool(*TSmartObject_GetIsFastTrain_Ptr)(_TSmartObject*);
	typedef void (*TSmartObject_SetIsFastTrain_Ptr)(_TSmartObject*, bool);
	// 探测项链 特殊物品:121
	typedef bool(*TSmartObject_GetIsProbeNecklace_Ptr)(_TSmartObject*);
	typedef void (*TSmartObject_SetIsProbeNecklace_Ptr)(_TSmartObject*, bool);
	// 记忆物品 特殊物品:122, 124, 125
	typedef bool(*TSmartObject_GetIsRecallSuite_Ptr)(_TSmartObject*);
	typedef void (*TSmartObject_SetIsRecallSuite_Ptr)(_TSmartObject*, bool);
	// 祈祷装备 特殊物品:126 - 129
	typedef bool(*TSmartObject_GetIsPirit_Ptr)(_TSmartObject*);
	typedef void (*TSmartObject_SetIsPirit_Ptr)(_TSmartObject*, bool);
	// 不死戒指 特殊物品:140
	typedef bool(*TSmartObject_GetIsSupermanItem_Ptr)(_TSmartObject*);
	typedef void (*TSmartObject_SetIsSupermanItem_Ptr)(_TSmartObject*, bool);
	// 经验物品 特殊物品:141
	typedef bool(*TSmartObject_GetIsExpItem_Ptr)(_TSmartObject*);
	typedef void (*TSmartObject_SetIsExpItem_Ptr)(_TSmartObject*, bool);
	// 经验物品值 特殊物品:141
	typedef float(*TSmartObject_GetExpItemValue_Ptr)(_TSmartObject*);
	typedef void (*TSmartObject_SetExpItemValue_Ptr)(_TSmartObject*, float);
	// 经验物品经验倍率 (物品装备->特殊属性->经验翻倍->倍率)
	typedef int(*TSmartObject_GetExpItemRate_Ptr)(_TSmartObject*);
	// 力量物品 特殊物品:142
	typedef bool(*TSmartObject_GetIsPowerItem_Ptr)(_TSmartObject*);
	typedef void (*TSmartObject_SetIsPowerItem_Ptr)(_TSmartObject*, bool);
	// 力量物品值 特殊物品:142
	typedef float(*TSmartObject_GetPowerItemValue_Ptr)(_TSmartObject*);
	typedef void (*TSmartObject_SetPowerItemValue_Ptr)(_TSmartObject*, float);
	// 力量物品经验倍率 (物品装备->特殊属性->攻击翻倍->倍率)
	typedef int(*TSmartObject_GetPowerItemRate_Ptr)(_TSmartObject*);
	// 行会传送装备 特殊物品:145
	typedef bool(*TSmartObject_GetIsGuildMove_Ptr)(_TSmartObject*);
	typedef void (*TSmartObject_SetIsGuildMove_Ptr)(_TSmartObject*, bool);
	// 幸运戒指 特殊物品 170
	typedef bool(*TSmartObject_GetIsAngryRing_Ptr)(_TSmartObject*);
	typedef void (*TSmartObject_SetIsAngryRing_Ptr)(_TSmartObject*, bool);
	// 流星戒指
	typedef bool(*TSmartObject_GetIsStarRing_Ptr)(_TSmartObject*);
	typedef void (*TSmartObject_SetIsStarRing_Ptr)(_TSmartObject*, bool);
	// 防御物品
	typedef bool(*TSmartObject_GetIsACItem_Ptr)(_TSmartObject*);
	typedef void (*TSmartObject_SetIsACItem_Ptr)(_TSmartObject*, bool);
	// 防御值
	typedef float(*TSmartObject_GetACItemValue_Ptr)(_TSmartObject*);
	typedef void (*TSmartObject_SetACItemValue_Ptr)(_TSmartObject*, float);
	// 魔御物品
	typedef bool(*TSmartObject_GetIsMACItem_Ptr)(_TSmartObject*);
	typedef void (*TSmartObject_SetIsMACItem_Ptr)(_TSmartObject*, bool);
	// 魔御值
	typedef float(*TSmartObject_GetMACItemValue_Ptr)(_TSmartObject*);
	typedef void (*TSmartObject_SetMACItemValue_Ptr)(_TSmartObject*, float);

	// 171不掉背包物品装备
	typedef bool(*TSmartObject_GetIsNoDropItem_Ptr)(_TSmartObject*);
	typedef void (*TSmartObject_SetIsNoDropItem_Ptr)(_TSmartObject*, bool);
	// 172不掉身上物品装备
	typedef bool(*TSmartObject_GetIsNoDropUseItem_Ptr)(_TSmartObject*);
	typedef void (*TSmartObject_SetIsNoDropUseItem_Ptr)(_TSmartObject*, bool);
	// 内功属性
	typedef bool(*TSmartObject_GetNGAbility_Ptr)(_TSmartObject*, /*pTAbilityNG*/void*);
	typedef void (*TSmartObject_SetNGAbility_Ptr)(_TSmartObject*, /*pTAbilityNG*/void*);
	// 酒属性
	typedef bool(*TSmartObject_GetAlcohol_Ptr)(_TSmartObject*, /*pTAbilityAlcohol*/void*);
	typedef void (*TSmartObject_SetAlcohol_Ptr)(_TSmartObject*, /*pTAbilityAlcohol*/void*);
	// 修复所有装备
	typedef void (*TSmartObject_RepairAllItem_Ptr)(_TSmartObject*);
	// 是否满足技能使用条件
	typedef bool(*TSmartObject_IsAllowUseMagic_Ptr)(_TSmartObject*, uint16_t);
	// 选择技能
	typedef int(*TSmartObject_SelectMagic_Ptr)(_TSmartObject*);
	// 攻击目标
	typedef bool(*TSmartObject_AttackTarget_Ptr)(_TSmartObject*, uint16_t, DWORD);

	//-------------------------------------------------------------------------------------------------------------------------------------------------------
	  //---------------------------------------------- TPlayObject 人物对象，继承TSmartObject -----------------------------------------------------------------
	  //-------------------------------------------------------------------------------------------------------------------------------------------------------


	// 账户名
	typedef bool(*TPlayObject_GetUserID)(_TPlayObject*, char*, DWORD*);
	// IP
	typedef bool(*TPlayObject_GetIPAddr)(_TPlayObject*, char*, DWORD*);
	// IP归属地
	typedef bool(*TPlayObject_GetIPLocal)(_TPlayObject*, char*, DWORD*);
	// MAC
	typedef bool(*TPlayObject_GetMachineID)(_TPlayObject*, char*, DWORD*);
	// 是否进入游戏完成
	typedef bool(*TPlayObject_GetIsReadyRun)(_TPlayObject*);
	// 登录时间
	typedef bool(*TPlayObject_GetLogonTime)(_TPlayObject*, void*);
	// 客户端版本号
	typedef int(*TPlayObject_GetSoftVerDate)(_TPlayObject*);
	// 客户端类型
	typedef int(*TPlayObject_GetClientType)(_TPlayObject*);
	// 是否为老客户端
	typedef bool(*TPlayObject_IsOldClient)(_TPlayObject*);
	// 客户端分辨率宽
	typedef uint16_t(*TPlayObject_GetScreenWidth)(_TPlayObject*);
	// 客户端分辨率高
	typedef uint16_t(*TPlayObject_GetScreenHeight)(_TPlayObject*);
	// 客户端视觉范围大小
	typedef uint16_t(*TPlayObject_GetClientViewRange)(_TPlayObject*);
	// 转生等级
	typedef uint8_t(*TPlayObject_GetRelevel)(_TPlayObject*);
	typedef void (*TPlayObject_SetRelevel)(_TPlayObject*, uint8_t);
	// 未分配属性点
	typedef int(*TPlayObject_GetBonusPoint)(_TPlayObject*);
	typedef void (*TPlayObject_SetBonusPoint)(_TPlayObject*, int);
	// 发送属性点
	typedef void (*TPlayObject_SendAdjustBonus)(_TPlayObject*);
	// 主将英雄名
	typedef bool(*TPlayObject_GetHeroName)(_TPlayObject*, char*, DWORD*);
	// 副将英雄名
	typedef bool(*TPlayObject_GetDeputyHeroName)(_TPlayObject*, char*, DWORD*);
	// 副将英雄职业
	typedef uint8_t(*TPlayObject_GetDeputyHeroJob)(_TPlayObject*);
	// 英雄对象
	typedef _THeroObject(*TPlayObject_GetMyHero)(_TPlayObject*);
	// 是否评定主副英雄
	typedef bool(*TPlayObject_GetFixedHero)(_TPlayObject*);
	// 召唤英雄
	typedef void (*TPlayObject_ClientHeroLogOn)(_TPlayObject*, bool);
	// 英雄是否寄存
	typedef bool(*TPlayObject_GetStorageHero)(_TPlayObject*);
	// 副将英雄是否寄存
	typedef bool(*TPlayObject_GetStorageDeputyHero)(_TPlayObject*);
	// 仓库是否开启
	typedef bool(*TPlayObject_GetIsStorageOpen)(_TPlayObject*, int);
	typedef void (*TPlayObject_SetIsStorageOpen)(_TPlayObject*, int, bool);
	// 金币数量
	typedef DWORD(*TPlayObject_GetGold)(_TPlayObject*);
	typedef void (*TPlayObject_SetGold)(_TPlayObject*, DWORD);
	// 元宝数量
	typedef DWORD(*TPlayObject_GetGameGold)(_TPlayObject*);
	typedef void (*TPlayObject_SetGameGold)(_TPlayObject*, DWORD);
	// 游戏点
	typedef DWORD(*TPlayObject_GetGamePoint)(_TPlayObject*);
	typedef void (*TPlayObject_SetGamePoint)(_TPlayObject*, DWORD);
	// 金刚石
	typedef DWORD(*TPlayObject_GetGameDiamond)(_TPlayObject*);
	typedef void (*TPlayObject_SetGameDiamond)(_TPlayObject*, DWORD);
	// 灵符
	typedef DWORD(*TPlayObject_GetGameGird)(_TPlayObject*);
	typedef void (*TPlayObject_SetGameGird)(_TPlayObject*, DWORD);
	// 新游戏点
	typedef int(*TPlayObject_GetGameGoldEx)(_TPlayObject*);
	typedef void (*TPlayObject_SetGameGoldEx)(_TPlayObject*, int);
	// 荣誉
	typedef int(*TPlayObject_GetGameGlory)(_TPlayObject*);
	typedef void (*TPlayObject_SetGameGlory)(_TPlayObject*, int);
	// 充值点
	typedef int(*TPlayObject_GetPayMentPoint)(_TPlayObject*);
	typedef void (*TPlayObject_SetPayMentPoint)(_TPlayObject*, int);
	// 会员类型
	typedef int(*TPlayObject_GetMemberType)(_TPlayObject*);
	typedef void (*TPlayObject_SetMemberType)(_TPlayObject*, int);
	// 会员等级
	typedef int(*TPlayObject_GetMemberLevel)(_TPlayObject*);
	typedef void (*TPlayObject_SetMemberLevel)(_TPlayObject*, int);
	// 贡献度
	typedef uint16_t(*TPlayObject_GetContribution)(_TPlayObject*);
	typedef void (*TPlayObject_SetContribution)(_TPlayObject*, uint16_t);

	// 加经验，调用些函数会自动刷新客户端
	typedef void (*TPlayObejct_IncExp)(_TPlayObject*, DWORD);
	// 经验改变
	typedef void (*TPlayObject_SendExpChanged)(_TPlayObject*);

	// 加内功经验，调用此函数会自动刷新客户端
	typedef void (*TPlayObject_IncExpNG)(_TPlayObject*, DWORD);
	// 内功经验改变
	typedef void (*TPlayObject_SendExpNGChanged)(_TPlayObject*);

	// 增加聚灵珠经验
	typedef void (*TPlayObject_IncBeadExp)(_TPlayObject*, DWORD, bool);

	// P变量 m_nVal  [0..999]
	typedef int(*TPlayObject_GetVarP)(_TPlayObject*, int);
	typedef void (*TPlayObject_SetVarP)(_TPlayObject*, int, int);

	// M变量 m_nMval [0..999]
	typedef int(*TPlayObject_GetVarM)(_TPlayObject*, int);
	typedef void (*TPlayObject_SetVarM)(_TPlayObject*, int, int);

	// D变量 m_DyVal [0..999]
	typedef int(*TPlayObject_GetVarD)(_TPlayObject*, int);
	typedef void (*TPlayObject_SetVarD)(_TPlayObject*, int, int);

	// U变量 m_UVal [0..254]
	typedef int(*TPlayObject_GetVarU)(_TPlayObject*, int);
	typedef void (*TPlayObject_SetVarU)(_TPlayObject*, int, int);

	// T变量 m_TVal [0..254]
	typedef bool(*TPlayObject_GetVarT)(_TPlayObject*, int, char*, DWORD*);
	typedef void (*TPlayObject_SetVarT)(_TPlayObject*, int, char*);

	// N变量 m_nInteger [0..999]
	typedef int(*TPlayObject_GetVarN)(_TPlayObject*, int);
	typedef void (*TPlayObject_SetVarN)(_TPlayObject*, int, int);

	// S变量 m_sString [0..999]
	typedef bool(*TPlayObject_GetVarS)(_TPlayObject*, int, char*, DWORD*);
	typedef void (*TPlayObject_SetVarS)(_TPlayObject*, int, char*);

	// 自定义动态变量列表 (结果的元素为 pTDynamicVar)
	typedef _TList* (*TPlayObject_GetDynamicVarList)(_TPlayObject*);

	// m_IntegerList: TQuickList; { N }
	// m_StringList: TValueList; { S }

	// 获取任务标志状态
	typedef int(*TPlayObject_GetQuestFlagStatus)(_TPlayObject*, int);
	// 设置任务标志状态
	typedef void (*TPlayObject_SetQuestFlagStatus)(_TPlayObject*, int, int);

	// 是否离线挂机
	typedef bool(*TPlayObject_IsOffLine)(_TPlayObject*);
	// 是否是师傅
	typedef bool(*TPlayObject_IsMaster)(_TPlayObject*);
	// 取得师傅名字
	typedef bool(*TPlayObject_GetMasterName)(_TPlayObject*, char*, DWORD*);
	// 师傅
	typedef _TPlayObject* (*TPlayObject_GetMasterHuman)(_TPlayObject*);
	// 徒弟排名
	typedef int(*TPlayObject_GetApprenticeNO)(_TPlayObject*);
	// 在线徒弟列表
	typedef _TList* (*TPlayObject_GetOnlineApprenticeList)(_TPlayObject*);
	// 所有徒弟列表 (结果的元素为 pTMasterRankInfo)
	typedef _TList* (*TPlayObject_GetAllApprenticeList)(_TPlayObject*);
	// 取得爱人名字
	typedef bool(*TPlayObject_GetDearName)(_TPlayObject*, char*, DWORD*);
	// 爱人
	typedef _TPlayObject* (*TPlayObject_GetDearHuman)(_TPlayObject*);
	// 离婚次数
	typedef uint8_t(*TPlayObject_GetMarryCount)(_TPlayObject*);
	// 队长
	typedef _TPlayObject* (*TPlayObject_GetGroupOwner)(_TPlayObject*);
	// 队员列表 Item: 队员名 Objects: _TBaseObject
	typedef _TStringList* (*TPlayObject_GetGroupMembers)(_TPlayObject*);
	// 锁定登录
	typedef bool(*TPlayObject_GetIsLockLogin)(_TPlayObject*);
	typedef void (*TPlayObject_SetIsLockLogin)(_TPlayObject*, bool);
	// 是否允许组队
	typedef bool(*TPlayObject_GetIsAllowGroup)(_TPlayObject*);
	typedef void (*TPlayObject_SetIsAllowGroup)(_TPlayObject*, bool);
	// 是否允许天地合一
	typedef bool(*TPlayObject_GetIsAllowGroupReCall)(_TPlayObject*);
	typedef void (*TPlayObject_SetIsAllowGroupReCall)(_TPlayObject*, bool);
	// 是否允许行会合一
	typedef bool(*TPlayObject_GetIsAllowGuildReCall)(_TPlayObject*);
	typedef void (*TPlayObject_SetIsAllowGuildReCall)(_TPlayObject*, bool);
	// 是否允许交易
	typedef bool(*TPlayObject_GetIsAllowTrading)(_TPlayObject*);
	typedef void (*TPlayObject_SetIsAllowTrading)(_TPlayObject*, bool);
	// 禁止邀请上马
	typedef bool(*TPlayObject_GetIsDisableInviteHorseRiding)(_TPlayObject*);
	typedef void (*TPlayObject_SetIsDisableInviteHorseRiding)(_TPlayObject*, bool);
	// 是否开启元宝交易
	typedef bool(*TPlayObject_GetIsGameGoldTrading)(_TPlayObject*);
	typedef void (*TPlayObject_SetIsGameGoldTrading)(_TPlayObject*, bool);
	// 合过区没有登录过的
	typedef bool(*TPlayObject_GetIsNewServer)(_TPlayObject*);
	// 过滤掉落提示信息
	typedef bool(*TPlayObject_GetIsFilterGlobalDropItemMsg)(_TPlayObject*);
	typedef void (*TPlayObject_SetIsFilterGlobalDropItemMsg)(_TPlayObject*, bool);
	// 过滤SendCenterMsg
	typedef bool(*TPlayObject_GetIsFilterGlobalCenterMsg)(_TPlayObject*);
	typedef void (*TPlayObject_SetIsFilterGlobalCenterMsg)(_TPlayObject*, bool);
	// 过滤SendMsg全局信息
	typedef bool(*TPlayObject_GetIsFilterGolbalSendMsg)(_TPlayObject*);
	typedef void (*TPlayObject_SetIsFilterGolbalSendMsg)(_TPlayObject*, bool);
	// 是否请过酒
	typedef bool(*TPlayObject_GetIsPleaseDrink)(_TPlayObject*);
	// 饮酒时酒的品质
	typedef int(*TPlayObject_GetIsDrinkWineQuality)(_TPlayObject*);
	typedef void (*TPlayObject_SetIsDrinkWineQuality)(_TPlayObject*, int);
	// 饮酒时酒的度数
	typedef int(*TPlayObject_GetIsDrinkWineAlcohol)(_TPlayObject*);
	typedef void (*TPlayObject_SetIsDrinkWineAlcohol)(_TPlayObject*, int);
	// 人是否喝酒醉了
	typedef bool(*TPlayObject_GetIsDrinkWineDrunk)(_TPlayObject*);
	typedef void (*TPlayObject_SetIsDrinkWineDrunk)(_TPlayObject*, bool);
	// 回城
	typedef void (*TPlayObject_MoveToHome)(_TPlayObject*);
	// 随机传送到回城地图
	typedef void (*TPlayObject_MoveRandomToHome)(_TPlayObject*);
	// 发送数据
	typedef void (*TPlayObject_SendSocket)(_TPlayObject*, PTDefaultMessage,char*);
	// 发送消息 int64_t 2021-01-06
	typedef void (*TPlayObject_SendDefMessage)(_TPlayObject*, uint16_t, int64_t, int, uint16_t, uint16_t, uint16_t, char*);
	// 发送移动消息
	typedef void (*TPlayObject_SendMoveMsg)(_TPlayObject*, char*, uint8_t, uint8_t, uint16_t, int, int, int);
	// 发送中心消息
	typedef void (*TPlayObject_SendCenterMsg)(_TPlayObject*, char*, uint8_t, uint8_t, int);
	// 发送顶部广播消息
	typedef bool(*TPlayObject_SendTopBroadCastMsg)(_TPlayObject*, char*, uint8_t, uint8_t, int, int);
	// 检测装备是否可穿戴
	typedef bool(*TPlayObject_CheckTakeOnItems)(_TPlayObject*, int, /*pTStdItem*/void*);
	// 处理装备穿脱时对应的技能
	typedef void (*TPlayObject_ProcessUseItemSkill)(_TPlayObject*, int, /*pTStdItem*/void*, bool);
	// 发送身上装备列表
	typedef void (*TPlayObject_SendUseItems)(_TPlayObject*);
	// 发送增加物品
	typedef void (*TPlayObject_SendAddItem)(_TPlayObject*, pTUserItem);
	// 客户端删除多个物品 2020-01-20修改 物品名称/MakeIndex/物品名称/MakeIndex....
	typedef void (*TPlayObject_SendDelItemList)(_TPlayObject*, char*, int);
	// 客户端删除物品
	typedef void (*TPlayObject_SendDelItem)(_TPlayObject*, pTUserItem);
	// 客户端刷新物品
	typedef void (*TPlayObject_SendUpdateItem)(_TPlayObject*, pTUserItem);
	// 客户端刷新装备持久改变
	typedef void (*TPlayObject_SendItemDuraChange)(_TPlayObject*, int, pTUserItem);
	// 刷新客户端包裹
	typedef void (*TPlayObject_SendBagItems)(_TPlayObject*);
	// 发送首饰盒物品
	typedef void (*TPlayObject_SendJewelryBoxItems)(_TPlayObject*);
	// 发送神佑袋物品
	typedef void (*TPlayObject_SendGodBlessItems)(_TPlayObject*);
	// 神佑格开启
	typedef void (*TPlayObject_SendOpenGodBlessItem)(_TPlayObject*, int);
	// 神佑格关闭
	typedef void (*TPlayObject_SendCloseGodBlessItem)(_TPlayObject*, int);
	// 发送技能列表
	typedef void (*TPlayObject_SendUseMagics)(_TPlayObject*);
	// 发送技能添加
	typedef void (*TPlayObject_SendAddMagic)(_TPlayObject*, pTUserMagic);
	// 发送技能删除
	typedef void (*TPlayObject_SendDelMagic)(_TPlayObject*, pTUserMagic);
	// 发送封号物品
	typedef void (*TPlayObject_SendFengHaoItems)(_TPlayObject*);
	// 发送封号增加
	typedef void (*TPlayObject_SendAddFengHaoItem)(_TPlayObject*, /*PTUserItem*/void*);
	// 发送封号删除
	typedef void (*TPlayObject_SendDelFengHaoItem)(_TPlayObject*, int);
	// 发送走路/跑步失败
	typedef void (*TPlayObject_SendSocketStatusFail)(_TPlayObject*);
	// 播放效果
	typedef void (*TPlayObject_PlayEffect)(_TPlayObject*, int, int, int, int, int, int, uint8_t, int, int);
	// 是否正在内挂挂机
	typedef bool(*TPlayObject_IsAutoPlayGame)(_TPlayObject*);
	// 开始内挂挂机
	typedef bool(*TPlayObject_StartAutoPlayGame)(_TPlayObject*);
	// 停止内挂挂机
	typedef bool(*TPlayObject_StopAutoPlayGame)(_TPlayObject*);


	//-------------------------------------------------------------------------------------------------------------------------------------------------------
	  //---------------------------------------------- TDummyObject 假人对象，继承TPlayObject -----------------------------------------------------------------
	  //-------------------------------------------------------------------------------------------------------------------------------------------------------


	// 假人是否开始挂机
	typedef bool(*TDummyObject_IsStart)(_TDummyObject*);
	// 假人开始挂机
	typedef void (*TDummyObject_Start)(_TDummyObject*);
	// 假人停止挂机
	typedef void (*TDummyObject_Stop)(_TDummyObject*);

	//-------------------------------------------------------------------------------------------------------------------------------------------------------
	//---------------------------------------------- THeroObject 英雄对象，继承TSmartObject -----------------------------------------------------------------
	//-------------------------------------------------------------------------------------------------------------------------------------------------------

  // 攻击模式
	typedef uint8_t (*THeroObject_GetAttackMode_Ptr)(_THeroObject*);
	// 设置攻击模式
	typedef bool(*THeroObject_SetAttackMode_Ptr)(_THeroObject*, uint8_t, bool);
	// 切换下一个攻击模式
	typedef void (*THeroObject_SetNextAttackMode_Ptr)(_THeroObject*);
	// 获取背包数量
	typedef int(*THeroObject_GetBagCount_Ptr)(_THeroObject*);
	// 当前怒气值
	typedef int(*THeroObject_GetAngryValue_Ptr)(_THeroObject*);
	// 忠诚度
	typedef float(*THeroObject_GetLoyalPoint_Ptr)(_THeroObject*);
	typedef void (*THeroObject_SetLoyalPoint_Ptr)(_THeroObject*, float);
	typedef void (*THeroObject_SendLoyalPointChanged_Ptr)(_THeroObject*);
	// 是否副将英雄
	typedef bool(*THeroObject_IsDeputy_Ptr)(_THeroObject*);
	// 主人名称
	typedef bool(*THeroObject_GetMasterName_Ptr)(_THeroObject*, char*, DWORD*);
	// 获取任务标志状态
	typedef int(*THeroObject_GetQuestFlagStatus_Ptr)(_THeroObject*, int);
	// 设置任务标志状态
	typedef void (*THeroObject_SetQuestFlagStatus_Ptr)(_THeroObject*, int, int);
	// 发送身上装备
	typedef void (*THeroObject_SendUseItems_Ptr)(_THeroObject*);
	// 刷新英雄背包
	typedef void (*THeroObject_SendBagItems_Ptr)(_THeroObject*);
	// 发送首饰盒物品
	typedef void (*THeroObject_SendJewelryBoxItems_Ptr)(_THeroObject*);
	// 发送神佑袋物品
	typedef void (*THeroObject_SendGodBlessItems_Ptr)(_THeroObject*);
	// 神佑格开启
	typedef void (*THeroObject_SendOpenGodBlessItem_Ptr)(_THeroObject*, int);
	// 神佑格关闭
	typedef void (*THeroObject_SendCloseGodBlessItem_Ptr)(_THeroObject*, int);
	// 发送增加物品
	typedef void (*THeroObject_SendAddItem_Ptr)(_THeroObject*, pTUserItem);
	// 客户端删除物品
	typedef void (*THeroObject_SendDelItem_Ptr)(_THeroObject*, pTUserItem);
	// 客户端刷新物品
	typedef void (*THeroObject_SendUpdateItem_Ptr)(_THeroObject*, pTUserItem);
	// 客户端刷新装备持久改变
	typedef void (*THeroObject_SendItemDuraChange_Ptr)(_THeroObject*, int, pTUserItem);
	// 发送技能列表
	typedef void (*THeroObject_SendUseMagics_Ptr)(_THeroObject*);
	// 发送技能添加
	typedef void (*THeroObject_SendAddMagic_Ptr)(_THeroObject*, pTUserMagic);
	// 发送技能删除
	typedef void (*THeroObject_SendDelMagic_Ptr)(_THeroObject*, pTUserMagic);
	// 取得合击技能
	typedef bool(*THeroObject_FindGroupMagic_Ptr)(_THeroObject*, pTUserMagic);
	// 取得合击技能ID
	typedef int(*THeroObject_GetGroupMagicId_Ptr)(_THeroObject*);
	// 发送封号物品
	typedef void (*THeroObject_SendFengHaoItems_Ptr)(_THeroObject*);
	// 发送封号增加
	typedef void (*THeroObject_SendAddFengHaoItem_Ptr)(_THeroObject*, pTUserItem);
	// 发送封号删除
	typedef void (*THeroObject_SendDelFengHaoItem_Ptr)(_THeroObject*, int);
	// 增加经验
	typedef void (*THeroObject_IncExp_Ptr)(_THeroObject*, DWORD);
	// 增加经验(NG版本)
	typedef void (*THeroObject_IncExpNG_Ptr)(_THeroObject*, DWORD);
	// 是否旧版客户端
	typedef bool(*THeroObject_IsOldClient_Ptr)(_THeroObject*);

	//-------------------------------------------------------------------------------------------------------------------------------------------------------
	//---------------------------------------------- TNormNpc NPC对象，继承TBaseObject ---------------------------------------------------------------------
	//-------------------------------------------------------------------------------------------------------------------------------------------------------
  // 创建NPC，该NPC由引擎自动释放
	typedef _TNormNpc* (*TNormNpc_Create_Ptr)(char*, char*, char*, int, int, uint16_t, bool);
	// 载入脚本
	typedef void (*TNormNpc_LoadNpcScript_Ptr)(_TNormNpc*);
	// 清脚本
	typedef void (*TNormNpc_ClearScript_Ptr)(_TNormNpc*);
	// 获取文件路径
	typedef bool(*TNormNpc_GetFilePath_Ptr)(_TNormNpc*, char*, DWORD*);
	// 设置文件路径
	typedef void (*TNormNpc_SetFilePath_Ptr)(_TNormNpc*, char*);
	// 获取路径
	typedef bool(*TNormNpc_GetPath_Ptr)(_TNormNpc*, char*, DWORD*);
	// 设置路径
	typedef void (*TNormNpc_SetPath_Ptr)(_TNormNpc*, char*);
	// 获取是否隐藏
	typedef bool(*TNormNpc_GetIsHide_Ptr)(_TNormNpc*);
	// 设置是否隐藏
	typedef void (*TNormNpc_SetIsHide_Ptr)(_TNormNpc*, bool);
	// 获取是否为任务NPC
	typedef bool(*TNormNpc_GetIsQuest_Ptr)(_TNormNpc*);
	// 获取行变量文本
	typedef bool(*TNormNpc_GetLineVariableText_Ptr)(_TNormNpc*, _TPlayObject*, char*, char*, DWORD*);
	// 跳转至标签
	typedef void (*TNormNpc_GotoLable_Ptr)(_TNormNpc*, _TPlayObject*, char*, bool);
	// 发送消息给用户
	typedef void (*TNormNpc_SendMsgToUser_Ptr)(_TNormNpc*, _TPlayObject*, char*);
	// 显示消息框
	typedef void (*TNormNpc_MessageBox_Ptr)(_TNormNpc*, _TPlayObject*, char*);
	// 获取变量值
	typedef bool(*TNormNpc_GetVarValue_Ptr)(_TNormNpc*, _TPlayObject*, char*, char*, DWORD*, int*);
	// 设置变量值
	typedef bool(*TNormNpc_SetVarValue_Ptr)(_TNormNpc*, _TPlayObject*, char*, char*, int);
	// 获取动态变量值
	typedef bool(*TNormNpc_GetDynamicVarValue_Ptr)(_TNormNpc*, _TPlayObject*, char*, char*, DWORD*, int*);
	// 设置动态变量值
	typedef bool(*TNormNpc_SetDynamicVarValue_Ptr)(_TNormNpc*, _TPlayObject*, char*, char*, int);

	//-------------------------------------------------------------------------------------------------------------------------------------------------------
	 //---------------------------------------------- TUserEngine 对象 --------------------------------------------------------------------------------------
	 //-------------------------------------------------------------------------------------------------------------------------------------------------------
   // 获取所有在线人物列表(含假人)
	typedef _TStringList* (*TUserEngine_GetPlayerList_Ptr)();
	// 根据在线人物名称获取对象
	typedef _TPlayObject* (*TUserEngine_GetPlayerByName_Ptr)(char*);
	// 根据在线帐户获取对象
	typedef _TPlayObject* (*TUserEngine_GetPlayerByUserID_Ptr)(char*);
	// 判断对象是否是一个合法的在线人物
	typedef _TPlayObject* (*TUserEngine_GetPlayerByObject_Ptr)(_TObjectType*);
	// 根据帐户获取一个离线挂机对象
	typedef _TPlayObject* (*TUserEngine_GetOfflinePlayer_Ptr)(char*);
	// 踢人
	typedef void (*TUserEngine_KickPlayer_Ptr)(char*);
	// 获取英雄列表
	typedef _TStringList* (*TUserEngine_GetHeroList_Ptr)();
	// 根据名称获取英雄对象
	typedef _THeroObject* (*TUserEngine_GetHeroByName_Ptr)(char*);
	// 踢英雄
	typedef bool(*TUserEngine_KickHero_Ptr)(char*);
	// 获取交易NPC列表
	typedef _TList* (*TUserEngine_GetMerchantList_Ptr)();
	// 获取自定义NPC配置列表
	typedef _TList* (*TUserEngine_GetCustomNpcConfigList_Ptr)();
	// 获取MapQuest.txt中定义的NPC列表
	typedef _TList* (*TUserEngine_GetQuestNPCList_Ptr)();
	// 
	typedef _TNormNpc* (*TUserEngine_GetManageNPC_Ptr)();
	// 
	typedef _TNormNpc* (*TUserEngine_GetFunctionNPC_Ptr)();
	// 
	typedef _TNormNpc* (*TUserEngine_GetRobotNPC_Ptr)();
	// 
	typedef _TNormNpc* (*TUserEngine_MissionNPC_Ptr)();
	// 判断NPC对象是否合法
	typedef _TNormNpc* (*TUserEngine_FindMerchant_Ptr)(_TObjectType*);
	// 根据地图坐标得到NPC
	typedef _TNormNpc* (*TUserEngine_FindMerchantByPos_Ptr)(char*, int, int);
	// 判断NPC对象是否合法
	typedef _TNormNpc* (*TUserEngine_FindQuestNPC_Ptr)(_TObjectType*);
	// Magic.DB
	typedef _TList* (*TUserEngine_GetMagicList_Ptr)();
	// 自定义技能配置列表
	typedef _TList* (*TUserEngine_GetCustomMagicConfigList_Ptr)();
	// M2 -> 功能设置 ->技能魔法 -> 技能破防百分比
	typedef _TMagicACList* (*TUserEngine_GetMagicACList_Ptr)();
	// 根据技能名查找技能
	typedef bool(*TUserEngine_FindMagicByName_Ptr)(char*, /*/*pTMagic*/void*);
	// 根据技能编号查找技能
	typedef bool(*TUserEngine_FindMagicByIndex_Ptr)(int, /*pTMagic*/void*);
	// 根据技能名及属性查找技能
	typedef bool(*TUserEngine_FindMagicByNameEx_Ptr)(char*, int, /*pTMagic*/void*);
	// 根据技能编号及属性查找技能
	typedef bool(*TUserEngine_FindMagicByIndexEx_Ptr)(int, int, /*pTMagic*/void*);
	// 根据技能名查找英雄技能
	typedef bool(*TUserEngine_FindHeroMagicByName_Ptr)(char*, /*pTMagic*/void*);
	// 根据技能编号查找英雄技能
	typedef bool(*TUserEngine_FindHeroMagicByIndex_Ptr)(int, /*pTMagic*/void*);
	// 根据技能名及属性查找英雄技能
	typedef bool(*TUserEngine_FindHeroMagicByNameEx_Ptr)(char*, int, /*pTMagic*/void*);
	// 根据技能编号及属性查找英雄技能
	typedef bool(*TUserEngine_FindHeroMagicByIndexEx_Ptr)(int, int, /*pTMagic*/void*);
	// StdItem.DB
	typedef _TList* (*TUserEngine_GetStdItemList_Ptr)();
	// 根据物品名得到数据库的物品信息
	typedef bool(*TUserEngine_GetStdItemByName_Ptr)(char*, void*);
	// 根据物品编号得到数据库的物品信息
	typedef bool(*TUserEngine_GetStdItemByIndex_Ptr)(int, void*);
	// 根据物品编号得到物品名
	typedef bool(*TUserEngine_GetStdItemName_Ptr)(int, char*, DWORD*);
	// 根据物品名得到物品编号
	typedef int(*TUserEngine_GetStdItemIndex_Ptr)(char*);
	// Monster.DB
	typedef _TList* (*TUserEngine_MonsterList_Ptr)();
	// 
	typedef bool(*TUserEngine_SendBroadCastMsg_Ptr)(char*, int, int, int);
	// 
	typedef bool(*TUserEngine_SendBroadCastMsgExt_Ptr)(char*, int);
	// 
	typedef bool(*TUserEngine_SendTopBroadCastMsg_Ptr)(char*, int, int, int, int);
	// 
	typedef void (*TUserEngine_SendMoveMsg_Ptr)(char*, uint8_t, uint8_t, int, int, int, int);
	// 
	typedef void (*TUserEngine_SendCenterMsg_Ptr)(char*, uint8_t, uint8_t, int);
	// 换行消息
	typedef void (*TUserEngine_SendNewLineMsg_Ptr)(char*, uint8_t, uint8_t, uint8_t, int, int, int);
	// 仿盛大顶部渐隐消息
	typedef void (*TUserEngine_SendSuperMoveMsg_Ptr)(char*, uint8_t, uint8_t, uint8_t, int, int);
	// 发送屏幕震动消息
	typedef void (*TUserEngine_SendSceneShake_Ptr)(int);
	// 
	typedef bool(*TUserEngine_CopyToUserItemFromName_Ptr)(char*, pTUserItem);
	// 
	typedef bool(*TUserEngine_CopyToUserItemFromItem_Ptr)(void*, int, pTUserItem);
	// 
	typedef void (*TUserEngine_RandomUpgradeItem_Ptr)(pTUserItem);
	// 随机生成元素属性
	typedef void (*TUserEngine_RandomItemNewAbil_Ptr)(pTUserItem);
	// 
	typedef void (*TUserEngine_GetUnknowItemValue_Ptr)(pTUserItem);
	// 所有假人数量
	typedef int(*TUserEngine_GetAllDummyCount_Ptr)();
	// 指定地图假人数量
	typedef int(*TUserEngine_GetMapDummyCount_Ptr)(_TEnvironment*);
	// 离线挂机人物数量
	typedef int(*TUserEngine_GetOfflineCount_Ptr)();
	// 在线真人数量(不含离线挂机)
	typedef int(*TUserEngine_GetRealPlayerCount_Ptr)();

	//-------------------------------------------------------------------------------------------------------------------------------------------------------
	 //---------------------------------------------- _TGuild 行会对象 --------------------------------------------------------------------------------------
	 //-------------------------------------------------------------------------------------------------------------------------------------------------------
   // 行会名称
	typedef bool (*TGuild_GetGuildName_Ptr)(_TGuild Guild, char* Dest, DWORD& DestLen);

	// 加入行会职业
	typedef int (*TGuild_GetJoinJob_Ptr)(_TGuild Guild);

	// 加入行会最低等级
	typedef DWORD(*TGuild_GetJoinLevel_Ptr)(_TGuild Guild);

	// 招贤消息
	typedef bool (*TGuild_GetJoinMsg_Ptr)(_TGuild Guild, char* Dest, DWORD& DestLen);

	// 建筑度
	typedef int (*TGuild_GetBuildPoint_Ptr)(_TGuild Guild);

	// 人气值/关注度
	typedef int (*TGuild_GetAurae_Ptr)(_TGuild Guild);

	// 安定度
	typedef int (*TGuild_GetStability_Ptr)(_TGuild Guild);

	// 繁荣度
	typedef int (*TGuild_GetFlourishing_Ptr)(_TGuild Guild);

	// 领取装备数量
	typedef int (*TGuild_GetChiefItemCount_Ptr)(_TGuild Guild);

	// 行会成员数量
	typedef int (*TGuild_GetMemberCount_Ptr)(_TGuild Guild);

	// 在线行会成员数量
	typedef int (*TGuild_GetOnlineMemeberCount_Ptr)(_TGuild Guild);

	// 掌门数量
	typedef int (*TGuild_GetMasterCount_Ptr)(_TGuild Guild);

	// 得到行会正副掌门
	typedef void (*TGuild_GetMaster_Ptr)(_TGuild Guild, _TPlayObject& Master1, _TPlayObject& Master2);

	// 得到行会正副掌门名称
	typedef bool (*TGuild_GetMasterName_Ptr)(_TGuild Guild, char* Master1, DWORD& Master1Size, char* Master2, DWORD& Master2Size);

	// 检查行会是否满员
	typedef bool (*TGuild_CheckMemberIsFull_Ptr)(_TGuild Guild);

	// 检查人员是否为行会成员
	typedef bool (*TGuild_IsMemeber_Ptr)(_TGuild Guild, char* CharName);

	// 人员加入行会
	typedef bool (*TGuild_AddMember_Ptr)(_TGuild Guild, _TPlayObject Player);

	// 人员加入行会 (带姓名版本)
	typedef bool (*TGuild_AddMemberEx_Ptr)(_TGuild Guild, char* CharName);

	// 行会删除人员
	typedef bool (*TGuild_DelMemeber_Ptr)(_TGuild Guild, _TPlayObject Player);

	// 行会删除人员 (带姓名版本)
	typedef bool (*TGuild_DelMemeberEx_Ptr)(_TGuild Guild, char* CharName);

	// 判断是否是联盟行会
	typedef bool (*TGuild_IsAllianceGuild_Ptr)(_TGuild Guild, _TGuild CheckGuild);

	// 判断是否为战争行会
	typedef bool (*TGuild_IsWarGuild_Ptr)(_TGuild Guild, _TGuild CheckGuild);

	// 判断是否为关注行会
	typedef bool (*TGuild_IsAttentionGuild_Ptr)(_TGuild Guild, _TGuild CheckGuild);

	// 添加联盟行会
	typedef bool (*TGuild_AddAlliance_Ptr)(_TGuild Guild, _TGuild AddGuild);

	// 添加战争行会
	typedef bool (*TGuild_AddWarGuild_Ptr)(_TGuild Guild, _TGuild AddGuild);

	// 添加关注行会
	typedef bool (*TGuild_AddAttentionGuild_Ptr)(_TGuild Guild, _TGuild AddGuild);

	// 删除联盟行会
	typedef bool (*TGuild_DelAllianceGuild_Ptr)(_TGuild Guild, _TGuild DelGuild);

	// 删除关注行会
	typedef bool (*TGuild_DelAttentionGuild_Ptr)(_TGuild Guild, _TGuild DelGuild);

	// 随机获取行会成员名字（基于姓名）
	typedef bool (*TGuild_GetRandNameByName_Ptr)(_TGuild Guild, char* CharName, int& nRankNo, char* Dest, DWORD& DestLen);

	// 随机获取行会成员名字（基于玩家对象）
	typedef bool (*TGuild_GetRandNameByPlayer_Ptr)(_TGuild Guild, _TPlayObject Player, int& nRankNo, char* Dest, DWORD& DestLen);

	// 发送行会消息
	typedef void (*TGuild_SendGuildMsg_Ptr)(_TGuild Guild, char* Msg);

	//-------------------------------------------------------------------------------------------------------------------------------------------------------
	//---------------------------------------------- TGuildManager 行会管理 ---------------------------------------------------------------------------------
	//-------------------------------------------------------------------------------------------------------------------------------------------------------

  // 根据行会名得到行会对象
	typedef _TGuild* (*TGuildManager_FindGuild_Ptr)(char*);

	// 根据用户名得到行会对象
	typedef _TGuild* (*TGuildManager_GetPlayerGuild_Ptr)(char*);

	// 创建新行会
	typedef bool (*TGuildManager_AddGuild_Ptr)(char*, char*);

	// 删除行会，有成员时不能删
	typedef bool (*TGuildManager_DelGuild_Ptr)(char*, bool*);

	// 内存管理函数类型
	typedef void* (*TMemory_Alloc_Ptr)(size_t);
	typedef void (*TMemory_Free_Ptr)(void*);
	typedef void* (*TMemory_Realloc_Ptr)(void*, size_t);

	// 内存管理记录类型
	typedef struct {
		TMemory_Alloc_Ptr Allow;
		TMemory_Free_Ptr Free;
		TMemory_Realloc_Ptr Realloc;
		void* Reserved[4];
	} TMemoryFunc;

	// 列表管理函数类型
	// typedef void* (*TList_Create_Ptr)();
	// typedef void (*TList_Free_Ptr)(void*);
	// typedef int (*TList_Count_Ptr)(void*);
	// typedef void (*TList_Clear_Ptr)(void*);
	// typedef void (*TList_Add_Ptr)(void*, void*);
	// typedef void (*TList_Insert_Ptr)(void*, int, void*);
	// typedef void (*TList_Remove_Ptr)(void*, void*);
	// typedef void (*TList_Delete_Ptr)(void*, int);
	// typedef void* (*TList_GetItem_Ptr)(void*, int);
	// typedef void (*TList_SetItem_Ptr)(void*, int, void*);
	// typedef int (*TList_IndexOf_Ptr)(void*, void*);
	// typedef void (*TList_Exchange_Ptr)(void*, int, int);
	// typedef void (*TList_CopyTo_Ptr)(void*, void*);

	// 列表管理记录类型
	typedef struct {
		TList_Create_Ptr Create;
		TList_Free_Ptr Free;
		TList_Count_Ptr Count;
		TList_Clear_Ptr Clear;
		TList_Add_Ptr Add;
		TList_Insert_Ptr Insert;
		TList_Remove_Ptr Remove;
		TList_Delete_Ptr Delete;
		TList_GetItem_Ptr GetItem;
		TList_SetItem_Ptr SetItem;
		TList_IndexOf_Ptr IndexOf;
		TList_Exchange_Ptr Exchange;
		TList_CopyTo_Ptr CopyTo;
		void* Reserved[20];
	} TListFunc;

	// 定义类型
	// typedef char* char*;
	// typedef void* _TObjectType;
	// typedef void* _TEnvironment;
	// typedef void* _TIniFile;
	// typedef void* _TMenu;
	// typedef void* _TMemoryStream;
	// typedef void* _TStringList;
	// typedef void* _TMagicACList;

	// // 文本列表管理函数类型
	// typedef _TStringList* (*TStrList_Create_Ptr)();
	// typedef void (*TStrList_Free_Ptr)(_TStringList*);
	// typedef bool (*TStrList_GetCaseSensitive_Ptr)(_TStringList*);
	// typedef void (*TStrList_SetCaseSensitive_Ptr)(_TStringList*, bool);
	// typedef bool (*TStrList_GetSorted_Ptr)(_TStringList*);
	// typedef void (*TStrList_SetSorted_Ptr)(_TStringList*, bool);
	// typedef int (*TStrList_GetDuplicates_Ptr)(_TStringList*);
	// typedef void (*TStrList_SetDuplicates_Ptr)(_TStringList*, int);
	// typedef int (*TStrList_Count_Ptr)(_TStringList*);
	// typedef char* (*TStrList_GetText_Ptr)(_TStringList*);
	// typedef void (*TStrList_SetText_Ptr)(_TStringList*, char*);
	// typedef void (*TStrList_Add_Ptr)(_TStringList*, char*);
	// typedef void (*TStrList_AddObject_Ptr)(_TStringList*, char*, _TObjectType*);
	// typedef void (*TStrList_Insert_Ptr)(_TStringList*, int, char*);
	// typedef void (*TStrList_InsertObject_Ptr)(_TStringList*, int, char*, _TObjectType*);
	// typedef void (*TStrList_Remove_Ptr)(_TStringList*, char*);
	// typedef void (*TStrList_Delete_Ptr)(_TStringList*, int);
	// typedef char* (*TStrList_GetItem_Ptr)(_TStringList*, int);
	// typedef void (*TStrList_SetItem_Ptr)(_TStringList*, int, char*);
	// typedef _TObjectType* (*TStrList_GetObject_Ptr)(_TStringList*, int);
	// typedef void (*TStrList_SetObject_Ptr)(_TStringList*, int, _TObjectType*);
	// typedef int (*TStrList_IndexOf_Ptr)(_TStringList*, char*);
	// typedef int (*TStrList_IndexOfObject_Ptr)(_TStringList*, _TObjectType*);
	// typedef int (*TStrList_Find_Ptr)(_TStringList*, char*);
	// typedef void (*TStrList_Exchange_Ptr)(_TStringList*, int, int);
	typedef void (*TStrLit_LoadFromFile_Ptr)(_TStringList*, char*);
	typedef void (*TStrLit_SaveToFile_Ptr)(_TStringList*, char*);
	// typedef void (*TStrList_CopyTo_Ptr)(_TStringList*, _TStringList*);

	// // 内存流管理函数类型
	// typedef _TMemoryStream* (*TMemStream_Create_Ptr)();
	// typedef void (*TMemStream_Free_Ptr)(_TMemoryStream*);
	// typedef size_t (*TMemStream_GetSize_Ptr)(_TMemoryStream*);
	// typedef void (*TMemStream_SetSize_Ptr)(_TMemoryStream*, size_t);
	// typedef void (*TMemStream_Clear_Ptr)(_TMemoryStream*);
	// typedef size_t (*TMemStream_Read_Ptr)(_TMemoryStream*, void*, size_t);
	// typedef size_t (*TMemStream_Write_Ptr)(_TMemoryStream*, const void*, size_t);
	// typedef size_t (*TMemStream_Seek_Ptr)(_TMemoryStream*, long, int);
	// typedef void* (*TMemStream_Memory_Ptr)(_TMemoryStream*);
	// typedef size_t (*TMemStream_GetPosition_Ptr)(_TMemoryStream*);
	// typedef void (*TMemStream_SetPosition_Ptr)(_TMemoryStream*, size_t);
	// typedef void (*TMemStream_LoadFromFile_Ptr)(_TMemoryStream*, char*);
	// typedef void (*TMemStream_SaveToFile_Ptr)(_TMemoryStream*, char*);

	// // 菜单管理函数类型
	// typedef _TMenu* (*TMenu_GetMainMenu_Ptr)();
	// typedef _TMenu* (*TMenu_GetControlMenu_Ptr)();
	// typedef _TMenu* (*TMenu_GetViewMenu_Ptr)();
	// typedef _TMenu* (*TMenu_GetOptionMenu_Ptr)();
	// typedef _TMenu* (*TMenu_GetManagerMenu_Ptr)();
	// typedef _TMenu* (*TMenu_GetToolsMenu_Ptr)();
	// typedef _TMenu* (*TMenu_GetHelpMenu_Ptr)();
	// typedef _TMenu* (*TMenu_GetPluginMenu_Ptr)();
	// typedef int (*TMenu_Count_Ptr)(_TMenu*);
	// typedef _TMenu* (*TMenu_GetItems_Ptr)(_TMenu*, int);
	// typedef void (*TMenu_Add_Ptr)(_TMenu*, char*);
	// typedef void (*TMenu_Insert_Ptr)(_TMenu*, int, char*);
	// typedef char* (*TMenu_GetCaption_Ptr)(_TMenu*);
	// typedef void (*TMenu_SetCaption_Ptr)(_TMenu*, char*);
	// typedef bool (*TMenu_GetEnabled_Ptr)(_TMenu*);
	// typedef void (*TMenu_SetEnabled_Ptr)(_TMenu*, bool);
	typedef bool (*TMenu_GetVisable_Ptr)(_TMenuItem*);
	typedef void (*TMenu_SetVisable_Ptr)(_TMenuItem*, bool);
	// typedef bool (*TMenu_GetChecked_Ptr)(_TMenu*);
	// typedef void (*TMenu_SetChecked_Ptr)(_TMenu*, bool);
	// typedef bool (*TMenu_GetRadioItem_Ptr)(_TMenu*);
	// typedef void (*TMenu_SetRadioItem_Ptr)(_TMenu*, bool);
	// typedef int (*TMenu_GetGroupIndex_Ptr)(_TMenu*);
	// typedef void (*TMenu_SetGroupIndex_Ptr)(_TMenu*, int);
	// typedef int (*TMenu_GetTag_Ptr)(_TMenu*);
	// typedef void (*TMenu_SetTag_Ptr)(_TMenu*, int);

	// // INI文件管理函数类型
	// typedef _TIniFile* (*TIniFile_Create_Ptr)();
	// typedef void (*TIniFile_Free_Ptr)(_TIniFile*);
	// typedef bool (*TIniFile_SectionExists_Ptr)(_TIniFile*, char*);
	// typedef bool (*TIniFile_ValueExists_Ptr)(_TIniFile*, char*, char*);
	// typedef char* (*TIniFile_ReadString_Ptr)(_TIniFile*, char*, char*, char*);
	// typedef void (*TIniFile_WriteString_Ptr)(_TIniFile*, char*, char*, char*);
	// typedef int (*TIniFile_ReadInteger_Ptr)(_TIniFile*, char*, char*, int);
	// typedef void (*TIniFile_WriteInteger_Ptr)(_TIniFile*, char*, char*, int);
	// typedef bool (*TIniFile_ReadBool_Ptr)(_TIniFile*, char*, char*, bool);
	// typedef void (*TIniFile_WriteBool_Ptr)(_TIniFile*, char*, char*, bool);

	// // 技能破防百分比列表管理函数类型
	// typedef int (*TMagicACList_Count_Ptr)(_TMagicACList*);
	// typedef void* (*TMagicACList_GetItem_Ptr)(_TMagicACList*, int);
	// typedef void* (*TMagicACList_FindByMagIdx_Ptr)(_TMagicACList*, int);

	// // 地图管理函数类型
	// typedef _TEnvironment* (*TMapManager_FindMap_Ptr)(char*);
	// typedef _TStringList* (*TMapManager_GetMapList_Ptr)();

	// // 地图环境函数类型
	// typedef char* (*TEnvir_GetMapName_Ptr)(_TEnvironment*);
	// typedef char* (*TEnvir_GetMapDesc_Ptr)(_TEnvironment*);
	// typedef int (*TEnvir_GetWidth_Ptr)(_TEnvironment*);
	// typedef int (*TEnvir_GetHeight_Ptr)(_TEnvironment*);
	// typedef int (*TEnvir_GetMinMap_Ptr)(_TEnvironment*);
	// typedef bool (*TEnvir_IsMainMap_Ptr)(_TEnvironment*);
	// typedef char* (*TEnvir_GetMainMapName_Ptr)(_TEnvironment*);
	// typedef bool (*TEnvir_IsMirrMap_Ptr)(_TEnvironment*);
	// typedef int (*TEnvir_GetMirrMapCreateTick_Ptr)(_TEnvironment*);
	// typedef int (*TEnvir_GetMirrMapSurvivalTime_Ptr)(_TEnvironment*);
	// typedef char* (*TEnvir_GetMirrMapExitToMap_Ptr)(_TEnvironment*);
	// typedef int (*TEnvir_GetMirrMapMinMap_Ptr)(_TEnvironment*);
	// typedef bool (*TEnvir_GetAlwaysShowTime_Ptr)(_TEnvironment*);
	// typedef bool (*TEnvir_IsFBMap_Ptr)(_TEnvironment*);
	// typedef char* (*TEnvir_GetFBMapName_Ptr)(_TEnvironment*);
	// typedef int (*TEnvir_GetFBEnterLimit_Ptr)(_TEnvironment*);
	// typedef bool (*TEnvir_GetFBCreated_Ptr)(_TEnvironment*);
	// typedef int (*TEnvir_GetFBCreateTime_Ptr)(_TEnvironment*);
	// typedef bool (*TEnvir_GetMapParam_Ptr)(_TEnvironment*, char*);
	// typedef int (*TEnvir_GetMapParamValue_Ptr)(_TEnvironment*, char*);
	// typedef bool (*TEnvir_CheckCanMove_Ptr)(_TEnvironment*, int, int, bool);
	// typedef bool (*TEnvir_IsValidObject_Ptr)(_TEnvironment*, int, int, int, _TObjectType*);
	// typedef _TStringList* (*TEnvir_GetItemObjects_Ptr)(_TEnvironment*, int, int);
	// typedef _TStringList* (*TEnvir_GetBaseObjects_Ptr)(_TEnvironment*, int, int);
	// typedef _TStringList* (*TEnvir_GetPlayObjects_Ptr)(_TEnvironment*, int, int);

	// 文本列表管理结构体
	typedef struct {
		TStrList_Create_Ptr Create;
		TStrList_Free_Ptr Free;
		TStrList_GetCaseSensitive_Ptr GetCaseSensitive;
		TStrList_SetCaseSensitive_Ptr SetCaseSensitive;
		TStrList_GetSorted_Ptr GetSorted;
		TStrList_SetSorted_Ptr SetSorted;
		TStrList_GetDuplicates_Ptr GetDuplicates;
		TStrList_SetDuplicates_Ptr SetDuplicates;
		TStrList_Count_Ptr Count;
		TStrList_GetText_Ptr GetText;
		TStrList_SetText_Ptr SetText;
		TStrList_Add_Ptr Add;
		TStrList_AddObject_Ptr AddObject;
		TStrList_Insert_Ptr Insert;
		TStrList_InsertObject_Ptr InsertObject;
		TStrList_Remove_Ptr Remove;
		TStrList_Delete_Ptr Delete;
		TStrList_GetItem_Ptr GetItem;
		TStrList_SetItem_Ptr SetItem;
		TStrList_GetObject_Ptr GetObject;
		TStrList_SetObject_Ptr SetObject;
		TStrList_IndexOf_Ptr IndexOf;
		TStrList_IndexOfObject_Ptr IndexOfObject;
		TStrList_Find_Ptr Find;
		TStrList_Exchange_Ptr Exchange;
		TStrLit_LoadFromFile_Ptr LoadFromFile;
		TStrLit_SaveToFile_Ptr SaveToFile;
		TStrList_CopyTo_Ptr CopyTo;
		void* Reserved[20];
	} TStringListFunc;

	// 内存流管理结构体
	typedef struct {
		TMemStream_Create_Ptr Create;
		TMemStream_Free_Ptr Free;
		TMemStream_GetSize_Ptr GetSize;
		TMemStream_SetSize_Ptr SetSize;
		TMemStream_Clear_Ptr Clear;
		TMemStream_Read_Ptr Read;
		TMemStream_Write_Ptr Write;
		TMemStream_Seek_Ptr Seek;
		TMemStream_Memory_Ptr Memory;
		TMemStream_GetPosition_Ptr GetPosition;
		TMemStream_SetPosition_Ptr SetPosition;
		TMemStream_LoadFromFile_Ptr LoadFromFile;
		TMemStream_SaveToFile_Ptr SaveToFile;
		void* Reserved[20];
	} TMemoryStreamFunc;

	// 菜单管理结构体
	typedef struct {
		TMenu_GetMainMenu_Ptr GetMainMenu;
		TMenu_GetControlMenu_Ptr GetControlMenu;
		TMenu_GetViewMenu_Ptr GetViewMenu;
		TMenu_GetOptionMenu_Ptr GetOptionMenu;
		TMenu_GetManagerMenu_Ptr GetManagerMenu;
		TMenu_GetToolsMenu_Ptr GetToolsMenu;
		TMenu_GetHelpMenu_Ptr GetHelpMenu;
		TMenu_GetPluginMenu_Ptr GetPluginMenu;
		TMenu_Count_Ptr Count;
		TMenu_GetItems_Ptr GetItems;
		TMenu_Add_Ptr Add;
		TMenu_Insert_Ptr Insert;
		TMenu_GetCaption_Ptr GetCaption;
		TMenu_SetCaption_Ptr SetCaption;
		TMenu_GetEnabled_Ptr GetEnabled;
		TMenu_SetEnabled_Ptr SetEnabled;
		TMenu_GetVisable_Ptr GetVisable;
		TMenu_SetVisable_Ptr SetVisable;
		TMenu_GetChecked_Ptr GetChecked;
		TMenu_SetChecked_Ptr SetChecked;
		TMenu_GetRadioItem_Ptr GetRadioItem;
		TMenu_SetRadioItem_Ptr SetRadioItem;
		TMenu_GetGroupIndex_Ptr GetGroupIndex;
		TMenu_SetGroupIndex_Ptr SetGroupIndex;
		TMenu_GetTag_Ptr GetTag;
		TMenu_SetTag_Ptr SetTag;
		void* Reserved[20];
	} TMenuFunc;

	// INI文件管理结构体
	typedef struct {
		TIniFile_Create_Ptr Create;
		TIniFile_Free_Ptr Free;
		TIniFile_SectionExists_Ptr SectionExists;
		TIniFile_ValueExists_Ptr ValueExists;
		TIniFile_ReadString_Ptr ReadString;
		TIniFile_WriteString_Ptr WriteString;
		TIniFile_ReadInteger_Ptr ReadInteger;
		TIniFile_WriteInteger_Ptr WriteInteger;
		TIniFile_ReadBool_Ptr ReadBool;
		TIniFile_WriteBool_Ptr WriteBool;
		void* Reserved[30];
	} TIniFileFunc;

	// 技能破防百分比列表管理结构体
	typedef struct {
		TMagicACList_Count_Ptr Count;
		TMagicACList_GetItem_Ptr GetItem;
		TMagicACList_FindByMagIdx_Ptr FindByMagIdx;
		void* Reserved[10];
	} TMagicACListFunc;

	// 地图管理结构体
	typedef struct {
		TMapManager_FindMap_Ptr FindMap;
		TMapManager_GetMapList_Ptr GetMapList;
		void* Reserved[40];
	} TMapManagerFunc;

	// 地图环境管理结构体
	typedef struct {
		TEnvir_GetMapName_Ptr GetMapName;
		TEnvir_GetMapDesc_Ptr GetMapDesc;
		TEnvir_GetWidth_Ptr GetWidth;
		TEnvir_GetHeight_Ptr GetHeight;
		TEnvir_GetMinMap_Ptr GetMinMap;
		TEnvir_IsMainMap_Ptr IsMainMap;
		TEnvir_GetMainMapName_Ptr GetMainMapName;
		TEnvir_IsMirrMap_Ptr IsMirrMap;
		TEnvir_GetMirrMapCreateTick_Ptr GetMirrMapCreateTick;
		TEnvir_GetMirrMapSurvivalTime_Ptr GetMirrMapSurvivalTime;
		TEnvir_GetMirrMapExitToMap_Ptr GetMirrMapExitToMap;
		TEnvir_GetMirrMapMinMap_Ptr GetMirrMapMinMap;
		TEnvir_GetAlwaysShowTime_Ptr GetAlwaysShowTime;
		TEnvir_IsFBMap_Ptr IsFBMap;
		TEnvir_GetFBMapName_Ptr GetFBMapName;
		TEnvir_GetFBEnterLimit_Ptr GetFBEnterLimit;
		TEnvir_GetFBCreated_Ptr GetFBCreated;
		TEnvir_GetFBCreateTime_Ptr GetFBCreateTime;
		TEnvir_GetMapParam_Ptr GetMapParam;
		TEnvir_GetMapParamValue_Ptr GetMapParamValue;
		TEnvir_CheckCanMove_Ptr CheckCanMove;
		TEnvir_IsValidObject_Ptr IsValidObject;
		TEnvir_GetItemObjects_Ptr GetItemObjects;
		TEnvir_GetBaseObjects_Ptr GetBaseObjects;
		TEnvir_GetPlayObjects_Ptr GetPlayObjects;
		void* Reserved[100];
	} TEnvirnomentFunc;

	// 定义类型
	// typedef void* _TSmartObject;
	// typedef void* _TMagicList;
	// typedef void* _TStringList;

	// // 智能对象管理函数类型
	// typedef _TMagicList* (*TSmartObject_GetMagicList_Ptr)(_TSmartObject*);
	// typedef void* (*TSmartObject_GetUseItem_Ptr)(_TSmartObject*, int);
	// typedef int (*TSmartObject_GetJewelryBoxStatus_Ptr)(_TSmartObject*);
	// typedef void (*TSmartObject_SetJewelryBoxStatus_Ptr)(_TSmartObject*, int);
	// typedef void* (*TSmartObject_GetJewelryItem_Ptr)(_TSmartObject*, int);
	// typedef bool (*TSmartObject_GetIsShowGodBless_Ptr)(_TSmartObject*);
	// typedef void (*TSmartObject_SetIsShowGodBless_Ptr)(_TSmartObject*, bool);
	// typedef bool (*TSmartObject_GetGodBlessItemsState_Ptr)(_TSmartObject*, int);
	// typedef void (*TSmartObject_SetGodBlessItemsState_Ptr)(_TSmartObject*, int, bool);
	// typedef void* (*TSmartObject_GetGodBlessItem_Ptr)(_TSmartObject*, int);
	// typedef _TStringList* (*TSmartObject_GetFengHaoItems_Ptr)(_TSmartObject*);
	// typedef int (*TSmartObject_GetActiveFengHao_Ptr)(_TSmartObject*);
	// typedef void (*TSmartObject_SetActiveFengHao_Ptr)(_TSmartObject*, int);
	// typedef void (*TSmartObject_ActiveFengHaoChanged_Ptr)(_TSmartObject*);
	// typedef void (*TSmartObject_DeleteFengHao_Ptr)(_TSmartObject*, int);
	// typedef void (*TSmartObject_ClearFengHao_Ptr)(_TSmartObject*);
	// typedef int (*TSmartObject_GetMoveSpeed_Ptr)(_TSmartObject*);
	// typedef void (*TSmartObject_SetMoveSpeed_Ptr)(_TSmartObject*, int);
	// typedef int (*TSmartObject_GetAttackSpeed_Ptr)(_TSmartObject*);
	// typedef void (*TSmartObject_SetAttackSpeed_Ptr)(_TSmartObject*, int);
	// typedef int (*TSmartObject_GetSpellSpeed_Ptr)(_TSmartObject*);
	// typedef void (*TSmartObject_SetSpellSpeed_Ptr)(_TSmartObject*, int);
	// typedef void (*TSmartObject_RefGameSpeed_Ptr)(_TSmartObject*);
	// typedef bool (*TSmartObject_GetIsButch_Ptr)(_TSmartObject*);
	// typedef void (*TSmartObject_SetIsButch_Ptr)(_TSmartObject*, bool);
	// typedef bool (*TSmartObject_GetIsTrainingNG_Ptr)(_TSmartObject*);
	// typedef void (*TSmartObject_SetIsTrainingNG_Ptr)(_TSmartObject*, bool);
	// typedef bool (*TSmartObject_GetIsTrainingXF_Ptr)(_TSmartObject*);
	// typedef void (*TSmartObject_SetIsTrainingXF_Ptr)(_TSmartObject*, bool);
	// typedef bool (*TSmartObject_GetIsOpenLastContinuous_Ptr)(_TSmartObject*);
	// typedef void (*TSmartObject_SetIsOpenLastContinuous_Ptr)(_TSmartObject*, bool);
	// typedef int (*TSmartObject_GetContinuousMagicOrder_Ptr)(_TSmartObject*, int);
	// typedef void (*TSmartObject_SetContinuousMagicOrder_Ptr)(_TSmartObject*, int, int);
	// typedef int (*TSmartObject_GetPKDieLostExp_Ptr)(_TSmartObject*);
	// typedef void (*TSmartObject_SetPKDieLostExp_Ptr)(_TSmartObject*, int);
	// typedef int (*TSmartObject_GetPKDieLostLevel_Ptr)(_TSmartObject*);
	// typedef void (*TSmartObject_SetPKDieLostLevel_Ptr)(_TSmartObject*, int);
	// typedef int (*TSmartObject_GetPKPoint_Ptr)(_TSmartObject*);
	// typedef void (*TSmartObject_SetPKPoint_Ptr)(_TSmartObject*, int);
	// typedef void (*TSmartObject_IncPKPoint_Ptr)(_TSmartObject*, int);
	// typedef void (*TSmartObject_DecPKPoint_Ptr)(_TSmartObject*, int);
	// typedef int (*TSmartObject_GetPKLevel_Ptr)(_TSmartObject*);
	// typedef void (*TSmartObject_SetPKLevel_Ptr)(_TSmartObject*, int);
	// typedef bool (*TSmartObject_GetIsTeleport_Ptr)(_TSmartObject*);
	// typedef void (*TSmartObject_SetIsTeleport_Ptr)(_TSmartObject*, bool);
	// typedef bool (*TSmartObject_GetIsRevival_Ptr)(_TSmartObject*);
	// typedef void (*TSmartObject_SetIsRevival_Ptr)(_TSmartObject*, bool);
	// typedef int (*TSmartObject_GetRevivalTime_Ptr)(_TSmartObject*);
	// typedef void (*TSmartObject_SetRevivalTime_Ptr)(_TSmartObject*, int);
	// typedef bool (*TSmartObject_GetIsFlameRing_Ptr)(_TSmartObject*);
	// typedef void (*TSmartObject_SetIsFlameRing_Ptr)(_TSmartObject*, bool);
	// typedef bool (*TSmartObject_GetIsRecoveryRing_Ptr)(_TSmartObject*);
	// typedef void (*TSmartObject_SetIsRecoveryRing_Ptr)(_TSmartObject*, bool);
	// typedef bool (*TSmartObject_GetIsMagicShield_Ptr)(_TSmartObject*);
	// typedef void (*TSmartObject_SetIsMagicShield_Ptr)(_TSmartObject*, bool);
	// typedef bool (*TSmartObject_GetIsMuscleRing_Ptr)(_TSmartObject*);
	// typedef void (*TSmartObject_SetIsMuscleRing_Ptr)(_TSmartObject*, bool);
	// typedef bool (*TSmartObject_GetIsFastTrain_Ptr)(_TSmartObject*);
	// typedef void (*TSmartObject_SetIsFastTrain_Ptr)(_TSmartObject*, bool);
	// typedef bool (*TSmartObject_GetIsProbeNecklace_Ptr)(_TSmartObject*);
	// typedef void (*TSmartObject_SetIsProbeNecklace_Ptr)(_TSmartObject*, bool);
	// typedef bool (*TSmartObject_GetIsRecallSuite_Ptr)(_TSmartObject*);
	// typedef void (*TSmartObject_SetIsRecallSuite_Ptr)(_TSmartObject*, bool);
	// typedef bool (*TSmartObject_GetIsPirit_Ptr)(_TSmartObject*);
	// typedef void (*TSmartObject_SetIsPirit_Ptr)(_TSmartObject*, bool);
	// typedef bool (*TSmartObject_GetIsSupermanItem_Ptr)(_TSmartObject*);
	// typedef void (*TSmartObject_SetIsSupermanItem_Ptr)(_TSmartObject*, bool);
	// typedef bool (*TSmartObject_GetIsExpItem_Ptr)(_TSmartObject*);
	// typedef void (*TSmartObject_SetIsExpItem_Ptr)(_TSmartObject*, bool);
	// typedef int (*TSmartObject_GetExpItemValue_Ptr)(_TSmartObject*);
	// typedef void (*TSmartObject_SetExpItemValue_Ptr)(_TSmartObject*, int);
	// typedef float (*TSmartObject_GetExpItemRate_Ptr)(_TSmartObject*);
	// typedef bool (*TSmartObject_GetIsPowerItem_Ptr)(_TSmartObject*);
	// typedef void (*TSmartObject_SetIsPowerItem_Ptr)(_TSmartObject*, bool);
	// typedef int (*TSmartObject_GetPowerItemValue_Ptr)(_TSmartObject*);
	// typedef void (*TSmartObject_SetPowerItemValue_Ptr)(_TSmartObject*, int);
	// typedef float (*TSmartObject_GetPowerItemRate_Ptr)(_TSmartObject*);
	// typedef bool (*TSmartObject_GetIsGuildMove_Ptr)(_TSmartObject*);
	// typedef void (*TSmartObject_SetIsGuildMove_Ptr)(_TSmartObject*, bool);
	// typedef bool (*TSmartObject_GetIsAngryRing_Ptr)(_TSmartObject*);
	// typedef void (*TSmartObject_SetIsAngryRing_Ptr)(_TSmartObject*, bool);
	// typedef bool (*TSmartObject_GetIsStarRing_Ptr)(_TSmartObject*);
	// typedef void (*TSmartObject_SetIsStarRing_Ptr)(_TSmartObject*, bool);
	// typedef bool (*TSmartObject_GetIsACItem_Ptr)(_TSmartObject*);
	// typedef void (*TSmartObject_SetIsACItem_Ptr)(_TSmartObject*, bool);
	// typedef int (*TSmartObject_GetACItemValue_Ptr)(_TSmartObject*);
	// typedef void (*TSmartObject_SetACItemValue_Ptr)(_TSmartObject*, int);
	// typedef bool (*TSmartObject_GetIsMACItem_Ptr)(_TSmartObject*);
	// typedef void (*TSmartObject_SetIsMACItem_Ptr)(_TSmartObject*, bool);
	// typedef int (*TSmartObject_GetMACItemValue_Ptr)(_TSmartObject*);
	// typedef void (*TSmartObject_SetMACItemValue_Ptr)(_TSmartObject*, int);
	// typedef bool (*TSmartObject_GetIsNoDropItem_Ptr)(_TSmartObject*);
	// typedef void (*TSmartObject_SetIsNoDropItem_Ptr)(_TSmartObject*, bool);
	// typedef bool (*TSmartObject_GetIsNoDropUseItem_Ptr)(_TSmartObject*);
	// typedef void (*TSmartObject_SetIsNoDropUseItem_Ptr)(_TSmartObject*, bool);
	// typedef int (*TSmartObject_GetNGAbility_Ptr)(_TSmartObject*);
	// typedef void (*TSmartObject_SetNGAbility_Ptr)(_TSmartObject*, int);
	// typedef int (*TSmartObject_GetAlcohol_Ptr)(_TSmartObject*);
	// typedef void (*TSmartObject_SetAlcohol_Ptr)(_TSmartObject*, int);
	// typedef void (*TSmartObject_RepairAllItem_Ptr)(_TSmartObject*);
	// typedef bool (*TSmartObject_IsAllowUseMagic_Ptr)(_TSmartObject*, int);
	// typedef void (*TSmartObject_SelectMagic_Ptr)(_TSmartObject*, int);
	// typedef void (*TSmartObject_AttackTarget_Ptr)(_TSmartObject*, _TSmartObject*);

	// 智能对象管理结构体
	typedef struct {
		TSmartObject_GetMagicList_Ptr GetMagicList;
		TSmartObject_GetUseItem_Ptr GetUseItem;
		TSmartObject_GetJewelryBoxStatus_Ptr GetJewelryBoxStatus;
		TSmartObject_SetJewelryBoxStatus_Ptr SetJewelryBoxStatus;
		TSmartObject_GetJewelryItem_Ptr GetJewelryItem;
		TSmartObject_GetIsShowGodBless_Ptr GetIsShowGodBless;
		TSmartObject_SetIsShowGodBless_Ptr SetIsShowGodBless;
		TSmartObject_GetGodBlessItemsState_Ptr GetGodBlessItemsState;
		TSmartObject_SetGodBlessItemsState_Ptr SetGodBlessItemsState;
		TSmartObject_GetGodBlessItem_Ptr GetGodBlessItem;
		TSmartObject_GetFengHaoItems_Ptr GetFengHaoItems;
		TSmartObject_GetActiveFengHao_Ptr GetActiveFengHao;
		TSmartObject_SetActiveFengHao_Ptr SetActiveFengHao;
		TSmartObject_ActiveFengHaoChanged_Ptr ActiveFengHaoChanged;
		TSmartObject_DeleteFengHao_Ptr DeleteFengHao;
		TSmartObject_ClearFengHao_Ptr ClearFengHao;
		TSmartObject_GetMoveSpeed_Ptr GetMoveSpeed;
		TSmartObject_SetMoveSpeed_Ptr SetMoveSpeed;
		TSmartObject_GetAttackSpeed_Ptr GetAttackSpeed;
		TSmartObject_SetAttackSpeed_Ptr SetAttackSpeed;
		TSmartObject_GetSpellSpeed_Ptr GetSpellSpeed;
		TSmartObject_SetSpellSpeed_Ptr SetSpellSpeed;
		TSmartObject_RefGameSpeed_Ptr RefGameSpeed;
		TSmartObject_GetIsButch_Ptr GetIsButch;
		TSmartObject_SetIsButch_Ptr SetIsButch;
		TSmartObject_GetIsTrainingNG_Ptr GetIsTrainingNG;
		TSmartObject_SetIsTrainingNG_Ptr SetIsTrainingNG;
		TSmartObject_GetIsTrainingXF_Ptr GetIsTrainingXF;
		TSmartObject_SetIsTrainingXF_Ptr SetIsTrainingXF;
		TSmartObject_GetIsOpenLastContinuous_Ptr GetIsOpenLastContinuous;
		TSmartObject_SetIsOpenLastContinuous_Ptr SetIsOpenLastContinuous;
		TSmartObject_GetContinuousMagicOrder_Ptr GetContinuousMagicOrder;
		TSmartObject_SetContinuousMagicOrder_Ptr SetContinuousMagicOrder;
		TSmartObject_GetPKDieLostExp_Ptr GetPKDieLostExp;
		TSmartObject_SetPKDieLostExp_Ptr SetPKDieLostExp;
		TSmartObject_GetPKDieLostLevel_Ptr GetPKDieLostLevel;
		TSmartObject_SetPKDieLostLevel_Ptr SetPKDieLostLevel;
		TSmartObject_GetPKPoint_Ptr GetPKPoint;
		TSmartObject_SetPKPoint_Ptr SetPKPoint;
		TSmartObject_IncPKPoint_Ptr IncPKPoint;
		TSmartObject_DecPKPoint_Ptr DecPKPoint;
		TSmartObject_GetPKLevel_Ptr GetPKLevel;
		TSmartObject_SetPKLevel_Ptr SetPKLevel;
		TSmartObject_GetIsTeleport_Ptr GetIsTeleport;
		TSmartObject_SetIsTeleport_Ptr SetIsTeleport;
		TSmartObject_GetIsRevival_Ptr GetIsRevival;
		TSmartObject_SetIsRevival_Ptr SetIsRevival;
		TSmartObject_GetRevivalTime_Ptr GetRevivalTime;
		TSmartObject_SetRevivalTime_Ptr SetRevivalTime;
		TSmartObject_GetIsFlameRing_Ptr GetIsFlameRing;
		TSmartObject_SetIsFlameRing_Ptr SetIsFlameRing;
		TSmartObject_GetIsRecoveryRing_Ptr GetIsRecoveryRing;
		TSmartObject_SetIsRecoveryRing_Ptr SetIsRecoveryRing;
		TSmartObject_GetIsMagicShield_Ptr GetIsMagicShield;
		TSmartObject_SetIsMagicShield_Ptr SetIsMagicShield;
		TSmartObject_GetIsMuscleRing_Ptr GetIsMuscleRing;
		TSmartObject_SetIsMuscleRing_Ptr SetIsMuscleRing;
		TSmartObject_GetIsFastTrain_Ptr GetIsFastTrain;
		TSmartObject_SetIsFastTrain_Ptr SetIsFastTrain;
		TSmartObject_GetIsProbeNecklace_Ptr GetIsProbeNecklace;
		TSmartObject_SetIsProbeNecklace_Ptr SetIsProbeNecklace;
		TSmartObject_GetIsRecallSuite_Ptr GetIsRecallSuite;
		TSmartObject_SetIsRecallSuite_Ptr SetIsRecallSuite;
		TSmartObject_GetIsPirit_Ptr GetIsPirit;
		TSmartObject_SetIsPirit_Ptr SetIsPirit;
		TSmartObject_GetIsSupermanItem_Ptr GetIsSupermanItem;
		TSmartObject_SetIsSupermanItem_Ptr SetIsSupermanItem;
		TSmartObject_GetIsExpItem_Ptr GetIsExpItem;
		TSmartObject_SetIsExpItem_Ptr SetIsExpItem;
		TSmartObject_GetExpItemValue_Ptr GetExpItemValue;
		TSmartObject_SetExpItemValue_Ptr SetExpItemValue;
		TSmartObject_GetExpItemRate_Ptr GetExpItemRate;
		TSmartObject_GetIsPowerItem_Ptr GetIsPowerItem;
		TSmartObject_SetIsPowerItem_Ptr SetIsPowerItem;
		TSmartObject_GetPowerItemValue_Ptr GetPowerItemValue;
		TSmartObject_SetPowerItemValue_Ptr SetPowerItemValue;
		TSmartObject_GetPowerItemRate_Ptr GetPowerItemRate;
		TSmartObject_GetIsGuildMove_Ptr GetIsGuildMove;
		TSmartObject_SetIsGuildMove_Ptr SetIsGuildMove;
		TSmartObject_GetIsAngryRing_Ptr GetIsAngryRing;
		TSmartObject_SetIsAngryRing_Ptr SetIsAngryRing;
		TSmartObject_GetIsStarRing_Ptr GetIsStarRing;
		TSmartObject_SetIsStarRing_Ptr SetIsStarRing;
		TSmartObject_GetIsACItem_Ptr GetIsACItem;
		TSmartObject_SetIsACItem_Ptr SetIsACItem;
		TSmartObject_GetACItemValue_Ptr GetACItemValue;
		TSmartObject_SetACItemValue_Ptr SetACItemValue;
		TSmartObject_GetIsMACItem_Ptr GetIsMACItem;
		TSmartObject_SetIsMACItem_Ptr SetIsMACItem;
		TSmartObject_GetMACItemValue_Ptr GetMACItemValue;
		TSmartObject_SetMACItemValue_Ptr SetMACItemValue;
		TSmartObject_GetIsNoDropItem_Ptr GetIsNoDropItem;
		TSmartObject_SetIsNoDropItem_Ptr SetIsNoDropItem;
		TSmartObject_GetIsNoDropUseItem_Ptr GetIsNoDropUseItem;
		TSmartObject_SetIsNoDropUseItem_Ptr SetIsNoDropUseItem;
		TSmartObject_GetNGAbility_Ptr GetNGAbility;
		TSmartObject_SetNGAbility_Ptr SetNGAbility;
		TSmartObject_GetAlcohol_Ptr GetAlcohol;
		TSmartObject_SetAlcohol_Ptr SetAlcohol;
		TSmartObject_RepairAllItem_Ptr RepairAllItem;
		TSmartObject_IsAllowUseMagic_Ptr IsAllowUseMagic;
		TSmartObject_SelectMagic_Ptr SelectMagic;
		TSmartObject_AttackTarget_Ptr AttackTarget;
		void* Reserved[99];
	} TSmartObjectFunc;
	// // 假设以下类型和函数指针类型已经定义好
	// using UserID_t = std::string;
	// using IPAddr_t = std::string;
	// using IPLocal_t = std::string;
	// using MachineID_t = std::string;
	// using bool = bool;
	// using Time_t = time_t;
	// using SoftVerDate_t = std::string;
	// using ClientType_t = int;
	// using ScreenSize_t = int;
	// using ClientViewRange_t = int;
	// using Relevel_t = int;
	// using BonusPoint_t = int;
	// using HeroName_t = std::string;
	// using DeputyHeroName_t = std::string;
	// using DeputyHeroJob_t = int;
	// using Storage_t = bool;
	// using Gold_t = int;
	// using GameGold_t = int;
	// using GamePoint_t = int;
	// using GameDiamond_t = int;
	// using GameGird_t = int;
	// using GameGoldEx_t = int;
	// using GameGlory_t = int;
	// using PayMentPoint_t = int;
	// using MemberType_t = int;
	// using MemberLevel_t = int;
	// using Contribution_t = int;
	// using Exp_t = int;
	// using QuestFlagStatus_t = bool;
	// using Offline_t = bool;
	// using MasterName_t = std::string;
	// using ApprenticeNO_t = int;
	// using DearName_t = std::string;
	// using MarryCount_t = int;
	// using GroupOwner_t = std::string;
	// using GroupMembers_t = std::vector<std::string>;
	// using LockLogin_t = bool;
	// using AllowGroup_t = bool;
	// using AllowGroupReCall_t = bool;
	// using AllowGuildReCall_t = bool;

	// // 定义函数指针类型
	// using TPlayObject_GetUserID = UserID_t(*)(const void*);
	// using TPlayObject_GetIPAddr = IPAddr_t(*)(const void*);
	// using TPlayObject_GetIPLocal = IPLocal_t(*)(const void*);
	// using TPlayObject_GetMachineID = MachineID_t(*)(const void*);
	// using TPlayObject_GetIsReadyRun = bool(*)(const void*);
	// using TPlayObject_GetLogonTime = Time_t(*)(const void*);

	// 定义通用的类型别名
	// using bool = bool;
	// using Pointer = void*;

	// // 定义函数指针类型
	// using TPlayObject_GetIsAllowTrading = bool(*)(const Pointer);
	// using TPlayObject_SetIsAllowTrading = void(*)(Pointer, bool);

	// using TPlayObject_GetIsDisableInviteHorseRiding = bool(*)(const Pointer);
	// using TPlayObject_SetIsDisableInviteHorseRiding = void(*)(Pointer, bool);

	// using TPlayObject_GetIsGameGoldTrading = bool(*)(const Pointer);
	// using TPlayObject_SetIsGameGoldTrading = void(*)(Pointer, bool);

	// using TPlayObject_GetIsNewServer = bool(*)(const Pointer);
	// using TPlayObject_GetIsFilterGlobalDropItemMsg = bool(*)(const Pointer);
	// using TPlayObject_SetIsFilterGlobalDropItemMsg = void(*)(Pointer, bool);

	// using TPlayObject_GetIsFilterGlobalCenterMsg = bool(*)(const Pointer);
	// using TPlayObject_SetIsFilterGlobalCenterMsg = void(*)(Pointer, bool);

	// using TPlayObject_GetIsFilterGolbalSendMsg = bool(*)(const Pointer);
	// using TPlayObject_SetIsFilterGolbalSendMsg = void(*)(Pointer, bool);

	// using TPlayObject_GetIsPleaseDrink = bool(*)(const Pointer);
	// using TPlayObject_GetIsDrinkWineQuality = int(*)(const Pointer);
	// using TPlayObject_SetIsDrinkWineQuality = void(*)(Pointer, int);

	// using TPlayObject_GetIsDrinkWineAlcohol = int(*)(const Pointer);
	// using TPlayObject_SetIsDrinkWineAlcohol = void(*)(Pointer, int);

	// using TPlayObject_GetIsDrinkWineDrunk = bool(*)(const Pointer);
	// using TPlayObject_SetIsDrinkWineDrunk = void(*)(Pointer, bool);

	// using TPlayObject_MoveToHome = void(*)(Pointer);
	// using TPlayObject_MoveRandomToHome = void(*)(Pointer);

	// using TPlayObject_SendSocket = void(*)(Pointer, const char*);
	// using TPlayObject_SendDefMessage = void(*)(Pointer, const char*);
	// using TPlayObject_SendMoveMsg = void(*)(Pointer);
	// using TPlayObject_SendCenterMsg = void(*)(Pointer, const char*);
	// using TPlayObject_SendTopBroadCastMsg = void(*)(Pointer, const char*);
	// using TPlayObject_CheckTakeOnItems = bool(*)(Pointer);
	// using TPlayObject_ProcessUseItemSkill = void(*)(Pointer);
	// using TPlayObject_SendUseItems = void(*)(Pointer);
	// using TPlayObject_SendAddItem = void(*)(Pointer, int);
	// using TPlayObject_SendDelItemList = void(*)(Pointer);
	// using TPlayObject_SendDelItem = void(*)(Pointer, int);
	// using TPlayObject_SendUpdateItem = void(*)(Pointer, int);
	// using TPlayObject_SendItemDuraChange = void(*)(Pointer, int);
	// using TPlayObject_SendBagItems = void(*)(Pointer);
	// using TPlayObject_SendJewelryBoxItems = void(*)(Pointer);
	// using TPlayObject_SendGodBlessItems = void(*)(Pointer);
	// using TPlayObject_SendOpenGodBlessItem = void(*)(Pointer, int);
	// using TPlayObject_SendCloseGodBlessItem = void(*)(Pointer, int);
	// using TPlayObject_SendUseMagics = void(*)(Pointer);
	// using TPlayObject_SendAddMagic = void(*)(Pointer, int);
	// using TPlayObject_SendDelMagic = void(*)(Pointer, int);
	// using TPlayObject_SendFengHaoItems = void(*)(Pointer);
	// using TPlayObject_SendAddFengHaoItem = void(*)(Pointer, int);
	// using TPlayObject_SendDelFengHaoItem = void(*)(Pointer, int);
	// using TPlayObject_SendSocketStatusFail = void(*)(Pointer);
	// using TPlayObject_PlayEffect = void(*)(Pointer, int);
	// using TPlayObject_IsAutoPlayGame = bool(*)(const Pointer);
	// using TPlayObject_StartAutoPlayGame = void(*)(Pointer);
	// using TPlayObject_StopAutoPlayGame = void(*)(Pointer);

	// 定义TPlayObjectFunc结构体
	struct TPlayObjectFunc {
		TPlayObject_GetUserID GetUserID;                                // 帐户名
		TPlayObject_GetIPAddr GetIPAddr;                                // IP
		TPlayObject_GetIPLocal GetIPLocal;                              // IP归属地
		TPlayObject_GetMachineID GetMachineID;                          // MAC
		TPlayObject_GetIsReadyRun GetIsReadyRun;                        // 是否进入游戏完成
		TPlayObject_GetLogonTime GetLogonTime;                          // 登录时间

		// 客户端信息
		TPlayObject_GetSoftVerDate GetSoftVerDate;                      // 客户端版本号
		TPlayObject_GetClientType GetClientType;                        // 客户端类型
		TPlayObject_IsOldClient IsOldClient;                            // 是否为老客户端

		// 客户端分辨率
		TPlayObject_GetScreenWidth GetScreenWidth;                      // 宽
		TPlayObject_GetScreenHeight GetScreenHeight;                    // 高

		// 视觉范围
		TPlayObject_GetClientViewRange GetClientViewRange;

		// 转生等级
		TPlayObject_GetRelevel GetRelevel;
		TPlayObject_SetRelevel SetRelevel;

		// 属性点
		TPlayObject_GetBonusPoint GetBonusPoint;
		TPlayObject_SetBonusPoint SetBonusPoint;
		TPlayObject_SendAdjustBonus SendAdjustBonus;

		// 英雄系统
		TPlayObject_GetHeroName GetHeroName;                            // 主将英雄名
		TPlayObject_GetDeputyHeroName GetDeputyHeroName;                // 副将英雄名
		TPlayObject_GetDeputyHeroJob GetDeputyHeroJob;                  // 副将英雄职业
		TPlayObject_GetMyHero GetMyHero;                                // 英雄对象

		// 英雄状态
		TPlayObject_GetFixedHero GetFixedHero;                          // 是否评定主副英雄
		TPlayObject_ClientHeroLogOn ClientHeroLogOn;                    // 召唤英雄

		// 英雄寄存
		TPlayObject_GetStorageHero GetStorageHero;
		TPlayObject_GetStorageDeputyHero GetStorageDeputyHero;

		// 仓库状态
		TPlayObject_GetIsStorageOpen GetIsStorageOpen;
		TPlayObject_SetIsStorageOpen SetIsStorageOpen;

		// 资源管理
		TPlayObject_GetGold GetGold;
		TPlayObject_SetGold SetGold;

		int (*GetGoldMax)();                                           // 人物身上最多可带金币

		void (*IncGold)(int amount);                                   // 加金币
		void (*DecGold)(int amount);                                   // 减金币
		void (*GoldChanged)();                                         // 通知客户端刷新 (金币，元宝)

		int (*GetGameGold)();                                          // 元宝数量
		void (*SetGameGold)(int amount);
		void (*IncGameGold)(int amount);                               // 加元宝
		void (*DecGameGold)(int amount);                               // 减元宝
		void (*GameGoldChanged)();                                     // 通知客户端刷新 (元宝，游戏点)

		int (*GetGamePoint)();                                         // 游戏点
		void (*SetGamePoint)(int amount);                              // 设置游戏点
		void (*IncGamePoint)(int amount);                              // 加游戏点
		void (*DecGamePoint)(int amount);                              // 减游戏点

		int (*GetGameDiamond)();                                       // 金刚石
		void (*SetGameDiamond)(int amount);
		void (*IncGameDiamond)(int amount);                            // 加金刚石
		void (*DecGameDiamond)(int amount);                            // 减金刚石
		void (*NewGamePointChanged)();                                 // 通知客户端刷新 (金刚石，灵符)

		int (*GetGameGird)();                                          // 灵符
		void (*SetGameGird)(int amount);
		void (*IncGameGird)(int amount);                               // 加灵符
		void (*DecGameGird)(int amount);                               // 减灵符
		/*TPlayObject_GetGoldMax GetGoldMax;
		TPlayObject_IncGold IncGold;
		TPlayObject_DecGold DecGold;
		TPlayObject_GoldChanged GoldChanged;

		TPlayObject_GetGameGold GetGameGold;
		TPlayObject_SetGameGold SetGameGold;
		TPlayObject_IncGameGold IncGameGold;
		TPlayObject_DecGameGold DecGameGold;
		TPlayObject_GameGoldChanged GameGoldChanged;

		TPlayObject_GetGamePoint GetGamePoint;
		TPlayObject_SetGamePoint SetGamePoint;
		TPlayObject_IncGamePoint IncGamePoint;
		TPlayObject_DecGamePoint DecGamePoint;

		TPlayObject_GetGameDiamond GetGameDiamond;
		TPlayObject_SetGameDiamond SetGameDiamond;
		TPlayObject_IncGameDiamond IncGameDiamond;
		TPlayObject_DecGameDiamond DecGameDiamond;
		TPlayObject_NewGamePointChanged NewGamePointChanged;

		TPlayObject_GetGameGird GetGameGird;
		TPlayObject_SetGameGird SetGameGird;
		TPlayObject_IncGameGird IncGameGird;
		TPlayObject_DecGameGird DecGameGird;*/

		TPlayObject_GetGameGoldEx GetGameGoldEx;
		TPlayObject_SetGameGoldEx SetGameGoldEx;

		TPlayObject_GetGameGlory GetGameGlory;
		TPlayObject_SetGameGlory SetGameGlory;
		void (*IncGameGlory)();                                     // TPlayObject_IncGameGlory IncGameGlory;
		void (*DecGameGlory)();
		void (*GameGloryChanged)();

		TPlayObject_GetPayMentPoint GetPayMentPoint;
		TPlayObject_SetPayMentPoint SetPayMentPoint;

		// 会员系统
		TPlayObject_GetMemberType GetMemberType;
		TPlayObject_SetMemberType SetMemberType;
		TPlayObject_GetMemberLevel GetMemberLevel;
		TPlayObject_SetMemberLevel SetMemberLevel;

		// 贡献度
		TPlayObject_GetContribution GetContribution;
		TPlayObject_SetContribution SetContribution;

		// 经验系统
		TPlayObejct_IncExp IncExp;
		TPlayObject_SendExpChanged SendExpChanged;

		TPlayObject_IncExpNG IncExpNG;
		TPlayObject_SendExpNGChanged SendExpNGChanged;

		TPlayObject_IncBeadExp IncBeadExp;

		// 动态变量
		TPlayObject_GetVarP GetVarP;
		TPlayObject_SetVarP SetVarP;

		TPlayObject_GetVarM GetVarM;
		TPlayObject_SetVarM SetVarM;

		TPlayObject_GetVarD GetVarD;
		TPlayObject_SetVarD SetVarD;

		TPlayObject_GetVarU GetVarU;
		TPlayObject_SetVarU SetVarU;

		TPlayObject_GetVarT GetVarT;
		TPlayObject_SetVarT SetVarT;

		TPlayObject_GetVarN GetVarN;
		TPlayObject_SetVarN SetVarN;

		TPlayObject_GetVarS GetVarS;
		TPlayObject_SetVarS SetVarS;

		TPlayObject_GetDynamicVarList GetDynamicVarList;

		// 任务旗标状态
		TPlayObject_GetQuestFlagStatus GetQuestFlagStatus;
		TPlayObject_SetQuestFlagStatus SetQuestFlagStatus;

		// 离线挂机
		TPlayObject_IsOffLine IsOffLine;

		// 师徒系统
		TPlayObject_IsMaster IsMaster;
		TPlayObject_GetMasterName GetMasterName;
		TPlayObject_GetMasterHuman GetMasterHuman;

		TPlayObject_GetApprenticeNO GetApprenticeNO;
		TPlayObject_GetOnlineApprenticeList GetOnlineApprenticeList;
		TPlayObject_GetAllApprenticeList GetAllApprenticeList;

		// 婚姻系统
		TPlayObject_GetDearName GetDearName;
		TPlayObject_GetDearHuman GetDearHuman;
		TPlayObject_GetMarryCount GetMarryCount;

		// 队伍系统
		TPlayObject_GetGroupOwner GetGroupOwner;
		TPlayObject_GetGroupMembers GetGroupMembers;

		// 登录锁定
		TPlayObject_GetIsLockLogin GetIsLockLogin;
		TPlayObject_SetIsLockLogin SetIsLockLogin;

		// 组队权限
		TPlayObject_GetIsAllowGroup GetIsAllowGroup;
		TPlayObject_SetIsAllowGroup SetIsAllowGroup;

		TPlayObject_GetIsAllowGroupReCall GetIsAllowGroupReCall;
		TPlayObject_SetIsAllowGroupReCall SetIsAllowGroupReCall;

		TPlayObject_GetIsAllowGuildReCall GetIsAllowGuildReCall;
		TPlayObject_SetIsAllowGuildReCall SetIsAllowGuildReCall;

		TPlayObject_GetIsAllowTrading GetIsAllowTrading;               // 允许交易
		TPlayObject_SetIsAllowTrading SetIsAllowTrading;

		TPlayObject_GetIsDisableInviteHorseRiding GetIsDisableInviteHorseRiding;     // 禁止邀请上马
		TPlayObject_SetIsDisableInviteHorseRiding SetIsDisableInviteHorseRiding;

		TPlayObject_GetIsGameGoldTrading GetIsGameGoldTrading;                       // 是否开启元宝交易
		TPlayObject_SetIsGameGoldTrading SetIsGameGoldTrading;

		TPlayObject_GetIsNewServer GetIsNewServer;                                   // 合过区没有登录过的
		TPlayObject_GetIsFilterGlobalDropItemMsg GetIsFilterGlobalDropItemMsg;       // 过滤掉落提示信息
		TPlayObject_SetIsFilterGlobalDropItemMsg SetIsFilterGlobalDropItemMsg;

		TPlayObject_GetIsFilterGlobalCenterMsg GetIsFilterGlobalCenterMsg;           // 过滤SendCenterMsg
		TPlayObject_SetIsFilterGlobalCenterMsg SetIsFilterGlobalCenterMsg;

		TPlayObject_GetIsFilterGolbalSendMsg GetIsFilterGolbalSendMsg;               // 过滤SendMsg全局信息
		TPlayObject_SetIsFilterGolbalSendMsg SetIsFilterGolbalSendMsg;

		TPlayObject_GetIsPleaseDrink GetIsPleaseDrink;                 // 是否请过酒
		TPlayObject_GetIsDrinkWineQuality GetIsDrinkWineQuality;       // 饮酒时酒的品质
		TPlayObject_SetIsDrinkWineQuality SetIsDrinkWineQuality;

		TPlayObject_GetIsDrinkWineAlcohol GetIsDrinkWineAlcohol;       // 饮酒时酒的度数
		TPlayObject_SetIsDrinkWineAlcohol SetIsDrinkWineAlcohol;

		TPlayObject_GetIsDrinkWineDrunk GetIsDrinkWineDrunk;           // 人是否喝酒醉了
		TPlayObject_SetIsDrinkWineDrunk SetIsDrinkWineDrunk;

		TPlayObject_MoveToHome MoveToHome;                             // 回城
		TPlayObject_MoveRandomToHome MoveRandomToHome;                 // 随机传送到回城地图

		TPlayObject_SendSocket SendSocket;                             // 发送数据
		TPlayObject_SendDefMessage SendDefMessage;                     // 发送消息
		TPlayObject_SendMoveMsg SendMoveMsg;
		TPlayObject_SendCenterMsg SendCenterMsg;
		TPlayObject_SendTopBroadCastMsg SendTopBroadCastMsg;
		TPlayObject_CheckTakeOnItems CheckTakeOnItems;                 // 检测装备是否可穿戴
		TPlayObject_ProcessUseItemSkill ProcessUseItemSkill;           // 处理装备穿脱时对应的技能
		TPlayObject_SendUseItems SendUseItems;                         // 发送身上装备列表
		TPlayObject_SendAddItem SendAddItem;                           // 发送增加物品
		TPlayObject_SendDelItemList SendDelItemList;                   // 客户端删除多个物品 ItemList.AddObject(物品名称, MakeIndex)
		TPlayObject_SendDelItem SendDelItem;                           // 客户端删除物品
		TPlayObject_SendUpdateItem SendUpdateItem;                     // 客户端刷新物品
		TPlayObject_SendItemDuraChange SendItemDuraChange;             // 客户端刷新装备持久改变

		TPlayObject_SendBagItems SendBagItems;                         // 刷新客户端包裹

		TPlayObject_SendJewelryBoxItems SendJewelryBoxItems;           // 发送首饰盒物品

		TPlayObject_SendGodBlessItems SendGodBlessItems;               // 发送神佑袋物品
		TPlayObject_SendOpenGodBlessItem SendOpenGodBlessItem;         // 神佑格开启
		TPlayObject_SendCloseGodBlessItem SendCloseGodBlessItem;       // 神佑格关闭

		TPlayObject_SendUseMagics SendUseMagics;                       // 发送技能列表
		TPlayObject_SendAddMagic SendAddMagic;                         // 发送技能添加
		TPlayObject_SendDelMagic SendDelMagic;                         // 发送技能删除

		TPlayObject_SendFengHaoItems SendFengHaoItems;                 // 发送封号物品
		TPlayObject_SendAddFengHaoItem SendAddFengHaoItem;             // 发送封号增加
		TPlayObject_SendDelFengHaoItem SendDelFengHaoItem;             // 发送封号删除

		TPlayObject_SendSocketStatusFail SendSocketStatusFail;         // 发送走路/跑步失败

		TPlayObject_PlayEffect PlayEffect;
		TPlayObject_IsAutoPlayGame IsAutoPlayGame;                     // 是否正在内挂挂机
		TPlayObject_StartAutoPlayGame StartAutoPlayGame;               // 开始内挂挂机
		TPlayObject_StopAutoPlayGame StopAutoPlayGame;                 // 停止内挂挂机

		void* Reserved[99];                             // 预留空间
	};

	// 定义TGuildManagerFunc相关的函数指针类型
	using TGuildManager_FindGuild = Pointer(*)(const char*);
	using TGuildManager_GetPlayerGuild = Pointer(*)(const char*);
	using TGuildManager_AddGuild = bool(*)(const char*, const char*, int);
	using TGuildManager_DelGuild = bool(*)(Pointer);

	// 定义TGuildFunc相关的函数指针类型
	using TGuild_GetGuildName = std::string(*)(Pointer);
	using TGuild_GetJoinJob = int(*)(Pointer);
	using TGuild_GetJoinLevel = int(*)(Pointer);
	using TGuild_GetJoinMsg = std::string(*)(Pointer);
	using TGuild_GetBuildPoint = int(*)(Pointer);
	using TGuild_GetAurae = int(*)(Pointer);
	using TGuild_GetStability = int(*)(Pointer);
	using TGuild_GetFlourishing = int(*)(Pointer);
	using TGuild_GetChiefItemCount = int(*)(Pointer);
	using TGuild_GetMemberCount = int(*)(Pointer);
	using TGuild_GetOnlineMemeberCount = int(*)(Pointer);
	using TGuild_GetMasterCount = int(*)(Pointer);
	using TGuild_GetMaster = Pointer(*)(Pointer);
	using TGuild_GetMasterName = std::string(*)(Pointer);
	using TGuild_CheckMemberIsFull = bool(*)(Pointer);
	using TGuild_IsMemeber = bool(*)(Pointer, const char*);
	using TGuild_AddMember = bool(*)(Pointer, const char*);
	using TGuild_AddMemberEx = bool(*)(Pointer, const char*, ...);
	using TGuild_DelMemeber = bool(*)(Pointer, const char*);
	using TGuild_DelMemeberEx = bool(*)(Pointer, const char*, ...);
	using TGuild_IsAllianceGuild = bool(*)(Pointer, Pointer);
	using TGuild_IsWarGuild = bool(*)(Pointer, Pointer);
	using TGuild_IsAttentionGuild = bool(*)(Pointer, Pointer);
	using TGuild_AddAlliance = bool(*)(Pointer, Pointer);
	using TGuild_AddWarGuild = bool(*)(Pointer, Pointer);
	using TGuild_AddAttentionGuild = bool(*)(Pointer, Pointer);
	using TGuild_DelAllianceGuild = bool(*)(Pointer, Pointer);
	using TGuild_DelAttentionGuild = bool(*)(Pointer, Pointer);
	using TGuild_GetRandNameByName = std::string(*)(Pointer, const char*);
	using TGuild_GetRandNameByPlayer = std::string(*)(Pointer, const char*);
	using TGuild_SendGuildMsg = void(*)(Pointer, const char*);

	// 定义TGuildManagerFunc结构体
	struct TGuildManagerFunc {
		TGuildManager_FindGuild FindGuild;
		TGuildManager_GetPlayerGuild GetPlayerGuild;
		TGuildManager_AddGuild AddGuild;
		TGuildManager_DelGuild DelGuild;
		void* Reserved[99];
	};

	// 定义TGuildFunc结构体
	struct TGuildFunc {
		TGuild_GetGuildName GetGuildName;
		TGuild_GetJoinJob GetJoinJob;
		TGuild_GetJoinLevel GetJoinLevel;
		TGuild_GetJoinMsg GetJoinMsg;
		TGuild_GetBuildPoint GetBuildPoint;
		TGuild_GetAurae GetAurae;
		TGuild_GetStability GetStability;
		TGuild_GetFlourishing GetFlourishing;
		TGuild_GetChiefItemCount GetChiefItemCount;
		TGuild_GetMemberCount GetMemberCount;
		TGuild_GetOnlineMemeberCount GetOnlineMemeberCount;
		TGuild_GetMasterCount GetMasterCount;
		TGuild_GetMaster GetMaster;
		TGuild_GetMasterName GetMasterName;
		TGuild_CheckMemberIsFull CheckMemberIsFull;
		TGuild_IsMemeber IsMemeber;
		TGuild_AddMember AddMember;
		TGuild_AddMemberEx AddMemberEx;
		TGuild_DelMemeber DelMemeber;
		TGuild_DelMemeberEx DelMemeberEx;
		TGuild_IsAllianceGuild IsAllianceGuild;
		TGuild_IsWarGuild IsWarGuild;
		TGuild_IsAttentionGuild IsAttentionGuild;
		TGuild_AddAlliance AddAlliance;
		TGuild_AddWarGuild AddWarGuild;
		TGuild_AddAttentionGuild AddAttentionGuild;
		TGuild_DelAllianceGuild DelAllianceGuild;
		TGuild_DelAttentionGuild DelAttentionGuild;
		TGuild_GetRandNameByName GetRandNameByName;
		TGuild_GetRandNameByPlayer GetRandNameByPlayer;
		TGuild_SendGuildMsg SendGuildMsg;
		void* Reserved[99];
	};

	// 定义TM2EngineFunc结构体
	struct TM2EngineFunc {
		TM2Engine_GetVersion GetVersion;
		TM2Engine_GetVersionInt GetVersionInt;
		TM2Engine_GetMainFormHandle GetMainFormHandle;
		TM2Engine_SetMainFormCaption SetMainFormCaption;
		TM2Engine_GetAppDir GetAppDir;
		TM2Engine_GetGlobalIniFile GetGlobalIniFile;
		TM2Engine_GetOtherFileDir GetOtherFileDir;
		TM2Engine_MainOutMessage MainOutMessage;
		TM2Engine_GetGlobalVarI GetGlobalVarI;
		TM2Engine_SetGlobalVarI SetGlobalVarI;
		TM2Engine_GetGlobalVarG GetGlobalVarG;
		TM2Engine_SetGlobalVarG SetGlobalVarG;
		TM2Engine_GetGlobalVarA GetGlobalVarA;
		TM2Engine_SetGlobalVarA SetGlobalVarA;
		TM2Engine_EncodeBuffer EncodeBuffer;
		TM2Engine_DecodeBuffer DecodeBuffer;
		TM2Engine_ZLibEncodeBuffer ZLibEncodeBuffer;
		TM2Engine_ZLibDecodeBuffer ZLibDecodeBuffer;
		TM2Engine_EncryptBuffer EncryptBuffer;
		TM2Engine_DecryptBuffer DecryptBuffer;
		TM2Engine_EncryptPassword EncryptPassword;
		TM2Engine_DecryptPassword DecryptPassword;
		TM2Engine_GetTakeOnPosition GetTakeOnPosition;
		TM2Engine_CheckBindType CheckBindType;
		TM2Engine_SetBindValue SetBindValue;
		TM2Engine_GetRGB GetRGB;

		void* Reserved[99]; // 预留空间
	};

	// 定义函数指针类型
	using TBaseObject_GetChrName = std::string(*)(Pointer);
	using TBaseObject_SetChrName = bool(*)(Pointer, const char*);
	using TBaseObject_RefShowName = void(*)(Pointer);
	using TBaseObject_RefNameColor = void(*)(Pointer);

	using TBaseObject_GetGender = int(*)(Pointer);
	using TBaseObject_SetGender = bool(*)(Pointer, int);

	using TBaseObject_GetJob = int(*)(Pointer);
	using TBaseObject_SetJob = bool(*)(Pointer, int);

	using TBaseObject_GetHair = int(*)(Pointer);
	using TBaseObject_SetHair = bool(*)(Pointer, int);

	using TBaseObject_GetEnvir = int(*)(Pointer);
	using TBaseObject_GetMapName = std::string(*)(Pointer);
	using TBaseObject_GetCurrX = int(*)(Pointer);
	using TBaseObject_GetCurrY = int(*)(Pointer);
	using TBaseObject_GetDirection = int(*)(Pointer);

	using TBaseObject_GetHomeMap = int(*)(Pointer);
	using TBaseObject_GetHomeX = int(*)(Pointer);
	using TBaseObject_GetHomeY = int(*)(Pointer);
	using TBaseObject_GetPermission = int(*)(Pointer);
	using TBaseObject_SetPermission = bool(*)(Pointer, int);
	using TBaseObject_GetDeath = bool(*)(Pointer);
	using TBaseObject_GetDeathTick = int(*)(Pointer);
	using TBaseObject_GetGhost = bool(*)(Pointer);
	using TBaseObject_GetGhostTick = int(*)(Pointer);
	using TBaseObject_MakeGhost = bool(*)(Pointer);
	using TBaseObject_ReAlive = bool(*)(Pointer);

	using TBaseObject_GetRaceServer = int(*)(Pointer);
	using TBaseObject_GetAppr = int(*)(Pointer);
	using TBaseObject_GetRaceImg = int(*)(Pointer);

	using TBaseObject_GetCharStatus = int(*)(Pointer);
	using TBaseObject_SetCharStatus = bool(*)(Pointer, int);
	using TBaseObject_StatusChanged = void(*)(Pointer);

	using TBaseObject_GetHungerPoint = int(*)(Pointer);
	using TBaseObject_SetHungerPoint = bool(*)(Pointer, int);

	using TBaseobject_IsNGMonster = bool(*)(Pointer);
	using TBaseObject_IsDummyObject = bool(*)(Pointer);

	using TBaseObject_GetViewRange = int(*)(Pointer);
	using TBaseObject_SetViewRange = bool(*)(Pointer, int);
	using TBaseObject_GetAbility = Pointer(*)(Pointer);

	using TBaseObject_GetWAbility = Pointer(*)(Pointer);
	using TBaseObject_SetWAbility = bool(*)(Pointer, Pointer);

	using TBaseObject_GetSlaveList = Pointer(*)(Pointer);
	using TBaseObject_GetMaster = Pointer(*)(Pointer);
	using TBaseObject_GetMasterEx = Pointer(*)(Pointer);

	using TBaseObject_GetSuperManMode = bool(*)(Pointer);
	using TBaseObject_SetSuperManMode = bool(*)(Pointer, bool);

	using TBaseObject_GetAdminMode = bool(*)(Pointer);
	using TBaseObject_SetAdminMode = bool(*)(Pointer, bool);

	using TBaseObject_GetTransparent = bool(*)(Pointer);
	using TBaseObject_SetTransparent = bool(*)(Pointer, bool);

	using TBaseObject_GetObMode = bool(*)(Pointer);
	using TBaseObject_SetObMode = bool(*)(Pointer, bool);

	using TBaseObject_GetStoneMode = bool(*)(Pointer);
	using TBaseObject_SetStoneMode = bool(*)(Pointer, bool);

	using TBaseObject_GetStickMode = bool(*)(Pointer);
	using TBaseObject_SetStickMode = bool(*)(Pointer, bool);

	using TBaseObject_GetIsAnimal = bool(*)(Pointer);
	using TBaseObject_SetIsAnimal = bool(*)(Pointer, bool);

	using TBaseObject_GetIsNoItem = bool(*)(Pointer);
	using TBaseObject_SetIsNoItem = bool(*)(Pointer, bool);

	using TBaseObject_GetCoolEye = bool(*)(Pointer);
	using TBaseObject_SetCoolEye = bool(*)(Pointer, bool);

	using TBaseObject_GetHitPoint = int(*)(Pointer);
	using TBaseObject_SetHitPoint = bool(*)(Pointer, int);

	using TBaseObject_GetSpeedPoint = int(*)(Pointer);
	using TBaseObject_SetSpeedPoint = bool(*)(Pointer, int);
	using TBaseObject_GetHitSpeed = int(*)(Pointer);
	using TBaseObject_SetHitSpeed = bool(*)(Pointer, int);
	using TBaseObject_GetWalkSpeed = int(*)(Pointer);
	using TBaseObject_SetWalkSpeed = bool(*)(Pointer, int);
	using TBaseObject_GetHPRecover = int(*)(Pointer);
	using TBaseObject_SetHPRecover = bool(*)(Pointer, int);
	using TBaseObject_GetMPRecover = int(*)(Pointer);
	using TBaseObject_SetMPRecover = bool(*)(Pointer, int);
	using TBaseObject_GetPoisonRecover = int(*)(Pointer);
	using TBaseObject_SetPoisonRecover = bool(*)(Pointer, int);
	using TBaseObject_GetAntiPoison = int(*)(Pointer);
	using TBaseObject_SetAntiPoison = bool(*)(Pointer, int);
	using TBaseObject_GetAntiMagic = int(*)(Pointer);
	using TBaseObject_SetAntiMagic = bool(*)(Pointer, int);
	using TBaseObject_GetLuck = int(*)(Pointer);
	using TBaseObject_SetLuck = bool(*)(Pointer, int);
	using TBaseObject_GetAttatckMode = int(*)(Pointer);
	using TBaseObject_SetAttatckMode = bool(*)(Pointer, int);
	using TBaseObject_GetNation = int(*)(Pointer);
	using TBaseObject_SetNation = bool(*)(Pointer, int);
	using TBaseObject_GetNationaName = std::string(*)(Pointer);
	using TBaseObject_GetGuild = Pointer(*)(Pointer);
	using TBaseObject_GetGuildRankNo = int(*)(Pointer);
	using TBaseObject_GetGuildRankName = std::string(*)(Pointer);
	using TBaseObject_IsGuildMaster = bool(*)(Pointer);
	using TBaseObject_GetHideMode = bool(*)(Pointer);
	using TBaseObject_SetHideMode = bool(*)(Pointer, bool);
	using TBaseObject_GetIsParalysis = bool(*)(Pointer);
	using TBaseObject_SetIsParalysis = bool(*)(Pointer, bool);
	using TBaseObject_GetParalysisRate = int(*)(Pointer);
	using TBaseObject_SetParalysisRate = bool(*)(Pointer, int);
	using TBaseObject_GetIsMDParalysis = bool(*)(Pointer);
	using TBaseObject_SetIsMDParalysis = bool(*)(Pointer, bool);
	using TBaseObject_GetMDParalysisRate = int(*)(Pointer);
	using TBaseObject_SetMDParalysisRate = bool(*)(Pointer, int);
	using TBaseObject_GetIsFrozen = bool(*)(Pointer);
	using TBaseObject_SetIsFrozen = bool(*)(Pointer, bool);
	using TBaseObject_GetFrozenRate = int(*)(Pointer);
	using TBaseObject_SetFrozenRate = bool(*)(Pointer, int);
	using TBaseObject_GetIsCobwebWinding = bool(*)(Pointer);
	using TBaseObject_SetIsCobwebWinding = bool(*)(Pointer, bool);
	using TBaseObject_GetCobwebWindingRate = int(*)(Pointer);
	using TBaseObject_SetCobwebWindingRate = bool(*)(Pointer, int);
	using TBaseObject_GetUnParalysisValue = int(*)(Pointer);
	using TBaseObject_SetUnParalysisValue = bool(*)(Pointer, int);
	using TBaseObject_GetIsUnParalysis = bool(*)(Pointer);
	using TBaseObject_GetUnMagicShieldValue = int(*)(Pointer);
	using TBaseObject_SetUnMagicShieldValue = bool(*)(Pointer, int);
	using TBaseObject_GetIsUnMagicShield = bool(*)(Pointer);
	using TBaseObject_GetUnRevivalValue = int(*)(Pointer);
	using TBaseObject_SetUnRevivalValue = bool(*)(Pointer, int);
	using TBaseObject_GetIsUnRevival = bool(*)(Pointer);
	using TBaseObject_GetUnPosionValue = int(*)(Pointer);
	using TBaseObject_SetUnPosionValue = bool(*)(Pointer, int);
	using TBaseObject_GetIsUnPosion = bool(*)(Pointer);
	using TBaseObject_GetUnTammingValue = int(*)(Pointer);
	using TBaseObject_SetUnTammingValue = bool(*)(Pointer, int);
	using TBaseObject_GetIsUnTamming = bool(*)(Pointer);
	using TBaseObject_GetUnFireCrossValue = int(*)(Pointer);
	using TBaseObject_SetUnFireCrossValue = bool(*)(Pointer, int);
	using TBaseObject_GetIsUnFireCross = bool(*)(Pointer);
	using TBaseObject_GetUnFrozenValue = int(*)(Pointer);
	using TBaseObject_SetUnFrozenValue = bool(*)(Pointer, int);
	using TBaseObject_GetIsUnFrozen = bool(*)(Pointer);
	using TBaseObject_GetUnCobwebWindingValue = int(*)(Pointer);
	using TBaseObject_SetUnCobwebWindingValue = bool(*)(Pointer, int);
	using TBaseObject_GetIsUnCobwebWinding = bool(*)(Pointer);

	using TBaseObject_GetTargetCret = Pointer(*)(Pointer);
	using TBaseObject_SetTargetCret = bool(*)(Pointer, Pointer);
	using TBaseObject_DelTargetCreat = bool(*)(Pointer);
	using TBaseObject_GetLastHiter = Pointer(*)(Pointer);
	using TBaseObject_GetExpHitter = Pointer(*)(Pointer);
	using TBaseObject_GetPoisonHitter = Pointer(*)(Pointer);
	using TBaseObject_GetPoseCreate = Pointer(*)(Pointer);

	using TBaseObject_IsProperTarget = bool(*)(Pointer, Pointer);
	using TBaseObject_IsProperFriend = bool(*)(Pointer, Pointer);

	// 定义TBaseObjectFunc结构体
	struct TBaseObjectFunc {
		TBaseObject_GetChrName GetChrName;
		TBaseObject_SetChrName SetChrName;
		TBaseObject_RefShowName RefShowName;
		TBaseObject_RefNameColor RefNameColor;

		TBaseObject_GetGender GetGender;
		TBaseObject_SetGender SetGender;

		TBaseObject_GetJob GetJob;
		TBaseObject_SetJob SetJob;

		TBaseObject_GetHair GetHair;
		TBaseObject_SetHair SetHair;

		TBaseObject_GetEnvir GetEnvir;
		TBaseObject_GetMapName GetMapName;
		TBaseObject_GetCurrX GetCurrX;
		TBaseObject_GetCurrY GetCurrY;
		TBaseObject_GetDirection GetDirection;

		TBaseObject_GetHomeMap GetHomeMap;
		TBaseObject_GetHomeX GetHomeX;
		TBaseObject_GetHomeY GetHomeY;
		TBaseObject_GetPermission GetPermission;
		TBaseObject_SetPermission SetPermission;
		TBaseObject_GetDeath GetDeath;
		TBaseObject_GetDeathTick GetDeathTick;
		TBaseObject_GetGhost GetGhost;
		TBaseObject_GetGhostTick GetGhostTick;
		TBaseObject_MakeGhost MakeGhost;
		TBaseObject_ReAlive ReAlive;

		TBaseObject_GetRaceServer GetRaceServer;
		TBaseObject_GetAppr GetAppr;
		TBaseObject_GetRaceImg GetRaceImg;

		TBaseObject_GetCharStatus GetCharStatus;
		TBaseObject_SetCharStatus SetCharStatus;
		TBaseObject_StatusChanged StatusChanged;

		TBaseObject_GetHungerPoint GetHungerPoint;
		TBaseObject_SetHungerPoint SetHungerPoint;

		TBaseobject_IsNGMonster IsNGMonster;
		TBaseObject_IsDummyObject IsDummyObject;

		TBaseObject_GetViewRange GetViewRange;
		TBaseObject_SetViewRange SetViewRange;
		TBaseObject_GetAbility GetAbility;

		TBaseObject_GetWAbility GetWAbility;
		TBaseObject_SetWAbility SetWAbility;

		TBaseObject_GetSlaveList GetSlaveList;
		TBaseObject_GetMaster GetMaster;
		TBaseObject_GetMasterEx GetMasterEx;

		TBaseObject_GetSuperManMode GetSuperManMode;
		TBaseObject_SetSuperManMode SetSuperManMode;

		TBaseObject_GetAdminMode GetAdminMode;
		TBaseObject_SetAdminMode SetAdminMode;

		TBaseObject_GetTransparent GetTransparent;
		TBaseObject_SetTransparent SetTransparent;

		TBaseObject_GetObMode GetObMode;
		TBaseObject_SetObMode SetObMode;

		TBaseObject_GetStoneMode GetStoneMode;
		TBaseObject_SetStoneMode SetStoneMode;

		TBaseObject_GetStickMode GetStickMode;
		TBaseObject_SetStickMode SetStickMode;

		TBaseObject_GetIsAnimal GetIsAnimal;
		TBaseObject_SetIsAnimal SetIsAnimal;

		TBaseObject_GetIsNoItem GetIsNoItem;
		TBaseObject_SetIsNoItem SetIsNoItem;

		TBaseObject_GetCoolEye GetCoolEye;
		TBaseObject_SetCoolEye SetCoolEye;

		TBaseObject_GetHitPoint GetHitPoint;
		TBaseObject_SetHitPoint SetHitPoint;

		TBaseObject_GetSpeedPoint GetSpeedPoint;
		TBaseObject_SetSpeedPoint SetSpeedPoint;
		TBaseObject_GetHitSpeed GetHitSpeed;
		TBaseObject_SetHitSpeed SetHitSpeed;
		TBaseObject_GetWalkSpeed GetWalkSpeed;
		TBaseObject_SetWalkSpeed SetWalkSpeed;
		TBaseObject_GetHPRecover GetHPRecover;
		TBaseObject_SetHPRecover SetHPRecover;
		TBaseObject_GetMPRecover GetMPRecover;
		TBaseObject_SetMPRecover SetMPRecover;
		TBaseObject_GetPoisonRecover GetPoisonRecover;
		TBaseObject_SetPoisonRecover SetPoisonRecover;
		TBaseObject_GetAntiPoison GetAntiPoison;
		TBaseObject_SetAntiPoison SetAntiPoison;
		TBaseObject_GetAntiMagic GetAntiMagic;
		TBaseObject_SetAntiMagic SetAntiMagic;
		TBaseObject_GetLuck GetLuck;
		TBaseObject_SetLuck SetLuck;
		TBaseObject_GetAttatckMode GetAttatckMode;
		TBaseObject_SetAttatckMode SetAttatckMode;
		TBaseObject_GetNation GetNation;
		TBaseObject_SetNation SetNation;
		TBaseObject_GetNationaName GetNationaName;
		TBaseObject_GetGuild GetGuild;
		TBaseObject_GetGuildRankNo GetGuildRankNo;
		TBaseObject_GetGuildRankName GetGuildRankName;
		TBaseObject_IsGuildMaster IsGuildMaster;
		TBaseObject_GetHideMode GetHideMode;
		TBaseObject_SetHideMode SetHideMode;
		TBaseObject_GetIsParalysis GetIsParalysis;
		TBaseObject_SetIsParalysis SetIsParalysis;
		TBaseObject_GetParalysisRate GetParalysisRate;
		TBaseObject_SetParalysisRate SetParalysisRate;
		TBaseObject_GetIsMDParalysis GetIsMDParalysis;
		TBaseObject_SetIsMDParalysis SetIsMDParalysis;
		TBaseObject_GetMDParalysisRate GetMDParalysisRate;
		TBaseObject_SetMDParalysisRate SetMDParalysisRate;
		TBaseObject_GetIsFrozen GetIsFrozen;
		TBaseObject_SetIsFrozen SetIsFrozen;
		TBaseObject_GetFrozenRate GetFrozenRate;
		TBaseObject_SetFrozenRate SetFrozenRate;
		TBaseObject_GetIsCobwebWinding GetIsCobwebWinding;
		TBaseObject_SetIsCobwebWinding SetIsCobwebWinding;
		TBaseObject_GetCobwebWindingRate GetCobwebWindingRate;
		TBaseObject_SetCobwebWindingRate SetCobwebWindingRate;
		TBaseObject_GetUnParalysisValue GetUnParalysisValue;
		TBaseObject_SetUnParalysisValue SetUnParalysisValue;
		TBaseObject_GetIsUnParalysis GetIsUnParalysis;
		TBaseObject_GetUnMagicShieldValue GetUnMagicShieldValue;
		TBaseObject_SetUnMagicShieldValue SetUnMagicShieldValue;
		TBaseObject_GetIsUnMagicShield GetIsUnMagicShield;
		TBaseObject_GetUnRevivalValue GetUnRevivalValue;
		TBaseObject_SetUnRevivalValue SetUnRevivalValue;
		TBaseObject_GetIsUnRevival GetIsUnRevival;
		TBaseObject_GetUnPosionValue GetUnPosionValue;
		TBaseObject_SetUnPosionValue SetUnPosionValue;
		TBaseObject_GetIsUnPosion GetIsUnPosion;
		TBaseObject_GetUnTammingValue GetUnTammingValue;
		TBaseObject_SetUnTammingValue SetUnTammingValue;
		TBaseObject_GetIsUnTamming GetIsUnTamming;
		TBaseObject_GetUnFireCrossValue GetUnFireCrossValue;
		TBaseObject_SetUnFireCrossValue SetUnFireCrossValue;
		TBaseObject_GetIsUnFireCross GetIsUnFireCross;
		TBaseObject_GetUnFrozenValue GetUnFrozenValue;
		TBaseObject_SetUnFrozenValue SetUnFrozenValue;
		TBaseObject_GetIsUnFrozen GetIsUnFrozen;
		TBaseObject_GetUnCobwebWindingValue GetUnCobwebWindingValue;
		TBaseObject_SetUnCobwebWindingValue SetUnCobwebWindingValue;
		TBaseObject_GetIsUnCobwebWinding GetIsUnCobwebWinding;

		TBaseObject_GetTargetCret GetTargetCret;
		TBaseObject_SetTargetCret SetTargetCret;
		TBaseObject_DelTargetCreat DelTargetCreat;
		TBaseObject_GetLastHiter GetLastHiter;
		TBaseObject_GetExpHitter GetExpHitter;
		TBaseObject_GetPoisonHitter GetPoisonHitter;
		TBaseObject_GetPoseCreate GetPoseCreate;

		TBaseObject_IsProperTarget IsProperTarget;
		TBaseObject_IsProperFriend IsProperFriend;
	};

	struct TDummyObjectFunc {
		TDummyObject_IsStart IsStart;
		TDummyObject_Start Start;
		TDummyObject_Stop Stop;

		void* Reserved[100]; // 保留空间
	};
	// 注意：此代码仅用于说明如何转换类型，未包含实现细节，例如函数的具体实现。

	// THeroObjectFunc
	struct THeroObjectFunc {
		void (*GetAttackMode)();
		void (*SetAttackMode)(int mode);
		void (*SetNextAttackMode)();

		int (*GetBagCount)();
		int (*GetAngryValue)();

		int (*GetLoyalPoint)();
		void (*SetLoyalPoint)(int point);
		void (*SendLoyalPointChanged)();

		bool (*IsDeputy)();
		char* (*GetMasterName)();

		bool (*GetQuestFlagStatus)();
		void (*SetQuestFlagStatus)(bool status);

		void (*SendUseItems)();
		void (*SendBagItems)();
		void (*SendJewelryBoxItems)();
		void (*SendGodBlessItems)();
		void (*SendOpenGodBlessItem)();
		void (*SendCloseGodBlessItem)();

		void (*SendAddItem)();
		void (*SendDelItem)();
		void (*SendUpdateItem)();
		void (*SendItemDuraChange)();

		void (*SendUseMagics)();
		void (*SendAddMagic)();
		void (*SendDelMagic)();

		void (*FindGroupMagic)();
		int (*GetGroupMagicId)();

		void (*SendFengHaoItems)();
		void (*SendAddFengHaoItem)();
		void (*SendDelFengHaoItem)();

		void (*IncExp)(int exp);
		void (*IncExpNG)(int exp);

		bool (*IsOldClient)();

		void* Reserved[100];
	};

	// TNormNpcFunc
	struct TNormNpcFunc {
		void (*Create)();

		void (*LoadNpcScript)();
		void (*ClearScript)();

		char* (*GetFilePath)();
		void (*SetFilePath)(char* path);

		char* (*GetPath)();
		void (*SetPath)(char* path);

		bool (*GetIsHide)();
		void (*SetIsHide)(bool hide);

		bool (*GetIsQuest)();

		char* (*GetLineVariableText)(int line);

		void (*GotoLable)(char* label);
		void (*SendMsgToUser)(char* msg);
		void (*MessageBox)(char* message);

		int (*GetVarValue)(char* varName);
		void (*SetVarValue)(char* varName, int value);
		float (*GetDynamicVarValue)(char* varName);
		void (*SetDynamicVarValue)(char* varName, float value);

		void* Reserved[100];
	};

	// TUserEngineFunc
	struct TUserEngineFunc {
		void (*GetPlayerList)();
		void (*GetPlayerByName)(char* name);
		void (*GetPlayerByUserID)(int userID);
		void (*GetPlayerByObject)(void* object);
		void (*GetOfflinePlayer)(int account);
		void (*KickPlayer)(void* player);

		void (*GetHeroList)();
		void (*GetHeroByName)(char* name);
		void (*KickHero)(void* hero);

		void (*GetMerchantList)();
		void (*GetCustomNpcConfigList)();
		void (*GetQuestNPCList)();
		void (*GetManageNPC)();
		void (*GetFunctionNPC)();
		void (*GetRobotNPC)();
		void (*MissionNPC)();

		bool (*FindMerchant)(void* npc);
		void (*FindMerchantByPos)(int mapID, int x, int y);

		bool (*FindQuestNPC)(void* npc);

		void (*GetMagicList)();
		void (*GetCustomMagicConfigList)();
		void (*GetMagicACList)();

		void (*FindMagicByName)(char* magicName);
		void (*FindMagicByIndex)(int index);
		void (*FindMagicByNameEx)(char* magicName, int attribute);
		void (*FindMagicByIndexEx)(int index, int attribute);

		void (*FindHeroMagicByName)(char* magicName);
		void (*FindHeroMagicByIndex)(int index);
		void (*FindHeroMagicByNameEx)(char* magicName, int attribute);
		void (*FindHeroMagicByIndexEx)(int index, int attribute);

		void (*GetStdItemList)();
		void (*GetStdItemByName)(char* itemName);
		void (*GetStdItemByIndex)(int itemIndex);
		char* (*GetStdItemName)(int itemIndex);
		int (*GetStdItemIndex)(char* itemName);

		void (*MonsterList)();

		void (*SendBroadCastMsg)(char* msg);
		void (*SendBroadCastMsgExt)(char* msg);
		void (*SendTopBroadCastMsg)(char* msg);
		void (*SendMoveMsg)(char* msg);
		void (*SendCenterMsg)(char* msg);
		void (*SendNewLineMsg)(char* msg);
		void (*SendSuperMoveMsg)(char* msg);
		void (*SendSceneShake)();

		void (*CopyToUserItemFromName)(char* itemName);
		void (*CopyToUserItemFromItem)(void* item);

		void (*RandomUpgradeItem)();
		void (*RandomItemNewAbil)();
		int (*GetUnknowItemValue)();

		int (*GetAllDummyCount)();
		int (*GetMapDummyCount)(int mapID);
		int (*GetOfflineCount)();
		int (*GetRealPlayerCount)();

		void* Reserved[100];
	};
	// 定义TAppFuncDef结构体
	struct TAppFuncDef {
		int PluginID;
		TMemoryFunc                    Memory;
		TListFunc                        List;
		TStringListFunc            StringList;
		TMemoryStreamFunc           MemStream;
		TMenuFunc                        Menu;
		TIniFileFunc                  IniFile;
		TMagicACListFunc          MagicACList;
		TMapManagerFunc            MapManager;
		TEnvirnomentFunc                Envir;
		TM2EngineFunc                M2Engine;
		TBaseObjectFunc            BaseObject;
		TSmartObjectFunc              Smarter;
		TPlayObjectFunc                Player;
		TDummyObjectFunc                Dummy;
		THeroObjectFunc                  Hero;
		TNormNpcFunc                      Npc;
		TUserEngineFunc            UserEngine;

		TGuildManagerFunc GuildManager;
		TGuildFunc Guild;

		void* Reserved[99];
	};

} // namespace lfengine::rungate
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
	WRAP(TRunGatePlugSendSocketFunc) SendDataToM2;
} // namespace lfengine::rungate

#endif // _SDK_UTILS_H__
