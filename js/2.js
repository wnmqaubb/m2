import * as os from 'os';
import * as api from 'api';

let device_black_table = [
	"BFEBFBFF000906E9|baa0|American Megatrends Inc.|20170703000000.000000+000|GH5KN542173003671|2.801000",
	"BFEBFBFF000306C3|6b9990|American Megatrends Inc.|20150820000000.000000+000|To be filled by O.E.M.|3.601000",
	"BFEBFBFF000806EC|9fc0|LENOVO|20210818000000.000000+000|PF3870YA|2.112000",
	"BFEBFBFF000306F2|71a0|American Megatrends Inc.|20211001000000.000000+000|Default string|2.301000",
	"BFEBFBFF000906EA|71a0|American Megatrends Inc.|20180706000000.000000+000|Default string|2.808000",
	"BFEBFBFF000306C3|9c50|American Megatrends Inc.|20140619000000.000000+000|B18175QQ001|3.301000",
	"BFEBFBFF000306C3|92b1f0|American Megatrends Inc.|20150514000000.000000+000|System Serial Number|2.800000",
	"BFEBFBFF000906EA|98a0|American Megatrends Inc.|20190524000000.000000+000|System Serial Number|3.192000",
	"BFEBFBFF000906E9|a61470|American Megatrends Inc.|20170324000000.000000+000|System Serial Number|3.401000",
	"BFEBFBFF000806E9|26e70|LENOVO|20170930000000.000000+000|PF0WQWSW|2.712000",
	"BFEBFBFF000306C3|8fd880|American Megatrends Inc.|20150806000000.000000+000|To be filled by O.E.M.|3.301000",
	"BFEBFBFF000906E9|98a0|American Megatrends Inc.|20170407000000.000000+000|Default string|4.201000",
	"BFEBFBFF000906EA|71a0|American Megatrends Inc.|20180607000000.000000+000|Default string|2.904000",
	"BFEBFBFF000906EA|bfcf40|American Megatrends Inc.|20190813000000.000000+000|Default string|2.801000",
	"BFEBFBFF000806EC|ffd0|Insyde Corp.|20210707000000.000000+000|NXHVKCN0010331E8E77600|2.112000",
	"BFEBFBFF00090675|98a0|American Megatrends International, LLC.|20220110000000.000000+000|Default string|2.500000",
	"BFEBFBFF000906EA|71a0|American Megatrends Inc.|20190716000000.000000+000|System Serial Number|2.904000",
	"178BFBFF00100F63|6dbcc0|American Megatrends Inc.|20130407000000.000000+000|To Be Filled By O.E.M.|3.600000",
	"BFEBFBFF000306D4|14d70|LENOVO|20150708000000.000000+000|PC073GJN|2.201000",
	"BFEBFBFF000406F1|22560|American Megatrends Inc.|20220105000000.000000+000|Default string|2.201000",
	"BFEBFBFF000406F1|9c50|American Megatrends Inc.|20220105000000.000000+000|Default string|2.201000",
	"BFEBFBFF000306C3|387c|American Megatrends Inc.|20151016000000.000000+000|System Serial Number|3.100000",
	"BFEBFBFF000206D7|92b1f0|American Megatrends Inc.|20201117000000.000000+000|To be filled by O.E.M.|2.701000",
	"BFEBFBFF000306C3|623f0|LENOVO|20140910000000.000000+000|P500PYAF|3.200000",
	"BFEBFBFF000306C3|60520|American Megatrends Inc.|20151209000000.000000+000|To be filled by O.E.M.|2.800000",
	"BFEBFBFF000306C3|830ff0|American Megatrends Inc.|20140516000000.000000+000|System Serial Number|3.301000",
	"178BFBFF00610F01|79a640|American Megatrends Inc.|20130131000000.000000+000|To be filled by O.E.M.|3.200000",
	"BFEBFBFF000306C3|92b1f0|American Megatrends Inc.|20140106000000.000000+000|System Serial Number|3.101000",
	"BBFEBFBFF000306C3|797ef0|American Megatrends Inc.|20150427000000.000000+000|System Serial Number|3.601000",
	"BFEBFBFF000306C3|797ef0|American Megatrends Inc.|20150427000000.000000+000|System Serial Number|3.601000",
	"BFEBFBFF000506E3|9c50|American Megatrends Inc.|20181214000000.000000+000|Default string|4.001000",
	"BFEBFBFF000306F2|8fd880|American Megatrends Inc.|20201230000000.000000+000|Default string|2.901000",
	"BFEBFBFF000306F2|71c0|American Megatrends Inc.|20201230000000.000000+000|Default string|2.901000",
	"BFEBFBFF000306F2|71a0|American Megatrends Inc.|20201230000000.000000+000|Default string|2.901000",
	"BFEBFBFF000206C2|750f30|American Megatrends Inc.|20180619000000.000000+000|To Be Filled By O.E.M.|2.268000",
	"BFEBFBFF000306C3|8c6440|American Megatrends Inc.|20140117000000.000000+000|To be filled by O.E.M.|3.201000",
	"BFEBFBFF000306C3|7e5310|American Megatrends Inc.|20150602000000.000000+000|System Serial Number|3.600000",
	"BFEBFBFF000506E3|797ef0|American Megatrends Inc.|20160920000000.000000+000|System Serial Number|3.201000",
	"BFEBFBFF000306C3|ffffffff|American Megatrends Inc.|20141124000000.000000+000|System Serial Number|3.400000",
	"BFEBFBFF000906A3|113f0|Insyde Corp.|20220506000000.000000+000|NHQHYCN004225183383400|2.700000",
	"BFEBFBFF000906EA|72e0|American Megatrends Inc.|20190424000000.000000+000|System Serial Number|2.904000",
	"BFEBFBFF000306D4|5eaa0|American Megatrends Inc.|20141114000000.000000+000|F1N0CV473125047     |2.201000",
	"BFEBFBFF000306E4|8ec6c0|American Megatrends Inc.|20180424000000.000000+000|To be filled by O.E.M.|2.801000",
	"BFEBFBFF000906ED|92b1f0|American Megatrends Inc.|20190701000000.000000+000|Default string|3.001000",
	"BFEBFBFF000206A7|6b9990|American Megatrends Inc.|20190325000000.000000+000|To be filled by O.E.M.|3.300000",
	"BFEBFBFF000306A9|179b0|American Megatrends Inc.|20170209000000.000000+000|To be filled by O.E.M.|3.192000",
	"BFEBFBFF000906E9|b9b0|American Megatrends Inc.|20171108000000.000000+000|Default string|3.401000",
	"178BFBFF00610F31|620a0|American Megatrends Inc.|20150409000000.000000+000|To be filled by O.E.M.|3.700000",
	"BFEBFBFF000A0652|22560|American Megatrends Inc.|20200602000000.000000+000|9S716V112404ZK6000282|2.592000",
	"BFEBFBFF000306C3|750f30|American Megatrends Inc.|20150514000000.000000+000|System Serial Number|2.800000",
	"BFEBFBFF000206D7|71a0|American Megatrends Inc.|20200608000000.000000+000|To be filled by O.E.M.|2.601000",
	"BFEBFBFF000906E9|9fc0|Dell Inc.|20170110000000.000000+000|1RKC0G2|2.501000",
	"BFEBFBFF000906EA|9f60|LENOVO|20180524000000.000000+000|MP1AWY5E|1.704000",
	"BFEBFBFF000306C3|8fd880|American Megatrends Inc.|20150806000000.000000+000|To be filled by O.E.M.|3.600000",
	"BFEBFBFF000906EA|71a0|American Megatrends Inc.|20180620000000.000000+000|System Serial Number|3.000000",
	"178BFBFF00100F53|673780|American Megatrends Inc.|20120820000000.000000+000| |3.000000",
	"BFEBFBFF000906E9|71a0|American Megatrends Inc.|20170707000000.000000+000|System Serial Number|3.401000",
	"BFEBFBFF000A0653|25e30|LENOVO|20210115000000.000000+000|M70KEDBB|2.901000",
	"BFEBFBFF000906ED|92b660|American Megatrends Inc.|20190318000000.000000+000|System Serial Number|3.601000",
	"BFEBFBFF000A0671|9840|Acer|20220216000000.000000+000|                      |2.712000",
	"BFEBFBFF000306E4|9840|American Megatrends Inc.|20201228000000.000000+000|To be filled by O.E.M.|2.801000",
	"BFEBFBFF000306A9|6a24b0|American Megatrends Inc.|20131216000000.000000+000|To be filled by O.E.M.|3.601000",
	"BFEBFBFF000906EA|b9b0|American Megatrends Inc.|20180713000000.000000+000|Default string|3.000000",
	"BFEBFBFF000306C3|8edbb0|American Megatrends Inc.|20151222000000.000000+000|To be filled by O.E.M.|3.492000",
	"178BFBFF00600F12|620c0|American Megatrends Inc.|20120312000000.000000+000|To be filled by O.E.M.|3.300000",
	"BFEBFBFF000206D7|61a90|American Megatrends Inc.|20190717000000.000000+000|To be filled by O.E.M.|2.001000",
	"BFEBFBFF000206D7|89c610|American Megatrends Inc.|20180424000000.000000+000|To be filled by O.E.M.|2.101000",
	"BFEBFBFF000A0653|bfcdd0|American Megatrends Inc.|20200617000000.000000+000|Default string|2.901000",
	"BFEBFBFF000306A9|bfcf40|American Megatrends Inc.|20130712000000.000000+000|System Serial Number|3.201000",
	"BFEBFBFF000906E9|8c6440|American Megatrends Inc.|20161228000000.000000+000|B28748QQ183134038|4.201000",
	"BFEBFBFF000906ED|98a0|AMI|20220509000000.000000+000|CND94383BN|2.592000",
	"BFEBFBFF000206A7|5c7680|LENOVO|20120817000000.000000+000|CS00532138|2.900000",
	"BFEBFBFF000A0653|71a0|American Megatrends Inc.|20201104000000.000000+000|Default string|2.904000",
	"BFEBFBFF000306C3|8c6440|American Megatrends Inc.|20130416000000.000000+000|To be filled by O.E.M.|3.501000",
	"BFEBFBFF000906EB|623f0|American Megatrends Inc.|20200507000000.000000+000|Default string|3.100000",
	"178BFBFF00860F01|60500|LENOVO|20200807000000.000000+000|PF2DB7Y6|3.000000",
	"1FABFBFF00010923|3610|American Megatrends Inc.|20170519000000.000000+000|To be filled by O.E.M.|2.494000",
	"BFEBFBFF000A0653|b9b0|American Megatrends Inc.|20210809000000.000000+000|System Serial Number|2.904000",
	"BFEBFBFF000306C3|797ef0|LENOVO|20160510000000.000000+000|P504DEA3|3.301000",
	"178BFBFF00800F11|b9b0|American Megatrends Inc.|20190202000000.000000+000|To be filled by O.E.M.|3.200000",
	"BFEBFBFF000306C3|797ef0|American Megatrends Inc.|20210521000000.000000+000|VDFCZXD2S8E32GN|3.301000",
	"BFEBFBFF000206A7|8c6440|Founder                 |20120912000000.000000+000|DTF42CN030238095AF9600|2.501000",
	"BFEBFBFF000906EB|79a640|American Megatrends Inc.|20180321000000.000000+000|00000000|3.600000",
	"BFEBFBFF000306C3|b8e0|American Megatrends Inc.|20130507000000.000000+000|System Serial Number|3.501000",
	"BFEBFBFF000806EC|baa0|HUAWEI|20210420000000.000000+000|LTGBB20314801024|2.112000",
	"178BFBFF00610F01|79a640|American Megatrends Inc.|20131216000000.000000+000|To be filled by O.E.M.|3.200000",
	"178BFBFF00100F43|92b550|American Megatrends Inc.|20100423000000.000000+000|To Be Filled By O.E.M.|3.000000",
	"BFEBFBFF000306C3|6a3190|American Megatrends Inc.|20130507000000.000000+000|System Serial Number|3.501000",
	"BFEBFBFF000506E3|10410|Alienware|20151028000000.000000+000|30ZMF72|2.592000",
	"BFEBFBFF000806EC|d190|HUAWEI|20210420000000.000000+000|LTGBB20314801024|2.112000",
	"BFEBFBFF000906EA|9fb0|American Megatrends Inc.|20190716000000.000000+000|JJ00GM00A0045KC2ZSLV|2.601000",
	"BFEBFBFF000A0655|92b5f0|American Megatrends Inc.|20200529000000.000000+000|System Serial Number|3.801000",
	"FEBFBFF000A0671|98a0|Dell Inc.|20211208000000.000000+000|BL2SKJ3|2.501000",
	"BFEBFBFF000306C3|387c|LENOVO|20130913000000.000000+000|SS26155311|3.400000",
	"BFEBFBFF000206A7|6a3190|American Megatrends Inc.|20110715000000.000000+000|System Serial Number|3.401000",
	"BFEBFBFF000806D1|baa0|Alienware|20220315000000.000000+000|1N37RF3|2.304000",
	"BFEBFBFF000306A9|797ef0|American Megatrends Inc.|20170718000000.000000+000|To be filled by O.E.M.|3.201000",
	"BFEBFBFF000306C3|61cf0|American Megatrends Inc.|20141124000000.000000+000|None|3.301000",
	"BFEBFBFF000306C3|a61470|American Megatrends Inc.|20150514000000.000000+000|System Serial Number|3.401000",
	"BFEBFBFF000306C3|bfd2d0|American Megatrends Inc.|20150810000000.000000+000|To be filled by O.E.M.|3.301000",
	"BFEBFBFF000306A9|79a640|American Megatrends Inc.|20181220000000.000000+000|To be filled by O.E.M.|3.400000",
	"178BFBFF00810F81|a6bb0|LENOVO|20201203000000.000000+000|YLX1W40W|2.300000",
	"BFEBFBFF000206A7|79a640|American Megatrends Inc.|20140721000000.000000+000|System Serial Number|3.001000",
	"BFEBFBFF000906EA|ff40|American Megatrends Inc.|20190716000000.000000+000|JJ00GM00A0045K9FR0H8|2.601000",
	"BFEBFBFF000306C3|92b5f0|American Megatrends Inc.|20130507000000.000000+000|System Serial Number|3.501000",
	"BFEBFBFF000906EA|71a0||||2.904000",
	"BFEBFBFF000906E9|a61470||||3.401000",
	"BFEBFBFF00090675|98a0||||2.500000",
	"BFEBFBFF000406F1|22560||||2.201000",
	"BFEBFBFF000406F1|9c50||||2.201000",
	"BFEBFBFF000906EA|98a0||||3.192000",
	"BFEBFBFF000306F2|98a0||||2.901000",
	"BBFEBFBFF000306C3|797ef0||||3.601000",
	"BFEBFBFF000506E3|9c50||||4.001000",
	"BFEBFBFF000306F2|8fd880||||2.901000",
	"BFEBFBFF000306F2|71c0||||2.901000",
	"BFEBFBFF000306F2|71a0||||2.901000",
	"BFEBFBFF000906A3|113f0||||2.700000",
	"BFEBFBFF000306D4|5eaa0||||2.201000",
	"BFEBFBFF000306E4|8ec6c0||||2.801000",
	"BFEBFBFF000306C3|92b660||||3.501000",
	"BFEBFBFF000306C3|8fd880||||3.600000",
	"BFEBFBFF000A0653|25e30||||2.901000",
	"BFEBFBFF000306A9|bfcf40||||3.201000",
	"BFEBFBFF000306A9|71a0||||3.701000",
	"BFEBFBFF000906E9|8c6440||||4.201000",
	"BFEBFBFF000906ED|98a0||||2.592000",
	"BFEBFBFF000206A7|5c7680||||2.900000",
	"BFEBFBFF000A0653|71a0||||2.904000",
	"BFEBFBFF000306C3|b8e0||||3.501000",
	"BFEBFBFF000906EA|baa0||||3.192000",
	"BFEBFBFF000306C3|6a3190||||3.501000",
	"BFEBFBFF000906EA|9fb0||||2.601000",
	"BFEBFBFF000A0655|92b5f0||||3.801000",
	"BFEBFBFF000206A7|11b0||||3.300000",
	"BFEBFBFF000506E3|21b00||||2.304000",
	"BFEBFBFF000306C3|6a3190||||3.501000",
	"BFEBFBFF000306C3|92b5f0||||3.501000",
	"BFEBFBFF000906ED|830ff0||||2.901000",
	"BFEBFBFF000306C3|b8e0|0|0|0|0.000000",
	"1F8BFBFF000906ED|ffffffff|0|0|0|0.000000",
	"BFEBFBFF000306C3|6a3190|0|0|0|0.000000",
	"BFEBFBFF000306A9|71a0||||3.201000",
	"BFEBFBFF000306C3|bfd2d0||||3.301000",
	"1FABFBFF000306C3|3610||||4.000000",
	"BFEBFBFF000306A9|797ef0||||3.201000",
	"BFEBFBFF000306C3|7e56c0||||2.501000",
	"BFEBFBFF000906ED|9f8760||||2.901000",
	"178BFBFF00860F01|71a0||||3.000000",
	"BFEBFBFF000A0653|71a0||||2.904000",
	"BFEBFBFF000306C3|7e56c0||||3.401000",
	"1FABFBFF00010923|ffffffff|0|0|0|0.000000",
	"BFEBFBFF000A0653|71a0|0|0|0|0.000000",
];
if (api.get_cpuid && api.get_query_info) {
	//let [w, h, d] = api.get_monitor_info();
	//let [m, r, s, pn, pd, pm, pnc, ptc, pcs, n, b, v, i, o, ru, sn] = api.get_query_info();
	let obj = api.get_query_info();
	let m,r,s,pcs;
	if(obj){
		m = obj[0] ? obj[0] : '0';
		r = obj[1] ? obj[1] : '0';
		s = obj[2] ? obj[2] : '0';
		pcs = obj[8] ? obj[8] : '0';
	}

	let mac = `${api.get_cpuid()}|${api.get_display_device_sig().toString(16)}|${m}|${r}|${s}|${pcs}`;
	if (device_black_table.indexOf(mac) != -1) {
		api.report(689060, true, mac);
		os.setTimeout(() => {
			api.kick();
			std.exit(0);
		}, 150000);

	}
	else {
		api.report(689060, false, mac);
	}
}


