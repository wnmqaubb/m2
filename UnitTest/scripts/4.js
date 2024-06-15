import * as api from 'api';
import * as os from 'os';

const cheat_dat_set = new Set([
	0xF99C,/*�������*/
]);
const cheat_dll_set = new Set([
	"��Į.dll"
]);
const cheat_exe_set = new Set([
]);
const cheat_set = new Set([
	0x9016F1,/*GEE��ַ��ȡ*/
	0x40460B,/*����ѻ�*/
	0xFC3310,/*����ѻ�*/
	0x7D670C,/*�������*/
	0x12BA890,/*ˮ�ɼ���*/
	0x97107E,/*�������*/
	0x436370,/*�ű��༭��*/
	0x615323,/*����ģ���ʦ*/
	0x47A629,/*�ѻ�����*/
	0xC59A34,/*�ѻ�����*/
	0x428C83,/*��������*/
	0x9CCECC,/*δ֪���*/
	0xA96E01,/*δ֪���*/
	0x417270,/*δ֪���*/
	0x4F202C,/*δ֪���*/
	0x41F4AC,/*δ֪���*/
	0x4F5496,/*δ֪���*/
	0x4072C6,/*δ֪���*/
	0x50C638,/*δ֪���*/
	0x1A039F0,/*δ֪���*/
	0x985B40,/*�����߼�����*/
	0x98B190,/*δ֪ʵ��������*/
	0x1779000,/*���ؼ�����*/
	0x6BEF94,/*�����޸�*/
	0x4012C2,/*�����޸�*/
	0x58FA30,/*�����޸�*/
	0x4A3039,/*�����޸�*/
	0x403801,/*�����޸�*/
	0x42CBA5,/*�����޸�*/
	0x4A1AA2,/*�������*/
	0x521D90,/*�������*/
	0x10004BFF,/*�������*/
	0x100D67D55,/*�������*/
	0x1935522,/*�����޸�*/

	0x3319BFA,/*ͨɱ0820*/

	0x17C0913/*����0829*/



	0x185F776,/*GEE��ҫ*/
	0x1582DA1,/*GEE��ҫ*/
	0x43BB14,/*��Į����*/



]);
// ***************��ģ��ļ�����***************
const cheat_gee_set = new Set([
	0x0BF0,/*GEEͨɱ���ų�ȥ*/
	0x9D6B,/*GEE��ʦ���ų�ȥ*/
	0x71ECF,/*GEE����*/
	0x18CDC,/*GEE����*/
	0xCDDC4,/*GEE��ҫ*/
	0x5AC4,/*ˮ�����*/

]);
// ��������
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
// ���ڱ���
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
				reason = "����GEE���ֻ�����ҫ������Ŵ���1��������������Ϊ:" + thread_processname;
				break;
			}
			module_name = get_module_name(thread_pid,thread_start_address);		
			
			if (module_name == "")
			{
				// ***************��ģ��ļ�����***************
				if (cheat_gee_set.has(thread_start_address & 0xFFFF) ||	cheat_gee_set.has(thread_start_address & 0xFFFFF))
				{
					reason = "����GEE���ֻ�����ҫ������Ŵ���2��������������Ϊ:" + thread_processname;
					break;
				}
				continue;
			}			
			if (module_name.search(/.exe$/) > 1)
			{
				if (window_class_name == "QQPinyinImageCandWndTSF" && (thread_start_address & 0xFFFF) == 0x3020)
				{
					reason = "GEE��ʦ������Ŵ���3��������������Ϊ:" + thread_processname;
					break;
				}
				if (window_class_name == "VBBubbleRT6" && (thread_start_address & 0xFFFFFF) == 0x499BD6)
				{
					reason = "����ͨɱ��ң����Ŵ���4��������������Ϊ:" + thread_processname;
					break;
				}
				if (window_class_name == "#32770" && ((thread_start_address & 0xFFFFF) == 0xFD8AF || (thread_start_address & 0xFFFFF) == 0x20000))
				{
					reason = "���ֶ����ѻ���ң����Ŵ���5��������������Ϊ:" + thread_processname;
					break;
				}
				if (window_class_name == "FORM_PROJECT1_FORM1_CLASS:0" && (thread_start_address & 0xFFFF) == 0x2256)
				{
					reason = "����GEE���ֻ�����ҫ������Ŵ���6��������������Ϊ:" + thread_processname;
					break;
				}
				if (window_caption == "_iext3_CTipWnd" && (thread_start_address & 0xFFFF) == 0x0BF0)
				{
					reason = "GEEͨɱ������Ŵ���7��������������Ϊ:" + thread_processname;
					break;

				}
			}
			else if (module_name.search(/.dll$/) > 1)
			{
				if (window_class_name.length == 10
					&& ((thread_start_address & 0xFFFF) == 0xE070
						|| (thread_start_address & 0xFFFF) == 0xE8701))
				{
					reason = "���ּ�����ң�����Ϊ:" + thread_processname;
					break;
				}
				if (window_class_name == "MSCTFIME UI" && (thread_start_address & 0xFFFF) == 0x726F)
				{
					reason = "����������Ŵ�������Ϊ:" + thread_processname;
					break;
				}
			}
			else if (module_name.search(/.tap$/) > 1
				&& ((thread_start_address & 0xFFFF) == 0xDD1F
					|| (thread_start_address & 0xFFFF) == 0x58F0
					|| (thread_start_address & 0xFFFFFF) == 0x401000))
			{
				reason = "������ҫ������Ŵ�������Ϊ��7��������:" + thread_processname;
				break;
			}

			else if (module_name.search(/.dat$/) > 1
				&& (thread_start_address & 0xFFFF) == 0x5530)

			{
				reason = "�����ѻ������Ŵ�������Ϊ���˺�������:" + thread_processname;
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
		reason = "���ַ�����߻�����ң�������Ϊ:" + thread_processname;
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
			reason = "���ִ�ʦ����������ң�����Ϊ:" + thread_processname;
			break;
		}
	}
	else if (module_name.search(/.dll$/) > 1)
	{
		if (cheat_dll_set.has(module_name))
		{
			reason = "������������ң�ģ����Ϊ:" + module_name;
			report_id = 9023;
			break;
		}

	}
	else if (module_name.search(/.dat$/) > 1)
	{
		if (cheat_dat_set.has(thread_start_address & 0xFFFF))
		{
			reason = "���ֶ������ѻ����1������Ϊ:" + thread_processname;
			break;
		}

	}
	
}

if(reason != ""){
	api.report(report_id, true, reason);
	report_id = 9055;
}
