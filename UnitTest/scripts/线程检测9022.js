import * as api from 'api';
import * as os from 'os';

const cheat_dat_set = new Set([
	0xF99C,/*定制外挂*/
	0x2D80,/*加速器*/
]);
const cheat_exe_set = new Set([
	0x271E,/*dragon鼠标*/
	0x9D6B,/*GEE大师*/
	0xC8EB,/*键盘录制*/
	0xC046,/*键盘录制*/
	0xA02F,/*开发工具*/
]);
const cheat_set = new Set([
	0xBAE094,/*刺客*/
	0x44AACC,/*CMD脱机*/
	0x13f40a8c0,/*CMD脱机*/
	0x2972108,/*内部定制*/
	0x2532E42,/*内部定制*/
	0x227CBAF,/*内部定制*/
	0x1731776,/*内部定制*/
	0x3F3F721,/*内部定制*/
	0x479980,/*强制修理*/
	0x9F9291,/*tyx*/
	0x9CDACC,/*tyx*/
	0x3F36C78,/*私人定制*/
	0x47135A,/*私人定制*/
	0x46D7F8,/*私人定制*/
	0x4C6F61,/*私人定制*/
	0x48667D,/*私人定制*/
	0x40A813B,/*私人定制*/
	0x4B530EC,/*私人定制*/
	0x369CFE5,/*私人定制*/
	0x3DC813F,/*私人定制*/
	0x9CA87F,/*私人定制*/
	0x9F9DA2,/*私人定制*/
	0x4AE10E8,/*私人定制*/
	0x4BBD05C,/*私人定制*/
	0x4FD8AE,/*私人定制*/
	0x1A123BD,/*私人定制*/
	0x2B19598,/*未收集*/
	0x61463c,/*未收集*/
	0x9C94FA,/*未收集*/
	0x13A8A74,/*GEE加速*/
	0xFD80F0,/*GEE加速*/
	0xFC54C0,/*飘刀加速*/
	0x412557,/*GEE加速*/
	0xFE7D20,/*GEE加速*/
	0x401C9F,/*GEE加速*/
	0x403861,/*GEE加速*/
	0x3238B49,/*GEE加速*/
	0x2359C60,/*GEE加速*/
	0x47C20B2,/*GEE加速*/
	0x4619E5,/*GEE加速*/
	0xCF7D32,/*GEE加速*/
	0x5B6F93,/*脱机回收*/
	0x4393AA,/*脱机回收*/
	0x4AC02F,/*脱机回收*/
	0x5A3DF3,/*脱机回收*/
	0x46B7C2,/*脱机回收*/
	0x72D26F,/*按键脱机回收*/
	0xE7320E,/*按键脱机回收*/
	0x450EDD,/*按键脱机回收*/
	0x41E8E5,/*按键脱机回收*/
	0x450EDB,/*按键脱机回收*/
	0x70BE30,/*按键脱机回收*/
	0x12CF584,/*按键脱机回收*/
	0xFFB9DA,/*暗影网络加速器*/
	0x496640,/*传奇私服辅助工具*/
	0x1705C08,/*刷金币*/
	0x737BD0,/*极速鼠标*/
	0x218CB19,/*风铃同步器*/
	0x469480,/*多倍攻击*/
	0xA8FD8A,/*多倍攻击*/
	0x6E3C195,/*GEE基质获取*/
	0x8E334E,/*GEE基质获取*/
	0x43BB34,/*过微信验证*/
	0x475351,/*鼠标脱机*/
	0x13CAD82,/*脱机代理*/
	0x43928A,/*脱机回收*/
	0x35F16DC,/*脱机回收*/
	0x228D82C,/*脱机回收*/
	0x43928A,/*脱机回收*/
	0x64F7A1,/*脱机回收*/
	0x6CE257,/*脱机回收*/
	0x10A3044,/*脱机回收*/
	0x42C44B,/*脱机回收*/
	0x4C8E34,/*脱机回收*/
	0x501E65,/*脱机回收*/
	0x18761E,/*脱机回收*/
	0x468EE1,/*脱机回收*/
	0x483031,/*脱机回收*/
	0x4ECE51,/*脱机回收*/
	0x467588,/*私人加速*/
	0x6B2BB0,/*网络拦截*/
	0x47AA51,/*网络拦截*/
	0x78BF40,/*ST加速器*/
	0x4ABBFD,/*私人定制*/
	0x1FED5FF,/*私人定制*/
	0x11542CB,/*私人定制*/
	0x40B634,/*私人定制*/
	0x45DD35,/*私人定制*/
	0x8A135F,/*私人定制*/
	0xF4A256,/*私人定制*/
	0x220177B,/*私人定制*/
	0x2BA30B,/*高端脱机*/
	0x1CC971A,/*高端脱机*/
	0x1F6883A,/*高端脱机*/
	0x35BE74E,/*高端脱机*/
	0x21DA70C,/*高端定制*/
	0x26388F0,/*高端定制*/
	0x22415C6,/*高端定制*/
	0x21266FC,/*高端定制*/
	0x209075C,/*高端定制*/
	0x24D4422,/*高端定制*/
	0x60AC96,/*沙盒*/
	0x139360E,/*CO外挂*/
	0x202C8C8,/*CO外挂*/
	0x1BFA1F8,/*CO外挂*/
	0x157102F,/*CO外挂*/
	0x106227E,/*CO外挂*/
	0x1249F07,/*CO外挂*/
	0x16B4AEF,/*CO外挂*/
	0x18FB006,/*CO外挂*/
	0x1F2395D,/*CO外挂*/
	0x1DDC2AA,/*CO外挂*/
	0x112BD3D,/*CO外挂*/
	0x180160E,/*CO外挂*/
	0x1BBB844,/*CO外挂*/
	0x1BCEA63,/*CO外挂*/
	0x42756DE,/*CO外挂*/
	0x177f4c6,/*CO外挂*/
	0x1AFE4F3,/*CO外挂*/

	0x50BA15,/*易语言脱机*/
	0x469520,/*未知脱机*/
	0x401B64,/*未知脱机*/
	0x40AC60,/*天使辅助*/
	0x51F393,/*脱机打钱*
	0x10A3044,/*脱机打钱*
	0x747880,/*血手鼠标*/
	0x404015,/*ck外挂*/
	0xCF4BAD,/*变速精灵*/
	0xF79CA9,/*变速精灵*/

	0xC49CA9,/*变速精灵*/
	0x69B13C,/*变速精灵*/
	0x49FB53,/*大漠脱机*/
	0x12CAFB6,/*沙盘*/
	0x41802E,/*沙盘*/
	0xD3AE56,/*沙盘*/
	0x5350D8,/*逆天辅助*/
	0x1C81927,/*变速器*/
	0x41827C,/*变速器*/
	0x47E001,/*一流变速器*/
	0x57F3D5,/*变速齿轮*/
	0x6A97D0,/*变速齿轮*/
	0x4A5FA0,/*鼠标脱机*/
	0x4F3330,/*简尚按键*/
	0x4E6001,/*小焱鼠标按键*/
	0x40469E,/*楼月鼠标按键*/
	0x659E28,/*多功能按键*/
	0x424001,/*水晶情缘按键*/
	0x63D388,/*PrjMouseAutoClick按键*/
	0x13FD966,/*macrorecorder按键*/
	0xBDD6B0,/*通用按键*/
	0x47A629,/*脱机辅助*/
	0x155C77E,/*荣耀外挂*/
	0x18FC3B8,/*荣耀外挂*/
	0xFE75D1,/*绿色脱机外挂*/
	0x4801AD,/*未知实锤外挂*/
	0x10309CD,/*进程隐藏工具*/
	0x520ADB,/*jjss破解补丁*/
	0x1231D5A,/*按键精灵*/
	0xA5D2D8,/*易语言加速器*/
	0x471E21,/*易语言加速器*/
	0xA6DBE2,/*守望驱动变速*/
	0xA5EF6B,/*收费驱动变速*/
	0xA72D48,/*收费驱动变速*/
	0x2F67000,/*收费驱动变速*/
	0x40DB94,/*北斗加速器*/
	0x163634C,/*GEE荣耀*/
	0x14B8094,/*GEE荣耀*/
	0x3B6A678,/*GEE通杀*/
	0x3AD4FF7,/*GEE通杀*/
	0x3B14424,/*GEE通杀*/
	0x11F8460,/*GEE通杀0922*/
	0x2D6D8B4,/*GEE通杀0806*/
	0x206F5CD,/*GEE通杀0311*/
	0x20D3123,/*GEE通杀0311*/
	0x1FEFBB2,/*GEE通杀0304*/
	0x2084ACC,/*GEE通杀0318*/
	0x1FD3F92,/*GEE通杀0317*/
	0x206F613,/*GEE通杀0330*/
	0x361EDE3,/*GEE通杀0330*/
	0x217DA46,/*GEE通杀0330*/
	0x205AB94,/*GEE通杀0327*/
	0x2155FC6,/*GEE通杀0328*/
	0x1FF0DED,/*GEE通杀0405*/
	0x2030F67,/*GEE通杀0408*/
	0x2017A61,/*GEE通杀0412*/
	0x2139893,/*GEE通杀0414*/
	0x1FF0DED,/*GEE通杀0417*/
	0x32553F0,/*GEE通杀0425*/
	0x215755C,/*GEE通杀0501*/
	0x3BE87BB,/*GEE通杀0727*/
	0x511DD66,/*GEE通杀0803*/
	0x3319BFA,/*GEE通杀0820*/
	0x21A1F62,/*GEE通杀0821*/
	0x1A91EDC,/*GEE通杀0822*/
	0x3350159,/*GEE通杀0826*/
	0x3AC2A31,/*GEE通杀0826*/
	0x1AFF4A1,/*GEE通杀0827*/
	0x2052D55,/*GEE通杀0827*/
	0x1566708,/*GEE通杀0831*/
	0x27DEE4B,/*GEE通杀0911*/
	0x251D51F,/*GEE通杀0919*/
	0x21D8C1B,/*GEE通杀1007*/
	0x1BD58AA,/*GEE通杀1009*/
	0x21AE209,/*GEE通杀1024*/
	0x21D2128,/*GEE通杀1029*/
	0x20D5FB9,/*GEE通杀1109*/
	0x161F22F,/*GEE通杀1112*/
	0x15DF90D,/*GEE通杀1120*/


	0x46F1C0,/*一键小腿*/
	0x2610A7,/*按键精灵自动回收*/
	0x2290253,/*奇刃辅助*/
	0x49ABB3,/*未知外挂*/
	0xD5768C,/*天文鼠标连点器*/
	0x52509C,/*脚本录制*/
	0x4AA678,/*华华键盘模拟*/
	0xBF12C0,/*鼠标键盘模拟*/
	0x643730,/*鼠标键盘模拟*/
	0x6D9A44,/*鼠标键盘模拟*/
	0x4C3860,/*鼠标键盘模拟*/
	0x4256D9,/*鼠标键盘模拟*/
	0x48BB90,/*鼠标键盘模拟*/
	0xE407F0,/*鼠标键盘模拟*/
	0x4EC310,/*鼠标键盘模拟*/
	0x51CE64,/*蓝点鼠标*/
	0x13B47AA,/*过验证脱机*/
	0x435EB0,/*脚本编辑器*/
	0x40C550,/*按键游侠*/
	0xEC4999,/*未知外挂*/
	0x40128C,/*反截图*/
	0x49E894,/*脚本脱机*/
	0x49B740,/*脚本编辑*/
	0x1B1D8EE,/*变速器大师*/
	0x494B42,/*六界词条4.5*/
	0x494762,/*六界词条4.2*/
	0x494ECD,/*六界词条4.1*/
	0x494B5D,/*六界词条3.5*/
	0x494C72,/*六界词条4.5*/
	0x41B06C,/*定制外挂*/
	0x41EAA1,/*定制外挂*/
	0x408BF7,/*定制外挂*/
	0x41F1D0,/*定制外挂*/
	0x20D60AE,/*猎手3.25*/
	0x2336093,/*猎手3.26*/
	0x40B325,/*同步器*/
	0x40FD28,/*同步器*/
	0x46EE11,/*同步器*/
	0x129E20C,/*麒麟同步器*/
	0x4E1E2D,/*GEE插件*/
	0xA36E3C,/*GEE插件*/
	0x40AA62,/*未知定制外挂*/
	0x5276E3,/*未知定制外挂*/
	0x525EA2,/*未知定制外挂*/
	0x525D82,/*未知定制外挂*/
	0x4DB9E3,/*未知定制外挂*/
	0x4DC783,/*未知定制外挂*/
	0x40B6C2,/*未知定制外挂*/
	0x4E3013,/*未知定制外挂*/
	0x4E2EE3,/*未知定制外挂*/
	0x4EFD8A,/*未知定制外挂*/
	0x4CE567,/*脚本脱机*/
	0xBF2564,/*大表哥辅助*/
	0x4CAF76,/*挂机宝*/
	0x5DE81C,/*天翼脱机*/
	0x1FF0DED,/*GEE通杀0417*/
	0x2170AE0,/*GEE通杀0506*/
	0x3BDCFBA,/*GEE通杀*/
	0x1FBFEE2,/*GEE通杀*/
	0x1FEFBB2,/*GEE通杀*/
	0x354ACCC,/*GEE通杀*/
	0x36D2378,/*GEE通杀*/
	0x36A4C40,/*GEE通杀*/
	0x36668FA,/*GEE通杀*/
	0x1EC137C,/*GEE通杀*/
	0x1F63D72,/*GEE通杀*/
	0x1F683F0,/*GEE通杀*/
	0x1FA7B66,/*GEE通杀*/
	0x1FD5EE1,/*GEE通杀*/
	0x2DBE1C3,/*GEE通杀0524*/
	0x2D3C81C,/*猎手*/
	0x219B611,/*猎手*/
	0x2B627EA,/*猎手*/
	0x2529E34,/*猎手*/
	0x200EA0D,/*猎手*/
	0x1840B67,/*猎手*/
	0x1832415,/*猎手*/
	0x1840B67,/*猎手*/
	0x19AB156,/*猎手*/
	0x1894B36,/*猎手*/
	0x192DFEE,/*猎手*/
	0x19162BC,/*猎手*/
	0x1A5092B,/*猎手*/
	0x19359F6,/*猎手*/
	0x1832415,/*猎手*/
	0x187625E,/*猎手*/
	0x143FF29,/*猎手0524*/
	0x19437BE4,/*赤龙加速器*/
	0x59BEE0,/*全能模拟王*/
	0x2025392,/*GEE通杀*/
	0x1FC7C7F,/*GEE通杀*/
	0x1FDE3A2,/*GEE通杀*/
	0x42284B,/*鼠标录制*/
	0x41D2FD,/*鼠标录制*/
	0x46C4BA,/*键盘录制*/
	0x40D5AF,/*键盘录制*/
	0x47E600,/*键盘录制*/
	0x4B20C8,/*王者加速器*/
	0x545B89,/*狩猎者加速器*/
	0x448CDD,/*水上漂加速器*/
	0x608ADD,/*水上漂加速器*/
	0x407C72,/*变速精灵*/
	0x401768,/*南蛮加速器*/
	0x679CE0,/*通用加速器*/
	0x21DFC50,/*通杀GOM*/
	0xC3B8DD,/*通杀GOM*/
	0x523647,/*通杀GOM*/
	0xFCC042,/*通杀GOM*/
	0xAEC647,/*通杀小助手*/
	0xBAE57C,/*GEE暗龙*/
	0x406539,/*GEE定制*/
	0x2B3BB37,/*猎手*/
	0x224D39C,/*猎手*/
	0x243F7FD,/*猎手*/
	0x2127D8B,/*猎手*/
	0x276316D,/*猎手*/
	0x1C12C82,/*猎手*/
	0x2359BC6,/*猎手*/
	0x2887FFA,/*猎手*/
	0x2848083,/*猎手*/
	0x21BE843,/*猎手*/
	0x167BA6E,/*猎手*/
	0x283EC33,/*猎手*/
	0x1006FCE6,/*猎手*/
	0x1006EE77,/*猎手*/
	0x1006FF45,/*猎手*/
	0x1006ED2C,/*猎手*/
	0x1006F998,/*猎手*/
	0x1006F038,/*猎手*/
	0x1006EB42,/*猎手*/
	0x10074B87,/*猎手*/
	0x10076661,/*猎手*/
	0x1006EBF3,/*猎手*/
	0x10072AA6,/*猎手*/
	0x401504,/*鼠标键盘模拟器*/
	0x47E600,/*鼠标键盘模拟器*/
	0x48C5F2,/*鼠标键盘模拟器*/
	0xC0FEF4,/*简单外挂*/
	0x5CF38F,/*简单外挂*/
	0xFE824C,/*2020小可爱*/
	0x64A146,/*脱机编辑器*/
	0x407D66,/*7T加速器*/
	0x40513D,/*8T加速器*/
	0x4118C0,/*小冷工具*/
	0x528380,/*线程手术*/
	0x545D39,/*狩猎者*/
	0x47198B,/*抓包工具*/
	0x642000,/*私服小助手*/
	0x93CFF6,/*简单外挂*/
	0x5B84FB,/*GEE通杀*/
	0x498001,/*GEE通杀*/
	0x499BD6,/*GEE通杀*/
	0x47705C,/*GEE通杀*/
	0x8F3A39,/*GEE通杀*/
	0x4AD153,/*GEE通杀*/
	0x400154,/*GEE通杀*/
	0x4781E2,/*GEE通杀*/
	0x4771E2,/*GEE通杀*/
	0x403AA8,/*GEE通杀*/
	0x403ACC,/*GEE通杀*/
	0x456BB7,/*GEE通杀*/
	0xB46774,/*CK1003*/
	0xB7324C,/*CK1003*/
	0xB8C35F,/*小可爱*/
	0xFE824C,/*2020小可爱*/
	0xF7CBE0,/*虚拟机*/
	0xC0945D,/*GEE大师*/
	0x77159490,/*虚拟机*/
	0x4EF02C,/*无敌外挂*/
	0x518890,/*GEE大师*/
	0x593FC7,/*GEE大师*/
	0x50EE75,/*GEE大师*/
	0xC79122,/*通杀GOM*/
	0x5CD7CDE,/*无视*/
	0xE7C16E,/*进程隐藏*/
	0x12C0D5D,/*CK*/
	0x12C3020,/*刀锋外挂*/
	0x40B76F,/*猎手外挂*/
	0x44FD84,/*定制外挂*/
	0x401214,/*定制外挂*/
	0xBAE089,/*刺客外挂*/
	0x14413D2,/*红牛大退*/
	0x548290,/*守望者2.03*/
	0x49D9FA,
	0x409E00,
	0x405DF0,
	0x46CBC3,
	0x1FBB35E,
	0x418101,
	0x449C71,
	0x44EA94,
	0x464FD7,
	0x44C4A2,
	0x466D95,
	0x131E022,
	0x1010DD1F,
	0x455188,
	0x468881,
	0xE87B43,
	0xD07A0B,
	0x421456,
	0x40BC0E,
	0x234488B,/*简单外挂*/
	0x45ED26,/*简单外挂*/
	0x4012DC,/*简单外挂*/
	0xC78F17,/*猎手*/
	0xDF3E94,/*猎手*/

	0x495CA4,/*封包*/
	0x12DE787,/*荣耀*/
	0x44C9B8,/*荣耀*/
	0x40D5AF,/*键盘鼠标录制*/
	0x18D3209,/*荣耀*/
	0x40110C,/*未知*/
	0x233F3386,/*一刀*/
	0x622FFD,/*南方天成*/
	0x114FC7A,/*Gee荣耀*/
	0x4AA37E,/*简单破解*/
	0x404F52,/*简单破解*/
	0x594017,/*GEE荣耀*/
	0x4298F6,/*猎手*/
	0x4FD970,/*猎手*/
	0x47B172,/*猎手*/
	0x6B6EB40,/*GEE荣耀*/
	0x25F0EF2,/*刀锋外挂*/
	0x40A735,/*进程隐藏*/
	0x0209F0,/*GEE大师*/
	0x10070B75,/*GEE荣耀*/
	0x4D6AE2,/*简单外挂*/
	0x4A3C45,/*简单外挂*/
	0x40148C,/*简单外挂*/
	0x545C69,/*简单外挂*/
	0x984500,/*守望者加速*/

]);
// ***************无模块的加在这***************
const cheat_gee_set = new Set([
	0x9B8A,/*乱世辅助*/
	0x680E,/*乱世辅助*/
	0x04ADF,/*GEE荣耀*/
	0x3A8D5,/*GEE荣耀*/
	0x89099,/*GEE荣耀*/
	0x0154,/*GEE通杀*/
	0x8CFD,/*定制外挂*/
	0x3B3D,/*GEE荣耀*/
	0xCA41,/*猎手*/
	0xCA2C,/*猎手*/
	0xE2DC75,/*猎手*/
	0x95AE,/*猎手*/
	0x453483,/*GEE荣耀*/
	0xEBAE,/*定制挂机*/
	0x94DD,/*猎手*/
	0xECCA,/*GEEPK*/
	0x1006FB55,
	0x4B45EF,
	0x402644,
	0x192FB9A,
	0xE09808,
	0xE65E94,
	0xE60ACF,
	0xD0E9,
	0x5247,
	0x33ED,
	0x115D7F,
	0xEBAE,
	0x6F659,
	0x2256,
	0x3C3A,
	0xD36B,
	0x443090,
	0xF33A,
	0x03A72,
	0x23C5,
	0x2644,
	0x8063,
	0x545D39,
	0x545B89,/*守望者*/
	0x475DA8,/*GEE定制*/
	0x406393,/*GEE定制*/
	0x40AFC8,/*GEE定制*/
	0x4106F9,/*GEE定制*/
	0x406345,/*GEE定制*/
	0x3C7EB40,/*GEE大师*/
	0x6841F0,/*GEE定制*/

]);

