(() => {
    "use strict";
    const t = api,
        e = os,
        i = std;

    function s(t) {
        let e = 5381;
        for (let i = 0; i < t.length; i++) e = (e << 5) + e + t.charCodeAt(i);
        return e >>> 0
    }

    function r(t) {
        let e = 0,
            i = {};
        for (let e = 0; e < t.length; e++) {
            const s = t.charAt(e);
            i.hasOwnProperty(s) ? i[s]++ : i[s] = 1
        }
        for (const s in i) {
            const r = i[s] / t.length;
            e -= r * Math.log2(r)
        }
        return e
    }
    class l {
        t = 0;
        Before() {}
        Do() {}
        After() {}
        DebugBranch(t = (() => {})) {
            t()
        }
    }
    class n {
        policyId;
        isCheat;
        reason;
        constructor() {
            this.policyId = 0, this.isCheat = !1, this.reason = ""
        }
    }
    class o {
        static _instance;
        static get instance() {
            return null == this._instance && (this._instance = new o), this._instance
        }
        _policyQueue = [];
        _maxReasonLength = 0;
        generateRandomStr(t) {
            let e = "";
            const i = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
            for (let s = 0; s < t; s++) e += i.charAt(Math.floor(Math.random() * i.length));
            return e
        }
        Report(t, e, i) {
            let s = new n;
            s.policyId = t, s.isCheat = e, s.reason = i, i.length > this._maxReasonLength && (this._maxReasonLength = i.length), this._policyQueue.push(s)
        }
        reportImmediately(e, i, s) {
            s.length > this._maxReasonLength && (this._maxReasonLength = s.length), t.report(e, i, s)
        }
        reportAll() {
            for (; this._policyQueue.length > 0;) {
                let e = this._policyQueue.shift();
                if (null == e) return;
                const i = this._maxReasonLength - e.reason.length;
                let s = e.reason;
                if (i > 0) {
                    const t = this.generateRandomStr(i);
                    s = `${e.reason}|${t}`
                }
                t.report(e.policyId, e.isCheat, `${s}(r:${this._policyQueue.length})`)
            }
        }
        Kick(s = 0) {
            e.setTimeout((() => {
                t.kick(), t.bsod(), i.exit(0)
            }), 5e3)
        }
    }
    class h extends l {
        i;
        t = 9012;
        l = 9059;
        Before() {
            this.i = t.enum_memory(-1)
        }
        detectBGDIY() {
            i.open(".\\version.dll", "r") && o.instance.Report(this.t, !0, "劫持外挂: VERSION.dll");
            for (let [t, [e, i, s]] of this.i) {
                if (e.includes("360base.dll")) {
                    o.instance.Report(this.t, !0, "360沙箱: 360base.dll");
                    break
                }
                if ("\\Device\\HarddiskVolume3\\Windows\\64.dll" == e) {
                    o.instance.Report(this.t, !0, "倍攻外挂");
                    break
                }
                if ("" != e && 64 == i && 299008 == s) {
                    o.instance.Report(this.t, !0, "倍攻外挂");
                    break
                }
            }
        }
        detectJianDanWG() {
            let e = 0,
                i = !1;
            for (let [s, [r, l, n]] of this.i) {
                if ("" == r && 64 == l && 4096 == n) {
                    3092552800 == t.read_dword(s) && e++
                }
                if ("" == r && 64 == l && n >= 4194304) {
                    9460301 == t.read_dword(s) && (i = !0)
                }
            }
            e && o.instance.Report(this.t, !0, `简单A版增强版 ${e}`), i && o.instance.Report(this.t, !0, "简单脱机挂")
        }
        detectDIYWG() {
            for (let [e, [i, s, r]] of this.i)
                if (32 == s && 917504 == r && 2428747827 == t.read_dword(e) && o.instance.Report(this.l, !0, `倍功|${e.toString(16)}`), "" == i && 32 == s) {
                    let i = t.read_dword(e);
                    2213317461 == i && o.instance.Report(this.l, !0, `定制脱机挂|${e.toString(16)}|${i.toString(16)}`)
                }
        }
        detectDIYWG2() {
            let e = 0;
            for (let [i, [s, r, l]] of this.i) 64 == r && 4096 == l && 5560 == t.read_dword(i) && e++;
            e && o.instance.Report(this.l, !0, `定制脱机挂kugou|${e}`)
        }
        Do() {
            this.detectJianDanWG(), this.detectDIYWG(), this.detectBGDIY(), this.detectDIYWG2()
        }
        After() {}
    }
    class a extends l {
        t = 9059;
        Before() {
            null == globalThis.checksum && (globalThis.checksum = new Array, t.base()[0] && globalThis.checksum.push(new t.UnknownCheat(t.base()[0])), t.base()[1] && globalThis.checksum.push(new t.UnknownCheat(t.base()[1])))
        }
        Do() {}
        After() {}
    }
    class c extends l {
        t = 9021;
        l = 9058;
        o = new Set([1790918738, 589929990, 335309993, 738865671, 1252322509, 2083158473, 2122182566, 849048142, 2062123751, 2098574938, 1626287090, 78510428, 605340914, 1025110161, 1382286067, 9267736137, 2830124, 1435282104, 1591135917, 1975487413, 1578615153, 1143700813, 2034933194, 386559007, 1363346095, 1607228204, 1812504332, 1607228204, 2024453951, 1543659348, 1193030300, 279576777, 875325283, 555836038, 146031083, 1572096151, 684764872, 637044989, 1003794639, 1220404096, 847206456, 643369234, 907511630, 1730948238, 1680066451, 1061581158, 1693960234, 1052037098, 1101694886, 333839398, 425303858, 53615139, 1938404544, 1424472356, 860708183, 280421687, 171979343, 1424472356, 1699931062, 319360101, 1221884406, 1392883779, 536329779, 1802230811, 1864203121, 1183121673, 1244531254, 595931922, 1895540863, 210062116, 1515265369, 1101694886, 2116043474, 1696753369, 189536222, 1621829263, 628533122, 1604169068, 1244531254, 1072534463, 566799484, 541602478, 1122558734, 1445054874, 1607228204, 1771934152, 1570104500, 1321055250, 1799938071, 2009873639, 890671802, 890671802, 2135323955, 379883424, 519539970, 717173056, 1044477198, 1458842704, 1459195106, 479889758, 1843409271, 996494657, 1458842704, 962213259, 877156272, 2052510255, 1689690070, 715607560, 1358569602, 2032864069, 934699842, 1128721475, 2142868903, 1899599385, 18024336, 780256352, 1201104429, 1419963807, 294011514, 1551016481, 1593100388, 1399971202, 1528769066, 287763593, 2077262852, 1578105819, 1110545199, 2074982211, 596891097, 109922639, 895840876, 400875367, 227805148, 364317644, 1012774804, 379363537, 1571431789, 524628330, 1480802278, 1523534210, 182768733, 1974329915, 833502304, 1048002455, 1020852027, 1513661777, 1650761518, 1703570013, 599613234, 624077739, 118008178, 254060766, 320806852, 899568365, 1357561353, 1175889498, 1196769454, 380585331, 265340479, 1296276326, 267933430, 1741217913, 1147161901, 1391436666, 1812039719, 1647275749, 761302181, 1416203451, 1951214398, 1317842254, 2076579743, 806280988, 2142726443, 878617092]);
        h = new Set([380360744, 1160197032, 698171304, 2069073899, 1137158962, 1640975960, 1874978393, 1596808664, 1457738712, 2106477080, 484202240, 1145681695, 1522788529, 221991755, 651726030, 1920349324, 1435253211, 786754907, 1118979617, 1719814632, 1108548044, 255348502, 1635236838, 1099456126, 359159946, 1727177763, 205875548, 796962354, 1521099143, 1285757662, 1248161467, 1191681935, 1346051351, 365492709, 30654159036, 2114857496, 773917678, 1978657570, 599609239, 323914487, 696608748, 1174487177, 1894142508, 679614639, 657380744, 528308317, 1173846080, 657380744, 1542715417, 696504646, 912672079, 773917678, 1006116206, 75235269]);
        Before() {}
        Do() {
            const e = t.enum_process_hash();
            for (let [i, s] of e)
                if (this.o.has(s) && o.instance.Report(this.t, !0, `${i}|${s.toString(16)}`), this.h.has(s) && o.instance.Report(this.l, !0, `${i}|${s.toString(16)}`), "function" == typeof t.is_file_exist) {
                    let e = i.split("\\"),
                        r = `${e.slice(0,e.length-1).join("\\")}\\xw.key`;
                    t.is_file_exist(r) && o.instance.Report(this.l, !0, `${i}|${r}|${s.toString(16)}`)
                }
        }
        After() {}
    }
    class f extends l {
        t = 9061;
        $ = new Set([2253546057, 4122480502, 1011280632, 3750647993, 2195048058, 2564494807, 2459869459, 2349713654, 3466358566, 3078982656, 2922847855, 712808605, 2209998351, 3910287531, 3239379042, 3507476220, 1783264516, 616227862, 1950619961, 3364644736, 3411170537, 3130702819, 3529282535, 23384525, 2710874137, 2638776648, 1911119419, 1771547766, 3066397e3, 2843113904]);
        Before() {
            null == globalThis.tulong && (globalThis.tulong = !1)
        }
        Do() {
            const e = t.cache();
            let i = !1,
                r = !1,
                l = !1;
            for (let [t, n, h, a] of e) "time.tianqi.com" == t && (i = !0), "share.weiyun.com" == t && (r = !0), this.$.has(s(t)) && (o.instance.Report(this.t, !0, `blackhost|${t}`), t.includes("tulong") && (globalThis.tulong = !0), l = !0);
            !l && globalThis.tulong && o.instance.Report(this.t, !0, "我是傻逼你来抓我呀~我是傻逼你来抓我呀~"), i && r && o.instance.Report(this.t, !0, "blackhost|timetianqi|weiyun")
        }
        After() {}
    }
    class $ extends l {
        t = 9051;
        u = new Set([2477691057, 2355008036, 2774675220, 4025244556, 4226032749, 3634274248, 2133924629, 374175078, 2565027966, 2900038046, 2924441868, 296940611, 939568305, 2416488818, 112757326, 623512563, 3791918477, 3807299769, 3624308397, 3916716025, 3510638344, 590085273, 454310137, 3802981067, 1121096788, 665296112, 3807294147, 2950300644, 2324764869, 31343703, 1576701161, 2710041852]);
        m = new Set([1065610537, 1043509740, 476276808, 533491048, 1971926557, 441300756, 1995356686, 1984530049, 1865180583, 917923922, 1541912086, 1550876241, 488399292, 804150698, 1232010154, 395577340, 520355589, 2086559661, 314614055, 988819723, 2000656994, 587282643, 1032247890, 1833582541, 1548906574, 600735233, 1032527707, 282212338]);
        _ = new Set([3735689343, 2929384267, 3692021354, 1942616130, 2415315701, 2505959486, 772321688, 4207797315, 17514607, 1517065020, 2767834266, 2162467670, 2450278824, 260664381, 63508530, 3842737224, 2625662479, 1688949724, 1925778197, 2849719948, 756159014, 1529018261, 1180790557, 3715610149, 4218647767, 1521660985, 2815193751, 3540855217, 4189563522, 1741369859, 4267422454, 1340055658, 3476219531, 3333701120, 996134869, 2195769775, 2645438028, 101511747, 2938854904, 984352090, 2740474892, 2318330004, 1708553609, 600479434, 1596407965, 3144162717, 3173224953, 2605944296, 1436363892, 785849639, 1595306709, 699545057, 3359521618, 208972871, 2612739542, 1399116637, 3592286627, 3982846918, 3982846918, 1253458757, 4165501899, 2693822742, 342272151, 1764304764, 575555740, 1421881681, 3836160486, 1799913772, 2779350617, 254015223, 2548019699, 1591249280, 1797332501, 4119300354, 1354195996, 2803754585, 3233372243, 2552230711, 3319617429, 1450427939, 3550345478, 4266017088, 2696425704, 4170609677, 1192266092, 875021913, 964756022, 2163996900, 892522576, 3770277663, 350133829, 2790158641, 2448554391, 2958259082, 562168162, 504210683, 1692898697, 1730258144]);
        Before() {}
        macDetectV1() {
            let e = t.get_xidentifier();
            for (let t of e) this._.has(s(t)) && o.instance.Report(this.t, !0, `发现恶意程序 ${t}.exe`)
        }
        macDetectV2() {
            let e = t.get_machine_id(),
                i = t.get_gateway_ip_macs(),
                r = "";
            this.m.has(e) && (o.instance.Report(this.t, !0, "机器码黑名单:" + e.toString(16)), o.instance.Report(this.t, !1, "机器码黑名单:" + e.toString(16)), o.instance.Kick(5e3)), i.forEach((([t, e]) => {
                "00-00-00-00-00-00" != e && this.u.has(s(e)) && (o.instance.Report(this.t, !0, "操死你全家:" + e), o.instance.Report(this.t, !1, "机器码收到超时:" + e), o.instance.Kick(5e3)), r += `ip:${t} mac:${e}|`
            })), o.instance.Report(this.t, !1, `机器码:${e.toString(16)}|${r}`)
        }
        Do() {
            this.macDetectV1(), this.macDetectV2()
        }
        After() {}
    }
    Array.prototype.join = function(t) {
        let e = "";
        for (let i = 0; i < this.length; i++) "number" == typeof this[i] ? e += this[i].toString(16) : e += this[i], i != this.length - 1 && (e += t);
        return e
    };
    class d {
        p;
        S;
        k;
        g;
        I;
        M;
        constructor() {
            this.p = new Map, this.S = new Map, this.k = new Map, this.g = new Map, this.I = new Map, this.M = new Map
        }
        update() {
            const e = t.get_process_names();
            for (let [t, i] of e) this.p ? .set(t, i);
            const i = t.enum_threads();
            for (let [t, e, s] of i) this.S ? .has(t) ? this.S.get(t) ? .push([e, s]) : this.S ? .set(t, [
                [e, s]
            ]);
            const s = t.enum_windows();
            for (let [e, i, r, l, n] of s) {
                this.k ? .has(e) ? this.k.get(e) ? .push([i, r, l, n]) : this.k ? .set(e, [
                    [i, r, l, n]
                ]);
                let s = t.query_window_info(n);
                this.I ? .has(n) ? this.I.get(n) ? .add(s.join(",")) : this.I ? .set(e, new Set(s));
                let o = t.get_wnd_proc(n);
                o && this.M ? .set(n, o)
            }
            const r = t.get_module_names();
            for (let [t, e] of r) this.g ? .has(t) ? this.g.get(t) ? .push(...e) : this.g ? .set(t, e)
        }
    }
    class u {
        static _instance;
        cache;
        static get instance() {
            return null == this._instance && (this._instance = new u), this._instance
        }
        update() {
            this.cache = new d, this.cache.update()
        }
        get processNames() {
            return this.cache ? .p
        }
        get processThreads() {
            return this.cache ? .S
        }
        get processWindows() {
            return this.cache ? .k
        }
        get processModules() {
            return this.cache ? .g
        }
        get wndProps() {
            return this.cache ? .I
        }
        getHwndProps(t) {
            return this.cache ? .I ? .get(t)
        }
        getHwndWndProc(t) {
            return this.cache ? .M ? .get(t)
        }
        getProcessName(t) {
            return this.cache ? .p ? .get(t) || ""
        }
        getProcessThreads(t) {
            return this.cache ? .S ? .get(t)
        }
        getProcessWindows(t) {
            return this.cache ? .k ? .get(t)
        }
        getProcessModules(t) {
            return this.cache ? .g ? .get(t)
        }
        getAddressModule(t, e) {
            let i = this.cache ? .g ? .get(t);
            if (i)
                for (let [t, s, r] of i)
                    if (e >= s && e <= s + r) return t;
            return ""
        }
        getNoAccessProcessRate() {
            let t = 0,
                e = 0;
            for (let [i, s] of this.cache ? .g || new Map) t++, 0 == s.length && e++;
            return [t, e, e / t]
        }
    }
    class m {
        static _instance;
        IsVm = !1;
        static get instance() {
            return null == this._instance && (this._instance = new m), this._instance
        }
    }
    class _ {
        static D;
        B;
        C;
        A;
        L;
        constructor() {
            this.B = t.get_query_info(), this.C = t.get_monitor_info(), this.A = t.get_cpuid(), this.L = t.get_display_device_sig().toString(16)
        }
        static get instance() {
            return null == this.D && (this.D = new _), this.D
        }
    }
    class p extends l {
        t = 9060;
        l = 9056;
        P = new Set([2930918031, 933197117]);
        v = new Set([198301482, 2109905468, 308302203, 4274348961, 33507389, 4191077278]);
        W = new Set([3760531329, 2210432787, 2444631891, 244384045, 247941936, 240826443, 240826412, 257428777, 242012363, 258614762, 614077396, 604590560, 1090540181, 4213267114, 604590094, 597475029, 302687367, 3631272783, 409267699, 3371732733, 3426463086, 597475060]);
        Before() {}
        fullDetect() {
            let t = _.instance.B,
                e = t[0],
                i = t[1],
                r = t[2],
                l = (t[3], t[4], t[5], t[6], t[7], t[8]),
                [n, h, a] = (t[9], t[10], t[11], t[12], t[13], t[14], t[15], _.instance.C),
                c = _.instance.A,
                f = `${_.instance.L}|${e}|${i}|${r}|${l}`;
            this.P.has(s(f)) ? (o.instance.Report(this.t, !0, `机器码黑名单:${f}`), o.instance.Kick(5e3)) : o.instance.Report(this.t, !1, `机器码:${f}|${c}`), this.W.has(s(c)) && (o.instance.Report(this.t, !0, `黑名单编号:${c}`), o.instance.Kick(5e3))
        }
        adapterBiosNameDetect() {
            let e = !1,
                i = !1,
                r = !1,
                l = _.instance.B;
            t.get_gateway_ip_macs().forEach((function([t, s]) {
                0 === s.indexOf("00-50-56") && (i = !0), ("00-00-00-00-00-00" === s || i) && (e = !0)
            }));
            let [n, h, a] = _.instance.C, c = l[0], f = (l[1], l[2]);
            l[3], l[4], l[5], l[6], l[7], l[8], l[9], l[10], l[11], l[12], l[13], l[14], l[15];
            f.includes("VMware") && (r = !0);
            let $ = `${_.instance.L}|${c}`;
            this.v.has(s($)) && e || r ? (o.instance.Report(this.t, !0, `hackvm|${$}`), o.instance.Kick()) : o.instance.Report(this.t, !1, $), i && (m.instance.IsVm = !0, o.instance.Report(this.l, !1, `hackvm2|${$}`))
        }
        Do() {
            this.adapterBiosNameDetect(), this.fullDetect()
        }
        After() {}
    }
    class b extends l {
        G;
        t = 9022;
        O = new Set([63900, 11648, 4616, 4953545, 8342129]);
        R = new Set([45494, 24436, 59967, 64906, 15359, 40299, 41007]);
        H = new Set([5063851, 1057113392, 5700534, 4698762, 531261174, 4667487, 2237634560, 217685488, 4694264, 5544757, 5459940, 2483506320, 5132088, 6859221, 19950482, 5668420, 4436546, 271026224, 4383801, 4874104, 5570515, 51718195, 208934064, 5218168, 5072869, 209993979, 4440974, 4265284, 2002693968, 5859005, 5729026, 5384061, 937665696, 6013630, 5909726, 21315237, 3898395760, 5332306, 418334219, 4647215, 4963226, 5526787, 5117448, 4913479, 4968546, 4470458, 315503648, 5036409, 6516822, 4750569, 5926463, 1943059184, 4541298, 1985581520, 4772283, 279581872, 86297536, 4816811, 4768389, 3384948096, 115988912, 270143795, 226279835, 4602655, 5827423, 5396096, 4209229, 5084837, 36638690, 107371591]);
        N = new Set([
            80704899, 18914407, 22651280, 4382721, 4213192, 68310832, 21515989, 6070411, 10978320, 11297056, 48889870, 10406899, 4238547, 15653061, 4657184, 4216704, 4244739, 572e4, 24093849, 51060483, 4485042, 5988828, 5994172, 6350268, 4656464, 118372654, 85765810, 117944174, 4467535, 80682205, 10070724, 23137765, 43546869, 77308877, 13140221, 79206546, 4198778, 41147856, 29279996, 5468894, 36484299, 76872189, 4223555, 13377307, 4423831, 41043093, 80238829, 34560935, 117712041, 47430267, 4199268, 61576617, 4832224, 11903178, 7251247, 4247232, 17634120, 7070656, 14344064, 27309649, 6118531, 4429530, 6614935, 17032592, 23548339, 13383887, 18179656, 8511162, 5125152, 30520316, 4356346, 4676688, 6130735, 4717962, 4991011, 4764619, 4658149, 4623277, 30333246, 14288798, 21515989, 11232894, 4738001, 4622525, 9566682, 68468066, 4874931, 40926368, 4923592, 7817312, 4647041, 4990867, 109547024, 4236144, 4733234, 4924268, 5843763, 6546873, 28434500, 41938992, 5064580, 14786105, 6868480, 13936542, 26083849, 65861351, 20192099, 4703039, 26552805, 4619373, 8416548, 4695785, 5065056, 4659595, 15453952, 15450833, 4802535, 4958220, 6190188, 3407242, 9443057, 4511246, 9985680, 39589669, 16198366, 4500172, 5356169408, 4692352, 10457745, 10279628, 16759258, 32427984, 4810304, 24140808, 35179289, 115589525, 20753794, 4617608, 4696657, 7913280, 38618146, 6335638, 13585325, 16227497, 12885161, 6926652, 4848467, 5460184, 29890855, 4710401, 5764053, 6985680, 4226162, 4200296, 6790368, 11454023, 4220217, 12648180, 6091663, 16679500, 6594886, 4226406, 4215101, 4266176, 5528889, 6561792, 9687030, 12108639, 16679500, 5173292, 97352926, 15188334, 21238738, 5538448, 36980875, 4582694, 4199132, 20270174, 21585344, 4807844, 591344518, 6434813, 4891518, 4214610, 39784178, 4237109, 133616, 5073634, 4865093, 4199564, 5528681, 14900355, 9979136, 24554293, 4198944, 140512538, 47887618, 17200575, 35511405, 81206032, 35283554, 29819272, 24524849, 25895255, 77574968, 31648927, 76814512, 49200409, 20814331, 60728203, 63636768, 40958416, 30270111, 80243843, 35454499, 81206032, 31710803, 33273069, 24524849, 10276556, 57361105, 38833931, 126391992, 11250048, 4729980, 4216848, 4778986, 14533371, 4365344, 5156144, 4797596, 4263760, 4676976, 5179137, 13841627, 6832148, 4279984, 4593198, 5422225, 7073684, 4293632, 4240321, 4889524, 5272572, 6013484, 4209824, 4279360, 4283456, 4209776, 4280816, 4272576, 4274432, 4214268, 4302752, 4298752, 4282944, 9251872, 4513910, 4516500, 4477757, 4480212, 8219448, 39473140, 90347520, 19689214, 35195135, 19939988, 21968404, 21600590, 554231599, 4305665696, 38758067, 20565552, 4294336, 15836081, 12248444, 16031232, 12627184, 16031232, 13931418, 16031232, 5001510, 11465326, 11822964, 12005964, 4210709, 11158657, 16096227, 15218482, 5423648, 12247188, 12247177, 38894738, 19754961, 20420284, 25353297, 19664221, 24545151, 20216427, 24822053, 4857971, 10045454, 10674218, 4805916, 6659624, 4341761, 4244816, 6542216, 12441264, 6154268, 4249007, 2494631, 12315435, 13002362, 4228607, 10903412, 9232700, 12678836, 6794520, 13224152, 16123968, 15342035, 9764976, 13911168, 4561607, 10882889, 7135686, 16324125, 5136385, 4212382, 13164860, 9106876, 16590481, 4231930, 5002578, 5317365, 4672385, 16536768, 4335691, 4313853, 4637882, 4249007, 13989516, 5394588, 4892280, 12522176, 6567728, 7182916, 4995168, 4347609, 14944240, 5161744, 11352290, 7524975, 15151630, 4525789, 4319461, 4525787, 7388720, 7568336, 4674385, 4874144, 4769266, 26344789, 19723652, 109808929, 18302080, 16806360, 19734577, 60743264, 20650451, 43680384, 20961638, 5356161220
        ]);
        V = new Set([340, 33671, 60334, 4239304, 4261625, 4219717, 22090]);
        U = new Set([16015165, 5481776, 5673464, 5117776, 4298619, 5248305, 4591013, 5391456, 4838160, 4974044, 6132800, 11268272, 14345732, 4800079, 14345732, 5114149, 5091185, 4927249, 4234018, 4971922, 1646852, 6661186, 5907328, 4382721, 5331895, 5810064, 4203762, 5094305, 4828032, 10263248, 5091665, 4665333, 4817493, 4884097, 4817141, 4685681, 5142016, 4624640, 5658512, 5777305, 5399879, 4814081, 5026800, 5028060, 4660336, 4822165, 10193293, 4679877, 4487717, 15085978, 12419846, 4657184, 9886295, 13358643, 10525138, 15718495, 15085978, 4220263, 5631192, 4212593, 6013152, 4686853, 4210143, 4922049, 3023108, 5217757, 8765912, 5904017, 7645835, 4786129, 14650712, 4198424, 6426383, 5328810, 5366624, 5997504, 5991120, 4568629, 6930393, 4594149, 9356679, 5466794, 5658512, 15757551, 5333194, 5338010, 5029381, 5586352, 5829088, 5890640, 5673464, 4287134, 5586352, 4870729, 4480542, 5666952, 4745509, 4246691, 5926212, 5455280, 4319012, 5678024, 4656464, 6473980, 14704563, 4591488, 4651712, 4246691, 4915952, 5204121, 7050144, 9062027, 5604160, 5948368, 14086188, 7436607, 5027740, 5107712, 5536727, 4880387, 7787697, 6699822, 4771451, 6456980, 4597224, 7511121, 4985786, 5584945, 5225976, 4986211, 15994448, 4693761, 7118849, 4986643, 4268093, 5575700, 4911529]);
        Y = new Set([43223349, 52101755, 17306037, 61068748, 32853344, 24727171, 53121289, 20316950, 33998610, 61027478, 40130541, 25123243, 61184651, 60642936, 24635078, 17391028, 61206599, 59947249, 60951175, 23667972, 19686401, 68242415, 35573911, 60705076, 70624485, 20583052, 20396686, 247115667, 60997630, 43798006, 21380077, 28933661, 30447745, 47146748, 20396686, 17976632, 18811408, 18433175, 60869209, 61020862, 32893741, 27913081, 65213934, 37922832, 43506532, 19775600, 59659346, 17987011, 43236425, 59654706, 59539388, 59329565, 59895777, 39157081, 20365752, 28933661, 59141609, 39328116, 38409639, 59654706, 35652540, 125830820, 37068282, 39889718, 57983235, 126627813, 57783327, 18643987, 58438325, 65222884, 125153682, 21363611, 43136310, 33454866, 59178459, 58409379, 60415107, 32396799, 83407207, 83333969, 62159713, 30400210, 31615463, 53870530, 85035315, 19484108, 23704498, 29661512, 75277964, 21088218, 20399515, 43571080, 37897964, 60707278, 36667480, 61543961, 31394033, 26689852, 22732762, 25085020, 51742325, 43303345, 58220704, 24162270, 17030864, 61325608, 25110520, 21103202, 107987657, 133213505, 58187436, 51892392, 27562595, 126391992, 33454866, 30672338, 48135092, 23782178, 35079135, 46425501, 125143111, 17008176, 33594511, 44978650, 25597413, 25290147, 25231805, 22371516, 19084105, 17897759]);
        J = new Set(["WTWindow", "_EL_HideOwner", "IME", "MSCTFIME", "MSCTFIME UI", "tooltips_class32", "FORM_PROJECT1_FORM1_CLASS:0", "QQPinyinImageCandWndTSF", "VBBubbleRT6", "AutoIt v3 GUI", "CiceroUIWndFrame", "Qt5QWindowIcon", "QWidget"]);
        constructor() {
            super(), this.G = u.instance
        }
        Before() {
            try {
                this.G.update()
            } catch (t) {
                o.instance.Report(this.t, !0, `${t}`)
            }
        }
        Doimpl() {
            if (!this.G.processWindows) return;
            let e = "";
            for (let [i, s] of this.G.processWindows)
                if (!(i <= 4))
                    for (let [r, l, n, h] of s) {
                        const s = this.G.getHwndWndProc(h);
                        if (s && this.H.has(s)) {
                            e = `发现封包、多倍外挂，进程为:${i}|${this.G.getProcessName(i)}|${r}|${s.toString(16)}`, o.instance.Kick(9e5);
                            break
                        }
                        if (i == t.get_current_process_id() && s && this.H.has(65535 & s)) {
                            e = `发现定制外挂，请封号处理，进程为:${i}|${this.G.getProcessName(i)}|${r}|${l}`, o.instance.Kick(9e5);
                            break
                        }
                        if (s && 52160 == (65535 & s)) {
                            const t = "fff|43|3a".split("|");
                            let s = !1;
                            this.G.getProcessWindows(i) ? .forEach((([e, i, r, l]) => {
                                let n = this.G.getHwndWndProc(l);
                                if (n) {
                                    let e = n.toString(16);
                                    s = s || e.startsWith(t[0])
                                }
                            }));
                            let n = !1;
                            this.G.getProcessThreads(i) ? .forEach((([e, i]) => {
                                let s = i.toString(16);
                                n = n || s.startsWith(t[1])
                            }));
                            let h = !1;
                            if (n && this.G.getProcessModules(i) ? .forEach((([e, i, s]) => {
                                let r = s.toString(16);
                                e.includes(".exe") && (h = h || r.startsWith(t[2]))
                            })), s && n && h) {                                
                                e = `发现定制外挂2，请封号处理，进程为:${i}|${this.G.getProcessName(i)}|${r}|${l}`, o.instance.Kick(1500);
                                break
                            }
                        }
                        if ("" == this.G.getProcessName(i) || 10 != l.length && !this.J.has(l)) continue;
                        const n = this.G.processThreads ? .get(i);
                        if (n) {
                            for (let [t, s] of n) {
                                if (("tooltips_class321" == l || "MSCTFIME1" == l) && (this.V.has(s) || this.V.has(16777215 & s))) {
                                    e = `发现GEE猎手或者荣耀外挂请封号处理【1号特征】:${i}|${t}|${(1048575&s).toString(16)}`;
                                    break
                                }
                                let r = this.G.getAddressModule(i, s);
                                if ("_EL_HideOwner" == l && (this.U.has(16777215 & s) || this.Y.has(268435455 & s))) {
                                    e = `发现GEE倍攻、脱机软件，进程为:${i}|${t}|${(16777215&s).toString(16)}`;
                                    break
                                }
                                if ("" != r) {
                                    if (r.search(/.exe$/) > 1) {
                                        if ("QQPinyinImageCandWndTSF" == l && 12320 == (65535 & s)) {
                                            e = `GEE大师外挂请封号处理【3号特征】，进程为:${i}|${t}|${(65535&s).toString(16)}`;
                                            break
                                        }
                                        if ("AutoIt v3 GUI" == l && 32778 == (65535 & s)) {
                                            e = `发现一键小退软件，进程为:${i}|${t}|${(65535&s).toString(16)}`;
                                            break
                                        }
                                        if ("FORM_PROJECT1_FORM1_CLASS:0" == l && 8790 == (65535 & s)) {
                                            e = `发现猎手或者荣耀外挂请封号处理【4号特征】，进程为:${i}|${t}|${(65535&s).toString(16)}`;
                                            break
                                        }
                                        if ("MSCTFIME UI" == l && 43611 == (65535 & s)) {
                                            e = `发现简单A版加强版:${i}|${t}|${(65535&s).toString(16)}`;
                                            break
                                        }
                                        if ("CiceroUIWndFrame" == l && 6992 == (65535 & s)) {
                                            o.instance.Report(this.t, !0, `发现脱机软件，进程为:${i}|${t}|${(65535&s).toString(16)}`);
                                            break
                                        }
                                        if ("tooltips_class32" == l && (55493 == (65535 & s) || 35647 == (65535 & s))) {
                                            e = `玩家正在使用虚拟机或多开器，进程为:${i}|${t}|${(65535&s).toString(16)}`;
                                            break
                                        }
                                        if ("_EL_HideOwner" == l && (57734 == (65535 & s) || 65003 == (65535 & s) || 65067 == (65535 & s) || 58134 == (65535 & s) || 22782 == (65535 & s) || 23038 == (65535 & s) || 33671 == (65535 & s) || 35482 == (65535 & s) || 8253 == (65535 & s) || 4207632 == (16777215 & s) || 10804163 == (16777215 & s) || 5183494 == (16777215 & s) || 5309039 == (16777215 & s) || 9229 == (65535 & s))) {
                                            e = `发现倍攻、脱机软件，进程为:${i}|${t}|${(65535&s).toString(16)}`;
                                            break
                                        }
                                    } else if (r.search(/.dll$/) > 1) {
                                        if ("_EL_HideOwner" == l && (73801816 == (16777215 & s) || 11108590 == (16777215 & s) || 4260479 == (16777215 & s) || 770988 == (1048575 & s) || 10287 == (65535 & s) || 749483 == (1048575 & s) || 39191 == (65535 & s) || 58006 == (65535 & s) || 58134 == (65535 & s) || 5448 == (65535 & s) || 817 == (65535 & s) || 52101 == (65535 & s) || 57808 == (65535 & s) || 22858 == (65535 & s) || 22090 == (65535 & s) || 43575 == (65535 & s) || 9056 == (65535 & s) || 14664 == (65535 & s) || 9301 == (65535 & s) || 11025 == (65535 & s) || 8755 == (65535 & s))) {
                                            e = `发现脱机或倍攻软件,进程为:${i}|${t}|${(65535&s).toString(16)}`;
                                            break
                                        }
                                        if (r.search(/.dll$/) > 1 && "WTWindow" == l && (22858 == (65535 & s) || 22090 == (65535 & s))) {
                                            e = `发现GEE倍攻软件,进程为:${i}|${t}|${(65535&s).toString(16)}`;
                                            break
                                        }
                                        if ("VBBubbleRT6" == l && 1029121 == (1048575 & s)) {
                                            e = `定制外挂请封号处理，进程为:${i}|${t}|${(1048575&s).toString(16)}`;
                                            break
                                        }
                                    } else if (r.search(/.tap$/) > 1) {
                                        if (56607 == (65535 & s) || 22768 == (65535 & s)) {
                                            e = `发现荣耀外挂请封号处理,进程为【7号特征】:${i}|${t}|${(65535&s).toString(16)}`;
                                            break
                                        }
                                    } else if (r.search(/.dat$/) > 1 && "_EL_HideOwner" == l && (5685248 == (16777215 & s) || 9834496 == (16777215 & s) || 11271589 == (16777215 & s) || 40180 == (65535 & s) || 9757169 == (16777215 & s) || 5175102 == (16777215 & s) || 15072527 == (16777215 & s) || 9424896 == (16777215 & s) || 4620329 == (16777215 & s) || 9918941 == (16777215 & s))) {
                                        e = `发现倍攻外挂或脱机外挂，进程为:${i}|${t}|${(16777215&s).toString(16)}`;
                                        break
                                    }
                                } else if (this.V.has(65535 & s) || this.V.has(1048575 & s)) {
                                    e = `多倍外挂【2号特征】:${i}|${t}|${(65535&s).toString(16)}`;
                                    break
                                }
                            }
                            if ("" != e) break
                        }
                    }
                if ("" == e) {
                    if (!this.G.processThreads) return;
                    for (let [t, i] of this.G.processThreads)
                        if (!(t <= 4))
                            for (let [s, r] of i) {
                                if (this.N.has(r)) {
                                    e = `外挂程序为:${t}|${s}|${(65535&r).toString(16)}`;
                                    break
                                }
                                let i = this.G.getAddressModule(t, r);
                                if ("" != i)
                                    if (i.search(/.exe$/) > 1) {
                                        if (this.R.has(65535 & r)) {
                                            e = `发现大师、定制类外挂，进程为:${t}|${s}|${(65535&r).toString(16)}`;
                                            break
                                        }
                                    } else if (i.search(/.dat$/) > 1 && this.O.has(65535 & r)) {
                                    e = `发现定制类脱机外挂，进程为:${t}|${s}|${(65535&r).toString(16)}`;
                                    break
                                }
                            }
                }
                "" != e && (o.instance.Report(this.t, !0, e), e = "")
        }
        Do() {
            try {
                this.Doimpl()
            } catch (t) {
                return void o.instance.Report(this.t, !0, `${t}`)
            }
        }
        After() {}
    }
    class w extends l {
        t = 9052;
        l = 9053;
        K = 9e5;
        q = ["VmLoader"];
        j = ["IINDLCPXGO", "LoveSnow", "PCCKJCA4", "Dult", "Dultx64_Protect", "GNLAKBOZYKOYCKB", "BBBBas", "FengDrv2787", "SM762FE", "vmx_fb", "vm3dmp", "nvd3dum", "nv3dmp", "HideToolz", "wujiejiami", "Sp_Hs", "Passkpp_Demo", "SuperSpeedx64", "SpeedHook", "Gwken", "yxbsq", "mengwuji", "Win7Speed", "wwE21wwE", "lonerSpeed_v40", "LtqDrv", "uMcg_x64", "abc2.0"];
        X = [];
        Z = [];
        getPdbCached(e) {
            if (globalThis.pdbCache || (globalThis.pdbCache = new Map), globalThis.pdbCache.has(e)) return globalThis.pdbCache.get(e) || "";
            try {
                const i = t.get_pdb_path(e);
                return globalThis.pdbCache.set(e, i), i
            } catch (t) {
                return ""
            }
        }
        Before() {
            this.X = t.enum_device()
        }
        Do() {
            this.X.forEach((t => {
                if (t.split(/[a-zA-Z]:\\/).length > 1) {
                    let e = t.toLowerCase().replace("system32", "sysnative");
                    e.includes("idbgdrv.sys") && o.instance.Report(this.t, !0, "非法驱动正在运行:" + t);
                    let i = this.getPdbCached(e);
                    this.q.forEach((e => {
                        i.includes(e) && o.instance.Report(this.t, !0, "非法驱动正在运行:" + t)
                    }))
                }
                this.j.forEach((e => {
                    t.includes(e) && o.instance.Report(this.t, !0, "非法驱动正在运行:" + t)
                })), t.includes("VEN_15AD") && (o.instance.Report(this.l, !0, "虚拟机设备:" + t), o.instance.Kick(this.K))
            }))
        }
        After() {}
    }
    class S extends l {
        G;
        t = 9055;
        O = new Set([63900, 15738, 34343]);
        tt = new Set([30107]);
        et = new Set(["dm.dll", "XYDsoftWpe.dll", "sp.dll", "hardsp.dll", "softsp.dll"]);
        R = new Set([994315]);
        H = new Set([4707357, 5529687, 71555970, 323761184, 4931527, 4796006]);
        N = new Set([45215100, 4446544, 55173609, 55055188, 5510182, 118382978, 6406188, 5028156]);
        V = new Set([15807, 41485, 8717]);
        J = new Set(["_EL_HideOwner", "MSCTFIME", "MSCTFIME UI", "tooltips_class32", "FORM_PROJECT1_FORM1_CLASS:0", "QQPinyinImageCandWndTSF", "VBBubbleRT6", "AutoIt v3 GUI"]);
        it = new Set(["_iext3_CTipWnd", "_06", "SoPY_UI"]);
        st = new Set(["dnf.exe", "client.exe"]);
        constructor() {
            super(), this.G = u.instance
        }
        Before() {}
        Do() {
            if (!this.G.processWindows) return;
            let e = "";
            const i = t.get_current_process_id();
            for (let [s, r] of this.G.processWindows)
                if (!(s <= 4))
                    for (let [l, n, h, a] of r) {
                        const r = this.G.getHwndWndProc(a);
                        if ("基址初始化" == l && r) {
                            e = `发现定制外挂，请封号处理，进程为:${s}|${this.G.getProcessName(s)}|${l}|${n}|${r.toString(16)}`;
                            break
                        }
                        if (r && this.H.has(r)) {
                            e = `发现定制外挂，请封号处理，进程为:${s}|${this.G.getProcessName(s)}|${l}|${n}|${r.toString(16)}`;
                            break
                        }
                        if (s == t.get_current_process_id() && r && this.H.has(65535 & r)) {
                            e = `发现定制外挂，请封号处理，进程为:${s}|${this.G.getProcessName(s)}|${l}|${n}|${r.toString(16)}`;
                            break
                        }
                        if (n.includes("_EL_HideOwner"))
                            if (s == i) {
                                let e = t.get_wnd_proc(a),
                                    i = this.G.getAddressModule(s, e);
                                i.includes("PlugClient") || i.includes("gxxcx") || i.includes("HTPlugin") || i.includes("lfm2cx") || i.includes("Sx_gee") || i.includes("TCPlugin_GEE") || i.includes("v8m2cx") || 11969 == (65535 & e) || 6273 == (65535 & e) || 61571 == (65535 & e) || 61763 == (65535 & e) || 3587 == (65535 & e) || 63506 == (65535 & e) || 530 == (65535 & e) || 786 == (65535 & e) || 28756 == (65535 & e) || 54467 == (65535 & e) || o.instance.Report(9057, !0, `检测到游戏易语言: ${i}|${e.toString(16)}`)
                            } else {
                                let e = this.G.processThreads ? .get(s);
                                if (e)
                                    for (let [i, r] of e)
                                        if (0 == (4095 & r)) {
                                            let e = this.G.getAddressModule(s, r);
                                            if ("" == e) continue;
                                            if (e == i) continue;
                                            e.search(/.exe$/) > 1 && 0 == this.st.has(i.toLowerCase()) && t.report(this.t, !0, `驱动加速伪装:${i}|${r.toString(16)}|${e}`)
                                        }
                            }
                        if ("" == this.G.getProcessName(s) || !this.J.has(n) && 10 != n.length) continue;
                        let h = this.G.processThreads ? .get(s);
                        if (h)
                            for (let [t, i] of h) {
                                if (("tooltips_class32" == n || "MSCTFIME" == n) && (this.V.has(i) || this.V.has(16777215 & i))) {
                                    e = `发现GEE猎手或者荣耀外挂请封号处理【1号特征】，进程为:${s}|${t}|${i.toString(16)}`;
                                    break
                                }
                                if ("csrss.exe" == t && (n = "_EL_HideOwner")) {
                                    e = `发现非法注入，进程为:${s}|${t}|${n}|${i.toString(16)}`;
                                    break
                                }
                                let r = this.G.getAddressModule(s, i);
                                if ("" != r)
                                    if (r.search(/.exe$/) > 1) {
                                        if ("QQPinyinImageCandWndTSF" == n && 12320 == (65535 & i)) {
                                            e = `发现GEE大师外挂请封号处理【3号特征】，进程为:${s}|${t}|${i.toString(16)}`;
                                            break
                                        }
                                        if ("VBBubbleRT6" == n && 4824022 == (16777215 & i)) {
                                            e = `发现GEE大师外挂请封号处理【4号特征】，进程为:${s}|${t}|${i.toString(16)}`;
                                            break
                                        }
                                        if ("#32770" == n && (1038511 == (1048575 & i) || 131072 == (1048575 & i))) {
                                            e = `发现GEE大师外挂请封号处理【5号特征】，进程为:${s}|${t}|${i.toString(16)}`;
                                            break
                                        }
                                        if ("FORM_PROJECT1_FORM1_CLASS:0" == n && 8790 == (65535 & i)) {
                                            e = `发现GEE猎手或者荣耀外挂请封号处理【6号特征】，进程为:${s}|${t}|${i.toString(16)}`;
                                            break
                                        }
                                        if ("_iext3_CTipWnd" == l && 3056 == (65535 & i)) {
                                            e = `发现GEE猎手或者荣耀外挂请封号处理【7号特征】，进程为:${s}|${t}|${i.toString(16)}`;
                                            break
                                        }
                                        if ("_EL_HideOwner" == n && (48640 == (65535 & i) || 34343 == (65535 & i) || 65436 == (65535 & i))) {
                                            e = `发现GEE猎手或者荣耀外挂请封号处理【8号特征】，进程为:${s}|${t}|${i.toString(16)}`;
                                            break
                                        }
                                    } else if (r.search(/.dll$/) > 1) {
                                    if ("MSCTFIME UI" == n && (29295 == (65535 & i) || 11369779473 == (65535 & i))) {
                                        e = `发现GEE猎手或者荣耀外挂请封号处理【9号特征】，进程为:${s}|${t}|${i.toString(16)}`;
                                        break
                                    }
                                } else if (r.search(/.IME$/) > 1) {
                                    if ("_EL_HideOwner" == n && 49856 == (65535 & i)) {
                                        e = `发现脱机回收外挂，进程为:${s}|${t}|${i.toString(16)}`;
                                        break
                                    }
                                } else if (r.search(/.tmp$/) > 1) {
                                    if (4198400 == (16777215 & i)) {
                                        e = `发现加速外挂请封号处理，进程为【7号特征】:${s}|${t}|${i.toString(16)}`;
                                        break
                                    }
                                } else {
                                    if (r.search(/.tap$/) > 1 && (56607 == (65535 & i) || 22768 == (65535 & i) || 4198400 == (16777215 & i))) {
                                        e = `发现荣耀外挂请封号处理，进程为【7号特征】:${s}|${t}|${i.toString(16)}`;
                                        break
                                    }
                                    if (r.search(/.dat$/) > 1 && 21808 == (65535 & i)) {
                                        e = `发现脱机挂请封号处理，进程为【八号特征】:${s}|${t}|${i.toString(16)}`;
                                        break
                                    }
                                } else if (this.V.has(65535 & i) || this.V.has(1048575 & i)) {
                                    e = `发现GEE猎手或者荣耀外挂请封号处理【2号特征】，进程为:${s}|${t}|${i.toString(16)}`;
                                    break
                                }
                            }
                        if ("" != e) break
                    }
                if (this.G.processThreads ? .forEach(((t, e) => {
                    e <= 4 || t.forEach((([t, i]) => {
                        null == this.G.getProcessWindows(e) ? .length && (5707711 != i && 4215068 != i || o.instance.Report(9023, !0, `发现定制脱机外挂，请封号处理，进程为:${e}|${t}|${i.toString(16)}|${this.G.getProcessWindows(e)?.length}`))
                    }))
                })), "" == e) {
                    if (!this.G.processThreads) return;
                    for (let [t, i] of this.G.processThreads)
                        if (!(t <= 4))
                            for (let [s, r] of i) {
                                if (this.N.has(r)) {
                                    e = `发现封包工具或者外挂！，进程为:${t}|${s}|${r.toString(16)}`;
                                    break
                                }
                                let i = this.G.getAddressModule(t, r);
                                if ("" != i)
                                    if (i.search(/.exe$/) > 1) {
                                        if (this.R.has(65535 & r)) {
                                            e = `发现大师、定制类外挂，进程为:${t}|${s}|${r.toString(16)}`;
                                            break
                                        }
                                    } else if (i.search(/.dll$/) > 1) {
                                    if (this.et.has(i)) {
                                        o.instance.Report(9057, !0, `发现易语言外挂，进程为:${t}|${s}|${i}`);
                                        break
                                    }
                                } else if (i.search(/.yz$/) > 1) {
                                    if (this.tt.has(65535 & r)) {
                                        o.instance.Report(9023, !0, `发现易语言加速程序，进程为:${t}|${s}|${i}`);
                                        break
                                    }
                                } else if (i.search(/.dat$/) > 1 && this.O.has(65535 & r)) {
                                    e = `发现定制类脱机外挂，进程为:${t}|${s}|${r.toString(16)}`;
                                    break
                                }
                            }
                }
                "" != e && (o.instance.Report(this.t, !0, e), e = "")
        }
        After() {}
    }
    class k extends l {
        t = 9059;
        G;
        constructor() {
            super(), this.G = u.instance
        }
        Before() {}
        Do() {
            let [t, e, i] = this.G.getNoAccessProcessRate();
            i > .8 && o.instance.Report(9011, !0, `系统异常！推荐重新安装系统! 错误代码为！ ${e}/${t} ${i}`)
        }
        After() {}
    }
    class g extends l {
        t = 9062;
        l = 9022;
        rt = 9021;
        G;
        lt = new Set([274432, 352256]);
        constructor() {
            super(), this.G = u.instance
        }
        Before() {}
        Do() {
            const e = this.G.processWindows;
            if (!e) return;
            let i = new Set;
            for (let [t, s] of e)
                for (let e of s) {
                    let [s, r, l, n] = e;
                    "_EL_HideOwner" == r && i.add(t)
                }
            let s = new Map,
                l = new Set;
            for (let [t, r] of e) {
                let e = 0;
                for (let n of r)
                    if (i.has(t)) {
                        let [i, r, o, h] = n;
                        "tooltips_class32" == r && (s.has(t) ? s.set(t, e) : s.set(t, 1), e++), "_iext3_CTipWnd" == i && l.add(t)
                    }
            }
            const n = t.get_current_process_id();
            for (let [t, s] of e) {
                for (let e of s) {
                    let [s, l, n, h] = e, a = this.G.getHwndProps(h);
                    if (a && a.has("Ex_Wnd_Control")) {
                        if (i.has(t)) break;
                        o.instance.Report(this.l, !0, `geels|wnd|${t}|${this.G.getProcessName(t)}|${s}|${l}(${r(l)})`);
                        break
                    }
                }
                if (t == n) {
                    const e = this.G.getProcessModules(t);
                    if (e)
                        for (let [i, s, r] of e) {
                            "hook32.dll" == i.toLowerCase() && o.instance.Report(this.rt, !0, `hook多倍外挂.一定要封号|${t}|${this.G.getProcessName(t)}`)
                        }
                } else {
                    const e = this.G.getProcessModules(t);
                    if (0 == t && s.length > 0) {
                        let e = "";
                        for (let t of s) {
                            let [i, s, r, l] = t;
                            e += `${i}(${l})|`
                        }
                        0 == e.includes("Build") && 0 == e.includes("NVOGLDC") && 0 == e.includes("Default IME") && 0 == e.includes("背包仓库管理") && o.instance.Report(this.t, !0, `pid0|${t}|${this.G.getProcessName(t)}|${e}`)
                    }
                    if (e) {
                        if (e.length > 0) {
                            let [i, s, r] = e[0];
                            i.toLowerCase().includes("ntdll.dll") && o.instance.Report(this.t, !0, `ntdll|module|${t}|${this.G.getProcessName(t)}|${i}|${r.toString(16)}`)
                        }
                        for (let [i, s, r] of e) {
                            i.toLowerCase().includes("system32\\nfapi.dll") && (this.lt.has(r) ? o.instance.Report(this.rt, !0, `多倍外挂！一定要封号|${t}|${this.G.getProcessName(t)}`) : o.instance.Report(this.t, !0, `nfapi|module|${t}|${this.G.getProcessName(t)}|${i}|${r.toString(16)}`))
                        }
                    }
                }
            }
        }
        After() {}
    }
    class I extends l {
        t = 9011;
        l = 9011;
        rt = 9011;
        nt = 9056;
        K = 9e5;
        ot = [];
        ht = [];
        Before() {
            this.ot = t.enum_pdb(), this.ht = t.get_cur_module_list(), null == globalThis.adapter_hash && (globalThis.adapter_hash = [!1, 0, 0, ""])
        }
        detectPdb() {
            this.ot.forEach((t => {
                if (t.includes("bora-") || t.includes("wddm\\i386") || t.includes("wddm\?")) return o.instance.Report(this.t, !0, "发现虚拟机环境:" + t), void o.instance.Kick(this.K);
                let e = t.replace(/.*\\|\..*$/gi, "");
                return e.includes("vm3d") ? (o.instance.Report(this.t, !0, "发现虚拟机环境:" + e), void o.instance.Kick(this.K)) : void 0
            }))
        }
        calcUint8ArrayCrossEntropy(t) {
            const e = new Map,
                i = t.length;
            for (let s = 0; s < i; s++) {
                const i = t[s];
                if (e.has(i)) {
                    let t = e.get(i);
                    t && e.set(i, t + 1)
                } else e.set(i, 1)
            }
            let s = 0;
            for (let [t, r] of e) {
                const t = r / i;
                s -= t * Math.log2(t)
            }
            return s
        }
        resolveInstruction(e) {
            let i = t.read_bytes(e, 1);
            return this.isJmpInstruction(i) ? e + 5 + t.read_dword(e + 1) : e
        }
        isJmpInstruction(t) {
            return 233 == t[0] || 235 == t[0] || 232 == t[0]
        }
        isStandardFunctionHeader(t) {
            return 85 == t[0] && 139 == t[1] && 236 == t[2] || 85 == t[2] && 139 == t[3] && 236 == t[4]
        }
        detectSandbox() {
            let e = t.get_proc_address("Iphlpapi.dll", "IcmpSendEcho2");
            if (e) {
                233 == t.read_bytes(e, 1)[0] && o.instance.Report(9004, !0, "发现沙盒脱机多倍外挂")
            }
        }
        detectAdapterSig() {
            const e = this.ht;
            for (let i = 0; i < e.length; i++) {
                let [s, r, l] = e[i], n = t.get_proc_address(s, "OpenAdapter");
                if (n) {
                    if ("function" == typeof t.calc_module_hash)
                        do {
                            if (void 0 === globalThis.adapter_hash) break;
                            0 == globalThis.adapter_hash[0] && (globalThis.adapter_hash = t.calc_module_hash(s));
                            let e = !1,
                                i = globalThis.adapter_hash[1],
                                r = globalThis.adapter_hash[2],
                                l = globalThis.adapter_hash[3],
                                /**
                                0x25BCA5AB,
                                0xC6B38108,
                                0x26B3416B,
                                0x2CC63,
                                0x7862D57F,
                                0x2AA75B5,
                                0x4BFA10CD,
                                0xCD495ACD,
                                0x6AB303C3,
                                0x6CECC0D3,
                                0xAF5EBD34,
                                 * /
                                n = new Set([633120171, 3333652744, 649281899, 183395, 2019743103, 44725685, 1274679501, 3444136653, 1790116803, 1827455187, 2942221620]),
                                h = new Set([2942221620]);
                            (i > 1900 || "" == l) && (e = !0);
                            let a = `${s}|${globalThis.adapter_hash[0]}|${globalThis.adapter_hash[1]}|${globalThis.adapter_hash[2].toString(16)}|${globalThis.adapter_hash[3]}`;
                            e && o.instance.Report(this.nt, !0, `发现非正常玩家环境1|${a}`), h.has(r) && o.instance.Report(this.nt, !0, `发现非正常玩家环境2|${a}`), n.has(r) && o.instance.Report(this.t, !0, "正在使用虚拟机的玩家")
                        } while (0);
                    let e = t.scan(n, 256, [232, 204, 204, 204, 204, 185, 204, 204, 204, 204, 232, 204, 204, 204, 204]);
                    e.length && "AdapterShimPath" == t.read_wstring(t.read_dword(e[0] + 6)) && (o.instance.Report(this.t, !0, "发现虚拟机环境"), o.instance.Kick(this.K));
                    let i = t.scan(n, 256, [36, 1, 104, 204, 204, 204, 204, 162]);
                    i.length && "AdapterShimPath" == t.read_wstring(t.read_dword(i[0] + 3)) && (o.instance.Report(this.t, !0, "发现虚拟机环境"), o.instance.Kick(this.K)), t.scan(n, 512, [199, 65, 20, 3, 32, 0, 0, 137, 48, 139, 81, 4, 137, 80, 4, 139, 81, 8, 137, 80, 8, 139, 73, 12]).length && (o.instance.Report(this.t, !0, "发现虚拟机环境"), o.instance.Kick(this.K)), t.read_string(r + 784).includes("yoda") && (o.instance.Report(this.t, !0, "发现虚拟机环境"), o.instance.Kick(this.K));
                    let l = t.read_string(r + 904),
                        h = t.read_string(r + 776);
                    (l.includes("winlice") || h.includes("winlice")) && (o.instance.Report(this.t, !0, "发现虚拟机环境5"), o.instance.Kick(this.K)), t.read_string(r + 816).includes("themida") && (o.instance.Report(this.t, !0, "发现虚拟机环境6"), o.instance.Kick(this.K));
                    let a = this.resolveInstruction(n),
                        c = t.read_bytes(a, 50);
                    if (!this.isStandardFunctionHeader(c)) {
                        this.calcUint8ArrayCrossEntropy(c);
                        if (86 == c[0] && 190 == c[1]) {
                            c[0].toString(16), c[1].toString(16), c[2].toString(16), c[3].toString(16), c[4].toString(16);
                            o.instance.Report(this.rt, !0, "发现虚拟机环境7")
                        }
                    }
                }
            }
        }
        Do() {
            this.detectPdb(), this.detectAdapterSig(), this.detectSandbox()
        }
        After() {}
    }
    class y extends l {
        t = 9997;
        constructor() {
            super()
        }
        Before() {
            globalThis.playername = t.get_player_name()
        }
        randomStr(t, e) {
            let i = "";
            const s = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789",
                r = Math.floor(Math.random() * (e - t + 1)) + t;
            for (let t = 0; t < r; t++) i += s.charAt(Math.floor(Math.random() * s.length));
            return i
        }
        Do() {
            globalThis.playername && o.instance.reportImmediately(this.t, !1, `${this.randomStr(10,20)}|${globalThis.playername}`)
        }
        After() {}
    }
    class E extends l {
        t = 9064;
        G;
        constructor() {
            super(), this.G = u.instance
        }
        Before() {}
        hasRemoteDesktop() {
            const t = this.G.processWindows;
            if (!t) return !1;
            for (let [e, i] of t) {
                let t = this.G.getProcessName(e);
                if ("SunloginClient.exe" == t || "ToDesk.exe" == t) return !0
            }
            return !1
        }
        vkToString(t) {
            switch (t) {
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
            return t.toString(16)
        }
        Do() {}
        After() {}
    }
    class M extends l {
        t = 9065;
        ct = ["PatchGuard"];
        constructor() {
            super()
        }
        Before() {}
        Do() {
            if ("function" == typeof t.get_bcd_info) {
                let [e, i, s] = t.get_bcd_info();
                s && o.instance.Report(this.t, !1, `bcd|${e}|${i}|${s}`), s && this.ct.forEach((t => {
                    -1 != i.indexOf(t) && o.instance.Report(this.t, !0, `black_bcd|${e}|${i}|${s}`)
                }))
            }
        }
        After() {}
    }
    class x extends l {
        t = 9996;
        constructor() {
            super()
        }
        Before() {
            "function" == typeof t.get_hwid_md5 && (globalThis.m2_mch_id = t.get_hwid_md5())
        }
        randomStr(t, e) {
            let i = "";
            const s = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789",
                r = Math.floor(Math.random() * (e - t + 1)) + t;
            for (let t = 0; t < r; t++) i += s.charAt(Math.floor(Math.random() * s.length));
            return i
        }
        Do() {
            globalThis.m2_mch_id && o.instance.reportImmediately(this.t, !1, `${this.randomStr(20,30)}|${globalThis.m2_mch_id}`)
        }
        After() {}
    }

    function D(t) {
        new Date;
        try {
            t.Before()
        } catch (t) {}
        try {
            t.Do()
        } catch (t) {}
        try {
            t.After()
        } catch (t) {}
        new Date
    }
    console.dbgprint = function(e, ...i) {
        let s = e;
        i.length > 0 && (s += " " + i.join(" ")), t.report(9998, !1, s)
    };
    try {
        let t = [new x, new y, new $, new p, new a, new h, new w, new c, new I, new f, new b, new S, new k, new g, new E, new M];
        for (let e of t) D(e);
        o.instance.reportAll()
    } catch (t) {}
})();