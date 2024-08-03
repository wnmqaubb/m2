set "Release=%1"

copy %Release%\task_1_hide_process_detect.obj.bin				%Release%\bin\shell\进程检测\进程检测.bin
copy %Release%\task_2_static_detect.obj.bin						%Release%\bin\shell\匹配封挂\标准检测.bin
copy %Release%\task_3_static_detect2.obj.bin					%Release%\bin\shell\匹配封挂\标准检测1.bin
copy %Release%\task_4_speed_detect.obj.bin						%Release%\bin\shell\变速检测\变速检测.bin
copy %Release%\task_5_debugview.obj.bin							%Release%\bin\shell\辅助封挂\窗口检测1.bin
copy %Release%\task_6_eyuyan_kill.obj.bin						%Release%\bin\shell\辅助封挂\窗口检测.bin
copy %Release%\task_7_admin_device_name.obj.bin					%Release%\内部专用云代码\驱动设备名检测.bin
copy %Release%\task_8_eyuyan_noaccess_kill.obj.bin				%Release%\内部专用云代码\易语言无权限窗口检测.bin
copy %Release%\task_9_show_window_hook.obj.bin					%Release%\内部专用云代码\非法窗口检测.bin
copy %Release%\task_10_handle_detect.obj.bin					%Release%\内部专用云代码\游戏句柄检测.bin