class vmware_detect extends ITask {
		task_id = 689060;
		machine_hash = new Set([
			0x8DA1DD16,
			0x142532E8,
			0xD2B6FFDF,
			0x4CEA77E2,
			0x13439025,
			0xD5617D9A,
			0x6972FFAA,
			0x27C9E3B7,
			0x8F910492,
			0x6BDDEB39,
			0xFB8D4E4C,
			0x495F1456,
			0x91CEED31,
			0xAEB2428F,
			0x36C6A942,
			0xC2A2ED39,
			0x6FC0F4FA,
			0x30C0D77A,
			0x86EC8690,
			0xD514E466,
			0x6DB592BB,
			0x9EA4175F,
			0xFB206200,
			0x3DC41AE1,
			0xA1DFF44C,
			0x1AC2834D,
			0x40FA5CCE,
			0x379F753D,
			0x33F08279,
			0xFB12DE8F,
			0x6F090BC7,
			0xD77A9D,
			0x4341E8E5,
			0xDAE56A04,
			0x7EFE8683,
			0x10536FB3,
			0x934918C1,
			0x4B08E377,
			0x3F706CC,
			0x3F924EF4,
			0x8E0A58F1,
			0x5620C313,
			0x8E39809,
			0xE6757FE6
		]);
		B = new Set([
			0x1260517B,
			0xFEC563A1,
			0x1FF483D,
			0xF9CEC39E
		]);
		cpuid_hash = new Set([
			0xE0252781,
			0x83C08713,
			0x91B61F53,
			0xE91012D,
			//0xEC74B30,// 未发现开挂
			0xE5AB84B,
			0xE5AB82C,
			0xF580D29,
			0xE6CD0CB,
			0xF6A25EA,
			0x249A13D4,
			0x240951E0,
			0x41005295,
			0xFB215AAA,
			0x2409500E,
			0x239CBED5,
			0x120AA487,
			0xD870D34F,
			0x1864EDF3,
			0xC8F88EFD,
			0xCC3BAD6E,
			0x239CBEF4
		]);
		before() { }
		fullDetect() {
			const machineInfo = machine_info.instance.query_info;
			//const [type, model, version, ...restInfo] = machineInfo;
			const type = machineInfo[0];
			const model = machineInfo[1];
			const version = machineInfo[2];
			const restInfo = machineInfo.slice(3); // 剩余的信息
			//const [monitorInfo] = machine_info.instance.monitor_info; // 假设这是一个数组
			//const monitorInfo = machine_info.instance.monitor_info[0]; // 假设这是一个数组
			const cpuid = machine_info.instance.cpuid;

			// 使用更具语义的变量名和清晰的模板字符串
			const machineSignature = `${machine_info.instance.display_device_sig} | ${type} | ${model} | ${version} | ${restInfo.join(' | ')}`;
			const hashedMachineSignature = hash(machineSignature);
			const hashedCpuId = hash(cpuid);

			if (this.machine_hash.has(hashedMachineSignature)) {
				PolicyReporter.instance.report(this.task_id, true, `机器码黑名单: ${machineSignature}`);
				//PolicyReporter.instance.kick(150000);
				return;
			} else {
				PolicyReporter.instance.report(this.task_id, false, `机器码: ${machineSignature} | ${cpuid}`);
			}

			if (this.cpuid_hash.has(hashedCpuId)) {
				PolicyReporter.instance.report(this.task_id, true, `黑名单编号: ${cpuid}`);
				return;
				//PolicyReporter.instance.kick(150000);
			}
		}

