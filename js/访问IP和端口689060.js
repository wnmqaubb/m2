import * as os from 'os';
import * as api from 'api';

let ip_black_table = [	
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
function detect_ip_port() {
	if (api.get_tcp_table) {
		let ip_port_array = api.get_tcp_table();
		let ip,port;
		for(const ip_black of ip_black_table){
			for(const ip_port of ip_port_array){
				ip = ip_port[0];
				port = ip_port[1];
				// 检测IP和端口
				if (ip_black.port != 0) {
					if (ip_black.ip == ip && ip_black.port == port) {
						api.report(689054, true, `检测到外挂:${ip_black.cheat_name},IP:${ip} 端口:${port}`);
						return;
					}
				}
				// 只检测IP
				else if (ip_black.port == 0) {
					if (ip_black.ip == ip) {
						api.report(689054, true, `检测到外挂:${ip_black.cheat_name},IP:${ip} 端口:${port}`);
						return;
					}
				}
			}
		}
	}
}
detect_ip_port();