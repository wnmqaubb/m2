import * as os from 'os';
import * as api from 'api';

let device_black_table = [
"1F8BFBFF000906ED|ffffffff|VMware, Inc.|20200515000000.000000+000|VMware-56 4d 9d 20 c7 46 59 41-62 84 29 17 14 fb 1a 79|",
];
if(api.get_cpuid && api.get_query_info)
{
    let [w,h,d] = api.get_monitor_info();
    let [m,r,s,pn, pd, pm, pnc, ptc, pcs,n,b,v,i,o,ru,sn] = api.get_query_info();
	let mac = `${api.get_cpuid()}|${api.get_display_device_sig().toString(16)}|${m}|${r}|${s}|${pn}|${pd}|${ptc}|${pcs}`;
    if(device_black_table.indexOf(mac) != -1)
	{
		api.report(9051, true, mac);
	}
	else
	{
		api.report(9051, false, mac);
	}
}
