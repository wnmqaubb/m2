import * as api from 'api';
import * as os from 'os';

const cheat_dat_set = new Set([
	0xF99C,/*�������*/
	0x2D80,/*������*/
]);
const cheat_exe_set = new Set([
	0x271E,/*dragon���*/
	0x9D6B,/*GEE��ʦ*/
	0xC8EB,/*����¼��*/
	0xC046,/*����¼��*/
	0xA02F,/*��������*/
]);
const cheat_set = new Set([
	0xBAE094,/*�̿�*/
	0x44AACC,/*CMD�ѻ�*/
	0x13f40a8c0,/*CMD�ѻ�*/
	0x2972108,/*�ڲ�����*/
	0x2532E42,/*�ڲ�����*/
	0x227CBAF,/*�ڲ�����*/
	0x1731776,/*�ڲ�����*/
	0x3F3F721,/*�ڲ�����*/
	0x479980,/*ǿ������*/
	0x9F9291,/*tyx*/
	0x9CDACC,/*tyx*/
	0x3F36C78,/*˽�˶���*/
	0x47135A,/*˽�˶���*/
	0x46D7F8,/*˽�˶���*/
	0x4C6F61,/*˽�˶���*/
	0x48667D,/*˽�˶���*/
	0x40A813B,/*˽�˶���*/
	0x4B530EC,/*˽�˶���*/
	0x369CFE5,/*˽�˶���*/
	0x3DC813F,/*˽�˶���*/
	0x9CA87F,/*˽�˶���*/
	0x9F9DA2,/*˽�˶���*/
	0x4AE10E8,/*˽�˶���*/
	0x4BBD05C,/*˽�˶���*/
	0x4FD8AE,/*˽�˶���*/
	0x1A123BD,/*˽�˶���*/
	0x2B19598,/*δ�ռ�*/
	0x61463c,/*δ�ռ�*/
	0x9C94FA,/*δ�ռ�*/
	0x13A8A74,/*GEE����*/
	0xFD80F0,/*GEE����*/
	0xFC54C0,/*Ʈ������*/
	0x412557,/*GEE����*/
	0xFE7D20,/*GEE����*/
	0x401C9F,/*GEE����*/
	0x403861,/*GEE����*/
	0x3238B49,/*GEE����*/
	0x2359C60,/*GEE����*/
	0x47C20B2,/*GEE����*/
	0x4619E5,/*GEE����*/
	0xCF7D32,/*GEE����*/
	0x5B6F93,/*�ѻ�����*/
	0x4393AA,/*�ѻ�����*/
	0x4AC02F,/*�ѻ�����*/
	0x5A3DF3,/*�ѻ�����*/
	0x46B7C2,/*�ѻ�����*/
	0x72D26F,/*�����ѻ�����*/
	0xE7320E,/*�����ѻ�����*/
	0x450EDD,/*�����ѻ�����*/
	0x41E8E5,/*�����ѻ�����*/
	0x450EDB,/*�����ѻ�����*/
	0x70BE30,/*�����ѻ�����*/
	0x12CF584,/*�����ѻ�����*/
	0xFFB9DA,/*��Ӱ���������*/
	0x496640,/*����˽����������*/
	0x1705C08,/*ˢ���*/
	0x737BD0,/*�������*/
	0x218CB19,/*����ͬ����*/
	0x469480,/*�౶����*/
	0xA8FD8A,/*�౶����*/
	0x6E3C195,/*GEE���ʻ�ȡ*/
	0x8E334E,/*GEE���ʻ�ȡ*/
	0x43BB34,/*��΢����֤*/
	0x475351,/*����ѻ�*/
	0x13CAD82,/*�ѻ�����*/
	0x43928A,/*�ѻ�����*/
	0x35F16DC,/*�ѻ�����*/
	0x228D82C,/*�ѻ�����*/
	0x43928A,/*�ѻ�����*/
	0x64F7A1,/*�ѻ�����*/
	0x6CE257,/*�ѻ�����*/
	0x10A3044,/*�ѻ�����*/
	0x42C44B,/*�ѻ�����*/
	0x4C8E34,/*�ѻ�����*/
	0x501E65,/*�ѻ�����*/
	0x18761E,/*�ѻ�����*/
	0x468EE1,/*�ѻ�����*/
	0x483031,/*�ѻ�����*/
	0x4ECE51,/*�ѻ�����*/
	0x467588,/*˽�˼���*/
	0x6B2BB0,/*��������*/
	0x47AA51,/*��������*/
	0x78BF40,/*ST������*/
	0x4ABBFD,/*˽�˶���*/
	0x1FED5FF,/*˽�˶���*/
	0x11542CB,/*˽�˶���*/
	0x40B634,/*˽�˶���*/
	0x45DD35,/*˽�˶���*/
	0x8A135F,/*˽�˶���*/
	0xF4A256,/*˽�˶���*/
	0x220177B,/*˽�˶���*/
	0x2BA30B,/*�߶��ѻ�*/
	0x1CC971A,/*�߶��ѻ�*/
	0x1F6883A,/*�߶��ѻ�*/
	0x35BE74E,/*�߶��ѻ�*/
	0x21DA70C,/*�߶˶���*/
	0x26388F0,/*�߶˶���*/
	0x22415C6,/*�߶˶���*/
	0x21266FC,/*�߶˶���*/
	0x209075C,/*�߶˶���*/
	0x24D4422,/*�߶˶���*/
	0x60AC96,/*ɳ��*/
	0x139360E,/*CO���*/
	0x202C8C8,/*CO���*/
	0x1BFA1F8,/*CO���*/
	0x157102F,/*CO���*/
	0x106227E,/*CO���*/
	0x1249F07,/*CO���*/
	0x16B4AEF,/*CO���*/
	0x18FB006,/*CO���*/
	0x1F2395D,/*CO���*/
	0x1DDC2AA,/*CO���*/
	0x112BD3D,/*CO���*/
	0x180160E,/*CO���*/
	0x1BBB844,/*CO���*/
	0x1BCEA63,/*CO���*/
	0x42756DE,/*CO���*/
	0x177f4c6,/*CO���*/
	0x1AFE4F3,/*CO���*/

	0x50BA15,/*�������ѻ�*/
	0x469520,/*δ֪�ѻ�*/
	0x401B64,/*δ֪�ѻ�*/
	0x40AC60,/*��ʹ����*/
	0x51F393,/*�ѻ���Ǯ*
	0x10A3044,/*�ѻ���Ǯ*
	0x747880,/*Ѫ�����*/
	0x404015,/*ck���*/
	0xCF4BAD,/*���پ���*/
	0xF79CA9,/*���پ���*/

	0xC49CA9,/*���پ���*/
	0x69B13C,/*���پ���*/
	0x49FB53,/*��Į�ѻ�*/
	0x12CAFB6,/*ɳ��*/
	0x41802E,/*ɳ��*/
	0xD3AE56,/*ɳ��*/
	0x5350D8,/*���츨��*/
	0x1C81927,/*������*/
	0x41827C,/*������*/
	0x47E001,/*һ��������*/
	0x57F3D5,/*���ٳ���*/
	0x6A97D0,/*���ٳ���*/
	0x4A5FA0,/*����ѻ�*/
	0x4F3330,/*���а���*/
	0x4E6001,/*С����갴��*/
	0x40469E,/*¥����갴��*/
	0x659E28,/*�๦�ܰ���*/
	0x424001,/*ˮ����Ե����*/
	0x63D388,/*PrjMouseAutoClick����*/
	0x13FD966,/*macrorecorder����*/
	0xBDD6B0,/*ͨ�ð���*/
	0x47A629,/*�ѻ�����*/
	0x155C77E,/*��ҫ���*/
	0x18FC3B8,/*��ҫ���*/
	0xFE75D1,/*��ɫ�ѻ����*/
	0x4801AD,/*δ֪ʵ�����*/
	0x10309CD,/*�������ع���*/
	0x520ADB,/*jjss�ƽⲹ��*/
	0x1231D5A,/*��������*/
	0xA5D2D8,/*�����Լ�����*/
	0x471E21,/*�����Լ�����*/
	0xA6DBE2,/*������������*/
	0xA5EF6B,/*�շ���������*/
	0xA72D48,/*�շ���������*/
	0x2F67000,/*�շ���������*/
	0x40DB94,/*����������*/
	0x163634C,/*GEE��ҫ*/
	0x14B8094,/*GEE��ҫ*/
	0x3B6A678,/*GEEͨɱ*/
	0x3AD4FF7,/*GEEͨɱ*/
	0x3B14424,/*GEEͨɱ*/
	0x11F8460,/*GEEͨɱ0922*/
	0x2D6D8B4,/*GEEͨɱ0806*/
	0x206F5CD,/*GEEͨɱ0311*/
	0x20D3123,/*GEEͨɱ0311*/
	0x1FEFBB2,/*GEEͨɱ0304*/
	0x2084ACC,/*GEEͨɱ0318*/
	0x1FD3F92,/*GEEͨɱ0317*/
	0x206F613,/*GEEͨɱ0330*/
	0x361EDE3,/*GEEͨɱ0330*/
	0x217DA46,/*GEEͨɱ0330*/
	0x205AB94,/*GEEͨɱ0327*/
	0x2155FC6,/*GEEͨɱ0328*/
	0x1FF0DED,/*GEEͨɱ0405*/
	0x2030F67,/*GEEͨɱ0408*/
	0x2017A61,/*GEEͨɱ0412*/
	0x2139893,/*GEEͨɱ0414*/
	0x1FF0DED,/*GEEͨɱ0417*/
	0x32553F0,/*GEEͨɱ0425*/
	0x215755C,/*GEEͨɱ0501*/
	0x3BE87BB,/*GEEͨɱ0727*/
	0x511DD66,/*GEEͨɱ0803*/
	0x3319BFA,/*GEEͨɱ0820*/
	0x21A1F62,/*GEEͨɱ0821*/
	0x1A91EDC,/*GEEͨɱ0822*/
	0x3350159,/*GEEͨɱ0826*/
	0x3AC2A31,/*GEEͨɱ0826*/
	0x1AFF4A1,/*GEEͨɱ0827*/
	0x2052D55,/*GEEͨɱ0827*/
	0x1566708,/*GEEͨɱ0831*/
	0x27DEE4B,/*GEEͨɱ0911*/
	0x251D51F,/*GEEͨɱ0919*/
	0x21D8C1B,/*GEEͨɱ1007*/
	0x1BD58AA,/*GEEͨɱ1009*/
	0x21AE209,/*GEEͨɱ1024*/
	0x21D2128,/*GEEͨɱ1029*/
	0x20D5FB9,/*GEEͨɱ1109*/
	0x161F22F,/*GEEͨɱ1112*/
	0x15DF90D,/*GEEͨɱ1120*/


	0x46F1C0,/*һ��С��*/
	0x2610A7,/*���������Զ�����*/
	0x2290253,/*���и���*/
	0x49ABB3,/*δ֪���*/
	0xD5768C,/*�������������*/
	0x52509C,/*�ű�¼��*/
	0x4AA678,/*��������ģ��*/
	0xBF12C0,/*������ģ��*/
	0x643730,/*������ģ��*/
	0x6D9A44,/*������ģ��*/
	0x4C3860,/*������ģ��*/
	0x4256D9,/*������ģ��*/
	0x48BB90,/*������ģ��*/
	0xE407F0,/*������ģ��*/
	0x4EC310,/*������ģ��*/
	0x51CE64,/*�������*/
	0x13B47AA,/*����֤�ѻ�*/
	0x435EB0,/*�ű��༭��*/
	0x40C550,/*��������*/
	0xEC4999,/*δ֪���*/
	0x40128C,/*����ͼ*/
	0x49E894,/*�ű��ѻ�*/
	0x49B740,/*�ű��༭*/
	0x1B1D8EE,/*��������ʦ*/
	0x494B42,/*�������4.5*/
	0x494762,/*�������4.2*/
	0x494ECD,/*�������4.1*/
	0x494B5D,/*�������3.5*/
	0x494C72,/*�������4.5*/
	0x41B06C,/*�������*/
	0x41EAA1,/*�������*/
	0x408BF7,/*�������*/
	0x41F1D0,/*�������*/
	0x20D60AE,/*����3.25*/
	0x2336093,/*����3.26*/
	0x40B325,/*ͬ����*/
	0x40FD28,/*ͬ����*/
	0x46EE11,/*ͬ����*/
	0x129E20C,/*����ͬ����*/
	0x4E1E2D,/*GEE���*/
	0xA36E3C,/*GEE���*/
	0x40AA62,/*δ֪�������*/
	0x5276E3,/*δ֪�������*/
	0x525EA2,/*δ֪�������*/
	0x525D82,/*δ֪�������*/
	0x4DB9E3,/*δ֪�������*/
	0x4DC783,/*δ֪�������*/
	0x40B6C2,/*δ֪�������*/
	0x4E3013,/*δ֪�������*/
	0x4E2EE3,/*δ֪�������*/
	0x4EFD8A,/*δ֪�������*/
	0x4CE567,/*�ű��ѻ�*/
	0xBF2564,/*���縨��*/
	0x4CAF76,/*�һ���*/
	0x5DE81C,/*�����ѻ�*/
	0x1FF0DED,/*GEEͨɱ0417*/
	0x2170AE0,/*GEEͨɱ0506*/
	0x3BDCFBA,/*GEEͨɱ*/
	0x1FBFEE2,/*GEEͨɱ*/
	0x1FEFBB2,/*GEEͨɱ*/
	0x354ACCC,/*GEEͨɱ*/
	0x36D2378,/*GEEͨɱ*/
	0x36A4C40,/*GEEͨɱ*/
	0x36668FA,/*GEEͨɱ*/
	0x1EC137C,/*GEEͨɱ*/
	0x1F63D72,/*GEEͨɱ*/
	0x1F683F0,/*GEEͨɱ*/
	0x1FA7B66,/*GEEͨɱ*/
	0x1FD5EE1,/*GEEͨɱ*/
	0x2DBE1C3,/*GEEͨɱ0524*/
	0x2D3C81C,/*����*/
	0x219B611,/*����*/
	0x2B627EA,/*����*/
	0x2529E34,/*����*/
	0x200EA0D,/*����*/
	0x1840B67,/*����*/
	0x1832415,/*����*/
	0x1840B67,/*����*/
	0x19AB156,/*����*/
	0x1894B36,/*����*/
	0x192DFEE,/*����*/
	0x19162BC,/*����*/
	0x1A5092B,/*����*/
	0x19359F6,/*����*/
	0x1832415,/*����*/
	0x187625E,/*����*/
	0x143FF29,/*����0524*/
	0x19437BE4,/*����������*/
	0x59BEE0,/*ȫ��ģ����*/
	0x2025392,/*GEEͨɱ*/
	0x1FC7C7F,/*GEEͨɱ*/
	0x1FDE3A2,/*GEEͨɱ*/
	0x42284B,/*���¼��*/
	0x41D2FD,/*���¼��*/
	0x46C4BA,/*����¼��*/
	0x40D5AF,/*����¼��*/
	0x47E600,/*����¼��*/
	0x4B20C8,/*���߼�����*/
	0x545B89,/*�����߼�����*/
	0x448CDD,/*ˮ��Ư������*/
	0x608ADD,/*ˮ��Ư������*/
	0x407C72,/*���پ���*/
	0x401768,/*����������*/
	0x679CE0,/*ͨ�ü�����*/
	0x21DFC50,/*ͨɱGOM*/
	0xC3B8DD,/*ͨɱGOM*/
	0x523647,/*ͨɱGOM*/
	0xFCC042,/*ͨɱGOM*/
	0xAEC647,/*ͨɱС����*/
	0xBAE57C,/*GEE����*/
	0x406539,/*GEE����*/
	0x2B3BB37,/*����*/
	0x224D39C,/*����*/
	0x243F7FD,/*����*/
	0x2127D8B,/*����*/
	0x276316D,/*����*/
	0x1C12C82,/*����*/
	0x2359BC6,/*����*/
	0x2887FFA,/*����*/
	0x2848083,/*����*/
	0x21BE843,/*����*/
	0x167BA6E,/*����*/
	0x283EC33,/*����*/
	0x1006FCE6,/*����*/
	0x1006EE77,/*����*/
	0x1006FF45,/*����*/
	0x1006ED2C,/*����*/
	0x1006F998,/*����*/
	0x1006F038,/*����*/
	0x1006EB42,/*����*/
	0x10074B87,/*����*/
	0x10076661,/*����*/
	0x1006EBF3,/*����*/
	0x10072AA6,/*����*/
	0x401504,/*������ģ����*/
	0x47E600,/*������ģ����*/
	0x48C5F2,/*������ģ����*/
	0xC0FEF4,/*�����*/
	0x5CF38F,/*�����*/
	0xFE824C,/*2020С�ɰ�*/
	0x64A146,/*�ѻ��༭��*/
	0x407D66,/*7T������*/
	0x40513D,/*8T������*/
	0x4118C0,/*С�乤��*/
	0x528380,/*�߳�����*/
	0x545D39,/*������*/
	0x47198B,/*ץ������*/
	0x642000,/*˽��С����*/
	0x93CFF6,/*�����*/
	0x5B84FB,/*GEEͨɱ*/
	0x498001,/*GEEͨɱ*/
	0x499BD6,/*GEEͨɱ*/
	0x47705C,/*GEEͨɱ*/
	0x8F3A39,/*GEEͨɱ*/
	0x4AD153,/*GEEͨɱ*/
	0x400154,/*GEEͨɱ*/
	0x4781E2,/*GEEͨɱ*/
	0x4771E2,/*GEEͨɱ*/
	0x403AA8,/*GEEͨɱ*/
	0x403ACC,/*GEEͨɱ*/
	0x456BB7,/*GEEͨɱ*/
	0xB46774,/*CK1003*/
	0xB7324C,/*CK1003*/
	0xB8C35F,/*С�ɰ�*/
	0xFE824C,/*2020С�ɰ�*/
	0xF7CBE0,/*�����*/
	0xC0945D,/*GEE��ʦ*/
	0x77159490,/*�����*/
	0x4EF02C,/*�޵����*/
	0x518890,/*GEE��ʦ*/
	0x593FC7,/*GEE��ʦ*/
	0x50EE75,/*GEE��ʦ*/
	0xC79122,/*ͨɱGOM*/
	0x5CD7CDE,/*����*/
	0xE7C16E,/*��������*/
	0x12C0D5D,/*CK*/
	0x12C3020,/*�������*/
	0x40B76F,/*�������*/
	0x44FD84,/*�������*/
	0x401214,/*�������*/
	0xBAE089,/*�̿����*/
	0x14413D2,/*��ţ����*/
	0x548290,/*������2.03*/
	0x49D9FA,
	0x409E00,
	0x405DF0,
	0x46CBC3,
	0x1FBB35E,
	0x418101,
	0x449C71,
	0x44EA94,
	0x464FD7,
	0x44C4A2,
	0x466D95,
	0x131E022,
	0x1010DD1F,
	0x455188,
	0x468881,
	0xE87B43,
	0xD07A0B,
	0x421456,
	0x40BC0E,
	0x234488B,/*�����*/
	0x45ED26,/*�����*/
	0x4012DC,/*�����*/
	0xC78F17,/*����*/
	0xDF3E94,/*����*/

	0x495CA4,/*���*/
	0x12DE787,/*��ҫ*/
	0x44C9B8,/*��ҫ*/
	0x40D5AF,/*�������¼��*/
	0x18D3209,/*��ҫ*/
	0x40110C,/*δ֪*/
	0x233F3386,/*һ��*/
	0x622FFD,/*�Ϸ����*/
	0x114FC7A,/*Gee��ҫ*/
	0x4AA37E,/*���ƽ�*/
	0x404F52,/*���ƽ�*/
	0x594017,/*GEE��ҫ*/
	0x4298F6,/*����*/
	0x4FD970,/*����*/
	0x47B172,/*����*/
	0x6B6EB40,/*GEE��ҫ*/
	0x25F0EF2,/*�������*/
	0x40A735,/*��������*/
	0x0209F0,/*GEE��ʦ*/
	0x10070B75,/*GEE��ҫ*/
	0x4D6AE2,/*�����*/
	0x4A3C45,/*�����*/
	0x40148C,/*�����*/
	0x545C69,/*�����*/
	0x984500,/*�����߼���*/

]);
// ***************��ģ��ļ�����***************
const cheat_gee_set = new Set([
	0x9B8A,/*��������*/
	0x680E,/*��������*/
	0x04ADF,/*GEE��ҫ*/
	0x3A8D5,/*GEE��ҫ*/
	0x89099,/*GEE��ҫ*/
	0x0154,/*GEEͨɱ*/
	0x8CFD,/*�������*/
	0x3B3D,/*GEE��ҫ*/
	0xCA41,/*����*/
	0xCA2C,/*����*/
	0xE2DC75,/*����*/
	0x95AE,/*����*/
	0x453483,/*GEE��ҫ*/
	0xEBAE,/*���ƹһ�*/
	0x94DD,/*����*/
	0xECCA,/*GEEPK*/
	0x1006FB55,
	0x4B45EF,
	0x402644,
	0x192FB9A,
	0xE09808,
	0xE65E94,
	0xE60ACF,
	0xD0E9,
	0x5247,
	0x33ED,
	0x115D7F,
	0xEBAE,
	0x6F659,
	0x2256,
	0x3C3A,
	0xD36B,
	0x443090,
	0xF33A,
	0x03A72,
	0x23C5,
	0x2644,
	0x8063,
	0x545D39,
	0x545B89,/*������*/
	0x475DA8,/*GEE����*/
	0x406393,/*GEE����*/
	0x40AFC8,/*GEE����*/
	0x4106F9,/*GEE����*/
	0x406345,/*GEE����*/
	0x3C7EB40,/*GEE��ʦ*/
	0x6841F0,/*GEE����*/

]);

