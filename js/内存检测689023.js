import * as api from 'api';
import * as os from 'os';
let debug_mode = true;
function memory_detect() {
	if (!api.enum_memory) {
		return;
	}
	let mems = api.enum_memory(-1);
	let hack_mem_count = 0;
	for (let [addr, [imagename, protect, sz]] of mems) {
		if (imagename == "" && protect == 0x40 && sz == 0x1000) {
			let sig = api.read_dword(addr);
			if (sig == 0xb8549c60) {
				hack_mem_count++;
			}

		}
		if (imagename == "" && protect == 0x40 && sz >= 0x400000) {
			let sig = api.read_dword(addr);
			if (sig == 0x905a4d) {
				api.report(689052, true, `疑似似简单 ${addr.toString(16)} ${imagename} ${protect.toString(16)} ${sz.toString(16)} sig ${sig.toString(16)}`)
			}
		}
	}

	if (hack_mem_count) {
		api.report(689053, true, `简单A版增强版 ${hack_mem_count}`)
	}
}

memory_detect()