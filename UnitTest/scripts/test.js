import * as os from 'os';
import * as api from 'api';

if (api.query_window_info) {
    let windows = api.enum_windows();
    let sigs = new Map();
    for (let i = 0; i < windows.length; i++) {
        let win = windows[i];
        let window_pid = win[0];
        let window_caption = win[1];
        let window_class_name = win[2];
        let hwnd = win[4];
        let infos = api.query_window_info(hwnd);
        if (sigs.has(hwnd) == false) {
            sigs[hwnd] = {
                hwnd: hwnd,
                pid: window_pid,
                caption: window_caption,
                class_name: window_class_name,
                props: new Set()
            };
        }
        for (let i = 0; i < infos.length; i++) {
            sigs[hwnd].props.add(infos[i]);
        }
        if (0) {
            console.log(`${hwnd}|${window_pid}|${window_caption}|${window_class_name}|${(() => {
                let ret = [];
                for (let i of sigs[hwnd]) {
                    ret.push(i);
                }
                return `props(${ret.join("|")})`;
            })()}`)
        }
    }
    for (let hwnd in sigs) {
        let props = sigs[hwnd].props;
        if (props.has("Layered") &&
            props.has("Tab_WindowDC") &&
            props.has("Tab_Hbitmap") &&
            props.has("Tab_Control")) {
            api.report(9066, true, `定制内部变速|${hwnd}|${sigs[hwnd].pid}|${sigs[hwnd].caption}|${sigs[hwnd].class_name}|props(${Array.from(props).join("|")})})`);
        }
    }
}