const window_class_name_black_set = new Set([
	"IME",
	"MSCTFIME",
	"tooltips_class32",
	"FORM_PROJECT1_FORM1_CLASS:0",
	"QQPinyinImageCandWndTSF",
	"VBBubbleRT6",
	"AutoIt v3 GUI",
]);

let threads = api.enum_threads();//[[pid,processname,thread_start_address]]
let windows = api.enum_windows();//[[window.pid, window.caption,window_class_name,window.is_hide_process]]
let module_name_map = new Map(api.get_module_names()); //std::map<uint32_t,std::vector<std::tuple<std::string,uint64_t, uint32_t>>>
let process_name_map = new Map(api.get_process_names()); //std::map<uint32_t, std::string>

function get_module_name(pid,thread_start_address){
	let module_name = "";
	let module_name_array;
	if(module_name_map.has(pid)){
		module_name_array = module_name_map.get(pid);
		for(let i=0;i<module_name_array.length;i++){
			let module = module_name_array[i];
			if(module[1] <= thread_start_address && thread_start_address <= (module[1]+module[2])){
				return module[0];
			}
		}
	}
	
	return module_name;
}

let reason = "";
let module_name = "";	
let win;
let window_pid;
let window_caption;
let window_class_name;
let process_name;
let thread;
let thread_pid;
let thread_processname;
let thread_start_address;

