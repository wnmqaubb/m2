class SecurityMonitorTask extends ITask {
        task_id = 689062;
        window_report_id = 689022;
        module_report_id = 689021;
        suspiciousModules = new Set([0x43000, 0x56000]);
        constructor() {
            super();
            this._window_util = window_util.instance;
            this.currentProcessId = api.get_current_process_id()
        }
        before() {
            this._window_util.update();
        }
        after() {}
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
                    if (typeof api.is_window_valid === "function" && !api.is_window_valid(handle)) 
                        continue;
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
        checkCurrentProcessModules(processId) {
            const modules = this._window_util.getProcessModules(processId);
            if (!modules || !modules.length) return;
            for(const v of modules){
                const moduleName = v[0];
                if (moduleName.toLowerCase() === "hook32.dll") {
                    PolicyReporter.instance.report(this.module_report_id, true, `多倍外挂！一定要封号|${processId}|${this.getProcessName(processId)}`, moduleName.toLowerCase());
                    return;
                }
            }
        }

        // 检查其他进程
        checkOtherProcess(processId, windowList) {
            if (processId === 0) {
                //this.handleSystemProcess(windowList);
            }
            this.checkSuspiciousModules(processId);
        }

        // 处理系统进程
        handleSystemProcess(windowList) {//[windowText, windowClass, processId, ownerHandle]
            //const windowTitles = windowList.map(([window_text, _, __, owner_handle]) => `${window_text} (${owner_handle})`).join(" | ");
            const windowTitles = windowList.map(v => `${v[0]}(${v[3]})`).join("|");
            const isSuspicious = !["Build", "NVOGLDC", "Default IME", "背包仓库管理"].some(keyword => windowTitles.includes(keyword));
            if (isSuspicious) {
                this.reportIssue(this.task_id, `pid0|0|${this.getProcessName(0)}|${windowTitles}`, windowTitles);
                return;
            }
        }
        checkSuspiciousModules(processId) {
            const modules = this._window_util.getProcessModules(processId);
            if (!modules || !modules.length) return;
            const firstModule = modules[0];
            this.checkNtdllModule(processId, firstModule);
            for(const module of modules){
                const moduleName = module[0];
                const moduleBase = module[1];
                //if (moduleName.toLowerCase().includes("system32\\nfapi.dll")) 原代码是这个,应该是有问题的,模块名称应该是nfapi.dll
                if (moduleName.toLowerCase().includes("nfapi.dll")) {
                    if(this.handleNfapiModule(processId, moduleBase, module)) break;
                }
            }
        }
        // 正常的程序第一个模块都是自己,第二个模块才是ntdll.dll
        //checkNtdllModule(processId, [moduleName, moduleBase]) {
        checkNtdllModule(processId, ml) {
            const moduleName = ml[0];
            const moduleBase = ml[1];
            if (moduleName.toLowerCase().includes("ntdll.dll")) {
                this.reportIssue(this.task_id, `ntdll|module|${processId}|${this.getProcessName(processId)}|${moduleName}|${moduleBase.toString(16)}`, "ntdll.dll");
                return;
            }
        }

        //handleNfapiModule(processId, moduleBase, [moduleName, , moduleSize]) {
        handleNfapiModule(processId, moduleBase, ml) {
            const moduleName = ml[0];
            const moduleSize = ml[2];
            const reportMessage = `nfapi|module|${processId}|${this.getProcessName(processId)}|${moduleName}|${moduleSize.toString(16)}`;
            if (this.suspiciousModules.has(moduleBase)) {
                PolicyReporter.instance.report(this.module_report_id, true, `多倍外挂！一定要封号|${processId}|${this.getProcessName(processId)}`, moduleBase);
                return true;
            }
            // else {
            //     this.reportIssue(this.task_id, reportMessage, "nfapi.dll");
            //     return true;
            // }
            return false;
        }

        // 辅助方法
        getProcessName(processId) {
            return this._window_util.getProcessName(processId);
        }
        reportWindowIssue(processId, window_text, window_class, signature) {
            const processName = this.getProcessName(processId);
            const entropy = calc_entropy(window_class);
            this.reportIssue(this.window_report_id, `geels|wnd|${processId}|${processName}|${window_text}|${window_class}(${entropy})`, signature);
            return;
        }
        reportIssue(reportCode, message, signature) {
            PolicyReporter.instance.report(reportCode, true, message, signature);
            return;
        }
    }