const window_class_name_black_set = new Set([
	"IME",
	"MSCTFIME",
	"tooltips_class32",
	"FORM_PROJECT1_FORM1_CLASS:0",
	"QQPinyinImageCandWndTSF",
	"VBBubbleRT6",
	"AutoIt v3 GUI",
]);

let threads = api.enum_threads();//[[pid,processname,thread_start_address]]
let windows = api.enum_windows();//[[window.pid, window.caption,window_class_name,window.is_hide_process]]
let module_name_map = new Map(api.get_module_names()); //std::map<uint32_t,std::vector<std::tuple<std::string,uint64_t, uint32_t>>>
let process_name_map = new Map(api.get_process_names()); //std::map<uint32_t, std::string>

function get_module_name(pid,thread_start_address){
	let module_name = "";
	let module_name_array;
	if(module_name_map.has(pid)){
		module_name_array = module_name_map.get(pid);
		for(let i=0;i<module_name_array.length;i++){
			let module = module_name_array[i];
			if(module[1] <= thread_start_address && thread_start_address <= (module[1]+module[2])){
				return module[0];
			}
		}
	}
	
	return module_name;
}

let reason = "";
let module_name = "";	
let win;
let window_pid;
let window_caption;
let window_class_name;
let process_name;
let thread;
let thread_pid;
let thread_processname;
let thread_start_address;

