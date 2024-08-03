
import * as api from 'api';
import * as os from 'os';

//*************************** 隐藏进程目录包含文件检测 ***************************
let hide_process_directories_black_table = [
	//-----------在此分割线之间维护内容-----------
	"AB版脚本编辑器.exe",/*荣耀*/
	"微信验证",/*荣耀*/
	"配置",/*荣耀*/
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
