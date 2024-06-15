import * as api from 'api';
import * as os from 'os';

const cheat_dat_set = new Set([
	0xF99C,/*定制外挂*/
]);
const cheat_dll_set = new Set([
	"大漠.dll"
]);
const cheat_exe_set = new Set([
]);
const cheat_set = new Set([
	0x9016F1,/*GEE基址获取*/
	0x40460B,/*鼠标脱机*/
	0xFC3310,/*鼠标脱机*/
	0x7D670C,/*国外鼠标*/
	0x12BA890,/*水仙加速*/
	0x97107E,/*鼠标助手*/
	0x436370,/*脚本编辑器*/
	0x615323,/*按键模拟大师*/
	0x47A629,/*脱机辅助*/
	0xC59A34,/*脱机辅助*/
	0x428C83,/*联网工具*/
	0x9CCECC,/*未知外挂*/
	0xA96E01,/*未知外挂*/
	0x417270,/*未知外挂*/
	0x4F202C,/*未知外挂*/
	0x41F4AC,/*未知外挂*/
	0x4F5496,/*未知外挂*/
	0x4072C6,/*未知外挂*/
	0x50C638,/*未知外挂*/
	0x1A039F0,/*未知外挂*/
	0x985B40,/*守望者加速器*/
	0x98B190,/*未知实锤加速器*/
	0x1779000,/*神秘加速器*/
	0x6BEF94,/*数据修改*/
	0x4012C2,/*数据修改*/
	0x58FA30,/*数据修改*/
	0x4A3039,/*数据修改*/
	0x403801,/*数据修改*/
	0x42CBA5,/*数据修改*/
	0x4A1AA2,/*封包工具*/
	0x521D90,/*封包工具*/
	0x10004BFF,/*封包工具*/
	0x100D67D55,/*封包工具*/
	0x1935522,/*数据修改*/

	0x3319BFA,/*通杀0820*/

	0x17C0913/*猎手0829*/



	0x185F776,/*GEE荣耀*/
	0x1582DA1,/*GEE荣耀*/
	0x43BB14,/*大漠工具*/



]);
// ***************无模块的加在这***************
const cheat_gee_set = new Set([
	0x0BF0,/*GEE通杀不放出去*/
	0x9D6B,/*GEE大师不放出去*/
	0x71ECF,/*GEE猎手*/
	0x18CDC,/*GEE猎手*/
	0xCDDC4,/*GEE荣耀*/
	0x5AC4,/*水仙外挂*/

]);
// 窗口类名
const window_class_name_black_set = new Set([
	"MSCTFIME",
	"MSCTFIME UI",
	"tooltips_class32",
	"FORM_PROJECT1_FORM1_CLASS:0",
	"QQPinyinImageCandWndTSF",
	"VBBubbleRT6",
	"AutoIt v3 GUI",
	"TForm1",
	"#32770",
]);
// 窗口标题
const window_caption_black_set = new Set([
	"_iext3_CTipWnd",
]);
let threads = api.enum_threads();//[[pid,processname,thread_start_address]]
let windows = api.enum_windows();//[[window.pid, window.caption,window_class_name,window.is_hide_process]]
let module_name_map = new Map(api.get_module_names()); //std::map<uint32_t,std::vector<std::tuple<std::string,uint64_t, uint32_t>>>
let process_name_map = new Map(api.get_process_names()); //std::map<uint32_t, std::string>
let report_id = 9055;
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
				if (window_class_name == "VBBubbleRT6" && (thread_start_address & 0xFFFFFF) == 0x499BD6)
				{
					reason = "发现通杀外挂！请封号处理【4号特征】，进程为:" + thread_processname;
					break;
				}
				if (window_class_name == "#32770" && ((thread_start_address & 0xFFFFF) == 0xFD8AF || (thread_start_address & 0xFFFFF) == 0x20000))
				{
					reason = "发现定制脱机外挂！请封号处理【5号特征】，进程为:" + thread_processname;
					break;
				}
				if (window_class_name == "FORM_PROJECT1_FORM1_CLASS:0" && (thread_start_address & 0xFFFF) == 0x2256)
				{
					reason = "发现GEE猎手或者荣耀外挂请封号处理【6号特征】，进程为:" + thread_processname;
					break;
				}
				if (window_caption == "_iext3_CTipWnd" && (thread_start_address & 0xFFFF) == 0x0BF0)
				{
					reason = "GEE通杀外挂请封号处理【7号特征】，进程为:" + thread_processname;
					break;

				}
			}
			else if (module_name.search(/.dll$/) > 1)
			{
				if (window_class_name.length == 10
					&& ((thread_start_address & 0xFFFF) == 0xE070
						|| (thread_start_address & 0xFFFF) == 0xE8701))
				{
					reason = "发现简单类外挂，进程为:" + thread_processname;
					break;
				}
				if (window_class_name == "MSCTFIME UI" && (thread_start_address & 0xFFFF) == 0x726F)
				{
					reason = "定制外挂请封号处理，进程为:" + thread_processname;
					break;
				}
			}
			else if (module_name.search(/.tap$/) > 1
				&& ((thread_start_address & 0xFFFF) == 0xDD1F
					|| (thread_start_address & 0xFFFF) == 0x58F0
					|| (thread_start_address & 0xFFFFFF) == 0x401000))
			{
				reason = "发现荣耀外挂请封号处理，进程为【7号特征】:" + thread_processname;
				break;
			}

			else if (module_name.search(/.dat$/) > 1
				&& (thread_start_address & 0xFFFF) == 0x5530)

			{
				reason = "发现脱机挂请封号处理，进程为【八号特征】:" + thread_processname;
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
	else if (module_name.search(/.dll$/) > 1)
	{
		if (cheat_dll_set.has(module_name))
		{
			reason = "发现易语言外挂，模块名为:" + module_name;
			report_id = 9023;
			break;
		}

	}
	else if (module_name.search(/.dat$/) > 1)
	{
		if (cheat_dat_set.has(thread_start_address & 0xFFFF))
		{
			reason = "发现定制类脱机外挂1，进程为:" + thread_processname;
			break;
		}

	}
	
}

if(reason != ""){
	api.report(report_id, true, reason);
	report_id = 9055;
}