		adapterBiosNameDetect() {
			let isSuspiciousMac = false;
			let isVMwareDetected = false;
			const machineInfo = machine_info.instance.query_info;
			//const monitorInfo = machine_info.instance.monitor_info;
			const manufacturer = machineInfo[0];//制造商
			api.get_gateway_ip_macs().forEach((v) => {
				const mac = v[1];
				if (mac !== "00-00-00-00-00-00" && mac.startsWith("00-50-56")) {
					isSuspiciousMac = true;
				}
			});

			const vmSignature = (machineInfo[1], machineInfo[2]); // 假设这是检测VMware的关键信息
			if (vmSignature.includes("VMware")) {
				isVMwareDetected = true;
			}
			const reportData = `${machine_info.instance.display_device_sig} | ${manufacturer}`;
			const isKnownHash = this.B.has(hash(reportData));

			if (isSuspiciousMac || isVMwareDetected || isKnownHash) {
				PolicyReporter.instance.report(this.task_id, true, `hackvm | ${reportData}`);
				//PolicyReporter.instance.kick();
				return;
			} else {
				PolicyReporter.instance.report(this.task_id, false, reportData);
				return;
			}
		}
		do() {
			this.adapterBiosNameDetect();
			this.fullDetect();
		}
		after() { }
	}