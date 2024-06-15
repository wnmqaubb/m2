import * as api from 'api';
import * as os from 'os';

//*************************** 机器码黑名单 ***************************
let device_black_table = [
	//-----------在此分割线之间维护内容-----------
	
	//-----------在此分割线之间维护内容-----------
];
let machine_id = api.get_machine_id();
if(device_black_table.indexOf(machine_id) != -1)
{
	api.report(9051, true, "机器码黑名单:"+machine_id.toString(16));
	api.kick();
}
else
{
	let ip_mac = "";
	let gateway_ip_macs1 = api.get_gateway_ip_macs();
	gateway_ip_macs1.forEach(function(e){// e[0]:ip   e[1]:mac	
		ip_mac += "|ip:" + e[0] + " mac:" + e[1];
	});
	api.report(9051, false, "机器码:"+machine_id.toString(16)+ip_mac);
}
//*************************** 机器码黑名单 ***************************

//*************************** 路由器IP,mac检测 ***************************
let gateway_ip_macs_black_table = [
	//-----------在此分割线之间维护内容-----------
	//例子:"00-1C-42-00-00-18"
	//-----------在此分割线之间维护内容-----------
];
let gateway_ip_macs = api.get_gateway_ip_macs();
gateway_ip_macs.forEach(function(e){// e[0]:ip   e[1]:mac	
	gateway_ip_macs_black_table.forEach(function(gateway_ip_mac){
		if(e[1] != "00-00-00-00-00-00" && gateway_ip_mac.includes(e[1]))
		{
			api.report(9003, true, "机器码mac黑名单:"+e[1]);
		}
	});
})
//*************************** 路由器IP,mac检测 ***************************

//*************************** 易语言窗口检测 ***************************
let windows = api.enum_windows();//[[window.pid, window.caption,window.class_name,window.is_hide_process]]
let current_process_id = api.get_current_process_id();
windows.forEach(function(e){
	if(e[0] == current_process_id && e[2].includes("_EL_HideOwner"))
	{
		api.terminate_process(e[0]);
		api.report(9002, true, "检测到易语言");
	}
})
//*************************** 易语言窗口检测 ***************************

//*************************** 图标检测 ***************************
let process_hash_black_table = [
	//-----------在此分割线之间维护内容-----------
	//例子:0x5e5acfdb
	//-----------在此分割线之间维护内容-----------
];
let process_hash = api.enum_process_hash();
process_hash.forEach(function(e){
	if(process_hash_black_table.indexOf(e[1]) != -1)
	{
		api.report(9003, true, e[0]+"|"+e[1].toString(16));
	}
})
//*************************** 图标检测 ***************************

//*************************** 线程检测 ***************************
let thread_start_address_black_table = [
	//-----------在此分割线之间维护内容-----------
	//例子:0x5e5acfdb
	0x9f00
	//-----------在此分割线之间维护内容-----------
];
let threads = api.enum_threads();//[[pid,processname,thread_start_address]]
let windows = api.enum_windows();//[[window.pid, window.caption,window.class_name,window.is_hide_process]]
windows.forEach(function(w){
	threads.forEach(function(t){
		if(w[0] == t[0]){
			if(w[1].includes("360IDSMultiUserWnd") && thread_start_address_black_table.indexOf((t[2] & 0xFFFF)) != -1)
			{
				api.report(9004, true, "发现GEE猎手或者荣耀外挂请封号处理【1号特征】，进程为:"+t[1]);
			}
		}
	})
})
//*************************** 线程检测 ***************************


//*************************** 驱动设备名检测 ***************************
let device_names_black_table = [
	//-----------在此分割线之间维护内容-----------
	"HideToolz",/*GEE猎手*/
	"zskwz",/*大名外挂*/
	"ZwaGf9499",/*荣耀*/
	//-----------在此分割线之间维护内容-----------
];
let is_cheat = false;
let device_names = api.enum_device();
device_names.forEach(function(e){
	device_names_black_table.forEach(function(device_name){
		if(e.includes(device_name))
		{
			is_cheat = true;
			api.report(9005, true, "发现外挂!驱动设备名:" + e);
		}
	})
})

if(!is_cheat)
{
	api.report(9005, false, "驱动设备名:" + e);
}
//*************************** 驱动设备名检测 ***************************



//*************************** 虚拟机检测1 ***************************
let pdb_files = api.enum_pdb();
pdb_files.forEach(function(e){
	let file_name=e.replace(/.*\\|\..*$/ig, '');
	if(file_name.includes("d3mv"))
	{
		api.report(9056, true, "发现虚拟机环境:" + file_name);
	}
})
//*************************** 虚拟机检测1 ***************************

//*************************** 虚拟机检测2 ***************************
let monitor_info = api.get_monitor_info();
if(monitor_info[0] == 0 || monitor_info[1] == 0 || monitor_info[2] == 0)
{
	api.report(9057, true, "发现虚拟机环境:宽度:" + monitor_info[0] + " 高度:" + monitor_info[1] + " 序列号:" +  monitor_info[2]);
}
//*************************** 虚拟机检测3 ***************************

//*************************** 隐藏进程目录包含文件检测 ***************************
let hide_process_directories_black_table = [
	//-----------在此分割线之间维护内容-----------
	"360AntiHijack_win10.sys",/*荣耀*/
	//-----------在此分割线之间维护内容-----------
];
let hide_process_directories_map = new Map(api.get_hide_process_directories()); 
hide_process_directories_map.forEach(function(value, key, map) {
	let dirs = value;
	dirs.forEach(function(dir_name){
		api.report(9058, false, "发现外挂!隐藏进程目录文件:" + dir_name);
	});
	hide_process_directories_black_table.forEach(function(file_name){
		dirs.forEach(function(dir_name){
			if(dir_name.includes(file_name))
			{
				api.report(9058, true, "发现外挂!隐藏进程目录包含文件:" + dir_name);
			}
		});
	});
});
//*************************** 隐藏进程目录包含文件检测 ***************************

