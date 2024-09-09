import * as api from 'api';
import * as os from 'os';
//*************************** 虚拟机检测 ***************************
let pdb_files = api.enum_pdb();
if (0) {
	pdb_files.forEach(function (e) {
		if (e.includes("bora-vm") || e.includes("wddm\\i386") || e.includes("wddm\\x86")) {
			api.report(689056, true, "发现虚拟机环境:" + e);
			os.setTimeout(() => {
				api.kick();
				std.exit(0);
			}, 5000);
			return;
		}
		let file_name = e.replace(/.*\\|\..*$/ig, '');
		if (file_name.includes("vm3d")) {
			api.report(689056, true, "发现虚拟机环境:" + file_name);
			os.setTimeout(() => {
				api.kick();
				std.exit(0);
			}, 5000);
		}
	})
}



if (api.get_cur_module_list) {
	let modules = api.get_cur_module_list();
	for (let i = 0; i < modules.length; i++) {
		//let [module_name, base, size] = modules[i];
		let module_name = modules[i][0];
		let func_ptr = api.get_proc_address(module_name, "OpenAdapter");
		if (func_ptr) {
			let sig_ret = api.scan(func_ptr, 0x100, [0xe8, 0xcc, 0xcc, 0xcc, 0xcc, 0xB9, 0xcc, 0xcc, 0xcc, 0xcc, 0xe8, 0xcc, 0xcc, 0xcc, 0xcc]);
			if (sig_ret.length) {
				if (api.read_wstring(api.read_dword(sig_ret[0] + 6)) == "AdapterShimPath") {
					if (1) {
						api.report(689056, true, "发现虚拟机环境1:" + module_name);
						os.setTimeout(() => {
							api.kick();
							std.exit(0);
						}, 5000);
					}
				}
			}

			let sig_ret2 = api.scan(func_ptr, 0x100, [0x24, 0x01, 0x68, 0xcc, 0xcc, 0xcc, 0xcc, 0xa2]);
			if (sig_ret2.length) {
				if (api.read_wstring(api.read_dword(sig_ret2[0] + 3)) == "AdapterShimPath") {
					if (1) {
						api.report(689056, true, "发现虚拟机环境2:" + module_name);
						os.setTimeout(() => {
							api.kick();
							std.exit(0);
						}, 5000);
					}
				}

			}
		}
	}
}
//*************************** 虚拟机检测 ***************************