import * as api from 'api';
import * as os from 'os';

const cheat_dat_set = new Set([
	0xF99C,/*定制外挂*/
]);
const cheat_exe_set = new Set([
]);
const cheat_set = new Set([
	0xA96E01,/*未知外挂*/
	0x417270,/*未知外挂*/
	0x4F202C,/*未知外挂*/
	0x41F4AC,/*未知外挂*/
	0x7BB000,/*未知外挂*/
	0x46FCCA,/*未知外挂*/
	0x4F5496,/*未知外挂*/
	0x4072C6,/*未知外挂*/
	0x50C638,/*未知外挂*/
	0x401208,/*未知外挂*/
	0x1A039F0,/*未知外挂*/
	0x985B40,/*守望者加速器*/
	0x1779000,/*神秘加速器*/
	0x6BEF94,/*数据修改*/
	0x7BB000,/*数据修改*/
	0x4012C2,/*数据修改*/
	0x58FA30,/*数据修改*/
	0x4A3039,/*数据修改*/
	0x403801,/*数据修改*/
	0x42CBA5,/*数据修改*/
	0x419B00,/*封包工具*/
	0x4A1AA2,/*封包工具*/
	0x521D90,/*封包工具*/
	0x10004BFF,/*封包工具*/
	0x1935522,/*数据修改*/
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
	0x215755C,/*GEE通杀0501*/


	0x22CB586,/*GEE猎手0602*/
	0x7D6B38,/*GEE猎手0228*/
	0x19C9D59,/*GEE猎手0311*/
	0x17885AB,/*GEE猎手0313*/
	0x24642CC,/*GEE猎手0311*/
	0x29466F0,/*GEE猎手0315*/
	0x1F49A78,/*GEE猎手0317*/
	0x2854D45,/*GEE猎手0318*/
	0x2336093,/*GEE猎手0320*/
	0x1702E3E,/*GEE猎手0331*/
	0x145972B,/*GEE猎手0402*/
	0x23A96BE,/*GEE猎手0407*/
	0x17885A8,/*GEE猎手0411*/
	0x1EA4656,/*GEE猎手0411*/
	0x17F7C24,/*GEE猎手0412*/
	0x234488B,/*GEE猎手0414*/
	0x17885AB,/*GEE猎手0415*/
	0x16143E9,/*GEE猎手0416*/
	0x140AAAB,/*GEE猎手0417*/
	0x15DD3C7,/*GEE猎手0421*/
	0x142A85D,/*GEE猎手0424*/
	0x14908CC,/*GEE猎手0428*/
	0x235A0FE,/*GEE猎手0430*/
	0x235CAD1,/*GEE猎手0501*/
	0x1614E39,/*GEE猎手0502*/
	0x17A23A5,/*GEE猎手0503*/
	0x199981C,/*GEE猎手0504*/
	0x14FBD68,/*GEE猎手0510*/
	0x245B882,/*GEE猎手0512*/
	0x234C9E7,/*GEE猎手0515*/
	0x23A393E,/*GEE猎手0518*/
	0x14602CF,/*GEE猎手0519*/
	0x238C675,/*GEE猎手0525*/



	0x198CF3C,/*GEE荣耀*/
	0x185F776,/*GEE荣耀*/



]);
// ***************无模块的加在这***************
const cheat_gee_set = new Set([
	0x0BF0,/*GEE通杀不放出去*/
	0x9D6B,/*GEE大师不放出去*/
	0x71ECF,/*GEE猎手*/
	0x18CDC,/*GEE猎手*/
	0xCDDC4,/*GEE荣耀*/
	0x5AC4,/*GEE插件版*/

]);

const window_class_name_black_set = new Set([
	"MSCTFIME",
	"tooltips_class32",
	"FORM_PROJECT1_FORM1_CLASS:0",
	"QQPinyinImageCandWndTSF",
	"VBBubbleRT6",
	"AutoIt v3 GUI",
	"TForm1",
	"#32770",
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
				if (window_class_name == "VBBubbleRT6" && (thread_start_address & 0xFFFFF) == 0xFB401)
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
	else if (module_name.search(/.dll$/) > 1)
	{
		if (cheat_dat_set.has(thread_start_address & 0xFFFF))
		{
			reason = "发现定制类脱机外挂，进程为:" + thread_processname;
			break;
		}
	}
	
}

if(reason != ""){
	api.report(9051, true, reason);
}
