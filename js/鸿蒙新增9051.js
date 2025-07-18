import * as api from "api";
import * as os from "os";
if (api.get_surface) {
    let res = api.get_surface();
    for (let path of res) {
        if (path.split(/一餐厅四楼合作协议|稻妻鹤观宝箱.txt|黑狼破解版|审判处理|封喉虚拟机|C盘就不会满.txt|PCHunter1.56.rar|一键硬件修改.exe|机器码解封器|出售软件啊|VTCheck.exe|坐标失败.lnk|挂机开始.txt|Gee猎手0821|引擎综合挂|一键修改系统消息|传奇封包解密|科技群工具|掘金硬件|商场买回城刷金币|OP组冬训名单|至善进程保护|难检测的驱动加载|biansu.dll|藏剑同款2.16|过SP|通杀倍攻绿化版|桌面文件－20180412|小伍w7审判|线程源码求改进|1900-6.24.km|审判显卡驱动|找句柄64位多开器|工具等24个|母鸡绑定|存高级装备.txt|虚拟机相关|更新v2.rar|过黑名单|账号保存带二级密码|G1229审判|封包数据分析/).length > 1) {
            api.report(9051, true, path);
        }
    }
}