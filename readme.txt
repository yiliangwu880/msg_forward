介绍：
	服务器间通讯用。消息转发。
	msg_forward的客户端，其实也是服务器，作为其他服务。
	特点：
		客户端定义唯一id,到msg_forward注册后就参与通讯。
		客户端 之间通讯，不需要配置彼此之间的IP地址,只需要知道对方的id就能实现通讯。

详细功能说明参考： doc.lua文件	

编译方法：
	整个文件夹放到linux目录，安装cmake gcc git等。

	git submodule init      --更新子模块
	git submodule update			
	git checkout -f	master		--强制删掉本地分支， track远程master分支
	去目录 External里面，参考说明编译每个文件夹，生成依赖库。
	主目录执行：sh clearBuild.sh 完成编译

vs浏览代码：
	执行.\vs\run.bat,生成sln文件

	
目录结构：
	mf_svr      ==msg forward server
	mf_driver   ==msg forward client driever。 客户端驱动库
	mf_proto	==msg forward 协议库
	bin			==执行文件
	test 		==测试