function check_no_access_process_rate()
{
	let sum = 0;
	let no_access_sum = 0;
	
	for(let [pid,modules] of module_name_map)
	{
		sum++;
		if(modules.length == 0)
		{
			no_access_sum++;
		}
	}
	let rate = no_access_sum / sum;
	if(rate >= 0.8)
	{
		api.report(9022, false, `无权限/总进程 ${no_access_sum}/${sum} (${rate})`);
	}
}
check_no_access_process_rate();

for(let i=0;i<windows.length;i++)
{
	win = windows[i];
	window_pid = win[0];
	window_caption = win[1];
	window_class_name = win[2];
	process_name = "";
	if(process_name_map.has(window_pid)){
		process_name = process_name_map.get(window_pid);
	}
	if (process_name == "" || (!window_class_name_black_set.has(window_class_name) && window_class_name.length != 10))
	{
		continue;
	}
	
	for(let j=0;j<threads.length;j++)
	{
		thread = threads[j];
		thread_pid = thread[0];
		thread_processname = thread[1];
		thread_start_address = thread[2];
		if(window_pid == thread_pid)
		{
			if ((window_class_name == "tooltips_class32" || window_class_name == "MSCTFIME")
				&& (cheat_gee_set.has(thread_start_address) || cheat_gee_set.has(thread_start_address & 0xFFFFFF)))
			{
				reason = "发现猎手或者荣耀外挂请封号处理【1号特征】，进程为:" + thread_processname;
				break;
			}
			module_name = get_module_name(thread_pid,thread_start_address);		
			
			if (module_name == "")
			{
				// ***************无模块的加在这***************
				if (cheat_gee_set.has(thread_start_address & 0xFFFF) ||	cheat_gee_set.has(thread_start_address & 0xFFFFF))
				{
					reason = "发现猎手或者荣耀外挂请封号处理【2号特征】，进程为:" + thread_processname + "|" + thread_start_address.toString(16);
					break;
				}
				continue;
			}			
			if (module_name.search(/.exe$/) > 1)
			{
				if (window_class_name == "QQPinyinImageCandWndTSF" && (thread_start_address & 0xFFFF) == 0x3020)
				{
					reason = "GEE大师外挂请封号处理【3号特征】，进程为:" + thread_processname;
					break;
				}
				if (window_class_name == "AutoIt v3 GUI" && (thread_start_address & 0xFFFF) == 0x800A)
				{
					reason = "发现一键小退软件，进程为:" + thread_processname;
					break;
				}
				if (window_class_name == "FORM_PROJECT1_FORM1_CLASS:0" && (thread_start_address & 0xFFFF) == 0x2256)
				{
					reason = "发现猎手或者荣耀外挂请封号处理【4号特征】，进程为:" + thread_processname;
					break;
				}

				if (window_class_name == "tooltips_class32" && (thread_start_address & 0xFFFF) == 0xD8C5)
				{
					reason = "发现GEE大师多开器，进程为:" + thread_processname;
					break;
				}

			}
			else if (module_name.search(/.dll$/) > 1)
			{
				if (window_class_name.length == 10
					&& ((thread_start_address & 0xFFFF) == 0xE070
						|| (thread_start_address & 0xFFFF) == 0xD490))
				{
					reason = "发现简单类外挂，进程为:" + thread_processname;
					break;
				}
				if (window_class_name == "VBBubbleRT6" && (thread_start_address & 0xFFFFF) == 0xFB401)
				{
					reason = "定制外挂请封号处理，进程为:" + thread_processname;
					break;
				}
			}
			else if (module_name.search(/.tap$/) > 1
				&& ((thread_start_address & 0xFFFF) == 0xDD1F
					|| (thread_start_address & 0xFFFF) == 0x58F0))
			{
				reason = "发现荣耀外挂请封号处理，进程为【7号特征】:" + thread_processname;
				break;
			}
		}
	}
		
	if(reason != "") break;
}
if(reason == "")
for(let j=0;j<threads.length;j++)
{
	thread = threads[j];
	thread_pid = thread[0];
	thread_processname = thread[1];
	thread_start_address = thread[2];
	if(thread_pid < 1 || thread_start_address < 1) continue;
	if (cheat_set.has(thread_start_address))
	{
		reason = "外挂程序为:" + thread_processname;
		break;
	}
	
	module_name = get_module_name(thread_pid,thread_start_address);
	if (module_name == "")
	{
		continue;
	}
	
	if (module_name.search(/.exe$/) > 1)
	{
		if (cheat_exe_set.has(thread_start_address & 0xFFFF))
		{
			reason = "发现大师、定制类外挂，进程为:" + thread_processname;
			break;
		}
	}
	else if (module_name.search(/.dat$/) > 1)
	{
		if (cheat_dat_set.has(thread_start_address & 0xFFFF))
		{
			reason = "发现定制类脱机外挂，进程为:" + thread_processname;
			break;
		}
	}
	
}

if(reason != ""){
	api.report(9022, true, reason);
}
