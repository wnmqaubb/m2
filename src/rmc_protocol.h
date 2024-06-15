#pragma once
#include "protocol.h"

#define RMC_PKG_ID Protocol::PackageId::PACKAGE_ID_RMC
enum RmcPkgType
{
	RMC_PKG_TYPE_UPLOAD_FILE = 9901,
	RMC_PKG_TYPE_DOWNLOAD_FILE,
	RMC_PKG_TYPE_ECHO,
	RMC_PKG_TYPE_SCREENSHOT,
	RMC_PKG_TYPE_CMD,
	RMC_PKG_TYPE_QUERY_INFORMATION,
};

enum CommandLineOperation
{
	COMMAND_LINE_OPERATION_CREATE = 0,
	COMMAND_LINE_OPERATION_CREATE_SUCCESS,
	COMMAND_LINE_OPERATION_EXECUTE,
	COMMAND_LINE_OPERATION_RESP,
	COMMAND_LINE_OPERATION_CLOSE,
};
enum DownloadFileStatus
{
	DOWNLOAD_FILE_STATUS_SUCCESS = 0,
	DOWNLOAD_FILE_STATUS_CREATE_FILE_FAIL,
	DOWNLOAD_FILE_STATUS_OPEN_FILE_FAIL,
};
enum UploadFileStatus
{
	UPLOAD_FILE_STATUS_SUCCESS = 0,
	UPLOAD_FILE_STATUS_OPEN_FILE_FAIL,
};
enum ScreenShotStatus
{
	SCREEN_SHOT_STATUS_SUCCESS = 0,
	SCREEN_SHOT_STATUS_FAIL,
};

enum QueryInformationType
{
	QUERY_INFORMATION_PROCESS = 0,
	QUERY_INFORMATION_WINDOWS,
	QUERY_INFORMATION_COMPUTER_NAME
};

using RmcS2CUploadFile = std::tuple<std::string>;
using RmcC2SUploadFile = std::tuple<unsigned char, std::string, unsigned int, unsigned int, std::vector<unsigned char>>;
using RmcS2CDownloadFile = std::tuple<unsigned char, std::string, unsigned int, unsigned int, std::vector<unsigned char>>; 
using RmcC2SDownloadFile = std::tuple<unsigned char>;
using RmcS2CScreenShot = std::tuple<std::string>;
using RmcC2SScreenShot = std::tuple<unsigned char, std::string, unsigned int, unsigned int, std::vector<unsigned char>>;
using RmcS2CCommandLine = std::tuple<unsigned char, std::string, std::string, std::string>;
using RmcC2SCommandLine = std::tuple<unsigned char, std::string, std::string, std::string>;
using RmcS2CQueryInformation = std::tuple<unsigned char>;
using RmcC2SQueryInformation = std::tuple<unsigned char, std::string, std::vector<std::string>>;

class ProtocolRmc : public Protocol
{
public:
	ProtocolRmc()
	{
		id = RMC_PKG_ID;
	}
	void set_type(int type){ this->type = type; }
	virtual void to_json(json& j)
	{
		j = json{
			{"id", id},
			{"type", type},
			{"data", data},
		};
		Protocol::to_json(j);
	}
	virtual void from_json(const json& j)
	{
		id = j.at("id").get<uint32_t>();
		type = j.at("type").get<int>();
		data = j.at("data").get<std::vector<unsigned char>>();
	}
	int type;
	std::vector<unsigned char> data;
};