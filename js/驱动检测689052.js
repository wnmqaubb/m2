
import * as api from 'api';
import * as os from 'os';

//*************************** 驱动设备名检测 ***************************
let device_names_black_table = [
	//-----------在此分割线之间维护内容-----------
	"BBBBas",/*未知外挂*/
	"vmx_fb",/*虚拟机*/
	"vm3dmp",/*虚拟机*/
	"nvd3dum",/*虚拟机*/
	"nv3dmp",/*虚拟机*/
	"HideToolz",/*GEE猎手*/
	"Sp_Hs",/*驱动加速*/
	"Passkpp_Demo",/*驱动加速*/
	"SpeedHook",/*驱动加速*/
	"Gwken",/*驱动加速*/
	"yxbsq",/*驱动加速*/
	"Win7Speed",/*驱动加速*/
	"wwE21wwE",/*封包拦截*/
	"lonerSpeed_v40",/*驱动加速*/
	//-----------在此分割线之间维护内容-----------
];
let device_names = api.enum_device();
device_names.forEach(function (e) {
	device_names_black_table.forEach(function (device_name) {
		if (e.includes(device_name)) {
			api.report(689052, true, "非法驱动正在运行:" + e);
		}
	})
})
//*************************** 驱动设备名检测 ***************************
