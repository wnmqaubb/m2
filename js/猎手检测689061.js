import * as api from 'api';
import * as os from 'os';
let dns_hack_set = new Set(
	[
		"gomyh.tulong2019.com",
		"xiake.xmvz.com",
		"api.ruikeyz.com"
	]
)
api.cache().forEach((host) => {
	if (dns_hack_set.has(host)) {
		api.report(689061, true, `blackhost|${host}`);
	}
});