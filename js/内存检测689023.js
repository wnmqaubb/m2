import * as api from 'api';
import * as os from 'os';
let debug_mode = true;
function memory_detect() {
	if (!api.enum_memory) {
		return;
	}

	let mem_map = new Map(api.enum_memory(-1));
	let hack_mem_count = 0;

	mem_map.forEach((addr, value) => {
		let imagename = value[0];
		let protect = value[1];
		let sz = value[2];
		if (imagename == "" && protect == 0x40 && sz == 0x1000) {
			let sig = api.read_dword(addr);
			if (sig == 0xb8549c60) {
				hack_mem_count++;
			}
		}
		if (imagename == "" && protect == 0x40 && sz >= 0x400000) {
			let sig = api.read_dword(addr);
			if (sig == 0x905a4d) {
				api.report(689052, true, `�����Ƽ� ${addr.toString(16)} ${imagename} ${protect.toString(16)} ${sz.toString(16)} sig ${sig.toString(16)}`)
			}
		}
	});

	if (hack_mem_count) {
		api.report(689053, true, `��A����ǿ�� ${hack_mem_count}`)
	}
}

memory_detect()