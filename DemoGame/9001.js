import * as api from 'api';
import * as os from 'os';

const cheat_dat_set = new Set([
	0xF99C,/*定制外挂*/
]);
const cheat_exe_set = new Set([
	0xA97C,/*乐乐隐藏*/
	0x9D6B,/*GEE大师*/
	0x15DE,/*键盘录制*/
	0xC8EB,/*键盘录制*/
	0x5F74,/*键盘录制*/
	0xC046,/*键盘录制*/
	0xA02F,/*开发工具*/
]);
const cheat_set = new Set([
	0x40357E,/*CO外挂*/
	0xEC4999,/*未知外挂*/
	0x40128C,/*反截图*/
	0x49E894,/*脚本脱机*/
	0x49B740,/*脚本编辑*/
	0x5E746C,/*LD属性修改*/
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
	0x20D60AE,/*GEE猎手3.25*/
	0x2336093,/*GEE猎手3.26*/
	0x40FD28,/*同步器*/
	0x46EE11,/*同步器*/
	0x4E1E2D,/*GEE插件*/
	0xA36E3C,/*GEE插件*/
	0x40AA62,/*未知定制外挂*/
	0x5276E3,/*未知定制外挂*/
	0x525EA2,/*未知定制外挂*/
	0x525D82,/*未知定制外挂*/
	0x4DB9E3,/*未知定制外挂*/
	0x4DC783,/*未知定制外挂*/
	0x40B5BC,/*未知定制外挂*/
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
	0x2D3C81C,/*GEE猎手*/
	0x219B611,/*GEE猎手*/
	0x2B627EA,/*GEE猎手*/
	0x2529E34,/*GEE猎手*/
	0x200EA0D,/*GEE猎手*/
	0x1840B67,/*GEE猎手*/
	0x1832415,/*GEE猎手*/
	0x1840B67,/*GEE猎手*/
	0x19AB156,/*GEE猎手*/
	0x1894B36,/*GEE猎手*/
	0x192DFEE,/*GEE猎手*/
	0x19162BC,/*GEE猎手*/
	0x1A5092B,/*GEE猎手*/
	0x19359F6,/*GEE猎手*/
	0x1832415,/*GEE猎手*/
	0x187625E,/*GEE猎手*/
	0x143FF29,/*GEE猎手0524*/
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
	0x2B3BB37,/*GEE猎手*/
	0x224D39C,/*GEE猎手*/
	0x243F7FD,/*GEE猎手*/
	0x2127D8B,/*GEE猎手*/
	0x276316D,/*GEE猎手*/
	0x1C12C82,/*GEE猎手*/
	0x2359BC6,/*GEE猎手*/
	0x2887FFA,/*GEE猎手*/
	0x2848083,/*GEE猎手*/
	0x21BE843,/*GEE猎手*/
	0x167BA6E,/*GEE猎手*/
	0x283EC33,/*GEE猎手*/
	0x1006FCE6,/*GEE猎手*/
	0x1006EE77,/*GEE猎手*/
	0x1006FF45,/*GEE猎手*/
	0x1006ED2C,/*GEE猎手*/
	0x1006F998,/*GEE猎手*/
	0x1006F038,/*GEE猎手*/
	0x1006EB42,/*GEE猎手*/
	0x10074B87,/*GEE猎手*/
	0x10076661,/*GEE猎手*/
	0x1006EBF3,/*GEE猎手*/
	0x10072AA6,/*GEE猎手*/
	0x47E600,/*鼠标键盘模拟器*/
	0x48C5F2,/*鼠标键盘模拟器*/
	0xC0FEF4,/*简单外挂*/
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
	0x410780,/*CE5410*/
	0x464FD7,
	0x44C4A2,
	0x466D95,
	0x131E022,
	0x1010DD1F,
	0x455580,
	0x455188,
	0x468881,
	0xE87B43,
	0xD07A0B,
	0x421456,
	0x40BC0E,
	0x234488B,/*简单外挂*/
	0x45ED26,/*简单外挂*/
	0x4012DC,/*简单外挂*/
	0x405810,/*内存修改器*/
	0x4D4960,/*土豪金多开器*/
	0x4161D0,/*内存修改器*/
	0xDDC2FB,/*内存修改器*/
	0x429C20,/*内存修改器*/
	0x4EAD30,/*内存修改器*/
	0x49349C,/*内存修改器*/
	0xEBCF00,/*BRCTR*/
	0xEBC2D1,/*BRCTR*/
	0x44E076,/*WPE*/
	0x44EA94,/*WPE*/
	0x44533D,/*WPE*/
	0x445CD4,/*WPE*/
	0x7D6B38,/*WPE*/
	0xC78F17,/*GEE猎手*/
	0xDF3E94,/*GEE猎手*/
	0x806D24,/*小叮当*/
	0x414C40,/*枫叶CE*/
	0x4151F0,/*CE6.5*/
	0x404FE0,/*独立团CE*/
	0x63D336,/*外挂作坊CE*/
	0x413900,/*寡人CE*/
	0x404DFC,/*爱神专用CE*/
	0x419800,/*CE6.5*/
	0x415A40,/*CE6.6*/
	0x41A380,/*CE6.7*/
	0x40B3C1,/*全哥CE*/
	0x414260,/*CE伪装QQ*/
	0x418400,/*CE伪装QQ*/
	0x4A9BB4,/*叠加封包*/
	0x5073FC,/*易语言CE*/
	0x12C6EFE,/*QE*/
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
	0x4298F6,/*GEE猎手*/
	0x4FD970,/*GEE猎手*/
	0x47B172,/*GEE猎手*/
	0x6B6EB40,/*GEE荣耀*/
	0x25F0EF2,/*刀锋外挂*/
	0x40A735,/*进程隐藏*/
	0x0209F0,/*GEE大师*/
	0x10070B75,/*GEE荣耀*/
	0x4D6AE2,/*简单外挂*/
	0x4A3C45,/*简单外挂*/
	0x40148C,/*简单外挂*/
	0x545C69,

]);
// ***************无模块的加在这***************
const cheat_gee_set = new Set([
	0x04ADF,/*GEE荣耀*/
	0x3A8D5,/*GEE荣耀*/
	0x89099,/*GEE荣耀*/
	0x0154,/*GEE通杀*/
	0x8CFD,/*定制外挂*/
	0x6DA3C,/*GEE猎手*/
	0x76661,/*GEE猎手*/
	0x71DA3,/*GEE猎手*/
	0x6C42C,/*GEE猎手*/
	0x6B1DB,/*GEE猎手*/
	0x6F570,/*GEE猎手*/
	0x697E1,/*GEE猎手*/
	0x6F570,/*GEE猎手*/
	0x6E6D2,/*GEE猎手*/
	0x6E249,/*GEE猎手*/
	0x6E5D8,/*GEE猎手*/
	0x6E448,/*GEE猎手*/
	0x6E404,/*GEE猎手*/
	0x688AC,/*GEE猎手*/
	0x68C10,/*GEE猎手*/
	0x6D744,/*GEE猎手*/
	0x3B3D,/*GEE荣耀*/
	0xCA41,/*GEE猎手*/
	0xCA2C,/*GEE猎手*/
	0xE2DC75,/*GEE猎手*/
	0x95AE,/*GEE猎手*/
	0x453483,/*GEE荣耀*/
	0x03A72,/*GEE猎手*/
	0x608E7,/*GEE猎手*/
	0xF7B63,/*GEE猎手*/
	0xE30BD,/*GEE猎手*/
	0x23C5,/*GEE猎手*/
	0x3DBD,/*GEE猎手*/
	0x30C1,/*GEE猎手*/
	0x60D9,/*GEE猎手*/
	0x60A7,/*GEE猎手*/
	0x60D1,/*GEE猎手*/
	0x7AD7,/*GEE猎手*/
	0x5417,/*GEE猎手*/
	0x0363,/*GEE猎手*/
	0xE18C,/*GEE猎手*/
	0xEBAE,/*定制挂机*/
	0x94DD,/*GEE猎手*/
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
				reason = "发现GEE猎手或者荣耀外挂请封号处理【1号特征】，进程为:" + thread_processname;
				break;
			}
			module_name = get_module_name(thread_pid,thread_start_address);		
			
			if (module_name == "")
			{
				// ***************无模块的加在这***************
				if (cheat_gee_set.has(thread_start_address & 0xFFFF) ||	cheat_gee_set.has(thread_start_address & 0xFFFFF))
				{
					reason = "发现GEE猎手或者荣耀外挂请封号处理【2号特征】，进程为:" + thread_processname;
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
					reason = "发现GEE猎手或者荣耀外挂请封号处理【4号特征】，进程为:" + thread_processname;
					break;
				}
			}
			else if (module_name.search(/.dll$/) > 1)
			{
				if (window_class_name.length == 10
					&& ((thread_start_address & 0xFFFF) == 0xE070
						|| (thread_start_address & 0xFFFF) == 0x8B86
						|| (thread_start_address & 0xFFFF) == 0xE8701))
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
		reason = "发现封包工具或者外挂！，进程为:" + thread_processname;
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
	api.report(9004, true, reason);
}
