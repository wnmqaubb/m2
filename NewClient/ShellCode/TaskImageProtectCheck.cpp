#include "../pch.h"
#include <Lightbone/utils.h>
#include "Service/AntiCheatClient.h"
#include "ClientImpl.h"

extern std::shared_ptr<CClientImpl> client_;
const unsigned int DEFINE_TIMER_ID(kImageProtectCheckTimerId);
void InitImageProtectCheck()
{
    if (Utils::ImageProtect::instance().is_init())
	{
        LOG(__FUNCTION__);
        client_->start_timer(kImageProtectCheckTimerId, std::chrono::seconds(60), []() {
            if (IsBadWritePtr(Utils::ImageProtect::instance().get_image_base(), 1) == FALSE) {
                ProtocolC2STaskEcho resp;
                resp.task_id = TASK_PKG_ID_IMAGE_PROTECT_DETECT;
                resp.is_cheat = true;
                resp.text = xorstr("发现A类外挂");
                client_->send(&resp);
            }
        });
    }
}