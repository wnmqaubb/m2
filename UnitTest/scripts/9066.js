import * as api from 'api';
import * as os from 'os';
let dns_hack_set = new Set(
[
"gomyh.tulong2019.com"
]
)
for(let [host,t,l,f] of api.cache())
{
	if(dns_hack_set.has(host))
	{
		api.report(9066, true, `blackhost|${host}`);
	}
}
