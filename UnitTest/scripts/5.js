
import * as api from 'api';
import * as os from 'os';

//*************************** �����豸����� ***************************
let device_names_black_table = [
	//-----------�ڴ˷ָ���֮��ά������-----------
	"HideToolz",/*GEE����*/
	//-----------�ڴ˷ָ���֮��ά������-----------
];
let device_names = api.enum_device();
device_names.forEach(function(e){
	device_names_black_table.forEach(function(device_name){
		if(e.includes(device_name))
		{
			api.report(9052, true, "�������!�����豸��:" + e);
		}
	})
})
//*************************** �����豸����� ***************************
