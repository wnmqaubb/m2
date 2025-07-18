import * as api from 'api';
import * as os from 'os';
//*************************** 机器码黑名单 ***************************
let device_black_table = [
  //-----------在此分割线之间维护内容-----------
  0x1879ebf4,/*定制外挂*/
  0x1510435,/*定制外挂*/
  0x448d608a,/*定制外挂*/
  0x575f12f9,/*定制外挂*/
  0x38b62b81,/*未知外挂等他来找我*/
  0x2f91c789,/*定制外挂*/
  0x72cbaa93,/*定制外挂*/

  0x6a6ae07e,/*定制外挂*/

  0x673e2344,/*定制外挂*/

  0x48b66c63,/*定制外挂*/

  0x35982067,/*定制外挂*/

  0x3a0e017e,/*定制外挂*/

  0x1e3c2648,/*定制外挂*/

  0x21ea5230,/*定制外挂*/

  0xa448041,/*定制外挂*/

  0x347003d4,/*定制外挂*/

  0x2dd6a2cf,/*定制外挂*/

  0x4ea724a4,/*定制外挂*/

  0x7fd86e13,/*定制外挂*/

  0x74058003,/*定制外挂*/

  0x5c0cf87c,/*定制外挂*/

  0x7863079,/*定制外挂*/

  0x1c340ce2,/*定制外挂*/

  0x1bc136e5,/*通杀外挂0821帝王*/

  0x1510435,/*通杀外挂0821帝王*/

  0x69ce6353,/*通杀外挂0821帝王*/

  0xe383a2e,/*通杀外挂0821帝王*/

  0x11b6b3d2,/*猎手外挂0821帝王*/

  0x3a36e3f9,/*猎手外挂0821帝王*/

  0x2d27931f,/*猎手外挂0821阿拉德*/

  0x127549c7,/*猎手外挂0821精品*/

  0x11b6b3d2,/*猎手外挂0821魔兽*/

  0x5a6b6e3c,/*猎手外挂0821魔兽*/

  0x4464d6ad,/*猎手外挂0821魔兽*/

  0x48cd8dcd,/*猎手外挂0821精品神*/

  0x50a1a24f,/*猎手外挂0821地下秩序*/

  0x29d369af,/*猎手外挂0821阿拉德*/

  0x7b7782a9,/*猎手外挂0821阿拉德*/

  0x6aff240,/*通杀外挂0821决死*/

  0x1d9f35a3,/*通杀外挂0821大象暗黑*/

  0x7d368c21,/*通杀外挂0820驭兽师*/

  0x7b7782a9,/*猎手外挂0820阿拉德*/

  0x48cd8dcd,/*猎手外挂0820帝王*/

  0x29d369af,/*猎手外挂0820阿拉德*/

  0x1a296b05,/*猎手外挂0820帝王*/

  0x567d590a,/*猎手外挂0820帝王*/

  0x6f318d99,/*猎手外挂0820阿拉德*/

  0x573c3c71,/*猎手外挂0820大象暗黑*/

  0x37028580,/*通杀外挂0820精品*/

  0x34ef9abf,/*通杀外挂0820大帝王*/

  0x4a2a9049,/*猎手外挂0820大帝王*/

  0x1e23b0d1,/*麒麟合击GM封*/

  0x51886e32,/*猎手外挂0818龍霄云上*/

  0xc588617,/*猎手外挂0818佣兵之王*/

  0x102854d0,/*小可爱0818暗黑*/

  0x126854a6,/*猎手外挂0818大帝王*/

  0x14d6da63,/*无限法则GM要求*/

  0x33b0676e,/*猎手外挂0818决死*/

  0x34ecbe87,/*猎手外挂0818西夏一品*/

  0x14632f04,/*通杀外挂0817昆仑*/

  0x60951fcb,/*通杀外挂0817昆仑*/

  0xd52bb87,/*猎手外挂0817神宠传说*/

  0x284a2bd,/*脱机外挂0817大帝王*/

  0x733794ea,/*脱机外挂0817大帝王*/

  0x733794ea,/*脱机外挂0817大帝王*/

  0x497ce6c1,/*脱机外挂0817大帝王*/

  0x9e65938,/*猎手外挂0817追忆神器*/

  0x6d5e12c7,/*猎手外挂0817沙城复古*/

  0x5194c861,/*猎手外挂0817沉默传奇*/

  0x1fb577b6,/*猎手外挂0817精品神转*/

  0x32d70d9d,/*荣耀外挂0817大帝王*/

  0x1f2edf2b,/*猎手外挂0817仙牛*/

  0x18db8a9f,/*猎手外挂0817仙牛*/

  0x140960dd,/*猎手外挂0817大帝王*/

  0x34fe0912,/*猎手外挂0816大帝王*/

  0x40258282,/*猎手外挂0816洛南*/

  0x483c78ef,/*猎手外挂0815昆仑*/

  0x11a95b1f,/*猎手外挂0815阿拉德*/

  0x49423c9e,/*未知外挂0815大帝王*/

  0x1ef9e970,/*猎手外挂0814暗夜精灵*/

  0x84c833e,/*简单定制挂需要远程确认*/

  0x9f91b6a,/*猎手外挂0814阿拉德*/

  0x5e50891b,/*猎手外挂0814暗夜精灵*/

  0x697e20f1,/*猎手外挂0814阿拉德*/

  0x42a70664,/*猎手外挂0814轮回*/

  0x54426578,/*猎手外挂0813大帝王*/

  0x1003432c,/*猎手外挂0813地下城*/

  0x68cf6c52,/*猎手外挂0813光芒*/

  0x198e9f29,/*猎手外挂0813江南*/

  0x12d51bfc,/*猎手外挂0812战火*/

  0x5f8fce98,/*猎手外挂0812托儿*/

  0x773163c5,/*猎手外挂0812幻想*/

  0x6c4b3dd4,/*猎手外挂0812王者传奇*/

  0x2498415d,/*猎手外挂0812井中月*/

  0x6a9cca33,/*猎手外挂0812地下城*/

  0x42612de0,/*猎手外挂0812阿萨德*/

  0x35f35f0d,/*猎手外挂0812井中月*/

  0x20e0244c,/*猎手外挂0812暗黑20职业*/

  0x72aeb262,/*猎手外挂0812梦回传奇*/

  0x52f305ad,/*猎手外挂0811不忘初心*/

  0x198e9f29,/*猎手外挂0810养老传奇*/

  0x6e9125ff,/*猎手外挂0810梦幻传奇*/

  0x54398ec6,/*通杀外挂0810阿萨德*/

  0x82f8a3e,/*猎手外挂0810豪情忘忧*/

  0x1ef8b81b,/*猎手外挂0810暗黑天灾*/

  0x419fcd30,/*猎手外挂0810暗黑20职业*/

  0x7bd7d48d,/*猎手外挂0810瓦络兰*/

  0x30f1a7dd,/*猎手外挂0810瓦络兰*/

  0x2e8ddb51,/*猎手外挂0810瓦络兰*/

  0x76a0e9c3,/*猎手外挂0810瓦络兰*/

  0x35b071f9,/*猎手外挂0810暗黑天灾*/

  0x1a534e33,/*通杀外挂0810地下城*/

  0x419FCD30,/*猎手外挂0810王者传奇*/

  0x2be90868,/*猎手外挂0810昆仑*/

  0x20f02279,/*猎手外挂0810昆仑*/

  0x2f5ea1b2,/*猎手外挂0810阿萨德*/

  0x65dce8e9,/*猎手外挂0810阿萨德*/

  0x7c169513,/*猎手外挂0810阿萨德*/

  0x35d02c8d,/*猎手外挂0810阿萨德*/

  0xf149e54,/*猎手外挂0809暗黑*/

  0x20dbcdba,/*猎手外挂0809暗黑*/

  0x7db7d48d,/*通杀外挂0809地下城*/

  0x531ada72,/*通杀外挂0809地下城*/

  0x2df9e9b2,/*未知实锤加速器换解封*/









  0x482bfdc6,/*建立小号拉人*/

  0x5f4ef24a,/*卡BUG刷东西*/

  0x45c2ec31,/*卡BUG刷东西*/

  0x142fdcf2,/*捣乱报警*/

  0x2d093652,/*捣乱*/

  0x15a2facd,/*捣乱*/

  0x65ea59e2,/*捣乱*/

  0x4f5eca91,/*捣乱*/

  0x29c76ada,/*捣乱*/

  0x3704b6,/*捣乱天界*/

  0x329e4092,/*捣乱天界*/

  0x41dc1599,/*捣乱天界*/

  0x1b5a9127,/*捣乱天界*/

  0x4f7d3a01,/*捣乱天界*/

  0x20d0c20b,/*捣乱天界*/

  0x4e38ca72,/*捣乱天界*/

  0x489ceca3,/*脱机*/

  0x770b6e15,/*脱机*/

  0x2c264a91,/*脱机*/

  0x5b6029b1,/*脱机*/

  0x19b0e33a,/*脱机*/

  0x3390eabb,/*脱机*/

  0x3add8e5e,/*脱机*/

  0x29fe3cef,/*脱机*/

  0x5059434b,/*脱机*/

  0x7d697afc,/*脱机*/

  0xdf01e0,/*脱机*/

  0x503fcee1,/*脱机*/

  0x68aeab84,/*脱机*/

  0x3eb8e1e1,/*脱机*/

  0x21aaa4,/*脱机*/

  0x3eb8e1e1,/*脱机0722七夜复古*/

  0x3ac714e,/*脱机0722七夜复古*/

  0x8291984,/*脱机0722七夜复古*/

  0x2944f9aa,/*脱机0722七夜复古*/

  0x723228b6,/*脱机0722七夜复古*/

  0x17af38ba,/*脱机0722七夜复古*/

  0x62cad142,/*脱机0722七夜复古*/

  0x4eed4eee,/*脱机0722七夜复古*/

  0x71c7ded2,/*脱机0722七夜复古*/

  0x48a9bcf1,/*脱机0722七夜复古*/

  0x493774b0,/*脱机0722七夜复古*/

  0x318dda04,/*脱机0722七夜复古*/

  0x6e1c52e7,/*脱机0722七夜复古*/

  0x3aed4790,/*脱机0722七夜复古*/

  0x2be17165,/*脱机0722七夜复古*/

  0x16b0017c,/*脱机0722七夜复古*/

  0x121b3efc,/*脱机0722七夜复古*/

  0x11d56c8f,/*脱机0722七夜复古*/

  0x70572259,/*脱机0722七夜复古*/

  0x51610552,/*脱机0722七夜复古*/

  0x1878570c,/*脱机0722七夜复古*/

  0x7e0a9b13,/*脱机0722七夜复古*/

  0xc8931ea,/*脱机0722七夜复古*/

  0x7788d95f,/*脱机0722七夜复古*/

  0x77f771c2,/*脱机0722七夜复古*/

  0x3fb763d5,/*脱机0722七夜复古*/

  0x5f5be02,/*脱机0722七夜复古*/

  0x44c67521,/*脱机0722七夜复古*/

  0x2d34c43d,/*脱机0722七夜复古*/

  0x7a7e966b,/*脱机0722七夜复古*/

  0x2ed94aec,/*脱机0722七夜复古*/

  0x27d37470,/*脱机0722七夜复古*/

  0x650cca25,/*脱机0722七夜复古*/

  0x3a0eda08,/*脱机0722七夜复古*/

  0x13c46aa8,/*脱机0722七夜复古*/

  0x5fa6d35d,/*脱机0722七夜复古*/

  0x1d4333e0,/*脱机0722七夜复古*/

  0x5a18fd0f,/*脱机0722七夜复古*/

  0x79023365,/*脱机0722七夜复古*/

  0x237ae811,/*脱机0722七夜复古*/

  0x86dd1be,/*脱机0722七夜复古*/

  0x7cd6db5,/*脱机0722七夜复古*/

  0x7a0fddfe,/*脱机0722七夜复古*/

  0x11e9f595,/*脱机0722七夜复古*/

  0x4fc33061,/*脱机0722七夜复古*/

  0x720e9ea6,/*脱机0722七夜复古*/

  0x4afdef86,/*脱机0722七夜复古*/

  0x4ef7abfa,/*脱机0722七夜复古*/

  0x3a384dfd,/*脱机0722七夜复古*/

  0x6bb0ba78,/*脱机0722七夜复古*/

  0x5084650c,/*脱机0722七夜复古*/

  0x2521853f,/*脱机0722七夜复古*/

  0x5827353b,/*脱机0722七夜复古*/

  0x25fb41fd,/*脱机0722七夜复古*/

  0x44a3ac3b,/*脱机0722七夜复古*/

  0x3b8cf203,/*脱机0722七夜复古*/

  0x582f2df6,/*脱机0722七夜复古*/

  0x5631ead9,/*脱机0722七夜复古*/

  0x57cd9c3d,/*脱机0723七夜复古*/

  0x72a0ed5a,/*脱机0723七夜复古*/

  0x72a0ed5a,/*脱机0723七夜复古*/

  0x3786bb,/*脱机0725燕子公益*/

  0x7fa9f5,/*脱机0725燕子公益*/

  0xb1154a3,/*脱机*/

  0x726a484e,/*脱机*/

  0x3376e9a,/*脱机*/

  0x61c4326e,/*脱机*/

  0xbf22b7a,/*脱机0726飞扬*/

  0x58cd6fc3,/*烈焰0726脱机*/

  0x4d9f2e6,/*烈焰0726脱机*/

  0x2dd8dd5f,/*烈焰0726脱机*/

  0x2631c2cb,/*烈焰0726脱机*/

  0x630ce66f,/*烈焰0726脱机*/

  0x25bf5fac,/*烈焰0726脱机*/

  0x59c33caf,/*烈焰0726脱机*/

  0x20b03718,/*烈焰0726脱机*/

  0x426ea3a8,/*烈焰0726脱机*/

  0x5051bbdb,/*虚拟机*/





  0x5542746a,/*水仙插件*/

  0x4f129653,/*水仙插件*/

  0x6e349830,/*水仙插件*/

  0x45b115f9,/*水仙插件*/

  0x468bbec2,/*水仙插件*/

  0x47df8242,/*水仙插件*/

  0x2a6b9f74,/*水仙插件*/

  0x59175a74,/*水仙插件*/

  0xd1ceb8f,/*水仙插件*/

  0x5ef6e544,/*水仙插件*/

  0x29060e3b,/*水仙插件*/

  0x7f08f25b,/*水仙插件*/

  0x38cac411,/*水仙插件*/

  0x4a99e9b8,/*水仙插件*/

  0x457fa8c6,/*水仙插件*/

  0x690375c7,/*水仙插件*/

  0x25542C73,/*水仙插件*/

  0xddd9138,/*水仙插件*/

  0x39e21347,/*水仙插件*/

  0x679457b8,/*水仙插件*/

  0x71c265e,/*水仙插件*/

  0x50f9f969,/*水仙插件*/

  0x39fb6bcb,/*水仙插件*/

  0x5a245e85,/*水仙插件*/

  0x1af2b368,/*水仙插件*/

  0x661970c8,/*水仙插件*/

  0xf80e0dc,/*水仙插件*/

  0x4ff4402,/*水仙插件*/

  0xf1b3a59,/*水仙插件*/

  0x885f36a,/*水仙插件*/

  0x506d6196,/*水仙插件*/

  0x254c8e9,/*水仙插件*/

  0x69ae182d,/*水仙插件*/

  0x1d607f4c,/*水仙插件*/

  0x393b1fe4,/*水仙插件*/

  0x487c27d4,/*水仙插件*/

  0x160a7262,/*水仙插件*/

  0x3759d748,/*水仙插件*/

  0x1419eb25,/*水仙插件*/

  0x7f787386,/*水仙插件*/

  0x59285a21,/*水仙插件*/

  0x7bd61847,/*水仙插件*/

  0x1925aad8,/*水仙插件*/

  0x7aef912f,/*卖水仙插件086*/

  0x10f26633,/*水仙插件*/

  0x7560ffa7,/*水仙插件*/

  0x40ae279d,/*水仙插件*/

  0x66d0cae2,/*水仙插件*/

  0xb9d5f8b,/*水仙插件*/

  0x1b64dc5b,/*水仙插件*/

  0x5d6c2430,/*水仙插件*/

  0x48cb7c81,/*水仙插件*/

  0x6477efc,/*水仙插件*/

  0x73e216da,/*水仙插件*/

  0x566165d5,/*水仙插件*/

  0x278a38e3,/*水仙插件*/

  0x5ab29958,/*水仙插件*/

  0x44415f82,/*水仙插件*/

  0xb9b514f,/*水仙插件*/

  0x523a47b6,/*水仙插件*/

  0x4326913a,/*水仙插件*/

  0x64dc113f,/*水仙插件*/

  0x3b2c448c,/*水仙插件*/

  0x4f742b32,/*水仙插件*/

  0x541302dd,/*水仙插件*/

  0x5e21699b,/*水仙插件*/

  0x9fc9058,/*水仙插件*/

  0xec3e2a5,/*水仙插件*/

  0x58984fec,/*水仙插件*/

  0x43bef704,/*水仙插件*/

  0x2737fa21,/*水仙插件*/

  0x5e73eae6,/*水仙插件*/

  0x5141510a,/*水仙插件*/

  0x5c7d1a6,/*水仙插件*/

  0x679baa0e,/*水仙插件*/

  0x601b3263,/*水仙插件*/

  0x2b1c98f2,/*水仙插件*/

  0xd441657,/*水仙插件*/

  0xb5d739d,/*水仙插件*/

  0x75bc90d1,/*水仙插件*/

  0x4238f752,/*水仙插件086*/

  0x67bf390e,/*水仙插件086*/

  0x9a7dc48,/*水仙插件086*/

  0x7cc4754e,/*水仙插件086*/

  0x22da8f82,/*水仙插件086*/

  0x21c8b06c,/*水仙插件086*/

  0x57de4526,/*水仙插件086*/

  0x11231d90,/*水仙插件086*/

  0x4336a813,/*水仙插件086*/

  0x78ddc75e,/*水仙插件086*/

  0x726aa082,/*水仙插件086*/

  0xafacbc7,/*水仙插件086*/

  0x4e9c768a,/*水仙插件086*/

  0x881dbb4,/*水仙插件086*/

  0x3e793d,/*水仙插件086*/

  0xbbb1952,/*水仙插件086*/

  0x5959faa4,/*水仙插件0815*/

  0x6e3934d3,/*水仙插件0815*/

  0xb4e62b,/*水仙插件0814*/

  0x13898f7a,/*水仙插件0814*/

  0x7978de18,/*水仙插件0814*/

  0x13898f7a,/*水仙插件0814*/

  0x73b2d4b6,/*水仙插件0814*/

  0x6e47e6c7,/*水仙插件0813*/

  0x48745675,/*水仙插件0813*/

  0x72aeb262,/*水仙插件0813*/

  0x6e9125ff,/*水仙插件0813*/

  0x7c63ad5c,/*水仙插件0813*/

  0xe116db9,/*水仙插件0812*/

  0x59975be7,/*水仙插件0812*/

  0x5ee29f3e,/*水仙插件0812*/

  0x1f00dc6,/*水仙插件0812*/

  0x7d832106,/*水仙插件0812*/

  0x23e14cdf,/*水仙插件0812*/

  0x447c0fba,/*水仙插件0811*/

  0x665f99af,/*水仙插件0811*/

  0x73b2d4d6,/*水仙插件0811*/

  0xa95a891,/*水仙插件0811*/

  0x27d30334,/*水仙插件0811*/

  0x3516b9b6,/*水仙插件0810*/

  0x53778dbc,/*水仙插件0810*/

  0x438c0015,/*水仙插件0810*/

  0x2301bbf4,/*水仙插件0810*/

  0x713ab9e7,/*水仙插件0810*/

  0x552159e7,/*水仙插件*/

  0x6311407e,/*水仙插件*/

  0x2e487705,/*水仙插件*/

  0x75b5ba8b,/*水仙插件*/

  0x2cbb65d1,/*水仙插件*/

  0x438c0015,/*水仙插件*/

  0x4d1e9c4,/*水仙插件*/

  0x50659169,/*水仙插件*/





  0x443b6821,/*外挂作者*/

  0x701dfeaf,/*外挂作者*/

  0x39af5e58,/*外挂作者*/

  0x579b5068,/*封包使用者*/

  0x579b5068,/*封包使用者*/

  0x7156d3a1,/*封包使用者*/

  0x1ba2edf0,/*封包使用者*/

  0x407efbcd,/*封包使用者*/

  0x6e1d13cd,/*封包使用者*/

  0x1011a38a,/*封包使用者*/

  0x723228b6,/*封包使用者*/

  0x580cb1d3,/*封包使用者*/

  0x1bcda48f,/*封包使用者*/

  0x27cfcda4,/*封包使用者*/

  0x1846102a,/*封包使用者*/

  0x77c16ccb,/*封包使用者*/

  0x615dc32f,/*封包使用者*/

  0x2bd91a7,/*未知封包*/

  0x3e9290d2,/*未知封包*/

  0x5a2566df,/*未知封包*/

  0x2fe386e2,/*封包使用者*/

  0x5530a8f5,/*封包使用者*/

  0x31596474,/*封包使用者*/

  0x6d01869a,/*封包使用者*/

  0x56aba997,/*封包使用者*/

  0x29c83dce,/*骗GM权限*/

  0xfd4b2a9,/*其他服GM拉人731*/

  0x35da5719,/*地下城GM要求0715*/

  0x22d1d550,/*豪情忘忧GM要求0711*/

  0x5efbfe74,/*开挂炫耀者*/

  0x32f30836,/*拉人*/

  0x490e59ea,/*拉人*/

  0x3c51ee82,/*封包使用者*/

  0x4c3c8d9a,/*封包使用者0810*/

  0x694d5562,/*封包使用者*/

  0x4c91803a,/*封包使用者0626*/

  0x44f3a409,/*封包使用者0626*/

  0x24ea6892,/*封包使用者0628*/

  0x304a8912,/*封包使用者0628*/

  0x613f73f0,/*封包使用者0628*/

  0x4fee018b,/*封包使用者0628*/

  0x64bff564,/*封包使用者0704*/

  0x74ee348b,/*封包使用者0704*/

  0x32f7b455,/*水仙外挂作者0704*/

  0x4410be0e,/*封包使用者0705*/

  0x24441cd6,/*封包使用者0706*/

  0x4430762e,/*封包使用者0706*/

  0x36e3831c,/*封包使用者0706*/

  0x5936be19,/*封包使用者0706*/

  0x41ce9f07,/*封包使用者0706*/

  0x545aa3ea,/*封包使用者0708*/

  0x54e4fa84,/*封包使用者0708*/

  0x49bfa770,/*封包使用者0709*/

  0x1df76cc,/*封包使用者0710*/

  0x50bafd97,/*封包使用者0710*/

  0x4d6b8b02,/*封包使用者0711*/

  0x7b30d897,/*封包使用者0714*/

  0x6842daad,/*封包使用者0714*/

  0x7f97281a,/*封包使用者0715*/

  0xe4bd8c2,/*封包使用者0715*/

  0x6182dd5b,/*封包使用者0717*/

  0x53ca081b,/*封包使用者0718录*/

  0x4c63486,/*封包使用者0718录*/

  0x749e547c,/*封包使用者0721录*/

  0x3243935F,/*封包使用者0721录*/

  0x2ade06dd,/*封包使用者0722录*/

  0x7691de2d,/*封包使用者0723录*/

  0x890f8d3,/*封包使用者0723录*/

  0x7c4f3e2b,/*封包使用者0723录*/

  0x3934d9b9,/*封包使用者0723录*/

  0x3bd73e7a,/*封包使用者0725录*/

  0x51b188b0,/*封包使用者0726录*/

  0x5b102677,/*封包使用者0726录*/

  0x5261c8f0,/*封包使用者0726录*/

  0x2D78F9B3,/*封包使用者0726录*/

  0x66125f77,/*封包使用者0727录*/

  0x24f089bc,/*封包使用者0728录*/

  0x26a0d9af,/*封包使用者0728录*/

  0x4780ec4f,/*封包使用者0728录*/

  0xae8f833,/*封包使用者0728录*/

  0x1da52a2,/*封包使用者0728录*/

  0x5822ce4d,/*封包使用者0728录*/

  0x491adab5,/*封包使用者0729录*/

  0x4ac3be10,/*封包使用者0731录*/

  0x4336a813,/*封包使用者0801录*/

  0x20458a5a,/*封包使用者0801录*/

  0x13bb077b,/*封包使用者0801录*/

  0x2a4c21dd,/*封包使用者0801录*/

  0x7bf8bb0a,/*封包使用者0801录*/

  0x26e97b70,/*封包使用者0801录*/

  0x5981e23b,/*封包使用者0801录*/

  0x3170efab,/*封包使用者0801录*/

  0x38a99b4e,/*封包使用者0801录*/

  0x33ea304c,/*封包使用者0802录*/

  0x59764405,/*封包使用者0803录*/

  0x17a9899,/*封包使用者0804录*/

  0x531b4b8,/*封包使用者0804录*/

  0x30bcfe68,/*封包使用者0805录*/

  0x41e5b7fa,/*封包使用者0805录*/

  0x48276fa8,/*封包使用者0805录*/

  0x6dc493d3,/*封包使用者0805录*/

  0x78ce6fbd,/*封包使用者0805录*/

  0x426ea3a8,/*封包使用者0809录*/

  0x1a5d6bf6,/*封包使用者0809录*/


  //-----------在此分割线之间维护内容-----------

];

let machine_id = api.get_machine_id();

if (device_black_table.indexOf(machine_id) != -1) {
  api.report(9051, true, "机器码黑名单:" + machine_id.toString(16));
}
else {
  api.report(9051, false, "机器码:" + machine_id.toString(16));
}

//*************************** 机器码黑名单 ***************************