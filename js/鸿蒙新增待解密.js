import * as std from "std";
import * as os from "os";
import * as api from "api";
(() => {
    "use strict";
    let is_detect_cheat = false;
    function hash(str) {
        let hashValue = 5381;
        for (let i = 0; i < str.length; i++) {
            hashValue = (hashValue << 5) + hashValue + str.charCodeAt(i);
        }
        return hashValue >>> 0; // 无符号右移，确保结果为正数
    }
    // 计算字符串的熵
    function calc_entropy(e) {
        const frequencyMap = new Map();
        let entropy = 0;

        // 计算每个字符的频率
        for (let i = 0; i < e.length; i++) {
            const char = e.charAt(i);
            frequencyMap.set(char, (frequencyMap.get(char) || 0) + 1);
        }

        // 计算熵
        for (let entry of frequencyMap.entries()) {
            const frequency = entry[1];
            const probability = frequency / e.length;
            entropy -= probability * Math.log2(probability);
        }

        return entropy;
    }

    //将array的原join方法替换为join_with_hex_number,不影响array的join方法
    function join_with_hex_number(arr, separator) {
        let result = "";
        for (let i = 0; i < arr.length; i++) {
            if (typeof arr[i] === "number") {
                result += arr[i].toString(16);
            } else {
                result += arr[i];
            }
            if (i !== arr.length - 1) {
                result += separator;
            }
        }
        return result;
    }

    class ITask {
		task_id = null;
        before() { }
        do() { }
        after() { }
        // DebugBranch(e = (() => { })) {
        // 	e();
        // }
    }

    class PolicyReport {
        constructor(policyId, isCheat, reason) {
            this.policyId = policyId;
            this.isCheat = isCheat;
            this.reason = reason;
        }
    }

    class PolicyReporter {
        static _instance;

        static get instance() {
            if (!this._instance) {
                this._instance = new PolicyReporter();
            }
            return this._instance;
        }

        _policyQueue = [];

        report(policyId, isCheat, reason, signature = "") {
            if(isCheat == true){
                is_detect_cheat = true;
            }
            // const report = new PolicyReport(policyId, isCheat, reason);
            // this._policyQueue.push(report);
            this.report_immediately(policyId, isCheat, reason, signature);
        }

        // 立即上报
        report_immediately(policyId, isCheat, reason, signature = "") {
            let r = reason;
            if (signature) {
                // 判断signature是否是字符串类型
                if (typeof signature == "string") {
                    r = '【' + signature + "】:" + reason;
                }
                if (typeof signature == "number") {
                    r = '【' + signature.toString(16) + "】:" + reason;
                }
            }
            api.report(policyId, isCheat, r);
        }

        report_all() {
            while (this._policyQueue.length > 0) {
                const report = this._policyQueue.shift();
                if (!report) return;
                api.report(report.policyId, report.isCheat, `${report.reason} (r: ${this._policyQueue.length})`);
            }
        }

        kick(delay = 5000) {
            // os.setTimeout(() => {
            // 	api.kick();
            // 	// 假设 bsod 是一个模拟蓝屏的方法
            // 	api.bsod();
            // 	std.exit(0);
            // }, delay);
        }
    }

    class memory_detect extends ITask {
        mem_map;//map<addr,[image_name, protect, sz]>
        task_id = 689012;
        o = 689059;
        before() {
            this.mem_map = api.enum_memory(-1)
        }
        detectBGDIY() {
            const suspiciousDll = ".\\version.dll";
            if (std.open(suspiciousDll, "r")) {
                PolicyReporter.instance.report(this.task_id, true, `劫持外挂`, suspiciousDll);
                return;
            }

            const suspiciousIndicators = {
                "360base.dll": "360沙箱: 360base.dll",
                "\\Device\\HarddiskVolume3\\Windows\\64.dll": "倍攻外挂",
                sizeAndType: { size: 0x4689000, type: 64, message: "倍攻外挂" }
            };

            //for (let [addr, [image_name, protect, sz]] of this.mem_map) {
            for (let mem of this.mem_map) {
                const addr = mem[0];
                const mem_arr = mem[1];
                const image_name = mem_arr[0];
                const protect = mem_arr[1];
                const sz = mem_arr[2];
                if (image_name.includes("360base.dll")) {
                    PolicyReporter.instance.report(this.task_id, true, suspiciousIndicators["360base.dll"]);
                    return;
                }
                if (image_name === "\\Device\\HarddiskVolume3\\Windows\\64.dll") {
                    PolicyReporter.instance.report(this.task_id, true, suspiciousIndicators["\\Device\\HarddiskVolume3\\Windows\\64.dll"], '\\Device\\HarddiskVolume3\\Windows\\64.dll');
                    return;
                }
                if (image_name !== "" && protect === 64 && sz === 0x49000) {
                    PolicyReporter.instance.report(this.task_id, true, suspiciousIndicators.sizeAndType.message);
                    return;
                }
            }
        }

        detectJianDanWG() {
            const ENHANCED_VERSION_SIGNATURE = 0xb8549c60;
            const OFFLINE_HACK_SIGNATURE = 0x905a4d;
            const MEMORY_BLOCK_SIZE_4K = 0x1000;
            const MEMORY_BLOCK_SIZE_4M = 0x400000;

            //for (let [addr, [image_name, protect, sz]] of this.mem_map) {
            for (let mem of this.mem_map) {
                const addr = mem[0];
                const mem_arr = mem[1];
                const image_name = mem_arr[0];
                const protect = mem_arr[1];
                const sz = mem_arr[2];
                if (image_name === "" && protect === 0x40) {
                    if (sz === MEMORY_BLOCK_SIZE_4K && api.read_dword(addr) === ENHANCED_VERSION_SIGNATURE) {
                        PolicyReporter.instance.report(this.task_id, true, `简单A版增强版`, ENHANCED_VERSION_SIGNATURE);
                        return;
                    } else if (sz >= MEMORY_BLOCK_SIZE_4M && api.read_dword(addr) === OFFLINE_HACK_SIGNATURE) {
                        PolicyReporter.instance.report(this.task_id, true, "简单脱机挂", OFFLINE_HACK_SIGNATURE);
                        return;
                    }
                }
            }
        }
        // 定制的脱机挂
        detectDIYWG() {
            const MEMORY_PROTECTION_CONSTANT = 0x20;
            const MEMORY_REGION_SIZE = 0xe0000;
            const SIGNATURE_1 = 0x90c3c033;
            const SIGNATURE_2 = 0x83ec8b55;

            //for (let [memoryAddress, [imageName, memoryProtection, regionSize]] of this.mem_map) {
            for (let mem of this.mem_map) {
                const memoryAddress = mem[0];
                const mem_arr = mem[1];
                const image_name = mem_arr[0];
                const protect = mem_arr[1];
                const sz = mem_arr[2];
                if (protect === MEMORY_PROTECTION_CONSTANT &&
                    sz === MEMORY_REGION_SIZE &&
                    api.read_dword(memoryAddress) === SIGNATURE_1) {
                    PolicyReporter.instance.report(this.o, true, `倍功 | ${memoryAddress.toString(16)}`, SIGNATURE_1);
                    return;
                }
                if (image_name === "" &&
                    protect === MEMORY_PROTECTION_CONSTANT) {
                    let dwordValue = api.read_dword(memoryAddress);
                    if (dwordValue === SIGNATURE_2) {
                        PolicyReporter.instance.report(this.o, true, `定制脱机挂 | ${memoryAddress.toString(16)} | ${dwordValue.toString(16)}`, SIGNATURE_2);
                        return;
                    }
                }
            }
        }
        do() {
            this.detectJianDanWG();
            this.detectDIYWG();
            this.detectBGDIY();
        }
        after() { }
    }
    class UnknownCheat extends ITask {
        task_id = 689059;
        before() {
            // 初始化checksum数组，如果它还未被初始化
            if (!globalThis.checksum) {
                globalThis.checksum = []; // 使用数组字面量替代new Array
                // 调用api.base()并检查返回值
                const baseResults = api.base();
                if (baseResults && baseResults.length > 0) {
                    if (baseResults[0]) {
                        try{
                            globalThis.checksum.push(new api.UnknownCheat(baseResults[0]));
                        }catch(e){
                            console.log("UnknownCheat error:", e.name, e.message, e.stack); 
                        }
                    }
                    if (baseResults.length > 1 && baseResults[1]) {
                        globalThis.checksum.push(new api.UnknownCheat(baseResults[1]));
                    }
                }
            }
            else {
                for (let t of globalThis.checksum) {
                    if (t.do() == false) {
                        api.report(689059, true, `代码篡改`);
                    }
                }
            }
        }
        do() {
            
        }
        after() { }
    }
    // PE文件图标哈希值
    class pe_ico_hash extends ITask {
        task_id = 689021;
        task_id1 = 689058;
        ico_hast_table1 = new Set([
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
            0x7A56D22F,/*Bloody7 鼠标宏 */
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
        ico_hast_table2 = new Set([
            0x45209245,
            0x23BD5F32,
        ]);
        before() { }
        do() {
            const processHashes = api.enum_process_hash(); // 使用更具描述性的变量名
            //console.log('processHashes: ' + (Date.now() - st) + 'ms');
            for (let p of processHashes) {
                let process_path = p[0];
                let hash = p[1];
                if (this.ico_hast_table1.has(hash)) {
                    PolicyReporter.instance.report(this.task_id, true, `${process_path} | ${hash.toString(16)}`, hash);
                }
                if (this.ico_hast_table2.has(hash)) {
                    PolicyReporter.instance.report(this.task_id1, true, `${process_path} | ${hash.toString(16)}`, hash);
                }
                if (typeof api.is_file_exist === 'function') { // 使用严格的全等操作符进行类型检查
                    const pathParts = process_path.split("\\");
                    const keyFilePath = `${pathParts.slice(0, pathParts.length - 1).join("\\")}\\xw.key`; // 检查进程当前目录下是否存在xw.key文件
                    if (api.is_file_exist(keyFilePath)) {
                        PolicyReporter.instance.report(this.task_id1, true, `${process_path} | ${keyFilePath} | ${hash.toString(16)}`, keyFilePath);
                    }
                }
            }
        }

        after() { }
    }
    class dns_cache_detect extends ITask {
        task_id = 689061;
        dns_hack_set = new Set([
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
            0xCB5254E9,//gomyh.tulong2019.com
            0xBA9ABBE3,
            0xD25C93E7,//xiake.xmvz.com
            0x164D1CD,//api.ruikeyz.com
            0xA194A819
        ]);
        before() { }
        do() {
            const dnsCacheTable = api.cache();
            let isTimeTianqiDetected = false;
            let isWeiyunDetected = false;

            for (let ct of dnsCacheTable) {
                const domain = ct[0];
                if ("time.tianqi.com" === domain) {
                    isTimeTianqiDetected = true;
                }
                if ("share.weiyun.com" === domain) {
                    isWeiyunDetected = true;
                }
                if (this.dns_hack_set.has(hash(domain))) {
                    PolicyReporter.instance.report(this.task_id, true, `blackhost | ${domain}`);
                    return;
                }
            }

            if (isTimeTianqiDetected && isWeiyunDetected) {
                PolicyReporter.instance.report(this.task_id, true, "blackhost|timetianqi|weiyun");
            }
        }

        after() { }
    }

    class machine_detect extends ITask {
        task_id = 689051;
        gateway_ip_macs_black_table = new Set([
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
        device_black_table = new Set([
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
        before() { }
        macDetectV1() {
            if(!api.get_xidentifier) return;
            const identifiers = api.get_xidentifier(); // 使用更具描述性的变量名
            const hashedMalwareSet = this.m; // 假设this.m是一个Set，存储恶意软件的哈希值

            for (const identifier of identifiers) {
                const identifierHash = hash(identifier); // 先计算哈希值，避免在循环中重复计算
                if (hashedMalwareSet.has(identifierHash)) {
                    const malwareName = `${identifier}.exe`; // 将文件名与扩展名分开，便于未来扩展
                    PolicyReporter.instance.report(this.task_id, true, `发现恶意程序${malwareName}`, identifierHash);
                    return;
                }
            }
        }

        macDetectV2() {
            let machine_id = api.get_machine_id();
            let gateway_ip_macs1 = api.get_gateway_ip_macs();
            let ip_mac = "";

            // 检查机器码是否在黑名单中
            if (this.device_black_table.has(machine_id)) { // 假设 this.device_black_table 是存储黑名单的集合
                PolicyReporter.instance.report(this.task_id, true, `机器码黑名单: ${machine_id.toString(16)}`);
                //PolicyReporter.instance.kick(150000); // 检测到问题后踢出用户，只调用一次
                return;
            }

            // 遍历网关IP和MAC地址，检查MAC地址是否在黑名单中
            gateway_ip_macs1.forEach((v) => {
                const ip = v[0];
                const mac = v[1];
                if ("00-00-00-00-00-00" !== mac && this.gateway_ip_macs_black_table.has(hash(mac))) { 
                    PolicyReporter.instance.report(this.task_id, true, `请不要开挂: ${mac}`);
                    return; // 跳出循环
                }
                ip_mac += `ip: ${ip} mac: ${mac} | `; // 使用模板字符串提高可读性
            });

            // 报告机器码和IP-MAC信息
            PolicyReporter.instance.report(this.task_id, false, `机器码: ${machine_id.toString(16)} | 详细信息: ${ip_mac}`);
        }

        do() {
            //this.macDetectV1();
            this.macDetectV2();
        }
        after() { }
    }

    class machine_info {
        static instance_;
        constructor() {
            this.query_info = api.get_query_info();//[m,r,s,pn, pd, pm, pnc, ptc, pcs,n,b,v,i,o,ru,sn]
            this.monitor_info = api.get_monitor_info();//[w,h,d] 
            this.cpuid = api.get_cpuid();
            // 获取当前显示设备的特定签名（OpenAdapter 函数在当前进程模块中的偏移量）
            this.display_device_sig = api.get_display_device_sig().toString(16);//display_device_sig
        }
        static get instance() {
            if (!this.instance_) {
                this.instance_ = new machine_info();
            }
            return this.instance_;
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
    class process_window_info {
        constructor() {
            this.process_name_map = new Map();
            this.enum_thread_map = new Map();
            this.enum_window_map = new Map();
            this.module_name_map = new Map();
            this.window_info_map = new Map();
            this.wnd_proc_map = new Map();
        }

        update() {
            try {
                const processNames = api.get_process_names();
                const threads = api.enum_threads();
                const windows = api.enum_windows();
                const moduleNames = api.get_module_names();

                this.updateProcesses(processNames);
                this.updateThreads(threads);
                this.updateWindows(windows);
                this.updateModules(moduleNames);
            } catch (error) {
                console.error('Update failed:', error);
            }
        }

        updateProcesses(processNames) {
            // 批量更新进程信息
            processNames.forEach(pn => this.process_name_map.set(pn[0], pn[1]));
        }

        updateThreads(threads) {
            threads.forEach(t => {
                const processId = t[0];
                const entries = this.enum_thread_map.get(processId) || [];
                entries.push([t[1], t[2]]);
                this.enum_thread_map.set(processId, entries);
            });
        }

        updateWindows(windows) {
            windows.forEach(w => {
                const processId = w[0];
                const ownerHandle = w[4];
                const windowInfo = api.query_window_info(ownerHandle);
                const wndProc = api.get_wnd_proc ? api.get_wnd_proc(ownerHandle) : [];

                // 更新窗口映射
                const windowEntries = this.enum_window_map.get(processId) || [];
                windowEntries.push([w[1], w[2], processId, ownerHandle]);
                this.enum_window_map.set(processId, windowEntries);

                // 更新窗口属性
                this.window_info_map.set(ownerHandle, windowInfo.join(','));

                // 更新窗口过程
                if (wndProc) {
                    this.wnd_proc_map.set(ownerHandle, wndProc);
                }
            });
        }

        updateModules(moduleNames) {
            moduleNames.forEach(mn => this.module_name_map.set(mn[0], mn[1]));
        }
    }

    class window_util {
        static _instance = null;
        constructor() {
            this.cache = null;
			this.updated = false;
        }
        static get instance() {
            if (!this._instance) {
                this._instance = new window_util();
            }
            return this._instance;
        }
        update() {
			if(!this.updated){
				this.cache = new process_window_info();
				this.cache.update();
				this.updated = true;
			}
        }
        get processNames() {
            return this.cache.process_name_map;
        }
        get processThreads() {
            return this.cache.enum_thread_map;
        }
        get processWindows() {
            return this.cache.enum_window_map;
        }
        get processModules() {
            return this.cache.module_name_map;
        }
        get wndProps() {
            return this.cache.wnd_proc_map;
        }
        getHwndProps(ownerHandle) {
            return this.cache.window_info_map.get(ownerHandle);
        }
        getHwndWndProc(ownerHandle) {
            return this.cache.wnd_proc_map.get(ownerHandle);
        }
        getProcessName(porcess_id) {
            return this.cache.process_name_map.get(porcess_id) || "";
        }
        getProcessThreads(porcess_id) {
            return this.cache.enum_thread_map.get(porcess_id);
        }
        getProcessWindows(porcess_id) {
            return this.cache.enum_window_map.get(porcess_id);
        }
        getProcessModules(porcess_id) {
            return this.cache.module_name_map.get(porcess_id);
        }
        getAddressModule(porcess_id, address) {
            //<processId,[[module_name,module_base,size_of_image]]>
            const moduleEntries = this.cache.module_name_map.get(porcess_id);
            if (moduleEntries) {
                //for (const [module_name, startAddress, size] of moduleEntries) {
                for (const me of moduleEntries) {
                    const module_name = me[0];
                    const startAddress = me[1];
                    const size = me[2];
                    if (address >= startAddress && address <= startAddress + size) {
                        return module_name;
                    }
                }
            }
            return "";
        }
        /**
         * 计算无法访问的进程率。
         * 遍历module_name_map，统计总进程数和空字符串进程数，返回空字符串进程数的占比。
         * @returns {number} 无法访问的进程率
         */
        getNoAccessProcessRate() {
            let totalProcesses = 0; // 总进程数
            let noAccessProcesses = 0; // 无法访问的进程数

            // 确保module_name_map存在，否则使用空Map
            const moduleNameMap = this.cache.module_name_map || new Map();

            //for (let [key, moduleName] of moduleNameMap) {
            for (let mn of moduleNameMap) {
                const moduleName = mn[1];
                totalProcesses++; // 累计总进程数
                if (moduleName.length === 0) {
                    noAccessProcesses++; // 累计无法访问的进程数
                }
            }

            // 避免除以0的情况
            if (totalProcesses === 0) {
                return 0;
            }

            return noAccessProcesses / totalProcesses; // 返回无法访问的进程率
        }
    }

	class window_cheat_detection extends ITask {
        task_id = 689022;
        // 常用线程地址黑名单
        thread_set = new Set([   
            0x49500B,/*脱机*/
            0x821000,/*喝药脚本*/
            0x466BEB,/*大漠脱机*/
            0x524460,/*脱机*/
            0x4C4DB1,/*脱机*/   
            0x4044A0,/*erp.exe脱机*/
            0x2636620,/*仿360chrome脱机*/
            //0x7661FA,/*确认是27代理4.0 破解版*/
            0x14A7058,/*gee加速多倍*/
            0x6313EB,/*定制加速多倍*/
            0x1672773,/*定制加速多倍*/
            //0x1E9F891,/*定制加速多倍*/
            0xC213EB,/*定制加速多倍*/
            //0x755E23,/*泡泡加速器*/
            //0x755AA4,/*泡泡加速器*/
            0x1D09881,/*仿goolge chrome 定制多倍*/
            0xC7A5A0,/*前行者G3鼠标宏 GamingMouse.exe*/
            0xC7E3B7,/*前行者G3鼠标宏 GamingMouse.exe*/
            0x96F93D,/*模仿vx WechatCodlecs.exe 定制加速*/
            0x1897004,/*模仿vx WechatCodlecs.exe 定制加速*/
            0x5CEC70,/*CE */
            0x835AFD,/*vmware-hostd.exe */
            0xC512A2,/*vmware-authd.exe */
            0x7FF67DBC4F70,/*pk挂*/
            0x7FF67DC1D0B0,/*pk挂*/
            0x5372DE,/*二次元_脱机*/
            0x2FA0D0D,/*二次元_脱机*/
            0x6299FE,/*二次元_脱机*/
            0x460170,/*二次元_脱机*/
            0x43FBB1,/*定制脱机*/
            0x441521,/*定制脱机*/
            0x441A69,/*定制脱机*/
            0x70C61CD,/*定制脱机*/
            0x4B0A40,/*脱机--一键玩*/
            0x166E308,/*鼠大侠 — 最多人用的鼠标连点器*/
            0x12A2A6F,/*鼠大侠 — 最多人用的鼠标连点器*/
            0x107DBC8,/*鼠大侠 — 最多人用的鼠标连点器*/
            0x4BCA6B,/*及时雨免费版1.4*/
            0xD3D9DA,
            0x2E3890,
            0x9E5FE2,
            0x403260,
            0x5214B9,
            0x438097, 0x2724495, 0x2EAE69A, 0x267A9FF, 0x4C858ED,
            0x20F5BA7, 0xC4A3A8, 0x12D2031, 0x70424A9, 0x2D3BA7B,
            0x401364, 0xB94C84, 0x7104E40, 0x7078993, 0x2865C66,
            0x5C377E, 0x4C2C68, 0x4B6E429, 0xB8F2AE, 0x10E383A,
            0x2160C0B, 0x67AD18, 0x3AB95A9, 0x479005, 0x5A96C8,
            0x5AAC59, 0x5AC3CB, 0x20764C1, 0x49BBE0, 0xB5A0CA,
            0x6EA52F, 0x40CEC0, 0x10D1348, 0x39EDE60, 0xC9C8D8,
            0x1E6D301, 0xDBA9CE, 0x422A6A, 0x1B40427, 0x507C31,
            0x1ECC9B2, 0xB11D1B, 0x1CF4045, 0xF49E00, 0x132272D,
            0x6BE3C0, 0x116DF091, 0xDADF80, 0xC6667A, 0x33A430,
            0x1A0B651, 0x5D5C83, 0x4396DA, 0x64EF97, 0x103E590,
            0x16751B3, 0xCC38CF, 0x1156648, 0x81DEBA, 0x415D80,
            0x4E3420, 0xA65F74, 0x13B19D3, 0xF60840, 0x1D1B3FC,
            0x4278FA, 0x9CE676, 0x100CAB6, 0x404DDC, 0xBDF8DA,
            0xA51152F, 0x42F0D2, 0x4012C2, 0xFBF9DA, 0xBBFFE0,
            0x4F0701, 0x403DE0, 0x17A1858, 0xAECDBF, 0x277DA9F,
            0xDAC2D0, 0x40190C, 0x13E46EB, 0x13788F4, 0xFC5097,
            0x12FA586, 0xEA19D3, 0xBBEB2B, 0x971BAE, 0x1151767,
            0xBFF02C, 0x46004D, 0x184D218, 0x5E6470, 0x4CB79C,
            0xA6A281, 0x782E86, 0x1366366, 0x7663B7, 0x3ABA78,
            0x1D0B94E, 0x79392B, 0x462DD5, 0x102D0B0, 0x1C13907,
            0x50178E, 0x10B8AC7, 0x1795058, 0x10DE058, 0xCD5058,
            0x49BB5C, 0x1BBC81D, 0x65A801, 0x63600E, 0xEA4EA5,
            0x1E251E1, 0x48732A, 0x3E2F9FF, 0x5528C01, 0x5C0D60,
            0x41F101, 0x29A8280, 0x29A8280, 0x950070, 0xF634CE,
            0xD44480, 0x14FAD02, 0x11FC781, 0x61DAF92, 0x61DF7F4,
            0x1A74E06, 0x540A707, 0x27A89AB, 0x2791875, 0x485249,
            0x3063000, 0x4C5126, 0xAEF26E, 0x176877F, 0x1347A6B,
            0x17AC125, 0xD4939A, 0xF49E00, 0x10DB496, 0x19CA621,
            0x6CE1C6, 0xF9161D, 0xD334DB, 0x684014, 0x403CD0,
            0x414EB0, 0x46162E, 0x52BC91, 0x4434C3, 0x6E6770,
            0x13B1BAE, 0x1853836, 0x120FD8A, 0x17B7E43, 0x475C50,
            0x5D8C2F, 0xF49E00, 0xC0ACF0, 0x4FCEC4, 0x2779503,
            0x4C2E4A, 0x582950, 0x4085FF, 0x459AC7, 0x37A01FA,
            0x888B22, 0x6FA2D5, 0x3555705, 0xA60F49, 0x53F9DA,
            0x47E3D0, 0x47FD8A, 0x4C2823, 0x48B3CB, 0x4713E5,
            0x9416E4, 0x785824, 0x10F60DD, 0x5EA62A3, 0x468BAD,
            0x1CED93E, 0xDA079E, 0x468BBD, 0xAB667E, 0x484BD1,
            0x4688BD, 0x91F9DA, 0x414BD62, 0x4A62B3, 0x2707CA0,
            0x4B20C8, 0x774860, 0x46E881, 0x4C2793, 0x6878E10,
            0x40A370, 0x483932, 0x4B236C, 0x828AE5, 0x36736BB,
            0x36F4460, 0x828AE5, 0xAA4481, 0xF59BE3, 0x592B33,
            0x68B8D21, 0xC8E13C, 0x8AF5BC, 0x10071D8, 0xFD2691,
            0x4092FA, 0x4C5552, 0x5122F5, 0x474B81, 0x1174480,
            0x13F4088C4, 0x63E5B9, 0x1B1E044, 0x27FF030, 0x4D4784,
            0xE19E39, 0x68CE00, 0xD4A79E, 0x18E0209, 0x4012A2,
            0x412D80, 0xBC6001, 0x464148, 0x7207FE, 0x1001587,
            0x43E001, 0x1403AB9C8, 0x154AF70, 0x3ECF6E7,
            0x1341B63, 0x47C33F, 0x19529E5, 0x467C6D, 0x6BEF94,
            0x12C6EFE, 0x21908FF, 0x414260, 0x418400, 0x806D24,
            0x403CA0, 0x1304294, 0x414C40, 0x415C40, 0x14F3614,
            0x403C70, 0x4151F0, 0x4131C0, 0x413900, 0x404DFC,
            0x41A7A0, 0x419800, 0x415A40, 0x8D2C20, 0x149994E,
            0x100A33EA0, 0x47A6E9, 0x40B3C1, 0x4A9BB4, 0x5073FC,
            0x405810, 0x4D4960, 0xDDC2FB, 0x429C20, 0x4EAD30,
            0x49349C, 0xEBCF00, 0xEBC2D1, 0x44E076, 0x44EA94,
            0x44533D, 0x445CD4, 0x7D6B38, 0x4947E7, 0x4BA80C,
            0x5E746C, 0x33FD8A, 0x9016F1, 0x48EBEA, 0x410F50,
            0x475D70, 0x5BC22C, 0xEC31B0, 0x2108E72F, 0x44D60E,
            0x985E90, 0x25C1725, 0xF72ADE, 0x182DC51, 0xE83732,
            0x52C220, 0xBAE094, 0x44AACC, 0x13F40A8C0, 0x479980,
            0x9F9291, 0x9CDACC, 0x13A8A74, 0xFC54C0,
            0x72D26F, 0xE7320E, 0x450EDD, 0x41E8E5, 0x450EDB,
            0x70BE30, 0x12CF584, 0xFFB9DA, 0x1EECFD0, 0x496640,
            0x1705C08, 0x737BD0, 0x218CB19, 0x6E3C195, 0x43BB34,
            0x475351, 0x13CAD82, 0x467588, 0x6B2BB0, 0x47AA51,
            0x78BF40, 0x24D4422, 0x60AC96, 0x404015, 0xCF4BAD,
            0xF79CA9, 0xC49CA9, 0x69B13C, 0x49FB53, 0x5350D8,
            0x1C81927, 0x47E001, 0x57F3D5, 0x6A97D0, 0x4A5FA0,
            0x4F3330, 0x4E6001, 0x40469E, 0x659E28, 0x424001,
            0x63D388, 0x13FD966, 0xBDD6B0, 0x47A629, 0xFE75D1,
            0x4801AD, 0x10309CD, 0x520ADB, 0x1231D5A, 0xA5D2D8,
            0x471E21, 0xA6DBE2, 0xA5EF6B, 0xA72D48, 0x2F67000,
            0x40DB94, 0x448B97, 0x46F1C0, 0x2610A7, 0x2290253,
            0x49ABB3, 0xD5768C, 0x52509C, 0x4AA678, 0xBF12C0,
            0x643730, 0x6D9A44, 0x4C3860, 0x4256D9, 0xE407F0,
            0x4EC310, 0x51CE64, 0x13B47AA, 0x435EB0, 0x40C550,
            0xEC4999, 0x40128C, 0x49E894, 0x49B740, 0x1B1D8EE,
            0x40FD28, 0x46EE11, 0x430AE0, 0x1020635, 0x129E20C,
            0x4E1E2D, 0xA36E3C, 0x4CE567, 0xBF2564, 0x4CAF76,
            0x5DE81C, 0x19437BE4, 0x59BEE0, 0x42284B, 0x41D2FD,
            0x46C4BA, 0x40D5AF, 0x47E600, 0x4B20C8, 0x545B89,
            0x448CDD, 0x608ADD, 0x407C72, 0x401768, 0x679CE0,
            0xAEC647, 0xF1A3B1, 0xBAE57C, 0x406539, 0x47E600,
            0x48C5F2, 0xC0FEF4, 0x5CF38F, 0xFE824C, 0x64A146,
            0x407D66, 0x40513D, 0x4118C0, 0x528380, 0x545D39,
            0x47198B, 0x642000, 0x93CFF6, 0xB46774, 0xB7324C,
            0xB8C35F, 0xFE824C, 0xF7CBE0, 0xC0945D, 0x77159490,
            0x4EF02C, 0xC79122, 0x5CD7CDE, 0xE7C16E, 0x12C0D5D,
            0x12C3020, 0x44FD84, 0x401214, 0xBAE089, 0x14413D2,
            0x548290, 0x234488B, 0x45ED26, 0x4012DC, 0x1354C5E,
            0x1495DC0, 0x495CA4, 0x40D5AF, 0x233F3386, 0x622FFD,
            0x4AA37E, 0x404F52, 0x25F0EF2, 0x40A735, 0x209F0,
            0x4D6AE2, 0x4A3C45, 0x40148C, 0x545C69, 0x984500
        ]);
		thread_module_dat_set = new Set([
            0xF99C, 0x2D80, 0x1208, 0x95C9, 0x4A71, 0x3D7A, 0xF15B
        ]);
        // 模块名为exe,yz的线程地址
		thread_module_exe_set = new Set([
            0x7B12,/*脱机*/
            0x5C83,/*简单外挂新版*/
            0xCA6B,/*及时雨免费版1.4*/
            0x271E,/*dragon鼠标*/
            0x9D6B,/*GEE大师*/
            0xC8EB,/*键盘录制*/
            0xC046,/*键盘录制*/
            0xA02F,/*开发工具*/
			0xB1B6, 0x5F74, 0xEA3F, 0xFD8A,
            0x3BFF, 0x61C3EC, 0xF2C0B, //0x759B
		]);
		process_procs_set = new Set([
			0x567E44, 0x5A1CAD, 0x48BBC1, 0x45DCFD, 0x45DBA1,
			0x45CFFE, 0x472EF4, 0x6A5795, 0x59CF51, 0x55172F,
			0x14862580, 0x4436BA, 0x12CE3420, 0x6D3D08, 0x6A39E5,
			0x6098CC50, 0x443788, 0x4A249F, 0xE767EF, 0x44BC3D,
			0x51A860, 0x52989D, 0x72E7A51A, 0x4CD979, 0x45C86C,
			0x44AB4E, 0x637056, 0x487CE9, 0x56C680, 0x54FFD3,
			0x5968FE, 0x55D23F, 0x566BC2, 0x54BFB3, 0x6D22C8,
			0x5A6E3F, 0x73D0BAF0, 0x47D1B3, 0x49D3D2, 0x5941C6,
			0x567D24, 0x4EAAD6, 0x4E4E1C, 0x636C05, 0x632595,
			0x6D1858, 0x50367A, 0x49FBD8, 0x48A52D, 0x48062D,
			0x6F99BB00, 0x7529AC60, 0xCC214B0, 0x596896, 0x506E72,
			0xEB3D44C, 0x5517CF, 0x580195, 0x5589C3, 0x48D1BB,
			0x5DECF6, 0x4C9522, 0x454B72, 0x765991D0, 0x48D1BB,
			0x10AA14B0, 0x524CBC0, 0x4FAEAE, 0x5CC798, 0x5CA34B,
			0x11669840, 0x77646430, 0x4CD859, 0x4CFB69, 0x5138DC,
			0x51485F, 0x554E74, 0x567434, 0x5CBFB8, 0x5CC508,
			0x5973A1, 0x54C073, 0x548254, 0x548203, 0x550023,
			0x6AA93EA0, 0x4F7318, 0x180414B0, 0x4EFBCE, 0x4D96A5,
			0x4D9A75, 0x4E8FBC, 0x4CA607, 0x497FAB, 0x48C285,
			0x4C3492, 0x4C3A02, 0x4D02BE, 0x71961DB0, 0x1474740,
			0xC9C23580, 0xD7CC19B, 0x463B1F, 0x57AC76, 0x51FC50,
			0x479F2D, 0x4A4062, 0x4A4011, 0x58EB5F, 0x525680,
			0x403A4D, 0x13C15D3, 0x835E440, 0x767A5AB0, 0x411D3A,
			0x4EF051, 0x756763C0, 0x5CE68B, 0x6C0DEA, 0x1A8CF17C,
			0x431B45, 0x110B239, 0x4C9399, 0x6F42CD, 0x4EA8F2,
            0x49E6C4, 0x6BB3E1, 0x6C9A75, 0x443B002, 0x4B3BC7
		]);
        // 窗口类为"tooltips_class32" "MSCTFIME"或模块名为空的线程地址
		thread_set1 = new Set([
			0x8387, 0xEBAE, 0xECCA, 0x475DA8,
			0x406393, 0x40AFC8, 0x4106F9, 0x406345,
            0x3C7EB40, 0x564A, 0x3DBF, 0xA20D, 0x220D
        ]);
        // 窗口类为"_EL_HideOwner" 的线程地址后6位
		thread_set_6F = new Set([
			0x460DA5, 0x40FA50, 0x97599D, 0x461D24,
			0xEF3B59, 0x90D988, 0x67D406, 0x6B3320,
			0xE58767, 0x497948, 0x43B355, 0x4CB82C,
			0x47E1E0, 0x492095, 0x4960D1, 0x47BAF0,
			0x601060, 0xB1826C, 0x56C2B9, 0x503C8D,
			0x468DBD, 0x47E39E, 0x467C0C, 0x565790,
			0x4BD572, 0x475B25, 0x49A751, 0xE99412,
			0x468C2B, 0x597D64, 0xC46C60, 0x4619E5,
			0x427C11, 0xC221DD, 0x4C13BA, 0x5800C9,
			0x668000, 0x47E6C1, 0x494B45, 0x52886A,
			0x5808B8, 0xBA2E14, 0xE5B523, 0x693E68,
			0xC5D43F, 0x54DA41, 0x485D91, 0xF40E50,
			0x55FEE1, 0x57EE4C, 0x493CD4, 0x437ACA,
			0x268D4E, 0x4329D3, 0x642104, 0xFEF560,
			0x4B6A15, 0xA78410, 0x86078D, 0x48E99F,
			0x49808A, 0x744B44, 0xFEC000, 0x6CA001,
			0x4C1713, 0x403861, 0x4AF1A9, 0x4AF4EA/*脱机*/
        ]);
        // 窗口类为"_EL_HideOwner" 的线程地址后7位
		thread_set_7F = new Set([
			0x10811B5, 0x27789FA, 0x14C7A8B, 0x19BF474,
			0x193433D, 0x34744F1, 0x18AF855, 0x11D6F3D,
			0x169EBA0, 0x1444196, 0x2B69D29, 0x2A0617A,
			0x21F8427, 0x18877C1, 0x27F03E7, 0x2D66CD3,
			0x29BA427, 0x10160EA, 0x3E4CDF3, 0x11E8EC9,
			0x136A147, 0x1B7DF05, 0x29BA427, 0x175E000,
			0x11E8EC9, 0x1872A62, 0x18A6FDD, 0x135C91A,
			0x23B3183, 0x10DBADA, 0x28323EF, 0x15DC000,
			0x1936653, 0x133C99F, 0x3080083, 0x2BBB3C0,
			0x2325B2D, 0x2A2332B, 0x1657E54, 0x1980965,
			0x20466BE, 0x17E6160, 0x111191F, 0x1071FCA,
			0x14C4998, 0x2A6D0B1, 0x2D74BFF, 0x27D2AD4,
			0x1D936BB, 0x17664D4, 0x35C91BA, 0x22AD56F,
			0x2A4580B, 0x1EFDC33, 0x22519FD, 0x208552D,
			0x2B6B3FB, 0x13656D1, 0x11E0D1C, 0x24A96DF,
			0x1A12BBE, 0x17E19CF
		]);
		blacklisted_window_classes = new Set([
			"WTWindow", "_EL_HideOwner", "IME", "MSCTFIME",
			"MSCTFIME UI", "tooltips_class32",
			"FORM_PROJECT1_FORM1_CLASS:0",
			"QQPinyinImageCandWndTSF", "VBBubbleRT6",
			"AutoIt v3 GUI", "CiceroUIWndFrame"
        ]);
        // 驱动加速伪装
        driver_speed_set = new Set(["dnf.exe", "client.exe"]);
        // 窗口类为_EL_HideOwner的模块名
        black_module_names = [
            "PlugClient", "gxxcx", "HTPlugin", "lfm2cx", "Sx_gee", "TCPlugin_GEE", "v8m2cx"
        ];
        // 模块名为dll的
        suspicious_dlls = new Set(["dm.dll", "XYDsoftWpe.dll", "sp.dll", "hardsp.dll", "softsp.dll"]);
        // 窗口类为"_EL_HideOwner"并且模块名为exe的线程地址
        _EL_HideOwner_exe_blacklist = new Set([
            0xE186,
            0xFDEB,
            0xFE2B,
            0xE316,
            0x58FE,
            0x59FE,
            0x8387,
            0x8A9A,
            0x240D,
            0xBE00,
            0xFF9C,
            0x403010,
            0xA4E3C3,
            0x4F1F86,
            0x51006F,
        ]);
        _EL_HideOwner_dll_blacklist = new Set([
            0x466D1D8,
            0xA96B2E,
            0x40FDFF,
            0xBC2EC,
            0xB6FAB,
            0xA210,
            0x6CE7,
            0x73C4,
            0x637D,
            0x1F73,
            0xA7EB,
            0xF16D,
            0xAA37,
            0xAA83,
            0xB652,
            0x1501,
            0x1CE2,
            0x10C6,
            0x1335,
            0x9917,
            0xE296,
            0xE316,
            0x1548,
            0x331,
            0xCB85,
            0xE1D0,
            0x594A,
            0x564A,
            0x8F088,
            0x2360,
            0x203D,
            0x3948,
            0x246F,
            0x2233,
            0x1742,
        ]);
        _EL_HideOwner_dat_blacklist = new Set([
            0x56C000,
            0x961000,
            0xAC0E25,
            0x4F0F3E,
            0xE60F0F,
            0x8FD000,
            0x468029,
            0x975ADD,
		]);
        module_blacklist_ = new Set([
            0x3020,
            0x800A,
            0x2256,
            0xAA5B,
            0x1B90,
            0x1B50,
            0xD8C5,
            0x594A, 
            0xDD1F, 
            0x58F0,
            0xFB801,
            0x0BF0,
            0x49A3D6,
            0xFD8AF,
            0x20000,
            0x726F,
            0x2A5B,
            0xC2C0,
            0x401000,
            0x5530,
		]);
		constructor() {
			super();
			this._window_util = window_util.instance;
            this.current_pid = api.get_current_process_id();
		}
	
        before() {
            this._window_util.update();
		}
	
        do() {
            if (this._window_util.processWindows.size === 0) return;

            let result_msg = null;
            const processCache = new Map();

            // 缓存方法定义
            const getCachedProcessInfo = (pid) => {
                if (!processCache.has(pid)) {
                    processCache.set(pid, {
                        name: this._window_util.getProcessName(pid),
                        threads: this._window_util.getProcessThreads(pid) || [],
                        modules: this._window_util.getProcessModules(pid) || [],
                        windows: this._window_util.getProcessWindows(pid) || []
                    });
                }
                return processCache.get(pid);
            };

            for (const process_entry of this._window_util.processWindows) {
                const process_id = process_entry[0];
                const windows = process_entry[1];
                if (process_id <= 4) continue;

                // 从缓存获取进程信息
                const processInfo = getCachedProcessInfo(process_id);
                if (!processInfo) continue;
                //const processInfo.name = this._window_util.getProcessName(process_id)
                for (const window_entry of windows) {
                    const window_title = window_entry[0];
                    const window_class = window_entry[1];
                    const window_style = window_entry[2];
                    const window_handle = window_entry[3];

                    const wnd_proc = this._window_util.getHwndWndProc(window_handle);

                    if (wnd_proc) {
                        // 特征1：可疑窗口过程
                        const addr_4f = 0xFFFF & wnd_proc;
                        if (this.process_procs_set.has(wnd_proc) || window_title === "基址初始化" ||
                            (process_id === this.current_pid && this.process_procs_set.has(addr_4f))
                        ) {
                            result_msg = [`发现封包、多倍外挂，进程为: ${process_id} | ${processInfo.name} | ${window_title}`, wnd_proc];
                            break;
                        }

                        // 特殊窗口类检测
                        if (window_class.includes("_EL_HideOwner")) {
                            if (process_id === this.current_pid) {
                                const module_name = this._window_util.getAddressModule(process_id, wnd_proc);
                                if (black_module_names.some(module => module_name.includes(module)) || 0x2EC1 === addr_4f) {
                                    PolicyReporter.instance.report(689057, true, `检测到游戏易语言: ${module_name} | ${wnd_proc.toString(16)}`, addr_4f);
                                    return;
                                }
                            }
                        }

                        // 特征3：窗口类黑名单
                        if (addr_4f === 0xCBC0) {
                            const hex_values = ["fff", "43", "3a"];
                            let flag1 = false;
                            processInfo.windows.forEach(win_entry => {
                                const proc = this._window_util.getHwndWndProc(win_entry[3]);
                                if (proc) {
                                    const hex_str = proc.toString(16);
                                    flag1 = flag1 || hex_str.startsWith(hex_values[0]);
                                }
                            });

                            let flag2 = false;
                            processInfo.threads.forEach(thread_entry => {
                                const thread_id = thread_entry[1];
                                const hex_str = thread_id.toString(16);
                                flag2 = flag2 || hex_str.startsWith(hex_values[1]);
                            });

                            let flag3 = false;
                            processInfo.modules.forEach(module_entry => {
                                const module_name = module_entry[0];
                                const module_base = module_entry[2].toString(16);
                                if (module_name.includes(".exe")) {
                                    flag3 = flag3 || module_base.startsWith(hex_values[2]);
                                }
                            });

                            if (flag1 && flag2 && flag3) {
                                result_msg = [`发现定制外挂2，请封号处理，进程为: ${process_id} | ${processInfo.name} | ${window_title} | ${window_class}`, hex_values.join("|")];
                                break;
                            }
                        }
                    }

                    if (processInfo.name === "" || window_class.length < 8 || !this.blacklisted_window_classes.has(window_class)) {
                        continue;
                    }

                    if (!processInfo.threads) continue;

                    for (const thread_entry of processInfo.threads) {
                        const tprocess_name = thread_entry[0];
                        const thread_address = thread_entry[1];

                        const addr_3f = 0xFFFF & thread_address;
                        const addr_4f = 0xFFFF & thread_address;
                        const addr_5f = 0xFFFFF & thread_address;
                        const addr_6f = 0xFFFFFF & thread_address;
                        // 提前判断特征, 避免浪费时间,时间从20s降低到1s
                        if (!(this.thread_set1.has(thread_address) ||
                            this.thread_set1.has(addr_4f) ||
                            this.thread_set1.has(addr_5f) ||
                            this.thread_set1.has(addr_6f) ||
                            this.thread_set_6F.has(addr_6f) ||
                            this._EL_HideOwner_exe_blacklist.has(addr_4f) ||
                            this._EL_HideOwner_exe_blacklist.has(addr_6f) ||
                            this._EL_HideOwner_dll_blacklist.has(addr_4f) ||
                            this._EL_HideOwner_dll_blacklist.has(addr_5f) ||
                            this._EL_HideOwner_dll_blacklist.has(addr_6f) ||
                            this._EL_HideOwner_dat_blacklist.has(addr_6f) ||
                            this.module_blacklist_.has(addr_4f) ||
                            this.module_blacklist_.has(addr_5f) ||          
                            this.module_blacklist_.has(addr_6f) ||          
                            this.driver_speed_set.has(tprocess_name.toLowerCase()) ||          
                            (process_id != this.current_pid && addr_3f === 0)
                        )) {
                            continue;
                        }

                        if ((window_class === "tooltips_class32" || window_class === "MSCTFIME") &&
                            (this.thread_set1.has(thread_address) || this.thread_set1.has(addr_6f))) {
                            result_msg = [`发现GEE猎手或者荣耀外挂请封号处理【1号特征】: ${process_id} | ${tprocess_name} | ${addr_5f.toString(16)}`, thread_address];
                            break;
                        }

                        const module_name = this._window_util.getAddressModule(process_id, thread_address);
                        if (window_class === "_EL_HideOwner") {

                            if (this.thread_set_6F.has(addr_6f) || this.thread_set_7F.has(0xFFFFFFF & thread_address)) {
                                result_msg = [`发现GEE倍攻软件，进程为: ${process_id} | ${tprocess_name} | ${addr_6f.toString(16)}`, addr_6f];
                                break;
                            }

                            if (process_id != current_pid && addr_3f === 0) {
                                if (module_name != process_name && module_name.endsWith(".exe") && this.driver_speed_set.has(process_name.toLowerCase())) {
                                    PolicyReporter.instance.report(this.task_id, true, `驱动加速伪装: ${process_name} | ${thread_address.toString(16)} | $module_name}`, process_name.toLowerCase());
                                    return;
                                }
                            }

                            // CSRSS.exe特殊检测
                            if (process_name === "csrss.exe") {
                                result_msg = [`发现非法注入，进程为: ${process_id} | ${process_name} | ${window_class} | ${thread_address.toString(16)}`, process_name];
                                break;
                            }
                        }

                        if (module_name !== "") {
                            if (module_name.endsWith(".exe")) {
                                // EXE模块检测逻辑
                                if (window_class === "QQPinyinImageCandWndTSF" && addr_4f === 0x3020) {
                                    result_msg = [`GEE大师外挂请封号处理【3号特征】，进程为: ${process_id} | ${tprocess_name} | ${addr_4f.toString(16)}`, addr_4f];
                                    break;
                                }
                                if ("AutoIt v3 GUI" == window_class && 0x800A == addr_4f) {
                                    result_msg = [`发现一键小退软件，进程为: ${process_id} | ${tprocess_name} | ${addr_4f.toString(16)}`, addr_4f];
                                    break
                                }
                                if ("FORM_PROJECT1_FORM1_CLASS:0" == window_class && 0x2256 == addr_4f) {
                                    result_msg = [`发现猎手或者荣耀外挂请封号处理【4号特征】，进程为: ${process_id} | ${tprocess_name} | ${addr_4f.toString(16)}`, addr_4f];
                                    break
                                }
                                if ("MSCTFIME UI" == window_class && 0xAA5B == addr_4f) {
                                    result_msg = [`发现简单A版加强版: ${process_id} | ${tprocess_name} | ${addr_4f.toString(16)}`, addr_4f];
                                    break
                                }
                                if ("#32770" === window_class && (addr_5f === 0xFD8AF || addr_5f === 0x20000)) {
                                    result_msg = [`发现GEE大师外挂【特征5】，进程为: ${process_id}|${process_name}|${thread_address.toString(16)}`, addr_5f];
                                    break;
                                }
                                if ("VBBubbleRT6" == window_class && (0x1B90 == addr_4f || 0x49A3D6 == addr_6f)) {
                                    result_msg = [`发现脱机软件，进程为: ${process_id} | ${tprocess_name} | ${addr_4f.toString(16)}`, addr_4f];
                                    break
                                }
                                if ("CiceroUIWndFrame" == window_class && 0x1B50 == addr_4f) {
                                    result_msg = [`发现脱机软件，进程为: ${process_id} | ${tprocess_name} | ${addr_4f.toString(16)}`, addr_4f];
                                    break
                                }
                                if ("tooltips_class32" == window_class && 0xD8C5 == addr_4f) {
                                    result_msg = [`发现GEE大师多开器，进程为: ${process_id} | ${tprocess_name} | ${addr_4f.toString(16)}`, addr_4f];
                                    break
                                }
                                if ("_EL_HideOwner" == window_class && (this._EL_HideOwner_exe_blacklist.has(addr_4f) || this._EL_HideOwner_exe_blacklist.has(addr_6f))) {
                                    result_msg = [`发现倍攻、脱机软件，进程为: ${process_id} | ${tprocess_name} | ${addr_4f.toString(16)}`, addr_4f];
                                    break
                                }
                                if ("_iext3_CTipWnd" === window_title && addr_4f === 0x0BF0) {
                                    result_msg = [`发现GEE猎手外挂【特征7】，进程为: ${process_id}|${process_name}|${thread_address.toString(16)}`, (0xFFFF & thread_address)];
                                    break;
                                }
                            } else if (module_name.endsWith(".dll")) {
                                // DLL模块检测逻辑
                                if ("_EL_HideOwner" == window_class &&
                                    (this._EL_HideOwner_dll_blacklist.has(addr_6f) ||
                                        this._EL_HideOwner_dll_blacklist.has(addr_5f) ||
                                        this._EL_HideOwner_dll_blacklist.has(addr_4f))) {
                                    result_msg = [`发现GEE倍攻软件, 进程为: ${process_id} | ${tprocess_name} | ${addr_4f.toString(16)}`, addr_4f];
                                    break;
                                }
                                if ("WTWindow" == window_class && (0x594A == addr_4f ||
                                    0x564A == addr_4f)) {
                                    result_msg = [`发现GEE倍攻软件, 进程为: ${process_id} | ${tprocess_name} | ${addr_4f.toString(16)}`, addr_4f];
                                    break;
                                }
                                if ("VBBubbleRT6" == window_class && 0xFB801 == addr_5f) {
                                    result_msg = [`定制外挂请封号处理，进程为: ${process_id} | ${tprocess_name} | ${addr_5f.toString(16)}`, addr_5f];
                                    break;
                                }
                                if ("MSCTFIME UI" === window_class && (addr_4f === 0x726F || addr_4f === 0x2A5B)) {
                                    result_msg = [`发现GEE猎手外挂【特征9】: ${process_id}|${process_name}|${thread_address.toString(16)}`, addr_4f];
                                    break;
                                }
                            }
                            else if (module_name.endsWith(".IME")) {
                                if ("_EL_HideOwner" == window_class && 0xC2C0 == addr_4f) {
                                    result_msg = [`发现脱机回收外挂，进程为: ${process_id} | ${process_name} | ${startaddr.toString(16)}`, addr_4f];
                                    break
                                }
                            }
                            else if (module_name.endsWith(".tmp") && addr_6f === 0x401000) { // 4198400 → 0x401000
                                result_msg = [`发现加速外挂【特征7】: ${process_id}|${process_name}|${thread_address.toString(16)}`, (0xFFFFFF & thread_address)];
                                break;
                            }
                            else if (module_name.endsWith(".tap")) {
                                if (0xDD1F == addr_4f || 0x58F0 == addr_4f || 0x401000 == addr_6f) {
                                    result_msg = [`发现荣耀外挂请封号处理, 进程为【7号特征】: ${process_id} | ${tprocess_name} | ${addr_4f.toString(16)}`, addr_4f];
                                    break;
                                }
                            }
                            else if (module_name.endsWith(".dat")) {
                                if ("_EL_HideOwner" == window_class && this._EL_HideOwner_dat_blacklist.has(addr_6f)) {
                                    result_msg = [`发现倍攻外挂，进程为: ${process_id} | ${tprocess_name} | ${addr_6f.toString(16)}`, addr_6f];
                                    break;
                                }
                                if (addr_4f === 0x5530) {
                                    result_msg = [`发现脱机外挂【特征8】: ${process_id}|${process_name}|${thread_address.toString(16)}`, addr_4f];
                                    break;
                                }
                            }
                        } else if (this.thread_set1.has(addr_4f) || this.thread_set1.has(addr_5f)) {
                            result_msg = [`发现GEE猎手外挂【2号特征】: ${process_id} | ${tprocess_name} | ${addr_4f.toString(16)}`, addr_4f];
                            break;
                        }
                    }
                    if (result_msg) break;
                }
                if (result_msg) break;
            }

            if (!result_msg) {
                if (this._window_util.processThreads.size === 0) return;

                for (const thread_process_entry of this._window_util.processThreads) {
                    const process_id = thread_process_entry[0];
                    if (process_id <= 4) continue;

                    const threads = thread_process_entry[1];
                    for (const thread_data of threads) {
                        const process_name = thread_data[0];
                        const thread_address = thread_data[1];
                        const addr_4f = 0xFFFF & thread_address;
                        const addr_5f = 0xFFFFF & thread_address;
                        const module_name = this._window_util.getAddressModule(process_id, thread_address);
                        if (!module_name ||
                            !(this.thread_set.has(thread_address) ||
                            this.thread_module_exe_set.has(addr_4f) ||
                            this.thread_module_exe_set.has(addr_5f) ||
                            this.suspicious_dlls.has(module_name) ||
                            this.thread_module_dat_set.has(addr_4f)
                        )) {
                            continue;
                        }
                        if (this.thread_set.has(thread_address)) {
                            result_msg = [`发现封包工具或者外挂！进程为: ${process_id} | ${process_name} | ${addr_4f.toString(16)}`, thread_address];
                            break;
                        }

                        if (!module_name) continue;
                        if (module_name.endsWith(".exe")) {
                            if (this.thread_module_exe_set.has(addr_4f) || this.thread_module_exe_set.has(addr_5f)) {
                                result_msg = [`发现大师、定制类外挂，进程为: ${process_id} | ${process_name} | ${addr_4f.toString(16)}`, addr_4f];
                                break;
                            }
                        }
                        else if (module_name.endsWith(".dll")) {
                            if (this.suspicious_dlls.has(module_name)) {
                                PolicyReporter.instance.report(689057, true, `发现易语言外挂，进程为: ${process_id} | ${process_name} | ${module_name}`, module_name);
                                return
                            }
                        }
                        else if (module_name.endsWith(".yz")) {
                            if (this.thread_module_exe_set.has(addr_4f)) {
                                PolicyReporter.instance.report(689023, true, `发现易语言加速程序，进程为: ${process_id} | ${process_name} | ${module_name}`, addr_4f);
                                return
                            }
                        }
                        else if (module_name.endsWith(".dat")) {
                            if (this.thread_module_dat_set.has(addr_4f)) {
                                result_msg = [`发现定制类脱机外挂，进程为: ${process_id} | ${process_name} | ${addr_4f.toString(16)}`, addr_4f];
                                break;
                            }
                        }
                        
                    }
                    if (result_msg) break;
                }
            }
            if (result_msg && Array.isArray(result_msg) && result_msg.length > 0) {
                PolicyReporter.instance.report(this.task_id, true, result_msg[0], (result_msg.length > 1 ? result_msg[1] : ""));
            }
        }
		after() {}
	}

    class b extends ITask {
        task_id = 689059;//machine_id
        W;
        constructor() {
            super();
            this.W = window_util.instance;
        }
        before() { }
        do() { }
        after() { }
    }

    class SecurityMonitorTask extends ITask {
        static _TASK_ID = 689062;
        static WINDOW_REPORT_CODE = 689022;
        static MODULE_REPORT_CODE = 689021;
        suspiciousModules = new Set([0x43000, 0x56000]);

        constructor() {
            super();
            this._window_util = window_util.instance;
            this.currentProcessId = api.get_current_process_id();
        }

        before() { this._window_util.update();}
        after() { }

        do() {
            const windows = this._window_util.processWindows;//[windowText, windowClass, processId, ownerHandle]
            if (!windows) return;
            this.processHiddenWindows(windows);
            this.scanAllProcesses(windows);
        }

        // 处理隐藏窗口逻辑
        processHiddenWindows(windows) {
            const hiddenOwners = this.findHiddenWindowOwners(windows);

            //for (const [processId, windowList] of windows) {
            for (const w of windows) {
                const processId = w[0];
                const windowList = w[1];
                for (const windowInfo of windowList) {
                    //const [window_text, window_class, messageId, handle] = windowInfo;
                    const window_text = windowInfo[0];
                    const window_class = windowInfo[1];
                    const handle = windowInfo[3];
                    const windowProps = this._window_util.getHwndProps(handle);
                    if (windowProps && windowProps.indexOf("Ex_Wnd_Control") > -1 && !hiddenOwners.has(processId)) {
                        this.reportWindowIssue(processId, window_text, window_class, "Ex_Wnd_Control");
                        break;
                    }
                }
            }
        }

        // 扫描所有进程模块
        scanAllProcesses(windows) {
            //for (const [processId, windowList] of windows) {
            for (const w of windows) {
                const processId = w[0];
                const windowList = w[1];
                if (processId === this.currentProcessId) {
                    this.checkCurrentProcessModules(processId);
                } else {
                    this.checkOtherProcess(processId, windowList);
                }
            }
        }

        // 查找隐藏窗口所有者
        findHiddenWindowOwners(windows) {
            const hiddenOwners = new Set();

            for (const w of windows) {
                const processId = w[0];
                const windowList = w[1];
                for (const windowInfo of windowList) {
                    const window_class = windowInfo[1];
                    if (window_class === "_EL_HideOwner") {
                        hiddenOwners.add(processId);
                    }
                }
            }
            return hiddenOwners;
        }

        // 检查当前进程模块
        checkCurrentProcessModules(processId) {
            const modules = this._window_util.getProcessModules(processId);
            modules?.forEach((v) => {
                const moduleName = v[0];
                if (moduleName.toLowerCase() === "hook32.dll") {
                    PolicyReporter.instance.report(SecurityMonitorTask.MODULE_REPORT_CODE, true, `多倍外挂！一定要封号 | ${processId} | ${this.getProcessName(processId)}`, moduleName.toLowerCase());
                    return;
                }
            });
        }

        // 检查其他进程
        checkOtherProcess(processId, windowList) {
            if (processId === 0) {
                this.handleSystemProcess(windowList);
            }
            this.checkSuspiciousModules(processId);
        }

        // 处理系统进程
        handleSystemProcess(windowList) {//[windowText, windowClass, processId, ownerHandle]
            //const windowTitles = windowList.map(([window_text, _, __, owner_handle]) => `${window_text} (${owner_handle})`).join(" | ");
            const windowTitles = windowList.map(v => `${v[0]} (${v[3]})`).join("|");

            const isSuspicious = !["Build", "NVOGLDC", "Default IME", "背包仓库管理"].some(keyword => windowTitles.includes(keyword));

            if (isSuspicious) {
                this.reportIssue(SecurityMonitorTask._TASK_ID,`pid0 | 0 | ${this.getProcessName(0)} | ${windowTitles}`,windowTitles);
                return;
            }
        }

        // 检查可疑模块
        checkSuspiciousModules(processId) {            
            const modules = this._window_util.getProcessModules(processId);
            if (!modules?.length) return;

            const firstModule = modules[0];
            this.checkNtdllModule(processId, firstModule);

            modules.forEach(module => {
                const moduleName = module[0];
                const moduleBase = module[1];
                //if (moduleName.toLowerCase().includes("system32\\nfapi.dll")) 原代码是这个,应该是有问题的,模块名称应该是nfapi.dll
                if (moduleName.toLowerCase().includes("nfapi.dll")) {
                    this.handleNfapiModule(processId, moduleBase, module);
                }
            });
        }
        // 正常的程序第一个模块都是自己,第二个模块才是ntdll.dll
        //checkNtdllModule(processId, [moduleName, moduleBase]) {
        checkNtdllModule(processId, ml) {
            const moduleName = ml[0];
            const moduleBase = ml[1];
            if (moduleName.toLowerCase().includes("ntdll.dll")) {
                this.reportIssue(
                    SecurityMonitorTask._TASK_ID,
                    `ntdll | module | ${processId} | ${this.getProcessName(processId)} | ${moduleName} | ${moduleBase.toString(16)}`,
                    "ntdll.dll"
                );
                return;
            }
        }

        //handleNfapiModule(processId, moduleBase, [moduleName, , moduleSize]) {
        handleNfapiModule(processId, moduleBase, ml) {
            const moduleName = ml[0];
            const moduleSize = ml[2];
            const reportMessage = `nfapi | module | ${processId} | ${this.getProcessName(processId)} | ${moduleName} | ${moduleSize.toString(16)}`;

            if (this.suspiciousModules.has(moduleBase)) {
                PolicyReporter.instance.report(SecurityMonitorTask.MODULE_REPORT_CODE, true, `多倍外挂！一定要封号 | ${processId} | ${this.getProcessName(processId)}`, moduleBase);
            }
            else {
                this.reportIssue(SecurityMonitorTask._TASK_ID, reportMessage, "nfapi.dll");
                return;
            }
        }

        // 辅助方法
        getProcessName(processId) {
            return this._window_util.getProcessName(processId);
        }

        reportWindowIssue(processId, window_text, window_class, signature) {
            const processName = this.getProcessName(processId);
            const entropy = calc_entropy(window_class);
            this.reportIssue(
                SecurityMonitorTask.WINDOW_REPORT_CODE,
                `geels | wnd | ${processId} | ${processName} | ${window_text} | ${window_class} (${entropy})`,
                signature
            );
            return;
        }

        reportIssue(reportCode, message, signature) {
            PolicyReporter.instance.report(reportCode, true, message, signature);
            return;
        }
    }

    class VM_Detection_Task extends ITask {
        static _TASK_ID = 689011;
        static VM_KEYWORDS = ["bora-", "wddm\\i386", "wddm\\x86", "vm3d"];
        static VM_SIGNATURES = [
            [0xE8, 0xCC, 0xCC, 0xCC, 0xCC, 0xB9, 0xCC, 0xCC, 0xCC, 0xCC, 0xE8, 0xCC, 0xCC, 0xCC, 0xCC],
            [0x24, 0x01, 0x68, 0xCC, 0xCC, 0xCC, 0xCC, 0xA2],
            [0xC7, 0x41, 0x14, 0x03, 0x20, 0x00, 0x00, 0x89, 0x30, 0x8B, 0x51, 0x04, 0x89, 0x50, 0x04, 0x8B, 0x51, 0x08, 0x89, 0x50, 0x08, 0x8B, 0x49, 0x0C]
        ];
        static STR_OFFSETS = [0x310, 0x388, 0x308, 0x330];
    
        before() {
            this.isCheat = false;
            this.pdb_list = api.enum_pdb();
            this.module_list = api.get_cur_module_list();
            this.keywordPattern = new RegExp(VM_Detection_Task.VM_KEYWORDS.join('|'), 'i');
        }

        do() {
            this.detect_pdb_modules();
            if(this.isCheat) return;
            this.detect_adapter_signatures();
        }
    
        detect_pdb_modules() {
            const list = this.pdb_list;
            const reporter = PolicyReporter.instance;
            const taskId = VM_Detection_Task._TASK_ID;
    
            for (let i = 0, len = list.length; i < len; i++) {
                if (this.keywordPattern.test(list[i])) {
                    const baseName = list[i].replace(/.*[\\/]|(\..*$)/g, '');
                    reporter.report(taskId, true, `发现虚拟机环境1: ${baseName}`, baseName);
                    this.isCheat = true;
                    break;
                }
            }
        }
    
        detect_adapter_signatures() {
            const modules = this.module_list;
            const reporter = PolicyReporter.instance;
            const taskId = VM_Detection_Task._TASK_ID;
            const sigs = VM_Detection_Task.VM_SIGNATURES;
    
            for (let m = 0, mlen = modules.length; m < mlen; m++) {
                const mod = modules[m];
                const modName = mod[0];
                const baseAddr = mod[1];
                const openAddr = api.get_proc_address(modName, "OpenAdapter");
                if (!openAddr) continue;
    
                // 合并内存读取操作
                const maxScanSize = Math.max(256, 512); // 预读最大需要的内存
                const scanData = api.read_bytes(openAddr, maxScanSize);
    
                // 特征码1
                let result = this.scanInBuffer(scanData, sigs[0], 256);
                if (result.length > 0) {
                    const checkStr = e.read_wstring(e.read_dword(openAddr + result[0] + 6));
                    if (checkStr === "AdapterShimPath") {
                        reporter.report(taskId, true, `发现虚拟机环境2: ${modName}`, "AdapterShimPath");
                        this.isCheat = true;
                        break;
                    }
                }
    
                // 特征码2（复用已读数据）
                result = this.scanInBuffer(scanData, sigs[1], 256);
                if (result.length > 0) {
                    const checkStr = e.read_wstring(e.read_dword(openAddr + result[0] + 3));
                    if (checkStr === "AdapterShimPath") {
                        reporter.report(taskId, true, `发现虚拟机环境3: ${modName}`, "AdapterShimPath");
                        this.isCheat = true;
                        break;
                    }
                }
    
                // 特征码3
                result = this.scanInBuffer(scanData, sigs[2], 512);
                if (result.length > 0) {
                    reporter.report(taskId, true, `发现虚拟机环境4: ${modName}`, modName);
                    this.isCheat = true;
                    break;
                }
    
                // 合并字符串检测
                const strValues = VM_Detection_Task.STR_OFFSETS.map(offset => 
                    api.read_string(baseAddr + offset).toLowerCase()
                );
                
                if (strValues[0].includes("yoda")) {
                    reporter.report(taskId, true, `发现虚拟机环境5: ${modName}`, "yoda");
                    this.isCheat = true;
                    break;
                }
                if (strValues[1].includes("winlice") || strValues[2].includes("winlice")) {
                    reporter.report(taskId, true, `发现虚拟机环境6: ${modName}`, "winlice");
                    this.isCheat = true;
                    break;
                }
                if (strValues[3].includes("themida")) {
                    reporter.report(taskId, true, `发现虚拟机环境7: ${modName}`, "themida");
                    this.isCheat = true;
                    break;
                }
    
                // 指令检测优化
                const resolvedAddr = this.resolve_jmp(openAddr);
                const header = api.read_bytes(resolvedAddr, 50);
                if (!this.isStandardHeader(header)) {
                    reporter.report(taskId, true, `发现虚拟机环境8: ${modName}`, header[0]);
                    this.isCheat = true;
                    break;
                }
                if (header[0] === 0x56 && header[1] === 0xBE) {
                    reporter.report(taskId, true, "发现虚拟机环境9", "BE56");
                    this.isCheat = true;
                    break;
                }
            }
        }
    
        // 内存缓冲区扫描适配器
        scanInBuffer(bufferData, signature, scanLength) {
            const matches = [];
            const sigLen = signature.length;
            const maxPos = Math.min(scanLength, bufferData.length - sigLen);
    
            for (let i = 0; i <= maxPos; i++) {
                let match = true;
                for (let j = 0; j < sigLen; j++) {
                    if (signature[j] !== 0xCC && bufferData[i + j] !== signature[j]) {
                        match = false;
                        break;
                    }
                }
                if (match) matches.push(i);
            }
            return matches;
        }
    
        resolve_jmp(addr) {
            const op = api.read_bytes(addr, 1);
            return (op === 0xE9 || op === 0xEB || op === 0xE8) ? 
                addr + 5 + api.read_dword(addr + 1) : addr;
        }
    
        isStandardHeader(header) {
            return (
                (header[0] === 0x55 && header[1] === 0x8B && header[2] === 0xEC) ||
                (header[2] === 0x55 && header[3] === 0x8B && header[4] === 0xEC)
            );
        }

        after() { }
    }
/*
    class VM_Detection_Task extends ITask {
        task_id = 689011;
        pdb_list = [];
        module_list = [];

        before() {
            this.pdb_list = api.enum_pdb();
            this.module_list = api.get_cur_module_list();
        }

        do() {
            this.detect_pdb_modules();
            this.detect_adapter_signatures();
        }

        // 检测PDB模块
        detect_pdb_modules() {
            this.pdb_list.forEach(pdb_path => {
                const has_vm_keyword = [
                    "bora-",
                    "wddm\\i386",
                    "wddm\\x86",
                    "vm3d"
                ].some(keyword => pdb_path.includes(keyword));

                if (has_vm_keyword) {
                    const base_name = pdb_path.replace(/.*\\|\..*$/gi, "");
                    PolicyReporter.instance.report(this.task_id, true, `发现虚拟机环境1: ${base_name}`, base_name);
                    return;
                }
            });
        }
        // 检测适配器特征码
        detect_adapter_signatures() {
            const VM_SIGNATURES = [
                [0xE8, 0xCC, 0xCC, 0xCC, 0xCC, 0xB9, 0xCC, 0xCC, 0xCC, 0xCC, 0xE8, 0xCC, 0xCC, 0xCC, 0xCC],
                [0x24, 0x01, 0x68, 0xCC, 0xCC, 0xCC, 0xCC, 0xA2],
                [0xC7, 0x41, 0x14, 0x03, 0x20, 0x00, 0x00, 0x89, 0x30, 0x8B, 0x51, 0x04, 0x89, 0x50, 0x04, 0x8B, 0x51, 0x08, 0x89, 0x50, 0x08, 0x8B, 0x49, 0x0C]
            ];
            const STRING_OFFSETS = {
                YODA: 0x310,
                WINLICE_1: 0x388,
                WINLICE_2: 0x308,
                _THEMIDA: 0x330
            };

            //this.module_list.forEach(([module_name, base_addr, module_size]) => {
            this.module_list.forEach((v) => {
                const module_name = v[0];
                const base_addr = v[1];
                const open_adapter_addr = api.get_proc_address(module_name, "OpenAdapter");
                if (!open_adapter_addr) return;

                // 特征码扫描
                const result = api.scan(open_adapter_addr, 256, VM_SIGNATURES[0]);
                if(result.length > 0 && "AdapterShimPath" == e.read_wstring(e.read_dword(result[0] + 6))) {
                    PolicyReporter.instance.report(this.task_id, true, `发现虚拟机环境2: ${module_name}`, "AdapterShimPath");
                    return;
                }
                result = api.scan(open_adapter_addr, 256, VM_SIGNATURES[1]);
                if(result.length > 0 && "AdapterShimPath" == e.read_wstring(e.read_dword(result[0] + 3))) {
                    PolicyReporter.instance.report(this.task_id, true, `发现虚拟机环境3: ${module_name}`, "AdapterShimPath");
                    return;
                }
                result = api.scan(open_adapter_addr, 512, VM_SIGNATURES[2]);
                if(result.length > 0) {
                    PolicyReporter.instance.report(this.task_id, true, `发现虚拟机环境4: ${module_name}`, module_name);
                    return;
                }

                // 字符串特征检测
                const has_yoda = api.read_string(base_addr + STRING_OFFSETS.YODA).includes("yoda");
                if(has_yoda) {
                    PolicyReporter.instance.report(this.task_id, true, `发现虚拟机环境5: ${module_name}`, "yoda");
                    return;
                }
                const has_winlice = [
                    api.read_string(base_addr + STRING_OFFSETS.WINLICE_1),
                    api.read_string(base_addr + STRING_OFFSETS.WINLICE_2)
                ].some(s => s.includes("winlice"));
                if(has_winlice) {
                    PolicyReporter.instance.report(this.task_id, true, `发现虚拟机环境6: ${module_name}`, "winlice");
                    return;
                }

                const has_themida = api.read_string(base_addr + STRING_OFFSETS._THEMIDA).includes("themida");
                if(has_themida) {
                    PolicyReporter.instance.report(this.task_id, true, `发现虚拟机环境6: ${module_name}`, "themida");
                    return;
                }

                // 指令特征检测
                const resolved_addr = this.resolve_jmp_instruction(open_adapter_addr);
                const header_bytes = api.read_bytes(resolved_addr, 50);
                const has_abnormal_header = !this.is_standard_function_header(header_bytes);
                const triggered = [];
                if (has_abnormal_header) {
                    PolicyReporter.instance.report(this.task_id, true, "发现虚拟机环境7", triggered[0]);
                    return;
                }

                // 特殊字节模式检测
                if (header_bytes[0] === 0x56 && header_bytes[1] === 0xBE) {
                    PolicyReporter.instance.report(this.task_id, true, "发现虚拟机环境8", "BE56");
                    return;
                }
            });
        }

        // 解析JMP指令
        resolve_jmp_instruction(instruction_addr) {
            const opcode = api.read_bytes(instruction_addr, 1);
            return this.is_jmp_instruction(opcode)
                ? instruction_addr + 5 + api.read_dword(instruction_addr + 1)
                : instruction_addr;
        }

        // JMP指令
        is_jmp_instruction(opcode) {
            return [0xE9, 0xEB, 0xE8].includes(opcode[0]);
        }

        // 是否是标准函数名
        is_standard_function_header(header) {
            return (
                (header[0] === 0x55 && header[1] === 0x8B && header[2] === 0xEC) ||
                (header[2] === 0x55 && header[3] === 0x8B && header[4] === 0xEC)
            );
        }

        after() { }
    }
    */

    // 驱动检测
    class Driver_Detection_Task extends ITask {
        task_id = 689052;
        suspicious_drivers = ["VmLoader"];
        blacklisted_drivers = ["IINDLCPXGO", "Ltq", "LoveSnow", "PCCKJCA4", "Dult", "Dultx64_Protect", "GNLAKBOZYKOYCKB", "BBBBas", "FengDrv2787", "SM762FE",
            "vmx_fb", "vm3dmp", "nvd3dum", "nv3dmp", "HideToolz", "wujiejiami", "Sp_Hs", "Passkpp_Demo", "SuperSpeedx64", "SpeedHook", "Gwken", "yxbsq",
            "mengwuji", "Win7Speed", "wwE21wwE", "lonerSpeed_v40", "LtqDrv"];
        device_list = [];
        pdb_cache = new Map();

        get_pdb_path_cached(driver_path) {
            if (this.pdb_cache.has(driver_path)) {
                return this.pdb_cache.get(driver_path) || "";
            }
            try {
                if(api.get_pdb_path) {
                    const pdb_path = api.get_pdb_path(driver_path);
                    this.pdb_cache.set(driver_path, pdb_path);
                    return pdb_path;
                }
                return "";
            } catch (e) {
                return "";
            }
        }

        before() {
            this.device_list = api.enum_device();
        }

        do() {
            this.device_list.forEach(device_path => {
                // 检测路径特征
                if (device_path.split(/[a-zA-Z]:\\/).length > 1) {
                    const lower_path = device_path.toLowerCase().replace("system32", "sysnative");

                    // 特定驱动检测
                    if (lower_path.includes("idbgdrv.sys")) {
                        PolicyReporter.instance.report(this.task_id, true, `非法驱动正在运行: ${device_path}`, lower_path);
                        return;
                    }

                    // PDB特征检测
                    const pdb_path = this.get_pdb_path_cached(lower_path);
                    this.suspicious_drivers.forEach(keyword => {
                        if (pdb_path.includes(keyword)) {
                            PolicyReporter.instance.report(this.task_id, true, `非法驱动正在运行: ${device_path}`, keyword);
                            return;
                        }
                    });
                }

                // 黑名单驱动检测
                this.blacklisted_drivers.forEach(keyword => {
                    if (device_path.includes(keyword)) {
                        PolicyReporter.instance.report(this.task_id, true, `非法驱动正在运行: ${device_path}`, keyword);
                        return;
                    }
                });

                // 虚拟机设备检测
                if (device_path.includes("VEN_15AD")) {
                    PolicyReporter.instance.report(this.task_id, true, `虚拟机设备: ${device_path}`, device_path);
                    return;
                }
            });
        }
    }

    class Player_Report_Task extends ITask {
        task_id = 689997;
        before() {
            globalThis.playername = api.get_player_name();
        }

        do() {
            if (globalThis.playername) {
                PolicyReporter.instance.report(this.task_id, false, globalThis.playername);
            }
        }
    }

    // class Remote_Desktop_Detector_Task extends ITask {
    //     task_id = 689064;
    //     window_util_instance = window_util.instance;

    //     has_remote_desktop() {
    //         const processes = this.window_util_instance.processWindows;
    //         if (!processes) return false;

    //         //return Array.from(processes).some(([pid, _]) => {
    //         return Array.from(processes).some((v) => {
    //             const pid = v[0];
    //             const name = this.window_util_instance.getProcessName(pid);
    //             return ["SunloginClient.exe", "ToDesk.exe"].includes(name);
    //         });
    //     }

    //     // 保持原有vkToString方法结构
    //     vkToString(e) {
    //         switch (e) {
    //             case 1:
    //                 return "MouseLeft";
    //             case 2:
    //                 return "MouseRight";
    //             case 9:
    //                 return "Tab";
    //             case 16:
    //                 return "Shift";
    //             case 17:
    //                 return "Ctrl";
    //             case 18:
    //                 return "Alt";
    //             case 48:
    //                 return "0";
    //             case 49:
    //                 return "1";
    //             case 50:
    //                 return "2";
    //             case 51:
    //                 return "3";
    //             case 52:
    //                 return "4";
    //             case 53:
    //                 return "5";
    //             case 54:
    //                 return "6";
    //             case 55:
    //                 return "7";
    //             case 56:
    //                 return "8";
    //             case 57:
    //                 return "9";
    //             case 112:
    //                 return "F1";
    //             case 113:
    //                 return "F2";
    //             case 114:
    //                 return "F3";
    //             case 115:
    //                 return "F4";
    //             case 116:
    //                 return "F5";
    //             case 117:
    //                 return "F6";
    //             case 118:
    //                 return "F7";
    //             case 119:
    //                 return "F8";
    //             case 120:
    //                 return "F9";
    //             case 121:
    //                 return "F10";
    //             case 122:
    //                 return "F11";
    //             case 123:
    //                 return "F12"
    //         }
    //         return e.toString(16)
    //     }

    //     do() { }
    //     after() { }
    // }

    class BCD_Check_Task extends ITask {
        task_id = 689065;
        bcd_blacklist = ["PatchGuard"];

        do() {
            if (typeof api.get_bcd_info === "function") {
                //const [Identifier, bcd_description, osdevice] = api.get_bcd_info();
                const bcd_info = api.get_bcd_info();
                const Identifier = bcd_info[0];
                const bcd_description = bcd_info[1];
                const osdevice = bcd_info[2];
                if (osdevice) {
                    // 基础报告
                    PolicyReporter.instance.report(this.task_id, false, `bcd | ${Identifier} | ${bcd_description} | ${osdevice}`);

                    // 黑名单检测
                    this.bcd_blacklist.forEach(keyword => {
                        if (bcd_description.includes(keyword)) {
                            PolicyReporter.instance.report(this.task_id, true, `black_bcd | ${Identifier} | ${bcd_description} | ${osdevice}`, keyword);
                            return;
                        }
                    });
                }
            }
        }
    }

    class detect_ip_port extends ITask {
        task_id = 689054;
        ip_black_table = [	
            { ip: "61.139.126.216", port: 0, cheat_name: "水仙" },
            { ip: "103.26.79.221", port: 0, cheat_name: "横刀辅助" },
            { ip: "220.166.64.104", port: 0, cheat_name: "猎手" },
            { ip: "2.59.155.42", port: 0, cheat_name: "暗龙" },
            { ip: "49.234.118.114", port: 0, cheat_name: "暗龙" },
            { ip: "39.98.211.193", port: 0, cheat_name: "通杀" },
            { ip: "103.90.172.154", port: 0, cheat_name: "小可爱" },
            { ip: "106.51.123.114", port: 0, cheat_name: "小可爱" },
            { ip: "119.28.129.124", port: 0, cheat_name: "刺客" },
            { ip: "103.45.161.70", port: 0, cheat_name: "荣耀" },
            { ip: "129.226.72.103", port: 0, cheat_name: "冰橙子" },
            { ip: "27.159.67.241", port: 0, cheat_name: "秒杀辅助" },
            { ip: "117.34.61.140", port: 0, cheat_name: "收费变速齿轮" },
            { ip: "1.117.175.89", port: 0, cheat_name: "北斗驱动变速" },
            { ip: "110.42.1.84", port: 0, cheat_name: "飘刀驱动变速" },
            { ip: "43.243.223.84", port: 0, cheat_name: "游行驱动变速" },
            { ip: "58.87.82.104", port: 0, cheat_name: "游行驱动变速" },
            { ip: "81.70.9.132", port: 0, cheat_name: "游行驱动变速" },
            { ip: "203.78.41.224", port: 0, cheat_name: "大名辅助" },
            { ip: "212.64.51.87", port: 0, cheat_name: "可可加速器" },
            { ip: "47.96.14.91", port: 0, cheat_name: "定制脱机回收" },
            { ip: "203.78.41.225", port: 0, cheat_name: "天使外挂" },
            { ip: "203.78.41.226", port: 0, cheat_name: "天使外挂" },
            { ip: "1.117.12.101", port: 0, cheat_name: "定制脱机外挂" },
            { ip: "150.138.81.222", port: 0, cheat_name: "GEE高端定制外挂" },
            { ip: "43.155.77.111", port: 0, cheat_name: "暗龙" },
            { ip: "81.68.81.124", port: 0, cheat_name: "暗龙" },		
            { ip: "120.26.96.96", port: 0, cheat_name: "定制脱机GEE" },		
            { ip: "47.97.193.19", port: 0, cheat_name: "定制脱机GEE" },		
            { ip: "110.42.3.77", port: 0, cheat_name: "定制脱机GEE" },		
            { ip: "124.70.141.59", port: 0, cheat_name: "私人定制" },  
            { ip: "23.224.81.232", port: 0, cheat_name: "暗兵加速器" },  
            { ip: "42.192.37.101", port: 0, cheat_name: "暗兵加速器" }, 
            { ip: "106.55.154.254", port: 0, cheat_name: "VIP定制" },
            { ip: "47.242.173.146", port: 0, cheat_name: "玛法辅助" },
            { ip: "202.189.5.225", port: 0, cheat_name: "K加速器" }, 			
            { ip: "121.204.253.152", port: 0, cheat_name: "变速精灵" }, 		
            { ip: "106.126.11.105", port: 0, cheat_name: "变速精灵" }, 		
            { ip: "106.126.11.37", port: 0, cheat_name: "变速精灵" }, 		
            { ip: "121.42.86.1", port: 0, cheat_name: "变速精灵" }, 	
            { ip: "112.45.33.236", port: 0, cheat_name: "瑞科网络验证" }, 	
            { ip: "114.115.154.170", port: 0, cheat_name: "定制内部变速" },
            { ip: "127.185.0.101", port: 0, cheat_name: "G盾无限蜂定制功能版464" },
            { ip: "127.132.0.140", port: 0, cheat_name: "猎手PK" },
            { ip: "123.99.192.124", port: 0, cheat_name: "定制大漠" },
            { ip: "175.178.252.26", port: 0, cheat_name: "键鼠大师" },
            { ip: "121.62.16.136", port: 0, cheat_name: "网关-加速-倍攻_脱机" },
            { ip: "121.62.16.150", port: 0, cheat_name: "网关-加速-倍攻_脱机" },
            { ip: "8.137.124.222", port: 80 , cheat_name: "猎手" },
            { ip: "106.52.196.103", port: 8686 , cheat_name: "龙影定制倍攻" },
            { ip: "45.207.9.108", port: 5050 , cheat_name: "login" },
        ];

        do() {
            if (api.get_tcp_table) {
                let ip_port_array = api.get_tcp_table();
                let ip,port;
                for(const ip_black of this.ip_black_table){
                    for(const ip_port of ip_port_array){
                        ip = ip_port[0];
                        port = ip_port[1];
                        // 检测IP和端口
                        if (ip_black.port != 0) {
                            if (ip_black.ip == ip && ip_black.port == port) {
                                PolicyReporter.instance.report(this.task_id, true, `检测到外挂:${ip_black.cheat_name},IP:${ip} 端口:${port}`);
                                return;
                            }
                        }
                        // 只检测IP
                        else if (ip_black.port == 0) {
                            if (ip_black.ip == ip) {
                                PolicyReporter.instance.report(this.task_id, true, `检测到外挂:${ip_black.cheat_name},IP:${ip} 端口:${port}`);
                                return;
                            }
                        }
                    }
                }
            }
        }
    }

    // 任务执行函数
    function execute_task(task_instance) {
		// 计算函数耗时
		//const start_time = Date.now();
        //console.log("execute_task : ", typeof task_instance === "object" ? task_instance.constructor.name : task_instance);
        try { task_instance.before(); } catch (e) { 
            //console.log("before error:", e.name, e.message, e.stack); 
        }
        try {
            //let start = Date.now();
            task_instance.do();
            //console.log(`执行时间: ${Date.now() - start}ms`);
        } catch (e) { 
            //console.log("do error:", e.name, e.message, e.stack); 
        }
        try { task_instance.after(); } catch (e) { 
            //console.log("after error:", e.name, e.message, e.stack); 
        }
		//console.log(`task execution time: ${Date.now() - start_time}ms`);
    }

    console.dbgprint = function (t, ...i) {
        let s = t;
        i.length > 0 && (s += " " + join_with_hex_number(i, " ")),
            api.report(9998, false, s)
    };

    try {
        //console.log("start==");
        let tasks = [
            //new Player_Report_Task(), // 暂时不用
            new detect_ip_port(),
            new machine_detect(), 
            new vmware_detect(), 
            new UnknownCheat(), 
            new memory_detect(), 
            new VM_Detection_Task(),
            new Driver_Detection_Task(), 
            new pe_ico_hash(), 
            new dns_cache_detect(), 
            new window_cheat_detection(),
            //new CheatDetectionTask(), //已合并到window_cheat_detection
            //////new b(), // 暂时不用
            new SecurityMonitorTask(), 
            //new Remote_Desktop_Detector_Task(), // 暂时不用
            new BCD_Check_Task(),
        ];
        for (let t of tasks) {            
            //console.log("start1==",is_detect_cheat);
            if(is_detect_cheat) return;
            execute_task(t);
        } 
        //console.log("end==");
    } catch (e) {console.log("tasks execute error:", e.name, e.message, e.stack); }
})();