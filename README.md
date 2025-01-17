# 项目说明
基于C/S架构的传奇反外挂系统，涉及多个模块和工具。以下是对各个项目的简要总结：
1.**Lightbone**：附属工具和一些第三方库，可能用于支持其他模块的功能。
2.**pe_bliss**：PE文件相关的操作库，用于处理可执行文件的结构和内容。
3.**newclient**：核心的反外挂模块，通过packer项目封装到登录器中。在登录器中，通过Hook CreateProcessInternalW和ResumeThread函数，完成反外挂模块DLL（即newclient）的加载。
4.**service**：核心的服务端中间层，负责接收newclient和gatef的连接，并将相关的业务请求转发到logicserver进行处理。
5.**logicserver**：核心的服务端业务层，处理service转发过来的请求，并返回相关的处理后数据或状态。
6.**packer**：相当于一个处理newclient注入的Loader工具，用于将反外挂模块封装到登录器中。
7.**packer_tool**：封装工具，调用packer来将newclient封装到对应的登录器中。
8.**gatef**：GM（游戏管理员）用的后台管理系统，主要管理所有连接过来的在线玩家，显示玩家是否开挂的日志，并对玩家做出相应的处罚。
9.**admingate**：管理员后台，用于管理所有的gatef和gatef中的所有玩家。


## data文件夹说明 

### policy 策略
策略有多种,比较task_basic.dll插件下发,js脚本,cmd命令_
#### js脚本
脚本主要是用来下发到客户端执行的,通过c++公开的一些函数用js来调用的方式来执行各种检测,这是一种比较重要的方式,可以热更新,一般是3分钟下发一次
到客户端执行,比较轻量化


# 6.**packer**：
## 封装工具，调用packer来将newclient封装到对应的登录器中。 
### 1.将NewClient.dll以新节的文件放到登录器的最后一个节中
### 2.将原oep的地址改到新节的client_entry(获取client_entry的rva地址)
### 3.调用client_entry()前先xor解密下,并且要判断是父进程还是子进程,只在子进程中执行
