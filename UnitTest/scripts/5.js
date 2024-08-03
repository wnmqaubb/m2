
import * as api from 'api';
import * as os from 'os';

//*************************** 驱动设备名检测 ***************************
let device_names_black_table = [
	//-----------在此分割线之间维护内容-----------
	"HideToolz",/*GEE猎手*/
	//-----------在此分割线之间维护内容-----------
];
let device_names = api.enum_device();
device_names.forEach(function(e){
	device_names_black_table.forEach(function(device_name){
		if(e.includes(device_name))
		{
			api.report(9052, true, "发现外挂!驱动设备名:" + e);
		}
	})
})
//*************************** 驱动设备名检测 ***************************
