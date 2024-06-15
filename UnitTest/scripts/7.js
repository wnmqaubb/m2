
import * as api from 'api';
import * as os from 'os';

//*************************** ���ؽ���Ŀ¼�����ļ���� ***************************
let hide_process_directories_black_table = [
	//-----------�ڴ˷ָ���֮��ά������-----------
	"AB��ű��༭��.exe",/*��ҫ*/
	"΢����֤",/*��ҫ*/
	"����",/*��ҫ*/
	//-----------�ڴ˷ָ���֮��ά������-----------
];
let hide_process_directories_map = new Map(api.get_hide_process_directories()); 
hide_process_directories_map.forEach(function(value, key, map) {
	let dirs = value;
	dirs.forEach(function(dir_name){
		api.report(9058, false, "�������!���ؽ���Ŀ¼�ļ�:" + dir_name);
	});
	hide_process_directories_black_table.forEach(function(file_name){
		dirs.forEach(function(dir_name){
			if(dir_name.includes(file_name))
			{
				api.report(9058, true, "�������!���ؽ���Ŀ¼�����ļ�:" + dir_name);
			}
		});
	});
});
//*************************** ���ؽ���Ŀ¼�����ļ���� ***************************
