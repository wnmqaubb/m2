
import * as api from 'api';
import * as os from 'os';

//*************************** �����豸����� ***************************
let device_names_black_table = [
	//-----------�ڴ˷ָ���֮��ά������-----------
	"BBBBas",/*δ֪���*/
	"vmx_fb",/*�����*/
	"vm3dmp",/*�����*/
	"nvd3dum",/*�����*/
	"nv3dmp",/*�����*/
	"HideToolz",/*GEE����*/
	"Sp_Hs",/*��������*/
	"Passkpp_Demo",/*��������*/
	"SpeedHook",/*��������*/
	"Gwken",/*��������*/
	"yxbsq",/*��������*/
	"Win7Speed",/*��������*/
	"wwE21wwE",/*�������*/
	"lonerSpeed_v40",/*��������*/
	//-----------�ڴ˷ָ���֮��ά������-----------
];
let device_names = api.enum_device();
device_names.forEach(function (e) {
	device_names_black_table.forEach(function (device_name) {
		if (e.includes(device_name)) {
			api.report(689052, true, "�Ƿ�������������:" + e);
		}
	})
})
//*************************** �����豸����� ***************************
