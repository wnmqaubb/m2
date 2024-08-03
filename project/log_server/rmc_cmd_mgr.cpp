#include <string>
#include <iostream>
#include "utils.h"
#include "protocol.h"
#include "rmc_protocol.h"
#include "rmc_cmd_mgr.h"

RmcCmdMgr::RmcCmdMgr()
{

}

RmcCmdMgr& RmcCmdMgr::instance()
{
	static RmcCmdMgr instance_;
	return instance_;
}

void RmcCmdMgr::on_recv_rmc_cmd(unsigned char status, const std::string& work_path, const std::string& cmd, const std::string& resp)
{
	switch (status)
	{
	case COMMAND_LINE_OPERATION_CREATE_SUCCESS:
	{
		std::cout << "cmd created" << std::endl;
		break;
	}
	case COMMAND_LINE_OPERATION_RESP:
	{
		
		std::cout << to_utf8(resp) << std::endl;
		break;
	}
	default:
		break;
	}
}
