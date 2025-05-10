#include "../pch.h"
#include <Lightbone/utils.h>
#include "Service/AntiCheatClient.h"
#include "ClientImpl.h"

extern std::shared_ptr<CClientImpl> client_;
extern std::shared_ptr<asio2::timer> g_timer;
const unsigned int DEFINE_TIMER_ID(kImageProtectCheckTimerId);
// 将SEH部分移至单独的函数，避免与C++对象析构冲突
__declspec(noinline) bool CheckImageProtectInternal(void* image_base) {
    __try {
        // 检查内存是否可写，若不可写会触发异常
        if (IsBadWritePtr(image_base, 1) == FALSE) {
            return true; // 检测到异常访问
        }
        return false;
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        LOG("线程异常: %s|%s|%d|0x%X", __FILE__, __FUNCTION__, __LINE__, GetExceptionCode());
        return false;
    }
}

__declspec(noinline) void CheckImageProtect() {
    auto image_base = Utils::ImageProtect::instance().get_image_base();
    ProtocolC2STaskEcho resp;

    // 调用分离的SEH函数进行检查
    if (CheckImageProtectInternal(image_base)) {
        resp.task_id = TASK_PKG_ID_IMAGE_PROTECT_DETECT;
        resp.is_cheat = true;
        resp.text = xorstr("发现A类外挂");
    }

    client_->send(&resp);
}

void InitImageProtectCheck() {
    if (Utils::ImageProtect::instance().is_init()) {
        LOG(__FUNCTION__);
        g_timer->start_timer(kImageProtectCheckTimerId, std::chrono::seconds(60), []() {
            CheckImageProtect();
        });
    }
}