function check_no_access_process_rate()
{
	let sum = 0;
	let no_access_sum = 0;
	
	for(let [pid,modules] of module_name_map)
	{
		sum++;
		if(modules.length == 0)
		{
			no_access_sum++;
		}
	}
	let rate = no_access_sum / sum;
	if(rate >= 0.8)
	{
		api.report(9022, false, `��Ȩ��/�ܽ��� ${no_access_sum}/${sum} (${rate})`);
	}
}
check_no_access_process_rate();

for(let i=0;i<windows.length;i++)
{
	win = windows[i];
	window_pid = win[0];
	window_caption = win[1];
	window_class_name = win[2];
	process_name = "";
	if(process_name_map.has(window_pid)){
		process_name = process_name_map.get(window_pid);
	}
	if (process_name == "" || (!window_class_name_black_set.has(window_class_name) && window_class_name.length != 10))
	{
		continue;
	}
	
	for(let j=0;j<threads.length;j++)
	{
		thread = threads[j];
		thread_pid = thread[0];
		thread_processname = thread[1];
		thread_start_address = thread[2];
		if(window_pid == thread_pid)
		{
			if ((window_class_name == "tooltips_class32" || window_class_name == "MSCTFIME")
				&& (cheat_gee_set.has(thread_start_address) || cheat_gee_set.has(thread_start_address & 0xFFFFFF)))
			{
				reason = "�������ֻ�����ҫ������Ŵ���1��������������Ϊ:" + thread_processname;
				break;
			}
			module_name = get_module_name(thread_pid,thread_start_address);		
			
			if (module_name == "")
			{
				// ***************��ģ��ļ�����***************
				if (cheat_gee_set.has(thread_start_address & 0xFFFF) ||	cheat_gee_set.has(thread_start_address & 0xFFFFF))
				{
					reason = "�������ֻ�����ҫ������Ŵ���2��������������Ϊ:" + thread_processname + "|" + thread_start_address.toString(16);
					break;
				}
				continue;
			}			
			if (module_name.search(/.exe$/) > 1)
			{
				if (window_class_name == "QQPinyinImageCandWndTSF" && (thread_start_address & 0xFFFF) == 0x3020)
				{
					reason = "GEE��ʦ������Ŵ���3��������������Ϊ:" + thread_processname;
					break;
				}
				if (window_class_name == "AutoIt v3 GUI" && (thread_start_address & 0xFFFF) == 0x800A)
				{
					reason = "����һ��С�����������Ϊ:" + thread_processname;
					break;
				}
				if (window_class_name == "FORM_PROJECT1_FORM1_CLASS:0" && (thread_start_address & 0xFFFF) == 0x2256)
				{
					reason = "�������ֻ�����ҫ������Ŵ���4��������������Ϊ:" + thread_processname;
					break;
				}

				if (window_class_name == "tooltips_class32" && (thread_start_address & 0xFFFF) == 0xD8C5)
				{
					reason = "����GEE��ʦ�࿪��������Ϊ:" + thread_processname;
					break;
				}

			}
			else if (module_name.search(/.dll$/) > 1)
			{
				if (window_class_name.length == 10
					&& ((thread_start_address & 0xFFFF) == 0xE070
						|| (thread_start_address & 0xFFFF) == 0xD490))
				{
					reason = "���ּ�����ң�����Ϊ:" + thread_processname;
					break;
				}
				if (window_class_name == "VBBubbleRT6" && (thread_start_address & 0xFFFFF) == 0xFB401)
				{
					reason = "����������Ŵ�������Ϊ:" + thread_processname;
					break;
				}
			}
			else if (module_name.search(/.tap$/) > 1
				&& ((thread_start_address & 0xFFFF) == 0xDD1F
					|| (thread_start_address & 0xFFFF) == 0x58F0))
			{
				reason = "������ҫ������Ŵ�������Ϊ��7��������:" + thread_processname;
				break;
			}
		}
	}
		
	if(reason != "") break;
}
if(reason == "")
for(let j=0;j<threads.length;j++)
{
	thread = threads[j];
	thread_pid = thread[0];
	thread_processname = thread[1];
	thread_start_address = thread[2];
	if(thread_pid < 1 || thread_start_address < 1) continue;
	if (cheat_set.has(thread_start_address))
	{
		reason = "��ҳ���Ϊ:" + thread_processname;
		break;
	}
	
	module_name = get_module_name(thread_pid,thread_start_address);
	if (module_name == "")
	{
		continue;
	}
	
	if (module_name.search(/.exe$/) > 1)
	{
		if (cheat_exe_set.has(thread_start_address & 0xFFFF))
		{
			reason = "���ִ�ʦ����������ң�����Ϊ:" + thread_processname;
			break;
		}
	}
	else if (module_name.search(/.dat$/) > 1)
	{
		if (cheat_dat_set.has(thread_start_address & 0xFFFF))
		{
			reason = "���ֶ������ѻ���ң�����Ϊ:" + thread_processname;
			break;
		}
	}
	
}

if(reason != ""){
	api.report(9022, true, reason);
}
