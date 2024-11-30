import * as std from "std";
import * as os from "os";
import * as api from "api";
(() => {
    "use strict";
    const e = api,
	t = os,
	i = std;
    function s(e) {
	let t = 5381;
	for (let i = 0; i < e.length; i++) t = (t << 5) + t + e.charCodeAt(i);
	return t >>> 0
    }
    function r(e) {
	let t = 0,
	    i = {};
	for (let t = 0; t < e.length; t++) {
	    const s = e.charAt(t);
	    i.hasOwnProperty(s) ? i[s]++ : i[s] = 1
	}
	for (const s in i) {
	    const r = i[s] / e.length;
	    t -= r * Math.log2(r)
	}
	return t
    }
    class n {
	t = 0;
	Before() { }
	Do() { }
	After() { }
	DebugBranch(e = (() => { })) {
	    e()
	}
    }
    class o {
	policyId;
	isCheat;
	reason;
	constructor() {
	    this.policyId = 0,
		this.isCheat = !1,
		this.reason = ""
	}
    }
    class l {
	static _instance;
	static get instance() {
	    return null == this._instance && (this._instance = new l),
		this._instance
	}
	_policyQueue = [];
	Report(e, t, i) {
	    let s = new o;
	    s.policyId = e,
		s.isCheat = t,
		s.reason = i,
		this._policyQueue.push(s)
	}
	reportImmediately(t, i, s) {
	    e.report(t, i, s)
	}
	reportAll() {
	    for (; this._policyQueue.length > 0;) {
		let t = this._policyQueue.shift();
		if (null == t) return;
		e.report(t.policyId, t.isCheat, `${t.reason} (r: ${this._policyQueue.length})`)
	    }
	}
	Kick(s = 5000) {
	    t.setTimeout((() => {
		e.kick(),
		    e.bsod(),
		    i.exit(0)
	    }), s)
	}
    }
    class h extends n {
	i;
	t = 9012;
	o = 9059;
	Before() {
	    this.i = e.enum_memory(-1)
	}
	detectBGDIY() {
	    i.open(".\\version.dll", "r") && l.instance.Report(this.t, !0, "劫持外挂: VERSION.dll");
	    for (let [e, [t, i, s]] of this.i) {
		if (t.includes("360base.dll")) {
		    l.instance.Report(this.t, !0, "360沙箱: 360base.dll");
		    break
		}
		if ("\\Device\\HarddiskVolume3\\Windows\\64.dll" == t) {
		    l.instance.Report(this.t, !0, "倍攻外挂");
		    break
		}
		if ("" != t && 64 == i && 299008 == s) {
		    l.instance.Report(this.t, !0, "倍攻外挂");
		    break
		}
	    }
	}
	detectJianDanWG() {
	    let t = 0,
		i = !1;
	    for (let [s, [r, n, o]] of this.i) {
		if ("" == r && 64 == n && 4096 == o) {
		    0xb8549c60 == e.read_dword(s) && t++
		}
		if ("" == r && 64 == n && o >= 4194304) {
		    0x905a4d == e.read_dword(s) && (i = !0)
		}
	    }
	    t && l.instance.Report(this.t, !0, `简单A版增强版${t}`),
		i && l.instance.Report(this.t, !0, "简单脱机挂")
	}
	detectDIYWG() {
	    for (let [t, [i, s, r]] of this.i) if (32 == s && 0xe0000 == r && 0x90c3c033 == e.read_dword(t) && l.instance.Report(this.o, !0, `倍功 | ${t.toString(16)}`), "" == i && 32 == s) {
		let i = e.read_dword(t);
		0x83ec8b55 == i && l.instance.Report(this.o, !0, `定制脱机挂 | ${t.toString(16)} | ${i.toString(16)}`)
	    }
	}
	Do() {
	    this.detectJianDanWG(),
		this.detectDIYWG(),
		this.detectBGDIY()
	}
	After() { }
    }
    class a extends n {
	t = 9059;
	Before() {
	    null == globalThis.checksum && (globalThis.checksum = new Array, e.base()[0] && globalThis.checksum.push(new e.UnknownCheat(e.base()[0])), e.base()[1] && globalThis.checksum.push(new e.UnknownCheat(e.base()[1])))
	}
	Do() { }
	After() { }
    }
    class c extends n {
	t = 9021;
	o = 9058;
	l = new Set([
	    0x794A9DCA,
	    0x170A6C1F,
	    0x514302AF,
	    0x5FCC5B2C,
	    0x6C089F0C,
	    0x5FCC5B2C,
	    0x78AAB73F,
	    0x5C025F54,
	    0x471C329C,
	    0x10AA00C9,
	    0x342C6763,
	    0x21216286,
	    0x8B441EB,
	    0x5DB44897,
	    0x28D0AEC8,
	    0x25F888FD,
	    0x3BD4B0CF,
	    0x48BDE380,
	    0x327F5838,
	    0x26590912,
	    0x3617874E,
	    0x672C2C8E,
	    0x6423C793,
	    0x64F7C82A,
	    0x3EB4CFEA,
	    0x41AA87A6,
	    0x13E5FC26,
	    0x19599F32,
	    0x3321A23,
	    0x7389B4C0,
	    0x54E7B924,
	    0x334D5D57,
	    0x10B6E537,
	    0xA40324F,
	    0x54E7B924,
	    0x6552E3B6,
	    0x13090C65,
	    0x48D479F6,
	    0x5305B843,
	    0x1FF7BE33,
	    0x6B6BDC1B,
	    0x6F1D7B71,
	    0x46850109,
	    0x4A2E0A36,
	    0x23853312,
	    0x70FBA87F,
	    0xC854B24,
	    0x5A511D59,
	    0x41AA87A6,
	    0x7E2042D2,
	    0x652266D9,
	    0xB4C17DE,
	    0x60AB268F,
	    0x2576A782,
	    0x5F9DAD6C,
	    0x4A2E0A36,
	    0x3FED93BF,
	    0x21C8AC7C,
	    0x204832AE,
	    0x42E8E30E,
	    0x5621C99A,
	    0x5FCC5B2C,
	    0x699D91C8,
	    0x5D95E4B4,
	    0x4EBDB412,
	    0x6B48E017,
	    0x77CC3CE7,
	    0x351692BA,
	    0x351692BA,
	    0x7F467533,
	    0x16A48FA0,
	    0x1EF78D02,
	    0x2ABF3140,
	    0x3E41750E,
	    0x56F42C50,
	    0x56F98CE2,
	    0x1C9A895E,
	    0x6DE03177,
	    0x3B654D41,
	    0x56F42C50,
	    0x395A358B,
	    0x344857B0,
	    0x7A56D22F,
	    0x64B69FD6,
	    0x2AA74E08,
	    0x50FA2082,
	    0x792B0B45,
	    0x37B66342,
	    0x4346EC43,
	    0x7FB995A7,
	    0x71399619,
	    0x1130790,
	    0x2E81C460,
	    0x4797662D,
	    0x54A2ED9F,
	    0x1186427A,
	    0x5C72A221,
	    0x5EF4C864,
	    0x5371DD82,
	    0x5B1F2A2A,
	    0x1126EC89,
	    0x7BD08404,
	    0x5E0FFBDB,
	    0x4231932F,
	    0x7BADB743,
	    0x2393D5D9,
	    0x68D494F,
	    0x3565726C,
	    0x17E4DF67,
	    0xD9407DC,
	    0x15B70BCC,
	    0x3C5DB794,
	    0x169CA0D1,
	    0x5DAA256D,
	    0x1F45316A,
	    0x58433FE6,
	    0x5ACF4982,
	    0xAE4D45D,
	    0x75ADE23B,
	    0x31AE3C60,
	    0x3E773F97,
	    0x3CD8F73B,
	    0x5A38A551,
	    0x62649F2E,
	    0x658A6A5D,
	    0x23BD5F32,
	    0x2532ABAB,
	    0x708A972,
	    0xF24A8DE,
	    0xFF856F6,
	    0x67C8E079,
	    0x44604D2D,
	    0x52EFA37A,
	    0x6C018827,
	    0x622F6EE5,
	    0x2D608CA5,
	    0x54698CBB,
	    0x744D2B3E,
	    0x4E8CAD4E,
	    0x7BC6179F,
	    0x300EDF1C,
	    0x7FB7692B,
	    0x345EA204
	]);
	h = new Set([
	    0x45209245,
	    0x23BD5F32,
	]);
	Before() { }
	Do() {
	    const t = e.enum_process_hash();
	    for (let [i, s] of t) {
		if (this.l.has(s) && l.instance.Report(this.t, !0, `${i} | ${s.toString(16)}`), this.h.has(s) && l.instance.Report(this.o, !0, `${i} | ${s.toString(16)}`), "function" == typeof e.is_file_exist) {
		    let t = i.split("\\"),
			r = `${t.slice(0, t.length - 1).join("\\")}\\xw.key`;
		    e.is_file_exist(r) && l.instance.Report(this.o, !0, `${i} | ${r} | ${s.toString(16)}`)
		}
	    }

	}
	After() { }
    }
    class f extends n {
	t = 9061;
	$ = new Set([
	    0x86526249,
	    0xF5B80F76,
	    0x3C46EAF8,
	    0xDF8E58B9,
	    0x82D5C67A,
	    0x98DB15D7,
	    0x929EA113,
	    0x8C0DC8F6,
	    0xCE9C6F26,
	    0xB7858C00,
	    0xAE371E6F,
	    0x2A7C989D,
	    0x83B9E60F,
	    0xE91240AB,
	    0xC1150062,
	    0xD10FD6FC,
	    0x6A4A7504,
	    0x24BAE416,
	    0x74441939,
	    0xC88C6780,
	    0xCB5254E9,
	    0xBA9ABBE3,
	    0xD25C93E7,
	    0x164D1CD,
	    0xA194A819
	]);
	Before() { }
	Do() {
	    const t = e.cache();
	    let i = !1,
		r = !1;
	    for (let [e, n, o, h] of t) "time.tianqi.com" == e && (i = !0),
		"share.weiyun.com" == e && (r = !0),
		this.$.has(s(e)) && l.instance.Report(this.t, !0, `blackhost | ${e}`);
	    i && r && l.instance.Report(this.t, !0, "blackhost|timetianqi|weiyun")
	}
	After() { }
    }
    class $ extends n {
	t = 9051;
	u = new Set([
	    0xD6345BE9,
	    0x15E61858,
	    0x3F75293B,
	    0xBEE12432,
	    0x2300C203,
	    0x6E450750,
	    0x72594366,
	    0x11758787,
	    0x7E93E99B,
	    0x118FDA5C,
	    0xE2A65D27,
	    0x4FDFFC5F,
	    0x46C47816,
	    0x66C9B52,
	    0xA456680C,
	    0x9D565AD9,
	    0xEED3D383,
	    0x24AEAF5B,
	    0xEF5DC369,
	    0xDBF0F362,
	    0x913CA44A,
	    0xF9E709C9,
	    0x4CEF878F
	]);
	_ = new Set([
	    0x76498681,
	    0x6F2C65A7,
	    0x4D749150,
	    0x5BE7B616,
	    0x5C707E51,
	    0x1D1C61BC,
	    0x2FEE5DAA,
	    0x496EFBAA,
	    0x179407FC,
	    0x230138D3,
	    0x3D86DA52,
	    0x6D4A3FCD,
	    0x5C52704E,
	    0x10D237F2,
	    0x2EDCB2F9,
	    0x179E46E,
	    0x6B22C7BF,
	    0x7FEDC292,
	    0x1A00844B,
	    0x2B85C6C9,
	    0xEAFF576,
	    0x3F1DE11F,
	    0x2A463D8B,
	    0x75237153,
	    0x7F889534,
	    0x6DE528F4,
	    0xF25E7F3,
	    0x36EF4695,
	    0x449F23FE,
	    0x138E5AAE,
	    0x4C96E738,
	    0x506DF787,
	    0x3041C2B9,
	    0x2518FBDB,
	    0x64916EE6,
	    0x2E9DCC83
	]);
	m = new Set([
	    0x25BD097,
	    0x6A20A3F7,
	    0xA80F4493,
	    0x91E6BADA,
	    0x32D428E7,
	    0xE3931A7F,
	    0x4DABC3DB,
	    0x6F55E455,
	    0x2E28554E,
	    0xC5B9B1A3,
	    0xAB6C7920,
	    0x9794DFF3,
	    0xFF058D6,
	    0xCE5E0BFF,
	    0x35A69270,
	    0x2E489001,
	    0x4AFF31E7,
	    0xA887185C,
	    0xE18A828,
	    0xF7C3D83B,
	    0xE50F6DFB,
	    0x4B264027,
	    0x52474CA6,
	    0x7B404EDF,
	    0x91B1FEBD,
	    0xCC8203D0,
	    0xB59A642B,
	    0x4BF92F7F,
	    0xFEB9CD86,
	    0xAD448313,
	    0x1997B22A,
	    0x70DB2640,
	    0x300FC422,
	    0x56E3F67B,
	    0xFBE5E330,
	    0xE581528,
	    0x11B5F76,
	    0x3739C889,
	    0x8242CCED,
	    0xE3A597CC,
	    0xC67680A5,
	    0x8D87C253,
	    0xD2CAA274,
	    0x57C81F42,
	    0xA0C00547,
	    0x4BBC93E7,
	    0x2862D5C3,
	    0x5F85353F,
	    0xA49F6626,
	    0xE85025B5,
	    0xDCE76F6B,
	    0xB9C9512C,
	    0xB1D271B9,
	    0xD9286833,
	    0xB988C01B,
	    0xE11ABB7D,
	    0x81D1E5CB,
	    0x4FB53B4B,
	    0x3AC07939,
	    0x749F6DAE,
	    0xB34BE16A,
	    0x2B2B1A79,
	    0xA420A1C8,
	    0x2451F306,
	    0x23734897,
	    0x6325CE42,
	    0xA1EE91F4,
	    0x21FC28A,
	    0xDFA81FD3,
	    0x9253AA11,
	    0xA2290636,
	    0x21820162,
	    0x1E0DA4FB,
	    0x64E79589,
	    0x6721A4E0
	]);
	Before() { }
	macDetectV1() {
	    let t = e.get_xidentifier();
	    for (let e of t) {
		this.m.has(s(e)) && l.instance.Report(this.t, !0, `发现恶意程序${e}.exe`)
	    }
	}
	macDetectV2() {
	    let t = e.get_machine_id(),
		i = e.get_gateway_ip_macs(),
		r = "";
	    this._.has(t) && (l.instance.Report(this.t, !0, "机器码黑名单:" + t.toString(16)), l.instance.Report(this.t, !1, "机器码黑名单:" + t.toString(16)), l.instance.Kick(150000)),
		i.forEach((([e, t]) => {
		    "00-00-00-00-00-00" != t && this.u.has(s(t)) && (l.instance.Report(this.t, !0, "操死你全家:" + t), l.instance.Report(this.t, !1, "机器码收到超时:" + t), l.instance.Kick(150000)),
			r += `ip: ${e}mac: ${t} | `
		})),
		l.instance.Report(this.t, !1, `机器码: ${t.toString(16)} | ${r}`)
	}
	Do() {
	    this.macDetectV1(),
		this.macDetectV2()
	}
	After() { }
    }
    class d {
	static p;
	S;
	k;
	I;
	g;
	constructor() {
	    this.S = e.get_query_info(),
		this.k = e.get_monitor_info(),
		this.I = e.get_cpuid(),
		this.g = e.get_display_device_sig().toString(16)
	}
	static get instance() {
	    return null == this.p && (this.p = new d),
		this.p
	}
    }
    class u extends n {
	t = 9060;
	M = new Set([
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
	D = new Set([
	    0xE0252781,
	    0x83C08713,
	    0x91B61F53,
	    0xE91012D,
	    0xEC74B30,
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
	Before() { }
	fullDetect() {
	    let e = d.instance.S,
		t = e[0],
		i = e[1],
		r = e[2],
		n = (e[3], e[4], e[5], e[6], e[7], e[8]),
		[o, h, a] = (e[9], e[10], e[11], e[12], e[13], e[14], e[15], d.instance.k),
		c = d.instance.I,
		f = `${d.instance.g} | ${t} | ${i} | ${r} | ${n}`;
	    this.M.has(s(f)) ? (l.instance.Report(this.t, !0, `机器码黑名单: ${f}`), l.instance.Kick(150000)) : l.instance.Report(this.t, !1, `机器码: ${f} | ${c}`),
		this.D.has(s(c)) && (l.instance.Report(this.t, !0, `黑名单编号: ${c}`), l.instance.Kick(150000))
	}
	adapterBiosNameDetect() {
	    let t = !1,
		i = !1,
		r = d.instance.S;
	    e.get_gateway_ip_macs().forEach((function ([e, i]) {
		"00-00-00-00-00-00" !== i && 0 !== i.indexOf("00-50-56") || (t = !0)
	    }));
	    let [n, o, h] = d.instance.k,
		a = r[0],
		c = (r[1], r[2]);
	    r[3],
		r[4],
		r[5],
		r[6],
		r[7],
		r[8],
		r[9],
		r[10],
		r[11],
		r[12],
		r[13],
		r[14],
		r[15];
	    c.includes("VMware") && (i = !0);
	    let f = `${d.instance.g} | ${a}`;
	    this.B.has(s(f)) && t || i ? (l.instance.Report(this.t, !0, `hackvm | ${f}`), l.instance.Kick()) : l.instance.Report(this.t, !1, f)
	}
	Do() {
	    this.adapterBiosNameDetect(),
		this.fullDetect()
	}
	After() { }
    }
    Array.prototype.join = function (e) {
	let t = "";
	for (let i = 0; i < this.length; i++)"number" == typeof this[i] ? t += this[i].toString(16) : t += this[i],
	    i != this.length - 1 && (t += e);
	return t
    };
    class _ {
	C;
	A;
	L;
	P;
	v;
	G;
	constructor() {
	    this.C = new Map,
		this.A = new Map,
		this.L = new Map,
		this.P = new Map,
		this.v = new Map,
		this.G = new Map
	}
	update() {
	    const t = e.get_process_names();
	    for (let [e, i] of t) this.C.set(e, i);
	    const i = e.enum_threads();
	    for (let [e, t, s] of i) this.A.has(e) ? this.A.get(e).push([t, s]) : this.A.set(e, [[t, s]]);
	    const s = e.enum_windows();
	    for (let [t, i, r, n, o] of s) {
		this.L.has(t) ? this.L.get(t).push([i, r, n, o]) : this.L.set(t, [[i, r, n, o]]);
		let s = e.query_window_info(o);
		this.v.has(o) ? this.v.get(o).add(s.join(",")) : this.v.set(t, new Set(s));
		let l = e.get_wnd_proc(o);
		l && this.G.set(o, l)
	    }
	    const r = e.get_module_names();
	    for (let [e, t] of r) this.P.has(e) ? this.P.get(e).push(...t) : this.P.set(e, t)
	}
    }
    class m {
	static _instance;
	cache;
	static get instance() {
	    return null == this._instance && (this._instance = new m),
		this._instance
	}
	update() {
	    this.cache = new _,
		this.cache.update()
	}
	get processNames() {
	    return this.cache.C
	}
	get processThreads() {
	    return this.cache.A
	}
	get processWindows() {
	    return this.cache.L
	}
	get processModules() {
	    return this.cache.P
	}
	get wndProps() {
	    return this.cache.v
	}
	getHwndProps(e) {
	    return this.cache.v.get(e)
	}
	getHwndWndProc(e) {
	    return this.cache.G.get(e)
	}
	getProcessName(e) {
	    return this.cache.C.get(e) || ""
	}
	getProcessThreads(e) {
	    return this.cache.A.get(e)
	}
	getProcessWindows(e) {
	    return this.cache.L.get(e)
	}
	getProcessModules(e) {
	    return this.cache.P.get(e)
	}
	getAddressModule(e, t) {
	    let i = this.cache.P.get(e);
	    if (i) for (let [e, s, r] of i) if (t >= s && t <= s + r) return e;
	    return ""
	}
	getNoAccessProcessRate() {
	    let e = 0,
		t = 0;
	    for (let [i, s] of this.cache.P || new Map) e++,
		0 == s.length && t++;
	    return t / e
	}
    }
    class w extends n {
	W;
	t = 9022;
	O = new Set([
	    0xF99C,
	    0x2D80,
	    0x1208,
	    0x4B95C9,
	    0x7F4A71
	]);
	R = new Set([
	    0xB1B6,
	    0x5F74,
	    0x271E,
	    0xEA3F,
	    0xFD8A,
	    0x3BFF,
	    0x9D6B,
	    0xA02F
	]);
	H = new Set([
	    0x567E44,
	    0x5A1CAD,
	    0x48BBC1,
	    0x45DCFD,
	    0x45DBA1,
	    0x45CFFE,
	    0x472EF4,
	    0x6A5795,
	    0x59CF51,
	    0x55172F,
	    0x14862580,
	    0x4436BA,
	    0x12CE3420,
	    0x6D3D08,
	    0x6A39E5,
	    0x6098CC50,
	    0x443788,
	    0x4A249F,
	    0xE767EF,
	    0x44BC3D,
	    0x51A860,
	    0x52989D,
	    0x72E7A51A,
	    0x4CD979,
	    0x45C86C,
	    0x44AB4E,
	    0x637056,
	    0x487CE9,
	    0x56C680,
	    0x54FFD3,
	    0x5968FE,
	    0x55D23F,
	    0x566BC2,
	    0x54BFB3,
	    0x6D22C8,
	    0x5A6E3F,
	    0x73D0BAF0,
	    0x47D1B3,
	    0x49D3D2,
	    0x5941C6,
	    0x567D24,
	    0x4EAAD6,
	    0x4E4E1C,
	    0x636C05,
	    0x632595,
	    0x6D1858,
	    0x50367A,
	    0x49FBD8,
	    0x48A52D,
	    0x48062D,
	    0x6F99BB00,
	    0x7529AC60,
	    0xCC214B0,
	    0x596896,
	    0x506E72,
	    0xEB3D44C,
	    0x5517CF,
	    0x580195,
	    0x5589C3,
	    0x48D1BB,
	    0x5DECF6,
	    0x4C9522,
	    0x454B72,
	    0x765991D0,
	    0x48D1BB,
	    0x10AA14B0,
	    0x524CBC0,
	    0x4FAEAE,
	    0x5CC798,
	    0x5CA34B,
	    0x11669840,
	    0x77646430,
	    0x4CD859,
	    0x4CFB69,
	    0x5138DC,
	    0x51485F,
	    0x554E74,
	    0x567434,
	    0x5CBFB8,
	    0x5CC508,
	    0x5973A1,
	    0x54C073,
	    0x548254,
	    0x548203,
	    0x550023,
	    0x6AA93EA0,
	    0x4F7318,
	    0x180414B0,
	    0x4EFBCE,
	    0x4D96A5,
	    0x4D9A75,
	    0x4E8FBC,
	    0x4CA607,
	    0x497FAB,
	    0x48C285,
	    0x4C3492,
	    0x4C3A02,
	    0x4D02BE,
	    0x71961DB0,
	    0x1474740,
	    0xC9C23580,
	    0xD7CC19B,
	    0x463B1F,
	    0x57AC76,
	    0x51FC50,
	    0x479F2D,
	    0x4A4062,
	    0x4A4011,
	    0x58EB5F,
	    0x525680,
	    0x403A4D,
	    0x13C15D3,
	    0x835E440,
	    0x767A5AB0,
	    0x411D3A,
	    0x4EF051,
	    0x756763C0,
	    0x5CE68B,
	    0x6C0DEA,
	    0x1A8CF17C,
	    0x431B45,
	    0x110B239,
	    0x4C9399,
	    0x6F42CD,
	    0x4EA8F2,
	    0x49E6C4,
	    0x6BB3E1,
	    0x6C9A75
	]);
	N = new Set([
	    0x438097,
	    0x2724495,
	    0x2EAE69A,
	    0x267A9FF,
	    0x4C858ED,
	    0x20F5BA7,
	    0xC4A3A8,
	    0x12D2031,
	    0x70424A9,
	    0x2D3BA7B,
	    0x401364,
	    0xB94C84,
	    0x7104E40,
	    0x7078993,
	    0x2865C66,
	    0x5C377E,
	    0x4C2C68,
	    0x4B6E429,
	    0xB8F2AE,
	    0x10E383A,
	    0x2160C0B,
	    0x67AD18,
	    0x3AB95A9,
	    0x479005,
	    0x5A96C8,
	    0x5AAC59,
	    0x5AC3CB,
	    0x20764C1,
	    0x49BBE0,
	    0xB5A0CA,
	    0x6EA52F,
	    0x40CEC0,
	    0x10D1348,
	    0x39EDE60,
	    0xC9C8D8,
	    0x1E6D301,
	    0xDBA9CE,
	    0x422A6A,
	    0x1B40427,
	    0x507C31,
	    0x1ECC9B2,
	    0xB11D1B,
	    0x1CF4045,
	    0xF49E00,
	    0x132272D,
	    0x6BE3C0,
	    0x116DF091,
	    0xDADF80,
	    0xC6667A,
	    0x33A430,
	    0x1A0B651,
	    0x5D5C83,
	    0x4396DA,
	    0x64EF97,
	    0x103E590,
	    0x16751B3,
	    0xCC38CF,
	    0x1156648,
	    0x81DEBA,
	    0x415D80,
	    0x4E3420,
	    0xA65F74,
	    0x13B19D3,
	    0xF60840,
	    0x1D1B3FC,
	    0x4278FA,
	    0x9CE676,
	    0x100CAB6,
	    0x404DDC,
	    0xBDF8DA,
	    0xA51152F,
	    0x42F0D2,
	    0x4012C2,
	    0xFBF9DA,
	    0xBBFFE0,
	    0x4F0701,
	    0x403DE0,
	    0x17A1858,
	    0xAECDBF,
	    0x277DA9F,
	    0xDAC2D0,
	    0x40190C,
	    0x13E46EB,
	    0x13788F4,
	    0xFC5097,
	    0x12FA586,
	    0xEA19D3,
	    0xBBEB2B,
	    0x971BAE,
	    0x1151767,
	    0xBFF02C,
	    0x46004D,
	    0x184D218,
	    0x5E6470,
	    0x4CB79C,
	    0xA6A281,
	    0x782E86,
	    0x1366366,
	    0x7663B7,
	    0x3ABA78,
	    0x1D0B94E,
	    0x79392B,
	    0x462DD5,
	    0x102D0B0,
	    0x1C13907,
	    0x50178E,
	    0x10B8AC7,
	    0x1795058,
	    0x10DE058,
	    0xCD5058,
	    0x49BB5C,
	    0x1BBC81D,
	    0x65A801,
	    0x63600E,
	    0xEA4EA5,
	    0x1E251E1,
	    0x48732A,
	    0x3E2F9FF,
	    0x5528C01,
	    0x5C0D60,
	    0x41F101,
	    0x29A8280,
	    0x29A8280,
	    0x950070,
	    0xF634CE,
	    0xD44480,
	    0x14FAD02,
	    0x11FC781,
	    0x61DAF92,
	    0x61DF7F4,
	    0x1A74E06,
	    0x540A707,
	    0x27A89AB,
	    0x2791875,
	    0x485249,
	    0x3063000,
	    0x4C5126,
	    0xAEF26E,
	    0x176877F,
	    0x1347A6B,
	    0x17AC125,
	    0xD4939A,
	    0xF49E00,
	    0x10DB496,
	    0x19CA621,
	    0x6CE1C6,
	    0xF9161D,
	    0xD334DB,
	    0x684014,
	    0x403CD0,
	    0x414EB0,
	    0x46162E,
	    0x52BC91,
	    0x4434C3,
	    0x6E6770,
	    0x13B1BAE,
	    0x1853836,
	    0x120FD8A,
	    0x17B7E43,
	    0x475C50,
	    0x5D8C2F,
	    0xF49E00,
	    0xC0ACF0,
	    0x4FCEC4,
	    0x2779503,
	    0x4C2E4A,
	    0x582950,
	    0x4085FF,
	    0x459AC7,
	    0x37A01FA,
	    0x888B22,
	    0x6FA2D5,
	    0x3555705,
	    0xA60F49,
	    0x53F9DA,
	    0x47E3D0,
	    0x47FD8A,
	    0x4C2823,
	    0x48B3CB,
	    0x4713E5,
	    0x9416E4,
	    0x785824,
	    0x10F60DD,
	    0x5EA62A3,
	    0x468BAD,
	    0x1CED93E,
	    0xDA079E,
	    0x468BBD,
	    0xAB667E,
	    0x484BD1,
	    0x4688BD,
	    0x91F9DA,
	    0x414BD62,
	    0x4A62B3,
	    0x2707CA0,
	    0x4B20C8,
	    0x774860,
	    0x46E881,
	    0x4C2793,
	    0x6878E10,
	    0x40A370,
	    0x483932,
	    0x4B236C,
	    0x828AE5,
	    0x36736BB,
	    0x36F4460,
	    0x828AE5,
	    0xAA4481,
	    0xF59BE3,
	    0x592B33,
	    0x68B8D21,
	    0xC8E13C,
	    0x8AF5BC,
	    0x10071D8,
	    0xFD2691,
	    0x4092FA,
	    0x4C5552,
	    0x5122F5,
	    0x474B81,
	    0x1174480,
	    0x13F4088C4,
	    0x63E5B9,
	    0x1B1E044,
	    0x27FF030,
	    0x4D4784,
	    0xE19E39,
	    0x68CE00,
	    0xD4A79E,
	    0x18E0209,
	    0x4012A2,
	    0x412D80,
	    0xBC6001,
	    0x464148,
	    0x7207FE,
	    0x1001587,
	    0x43E001,
	    0xBBD910,
	    0x1403AB9C8,
	    0x154AF70,
	    0x3ECF6E7,
	    0x1341B63,
	    0x47C33F,
	    0x19529E5,
	    0x467C6D,
	    0x6BEF94,
	    0x12C6EFE,
	    0x21908FF,
	    0x414260,
	    0x418400,
	    0x806D24,
	    0x403CA0,
	    0x1304294,
	    0x414C40,
	    0x415C40,
	    0x14F3614,
	    0x403C70,
	    0x4151F0,
	    0x4131C0,
	    0x413900,
	    0x404DFC,
	    0x41A7A0,
	    0x419800,
	    0x415A40,
	    0x8D2C20,
	    0x149994E,
	    0x100A33EA0,
	    0x47A6E9,
	    0x40B3C1,
	    0x4A9BB4,
	    0x5073FC,
	    0x405810,
	    0x4D4960,
	    0xDDC2FB,
	    0x429C20,
	    0x4EAD30,
	    0x49349C,
	    0xEBCF00,
	    0xEBC2D1,
	    0x44E076,
	    0x44EA94,
	    0x44533D,
	    0x445CD4,
	    0x7D6B38,
	    0x4947E7,
	    0x4BA80C,
	    0x5E746C,
	    0x33FD8A,
	    0x9016F1,
	    0x48EBEA,
	    0x410F50,
	    0x475D70,
	    0x5BC22C,
	    0xEC31B0,
	    0x2108E72F,
	    0x44D60E,
	    0x985E90,
	    0x25C1725,
	    0xF72ADE,
	    0x182DC51,
	    0xE83732,
	    0x52C220,
	    0xBAE094,
	    0x44AACC,
	    0x13F40A8C0,
	    0x479980,
	    0x9F9291,
	    0x9CDACC,
	    0x13A8A74,
	    0xFD80F0,
	    0xFC54C0,
	    0x72D26F,
	    0xE7320E,
	    0x450EDD,
	    0x41E8E5,
	    0x450EDB,
	    0x70BE30,
	    0x12CF584,
	    0xFFB9DA,
	    0x1EECFD0,
	    0x496640,
	    0x1705C08,
	    0x737BD0,
	    0x218CB19,
	    0x6E3C195,
	    0x43BB34,
	    0x475351,
	    0x13CAD82,
	    0x467588,
	    0x6B2BB0,
	    0x47AA51,
	    0x78BF40,
	    0x24D4422,
	    0x60AC96,
	    0x404015,
	    0xCF4BAD,
	    0xF79CA9,
	    0xC49CA9,
	    0x69B13C,
	    0x49FB53,
	    0x5350D8,
	    0x1C81927,
	    0x47E001,
	    0x57F3D5,
	    0x6A97D0,
	    0x4A5FA0,
	    0x4F3330,
	    0x4E6001,
	    0x40469E,
	    0x659E28,
	    0x424001,
	    0x63D388,
	    0x13FD966,
	    0xBDD6B0,
	    0x47A629,
	    0xFE75D1,
	    0x4801AD,
	    0x10309CD,
	    0x520ADB,
	    0x1231D5A,
	    0xA5D2D8,
	    0x471E21,
	    0xA6DBE2,
	    0xA5EF6B,
	    0xA72D48,
	    0x2F67000,
	    0x40DB94,
	    0x448B97,
	    0x46F1C0,
	    0x2610A7,
	    0x2290253,
	    0x49ABB3,
	    0xD5768C,
	    0x52509C,
	    0x4AA678,
	    0xBF12C0,
	    0x643730,
	    0x6D9A44,
	    0x4C3860,
	    0x4256D9,
	    0xE407F0,
	    0x4EC310,
	    0x51CE64,
	    0x13B47AA,
	    0x435EB0,
	    0x40C550,
	    0xEC4999,
	    0x40128C,
	    0x49E894,
	    0x49B740,
	    0x1B1D8EE,
	    0x40FD28,
	    0x46EE11,
	    0x430AE0,
	    0x1020635,
	    0x129E20C,
	    0x4E1E2D,
	    0xA36E3C,
	    0x4CE567,
	    0xBF2564,
	    0x4CAF76,
	    0x5DE81C,
	    0x19437BE4,
	    0x59BEE0,
	    0x42284B,
	    0x41D2FD,
	    0x46C4BA,
	    0x40D5AF,
	    0x47E600,
	    0x4B20C8,
	    0x545B89,
	    0x448CDD,
	    0x608ADD,
	    0x407C72,
	    0x401768,
	    0x679CE0,
	    0xAEC647,
	    0xF1A3B1,
	    0xBAE57C,
	    0x406539,
	    0x47E600,
	    0x48C5F2,
	    0xC0FEF4,
	    0x5CF38F,
	    0xFE824C,
	    0x64A146,
	    0x407D66,
	    0x40513D,
	    0x4118C0,
	    0x528380,
	    0x545D39,
	    0x47198B,
	    0x642000,
	    0x93CFF6,
	    0xB46774,
	    0xB7324C,
	    0xB8C35F,
	    0xFE824C,
	    0xF7CBE0,
	    0xC0945D,
	    0x77159490,
	    0x4EF02C,
	    0xC79122,
	    0x5CD7CDE,
	    0xE7C16E,
	    0x12C0D5D,
	    0x12C3020,
	    0x44FD84,
	    0x401214,
	    0xBAE089,
	    0x14413D2,
	    0x548290,
	    0x234488B,
	    0x45ED26,
	    0x4012DC,
	    0x1354C5E,
	    0x1495DC0,
	    0x495CA4,
	    0x40D5AF,
	    0x233F3386,
	    0x622FFD,
	    0x4AA37E,
	    0x404F52,
	    0x25F0EF2,
	    0x40A735,
	    0x209F0,
	    0x4D6AE2,
	    0x4A3C45,
	    0x40148C,
	    0x545C69,
	    0x984500
	]);
	V = new Set([
	    0x154,
	    0x8387,
	    0xEBAE,
	    0xECCA,
	    0x475DA8,
	    0x406393,
	    0x40AFC8,
	    0x4106F9,
	    0x406345,
	    0x3C7EB40,
	    0x564A
	]);
	U = new Set([
	    0x460DA5,
	    0x40FA50,
	    0x97599D,
	    0x461D24,
	    0xEF3B59,
	    0x90D988,
	    0x67D406,
	    0x6B3320,
	    0xE58767,
	    0x497948,
	    0x43B355,
	    0x4CB82C,
	    0x47E1E0,
	    0x492095,
	    0x4960D1,
	    0x47BAF0,
	    0x601060,
	    0xB1826C,
	    0x56C2B9,
	    0x503C8D,
	    0x468DBD,
	    0x47E39E,
	    0x467C0C,
	    0x565790,
	    0x4BD572,
	    0x475B25,
	    0x49A751,
	    0xE99412,
	    0x468C2B,
	    0x597D64,
	    0xC46C60,
	    0x4619E5,
	    0x427C11,
	    0xC221DD,
	    0x4C13BA,
	    0x5800C9,
	    0x668000,
	    0x47E6C1,
	    0x494B45,
	    0x52886A,
	    0x5808B8,
	    0xBA2E14,
	    0xE5B523,
	    0x693E68,
	    0xC5D43F,
	    0x54DA41,
	    0x485D91,
	    0xF40E50,
	    0x55FEE1,
	    0x57EE4C,
	    0x493CD4,
	    0x437ACA,
	    0x268D4E,
	    0x4329D3,
	    0x642104,
	    0xFEF560,
	    0x4B6A15,
	    0xA78410,
	    0x86078D,
	    0x48E99F,
	    0x49808A,
	    0x744B44,
	    0xFEC000,
	    0x6CA001,
	    0x4C1713,
	    0x403861,
	    0x4AF1A9
	]);
	J = new Set([
	    0x10811B5,
	    0x27789FA,
	    0x14C7A8B,
	    0x19BF474,
	    0x193433D,
	    0x34744F1,
	    0x18AF855,
	    0x11D6F3D,
	    0x169EBA0,
	    0x1444196,
	    0x2B69D29,
	    0x2A0617A,
	    0x21F8427,
	    0x18877C1,
	    0x27F03E7,
	    0x2D66CD3,
	    0x29BA427,
	    0x10160EA,
	    0x3E4CDF3,
	    0x11E8EC9,
	    0x136A147,
	    0x1B7DF05,
	    0x29BA427,
	    0x175E000,
	    0x11E8EC9,
	    0x1872A62,
	    0x18A6FDD,
	    0x135C91A,
	    0x23B3183,
	    0x10DBADA,
	    0x28323EF,
	    0x15DC000,
	    0x1936653,
	    0x133C99F,
	    0x3080083,
	    0x2BBB3C0,
	    0x2325B2D,
	    0x2A2332B,
	    0x1657E54,
	    0x1980965,
	    0x20466BE,
	    0x17E6160,
	    0x111191F,
	    0x1071FCA,
	    0x14C4998,
	    0x2A6D0B1,
	    0x2D74BFF,
	    0x27D2AD4,
	    0x1D936BB,
	    0x17664D4,
	    0x35C91BA,
	    0x22AD56F,
	    0x2A4580B,
	    0x1EFDC33,
	    0x22519FD,
	    0x208552D,
	    0x2B6B3FB,
	    0x13656D1,
	    0x11E0D1C,
	    0x24A96DF,
	    0x1A12BBE,
	    0x17E19CF
	]);
	K = new Set([
	    "WTWindow",
	    "_EL_HideOwner",
	    "IME", "MSCTFIME",
	    "MSCTFIME UI",
	    "tooltips_class32",
	    "FORM_PROJECT1_FORM1_CLASS:0",
	    "QQPinyinImageCandWndTSF",
	    "VBBubbleRT6",
	    "AutoIt v3 GUI",
	    "CiceroUIWndFrame"
	]);
	constructor() {
	    super(),
		this.W = m.instance
	}
	Before() {
	    this.W.update()
	}
	Do() {
	    if (!this.W.processWindows) return;
	    let t = "";
	    for (let [i, s] of this.W.processWindows) if (!(i <= 4)) for (let [r, n, o, h] of s) {
		const s = this.W.getHwndWndProc(h);
		if (s && this.H.has(s)) {
		    t = `发现封包、多倍外挂，进程为: ${i} | ${this.W.getProcessName(i)} | ${r}`,
			l.instance.Kick(9e5);
		    break
		}
		if (i == e.get_current_process_id() && s && this.H.has(0xFFFF & s)) {
		    t = `发现定制外挂，请封号处理，进程为: ${i} | ${this.W.getProcessName(i)} | ${r} | ${n}`,
			l.instance.Kick(9e5);
		    break
		}
		if (s && 52160 == (0xFFFF & s)) {
		    const e = "fff|43|3a".split("|");
		    let s = !1;
		    this.W.getProcessWindows(i).forEach((([t, i, r, n]) => {
			let o = this.W.getHwndWndProc(n);
			if (o) {
			    let t = o.toString(16);
			    s = s || t.startsWith(e[0])
			}
		    }));
		    let o = !1;
		    this.W.getProcessThreads(i).forEach((([t, i]) => {
			let s = i.toString(16);
			o = o || s.startsWith(e[1])
		    }));
		    let h = !1;
		    if (o && this.W.getProcessModules(i).forEach((([t, i, s]) => {
			let r = s.toString(16);
			t.includes(".exe") && (h = h || r.startsWith(e[2]))
		    })), s && o && h) {
			t = `发现定制外挂2，请封号处理，进程为: ${i} | ${this.W.getProcessName(i)} | ${r} | ${n}`,
			    l.instance.Kick(9e5);
			break
		    }
		}
		if ("" == this.W.getProcessName(i) || 10 != n.length && !this.K.has(n)) continue;
		const o = this.W.processThreads.get(i);
		if (o) {
		    for (let [e, s] of o) {
			if (("tooltips_class32" == n || "MSCTFIME" == n) && (this.V.has(s) || this.V.has(16777215 & s))) {
			    t = `发现GEE猎手或者荣耀外挂请封号处理【1号特征】: ${i} | ${e} | ${(1048575 & s).toString(16)}`;
			    break
			}
			let r = this.W.getAddressModule(i, s);
			if ("_EL_HideOwner" == n && (this.U.has(16777215 & s) || this.J.has(268435455 & s))) {
			    t = `发现GEE倍攻软件，进程为: ${i} | ${e} | ${(16777215 & s).toString(16)}`;
			    break
			}
			if ("" != r) {
			    if (r.search(/.exe$/) > 1) {
				if ("QQPinyinImageCandWndTSF" == n && 12320 == (0xFFFF & s)) {
				    t = `GEE大师外挂请封号处理【3号特征】，进程为: ${i
					} | ${e
					} | ${(0xFFFF & s).toString(16)
					}`;
				    break
				}
				if ("AutoIt v3 GUI" == n && 32778 == (0xFFFF & s)) {
				    t = `发现一键小退软件，进程为: ${i
					} | ${e
					} | ${(0xFFFF & s).toString(16)
					}`;
				    break
				}
				if ("FORM_PROJECT1_FORM1_CLASS:0" == n && 8790 == (0xFFFF & s)) {
				    t = `发现猎手或者荣耀外挂请封号处理【4号特征】，进程为: ${i
					} | ${e
					} | ${(0xFFFF & s).toString(16)
					}`;
				    break
				}
				if ("MSCTFIME UI" == n && 43611 == (0xFFFF & s)) {
				    t = `发现简单A版加强版: ${i
					} | ${e
					} | ${(0xFFFF & s).toString(16)
					}`;
				    break
				}
				if ("VBBubbleRT6" == n && 7056 == (0xFFFF & s)) {
				    l.instance.Report(this.t, !0, `发现脱机软件，进程为: ${i
					} | ${e
					} | ${(0xFFFF & s).toString(16)
					}`);
				    break
				}
				if ("CiceroUIWndFrame" == n && 6992 == (0xFFFF & s)) {
				    l.instance.Report(this.t, !0, `发现脱机软件，进程为: ${i
					} | ${e
					} | ${(0xFFFF & s).toString(16)
					}`);
				    break
				}
				if ("tooltips_class32" == n && 55493 == (0xFFFF & s)) {
				    t = `发现GEE大师多开器，进程为: ${i
					} | ${e
					} | ${(0xFFFF & s).toString(16)
					}`;
				    break
				}
				if ("_EL_HideOwner" == n && (57734 == (0xFFFF & s) || 65003 == (0xFFFF & s) || 65067 == (0xFFFF & s) || 58134 == (0xFFFF & s) || 22782 == (0xFFFF & s) || 23038 == (0xFFFF & s) || 33671 == (0xFFFF & s) || 35482 == (0xFFFF & s) || 4207632 == (16777215 & s) || 10804163 == (16777215 & s) || 5183494 == (16777215 & s) || 5309039 == (16777215 & s) || 9229 == (0xFFFF & s))) {
				    t = `发现倍攻、脱机软件，进程为: ${i
					} | ${e
					} | ${(0xFFFF & s).toString(16)
					}`;
				    break
				}
			    } else if (r.search(/.dll$/) > 1) {
				if ("_EL_HideOwner" == n && (73801816 == (16777215 & s) || 11108590 == (16777215 & s) || 4260479 == (16777215 & s) || 770988 == (1048575 & s) || 749483 == (1048575 & s) || 41488 == (0xFFFF & s) || 27879 == (0xFFFF & s) || 29636 == (0xFFFF & s) || 25469 == (0xFFFF & s) || 8051 == (0xFFFF & s) || 42987 == (0xFFFF & s) || 61805 == (0xFFFF & s) || 43575 == (0xFFFF & s) || 43651 == (0xFFFF & s) || 46674 == (0xFFFF & s) || 5377 == (0xFFFF & s) || 7394 == (0xFFFF & s) || 4294 == (0xFFFF & s) || 4917 == (0xFFFF & s) || 39191 == (0xFFFF & s) || 58006 == (0xFFFF & s) || 58134 == (0xFFFF & s) || 5448 == (0xFFFF & s) || 817 == (0xFFFF & s) || 52101 == (0xFFFF & s) || 57808 == (0xFFFF & s) || 22858 == (0xFFFF & s) || 22090 == (0xFFFF & s) || 585224 == (1048575 & s) || 9056 == (0xFFFF & s) || 8253 == (0xFFFF & s) || 14664 == (0xFFFF & s) || 9327 == (0xFFFF & s) || 8755 == (0xFFFF & s) || 5954 == (0xFFFF & s))) {
				    t = `发现GEE倍攻软件,
									进程为: ${i
					} | ${e
					} | ${(0xFFFF & s).toString(16)
					}`;
				    break
				}
				if (r.search(/.dll$/) > 1 && "WTWindow" == n && (22858 == (0xFFFF & s) || 22090 == (0xFFFF & s))) {
				    t = `发现GEE倍攻软件,
									进程为: ${i
					} | ${e
					} | ${(0xFFFF & s).toString(16)
					}`;
				    break
				}
				if ("VBBubbleRT6" == n && 1029121 == (1048575 & s)) {
				    t = `定制外挂请封号处理，进程为: ${i
					} | ${e
					} | ${(1048575 & s).toString(16)
					}`;
				    break
				}
			    } else if (r.search(/.tap$/) > 1) {
				if (56607 == (0xFFFF & s) || 22768 == (0xFFFF & s)) {
				    t = `发现荣耀外挂请封号处理,
									进程为【7号特征】: ${i
					} | ${e
					} | ${(0xFFFF & s).toString(16)
					}`;
				    break
				}
			    } else if (r.search(/.dat$/) > 1 && "_EL_HideOwner" == n && (5685248 == (16777215 & s) || 9834496 == (16777215 & s) || 11271589 == (16777215 & s) || 5175102 == (16777215 & s) || 15072527 == (16777215 & s) || 9424896 == (16777215 & s) || 4620329 == (16777215 & s) || 9918941 == (16777215 & s))) {
				t = `发现倍攻外挂，进程为: ${i
				    } | ${e
				    } | ${(16777215 & s).toString(16)
				    }`;
				break
			    }
			} else if (this.V.has(0xFFFF & s) || this.V.has(1048575 & s)) {
			    t = `多倍外挂【2号特征】: ${i
				} | ${e
				} | ${(0xFFFF & s).toString(16)
				}`;
			    break
			}
		    }
		    if ("" != t) break
		}
	    }
	    if ("" == t) {
		if (!this.W.processThreads) return;
		for (let [e, i] of this.W.processThreads) if (!(e <= 4)) for (let [s, r] of i) {
		    if (this.N.has(r)) {
			t = `外挂程序为: ${e
			    } | ${s
			    } | ${(0xFFFF & r).toString(16)
			    }`;
			break
		    }
		    let i = this.W.getAddressModule(e, r);
		    if ("" != i) if (i.search(/.exe$/) > 1) {
			if (this.R.has(0xFFFF & r)) {
			    t = `发现大师、定制类外挂，进程为: ${e
				} | ${s
				} | ${(0xFFFF & r).toString(16)
				}`;
			    break
			}
		    } else if (i.search(/.dat$/) > 1 && this.O.has(0xFFFF & r)) {
			t = `发现定制类脱机外挂，进程为: ${e
			    } | ${s
			    } | ${(0xFFFF & r).toString(16)
			    }`;
			break
		    }
		}
	    }
	    "" != t && (l.instance.Report(this.t, !0, t), t = "")
	}
	After() { }
    }
    class p extends n {
	W;
	t = 9055;
	O = new Set([63900, 15738, 61787]);
	Y = new Set([30107]);
	q = new Set(["dm.dll", "XYDsoftWpe.dll", "sp.dll", "hardsp.dll", "softsp.dll"]);
	R = new Set([994315]);
	H = new Set([71555970, 4931527]);
	N = new Set([6406188]);
	V = new Set([15807, 41485, 8717]);
	K = new Set(["_EL_HideOwner", "MSCTFIME", "MSCTFIME UI", "tooltips_class32", "FORM_PROJECT1_FORM1_CLASS:0", "QQPinyinImageCandWndTSF", "VBBubbleRT6", "AutoIt v3 GUI"]);
	j = new Set(["_iext3_CTipWnd", "_06", "SoPY_UI"]);
	X = new Set(["dnf.exe", "client.exe"]);
	constructor() {
	    super(),
		this.W = m.instance
	}
	Before() { }
	Do() {
	    if (!this.W.processWindows) return;
	    let t = "";
	    const i = e.get_current_process_id();
	    for (let [s, r] of this.W.processWindows) if (!(s <= 4)) for (let [n, o, h, a] of r) {
		const r = this.W.getHwndWndProc(a);
		if ("基址初始化" == n && r) {
		    t = `发现定制外挂，请封号处理，进程为: ${s
			} | ${this.W.getProcessName(s)
			} | ${n
			} | ${o
			} | ${r.toString(16)
			}`;
		    break
		}
		if (r && this.H.has(r)) {
		    t = `发现定制外挂，请封号处理，进程为: ${s
			} | ${this.W.getProcessName(s)
			} | ${n
			} | ${o
			} | ${r.toString(16)
			}`;
		    break
		}
		if (s == e.get_current_process_id() && r && this.H.has(0xFFFF & r)) {
		    t = `发现定制外挂，请封号处理，进程为: ${s
			} | ${this.W.getProcessName(s)
			} | ${n
			} | ${o
			} | ${r.toString(16)
			}`;
		    break
		}
		if (o.includes("_EL_HideOwner")) if (s == i) {
		    let t = e.get_wnd_proc(a),
			i = this.W.getAddressModule(s, t);
		    i.includes("PlugClient") || i.includes("gxxcx") || i.includes("HTPlugin") || i.includes("lfm2cx") || i.includes("Sx_gee") || i.includes("TCPlugin_GEE") || i.includes("v8m2cx") || 11969 == (0xFFFF & t) || l.instance.Report(9057, !0, `检测到游戏易语言: ${i
			} | ${t.toString(16)
			}`)
		} else {
		    let t = this.W.processThreads.get(s);
		    if (t) for (let [i, r] of t) if (0 == (4095 & r)) {
			let t = this.W.getAddressModule(s, r);
			if ("" == t) continue;
			if (t == i) continue;
			t.search(/.exe$/) > 1 && 0 == this.X.has(i.toLowerCase()) && e.report(this.t, !0, `驱动加速伪装: ${i
			    } | ${r.toString(16)
			    } | ${t
			    }`)
		    }
		}
		if ("" == this.W.getProcessName(s) || !this.K.has(o) && 10 != o.length) continue;
		let h = this.W.processThreads.get(s);
		if (h) for (let [e, i] of h) {
		    if (("tooltips_class32" == o || "MSCTFIME" == o) && (this.V.has(i) || this.V.has(16777215 & i))) {
			t = `发现GEE猎手或者荣耀外挂请封号处理【1号特征】，进程为: ${s
			    } | ${e
			    } | ${i.toString(16)
			    }`;
			break
		    }
		    if ("csrss.exe" == e && (o = "_EL_HideOwner")) {
			t = `发现非法注入，进程为: ${s
			    } | ${e
			    } | ${o
			    } | ${i.toString(16)
			    }`;
			break
		    }
		    let r = this.W.getAddressModule(s, i);
		    if ("" != r) if (r.search(/.exe$/) > 1) {
			if ("QQPinyinImageCandWndTSF" == o && 12320 == (0xFFFF & i)) {
			    t = `发现GEE大师外挂请封号处理【3号特征】，进程为: ${s
				} | ${e
				} | ${i.toString(16)
				}`;
			    break
			}
			if ("VBBubbleRT6" == o && 4824022 == (16777215 & i)) {
			    t = `发现GEE大师外挂请封号处理【4号特征】，进程为: ${s
				} | ${e
				} | ${i.toString(16)
				}`;
			    break
			}
			if ("#32770" == o && (1038511 == (1048575 & i) || 131072 == (1048575 & i))) {
			    t = `发现GEE大师外挂请封号处理【5号特征】，进程为: ${s
				} | ${e
				} | ${i.toString(16)
				}`;
			    break
			}
			if ("FORM_PROJECT1_FORM1_CLASS:0" == o && 8790 == (0xFFFF & i)) {
			    t = `发现GEE猎手或者荣耀外挂请封号处理【6号特征】，进程为: ${s
				} | ${e
				} | ${i.toString(16)
				}`;
			    break
			}
			if ("_iext3_CTipWnd" == n && 3056 == (0xFFFF & i)) {
			    t = `发现GEE猎手或者荣耀外挂请封号处理【7号特征】，进程为: ${s
				} | ${e
				} | ${i.toString(16)
				}`;
			    break
			}
			if ("_EL_HideOwner" == o && (48640 == (0xFFFF & i) || 65436 == (0xFFFF & i))) {
			    t = `发现GEE猎手或者荣耀外挂请封号处理【8号特征】，进程为: ${s
				} | ${e
				} | ${i.toString(16)
				}`;
			    break
			}
		    } else if (r.search(/.dll$/) > 1) {
			if ("MSCTFIME UI" == o && (29295 == (0xFFFF & i) || 11369779473 == (0xFFFF & i))) {
			    t = `发现GEE猎手或者荣耀外挂请封号处理【9号特征】，进程为: ${s
				} | ${e
				} | ${i.toString(16)
				}`;
			    break
			}
		    } else if (r.search(/.IME$/) > 1) {
			if ("_EL_HideOwner" == o && 49856 == (0xFFFF & i)) {
			    t = `发现脱机回收外挂，进程为: ${s
				} | ${e
				} | ${i.toString(16)
				}`;
			    break
			}
		    } else if (r.search(/.tmp$/) > 1) {
			if (4198400 == (16777215 & i)) {
			    t = `发现加速外挂请封号处理，进程为【7号特征】: ${s
				} | ${e
				} | ${i.toString(16)
				}`;
			    break
			}
		    } else {
			if (r.search(/.tap$/) > 1 && (56607 == (0xFFFF & i) || 22768 == (0xFFFF & i) || 4198400 == (16777215 & i))) {
			    t = `发现荣耀外挂请封号处理，进程为【7号特征】: ${s
				} | ${e
				} | ${i.toString(16)
				}`;
			    break
			}
			if (r.search(/.dat$/) > 1 && 21808 == (0xFFFF & i)) {
			    t = `发现脱机挂请封号处理，进程为【八号特征】: ${s
				} | ${e
				} | ${i.toString(16)
				}`;
			    break
			}
		    } else if (this.V.has(0xFFFF & i) || this.V.has(1048575 & i)) {
			t = `发现GEE猎手或者荣耀外挂请封号处理【2号特征】，进程为: ${s
			    } | ${e
			    } | ${i.toString(16)
			    }`;
			break
		    }
		}
		if ("" != t) break
	    }
	    if ("" == t) {
		if (!this.W.processThreads) return;
		for (let [e, i] of this.W.processThreads) if (!(e <= 4)) for (let [s, r] of i) {
		    if (this.N.has(r)) {
			t = `发现封包工具或者外挂！，进程为: ${e
			    } | ${s
			    } | ${r.toString(16)
			    }`;
			break
		    }
		    let i = this.W.getAddressModule(e, r);
		    if ("" != i) if (i.search(/.exe$/) > 1) {
			if (this.R.has(0xFFFF & r)) {
			    t = `发现大师、定制类外挂，进程为: ${e
				} | ${s
				} | ${r.toString(16)
				}`;
			    break
			}
		    } else if (i.search(/.dll$/) > 1) {
			if (this.q.has(i)) {
			    l.instance.Report(9057, !0, `发现易语言外挂，进程为: ${e
				} | ${s
				} | ${i
				}`);
			    break
			}
		    } else if (i.search(/.yz$/) > 1) {
			if (this.Y.has(0xFFFF & r)) {
			    l.instance.Report(9023, !0, `发现易语言加速程序，进程为: ${e
				} | ${s
				} | ${i
				}`);
			    break
			}
		    } else if (i.search(/.dat$/) > 1 && this.O.has(0xFFFF & r)) {
			t = `发现定制类脱机外挂，进程为: ${e
			    } | ${s
			    } | ${r.toString(16)
			    }`;
			break
		    }
		}
	    }
	    "" != t && (l.instance.Report(this.t, !0, t), t = "")
	}
	After() { }
    }
    class b extends n {
	t = 9059;
	W;
	constructor() {
	    super(),
		this.W = m.instance
	}
	Before() { }
	Do() { }
	After() { }
    }
    class S extends n {
	t = 9062;
	o = 9022;
	Z = 9021;
	W;
	ee = new Set([274432, 352256]);
	constructor() {
	    super(),
		this.W = m.instance
	}
	Before() { }
	Do() {
	    const t = this.W.processWindows;
	    if (!t) return;
	    let i = new Set;
	    for (let [e, s] of t) for (let t of s) {
		let [s, r, n, o] = t;
		"_EL_HideOwner" == r && i.add(e)
	    }
	    const s = e.get_current_process_id();
	    for (let [e, n] of t) {
		for (let t of n) {
		    let [s, n, o, h] = t,
			a = this.W.getHwndProps(h);
		    if (a && a.has("Ex_Wnd_Control")) {
			if (i.has(e)) break;
			l.instance.Report(this.o, !0, `geels | wnd | ${e
			    } | ${this.W.getProcessName(e)
			    } | ${s
			    } | ${n
			    } (${r(n)
			    })`);
			break
		    }
		}
		if (e == s) {
		    const t = this.W.getProcessModules(e);
		    if (t) for (let [i, s, r] of t) {
			"hook32.dll" == i.toLowerCase() && l.instance.Report(this.Z, !0, `hook多倍外挂.一定要封号 | ${e
			    } | ${this.W.getProcessName(e)
			    }`)
		    }
		} else {
		    const t = this.W.getProcessModules(e);
		    if (0 == e && n.length > 0) {
			let t = "";
			for (let e of n) {
			    let [i, s, r, n] = e;
			    t += `${i
				} (${n
				}) | `
			}
			0 == t.includes("Build") && 0 == t.includes("NVOGLDC") && 0 == t.includes("Default IME") && 0 == t.includes("背包仓库管理") && l.instance.Report(this.t, !0, `pid0 | ${e
			    } | ${this.W.getProcessName(e)
			    } | ${t
			    }`)
		    }
		    if (t) {
			if (t.length > 0) {
			    let [i, s, r] = t[0];
			    i.toLowerCase().includes("ntdll.dll") && l.instance.Report(this.t, !0, `ntdll | module | ${e
				} | ${this.W.getProcessName(e)
				} | ${i
				} | ${r.toString(16)
				}`)
			}
			for (let [i, s, r] of t) {
			    i.toLowerCase().includes("system32\\nfapi.dll") && (this.ee.has(r) ? l.instance.Report(this.Z, !0, `多倍外挂！一定要封号 | ${e
				} | ${this.W.getProcessName(e)
				}`) : l.instance.Report(this.t, !0, `nfapi | module | ${e
				    } | ${this.W.getProcessName(e)
				    } | ${i
				    } | ${r.toString(16)
				    }`))
			}
		    }
		}
	    }
	}
	After() { }
    }
    class k extends n {
	t = 9011;
	o = 9011;
	Z = 9011;
	te = 9e5;
	ie = [];
	se = [];
	Before() {
	    this.ie = e.enum_pdb(),
		this.se = e.get_cur_module_list()
	}
	detectPdb() {
	    this.ie.forEach((e => {
		if (e.includes("bora-") || e.includes("wddm\\i386") || e.includes("wddm\\x86")) return l.instance.Report(this.t, !0, "发现虚拟机环境:" + e),
		    void l.instance.Kick(this.te);
		let t = e.replace(/.*\\|\..*$/gi, "");
		return t.includes("vm3d") ? (l.instance.Report(this.t, !0, "发现虚拟机环境:" + t), void l.instance.Kick(this.te)) : void 0
	    }))
	}
	calcUint8ArrayCrossEntropy(e) {
	    const t = new Map,
		i = e.length;
	    for (let s = 0; s < i; s++) {
		const i = e[s];
		if (t.has(i)) {
		    let e = t.get(i);
		    e && t.set(i, e + 1)
		} else t.set(i, 1)
	    }
	    let s = 0;
	    for (let [e, r] of t) {
		const e = r / i;
		s -= e * Math.log2(e)
	    }
	    return s
	}
	resolveInstruction(t) {
	    let i = e.read_bytes(t, 1);
	    return this.isJmpInstruction(i) ? t + 5 + e.read_dword(t + 1) : t
	}
	isJmpInstruction(e) {
	    return 233 == e[0] || 235 == e[0] || 232 == e[0]
	}
	isStandardFunctionHeader(e) {
	    return 85 == e[0] && 139 == e[1] && 236 == e[2] || 85 == e[2] && 139 == e[3] && 236 == e[4]
	}
	detectAdapterSig() {
	    const t = this.se;
	    for (let i = 0; i < t.length; i++) {
		let [s, r, n] = t[i],
		    o = e.get_proc_address(s, "OpenAdapter");
		if (o) {
		    let t = e.scan(o, 256, [232, 204, 204, 204, 204, 185, 204, 204, 204, 204, 232, 204, 204, 204, 204]);
		    t.length && "AdapterShimPath" == e.read_wstring(e.read_dword(t[0] + 6)) && (l.instance.Report(this.t, !0, "发现虚拟机环境"), l.instance.Kick(this.te));
		    let i = e.scan(o, 256, [36, 1, 104, 204, 204, 204, 204, 162]);
		    i.length && "AdapterShimPath" == e.read_wstring(e.read_dword(i[0] + 3)) && (l.instance.Report(this.t, !0, "发现虚拟机环境"), l.instance.Kick(this.te)),
			e.scan(o, 512, [199, 65, 20, 3, 32, 0, 0, 137, 48, 139, 81, 4, 137, 80, 4, 139, 81, 8, 137, 80, 8, 139, 73, 12]).length && (l.instance.Report(this.t, !0, "发现虚拟机环境"), l.instance.Kick(this.te)),
			e.read_string(r + 784).includes("yoda") && (l.instance.Report(this.t, !0, "发现虚拟机环境"), l.instance.Kick(this.te));
		    let s = e.read_string(r + 904),
			n = e.read_string(r + 776); (s.includes("winlice") || n.includes("winlice")) && (l.instance.Report(this.t, !0, "发现虚拟机环境5"), l.instance.Kick(this.te)),
			    e.read_string(r + 816).includes("themida") && (l.instance.Report(this.t, !0, "发现虚拟机环境6"), l.instance.Kick(this.te));
		    let h = this.resolveInstruction(o),
			a = e.read_bytes(h, 50);
		    if (!this.isStandardFunctionHeader(a)) {
			this.calcUint8ArrayCrossEntropy(a);
			if (86 == a[0] && 190 == a[1]) {
			    a[0].toString(16),
				a[1].toString(16),
				a[2].toString(16),
				a[3].toString(16),
				a[4].toString(16);
			    l.instance.Report(this.Z, !0, "发现虚拟机环境7")
			}
		    }
		}
	    }
	}
	Do() {
	    this.detectPdb(),
		this.detectAdapterSig()
	}
	After() { }
    }
    class E extends n {
	t = 9052;
	o = 9053;
	te = 9e5;
	re = ["VmLoader"];
	ne = ["IINDLCPXGO", "Ltq", "LoveSnow", "PCCKJCA4", "Dult", "Dultx64_Protect", "GNLAKBOZYKOYCKB", "BBBBas", "FengDrv2787", "SM762FE", "vmx_fb", "vm3dmp", "nvd3dum", "nv3dmp", "HideToolz", "wujiejiami", "Sp_Hs", "Passkpp_Demo", "SuperSpeedx64", "SpeedHook", "Gwken", "yxbsq", "mengwuji", "Win7Speed", "wwE21wwE", "lonerSpeed_v40", "LtqDrv"];
	oe = [];
	le = [];
	getPdbCached(t) {
	    if (globalThis.pdbCache || (globalThis.pdbCache = new Map), globalThis.pdbCache.has(t)) return globalThis.pdbCache.get(t) || "";
	    try {
		const i = e.get_pdb_path(t);
		return globalThis.pdbCache.set(t, i),
		    i
	    } catch (e) {
		return ""
	    }
	}
	Before() {
	    this.oe = e.enum_device()
	}
	Do() {
	    this.oe.forEach((e => {
		if (e.split(/[a-zA-Z]:\\/).length > 1) {
		    let t = e.toLowerCase().replace("system32", "sysnative");
		    t.includes("idbgdrv.sys") && l.instance.Report(this.t, !0, "非法驱动正在运行:" + e);
		    let i = this.getPdbCached(t);
		    this.re.forEach((t => {
			i.includes(t) && l.instance.Report(this.t, !0, "非法驱动正在运行:" + e)
		    }))
		}
		this.ne.forEach((t => {
		    e.includes(t) && l.instance.Report(this.t, !0, "非法驱动正在运行:" + e)
		})),
		    e.includes("VEN_15AD") && (l.instance.Report(this.o, !0, "虚拟机设备:" + e), l.instance.Kick(this.te))
	    }))
	}
	After() { }
    }
    class I extends n {
	t = 9997;
	constructor() {
	    super()
	}
	Before() {
	    globalThis.playername = e.get_player_name()
	}
	Do() {
	    globalThis.playername && l.instance.reportImmediately(this.t, !1, globalThis.playername)
	}
	After() { }
    }
    class y extends n {
	t = 9064;
	W;
	constructor() {
	    super(),
		this.W = m.instance
	}
	Before() { }
	hasRemoteDesktop() {
	    const e = this.W.processWindows;
	    if (!e) return !1;
	    for (let [t, i] of e) {
		let e = this.W.getProcessName(t);
		if ("SunloginClient.exe" == e || "ToDesk.exe" == e) return !0
	    }
	    return !1
	}
	vkToString(e) {
	    switch (e) {
		case 1:
		    return "MouseLeft";
		case 2:
		    return "MouseRight";
		case 9:
		    return "Tab";
		case 16:
		    return "Shift";
		case 17:
		    return "Ctrl";
		case 18:
		    return "Alt";
		case 48:
		    return "0";
		case 49:
		    return "1";
		case 50:
		    return "2";
		case 51:
		    return "3";
		case 52:
		    return "4";
		case 53:
		    return "5";
		case 54:
		    return "6";
		case 55:
		    return "7";
		case 56:
		    return "8";
		case 57:
		    return "9";
		case 112:
		    return "F1";
		case 113:
		    return "F2";
		case 114:
		    return "F3";
		case 115:
		    return "F4";
		case 116:
		    return "F5";
		case 117:
		    return "F6";
		case 118:
		    return "F7";
		case 119:
		    return "F8";
		case 120:
		    return "F9";
		case 121:
		    return "F10";
		case 122:
		    return "F11";
		case 123:
		    return "F12"
	    }
	    return e.toString(16)
	}
	Do() { }
	After() { }
    }
    class g extends n {
	t = 9065;
	he = ["PatchGuard"];
	constructor() {
	    super()
	}
	Before() { }
	Do() {
	    if ("function" == typeof e.get_bcd_info) {
		let [t, i, s] = e.get_bcd_info();
		s && l.instance.Report(this.t, !1, `bcd | ${t
		    } | ${i
		    } | ${s
		    }`),
		    s && this.he.forEach((e => { - 1 != i.indexOf(e) && l.instance.Report(this.t, !0, `black_bcd | ${t} | ${i} | ${s}`) }))
	    }
	}
	After() { }
    }
    function M(e) {
	new Date;
	try {
	    e.Before()
	} catch (e) { }
	try {
	    e.Do()
	} catch (e) { }
	try {
	    e.After()
	} catch (e) { }
	new Date
    }
    console.dbgprint = function (t, ...i) {
	let s = t;
	i.length > 0 && (s += " " + i.join(" ")),
	    e.report(9998, !1, s)
    };
    try {
	let e = [new I, new $, new u, new a, new h, new k, new E, new c, new E, new f, new w, new p, new b, new S, new y, new g];
	for (let t of e) M(t);
	l.instance.reportAll()
    } catch (e) { }
})();