介绍：
	服务器间通讯用。消息转发。
	msg_forward的客户端，通常做为其他服务的服务器。
	特点：
		多个客户端，定义唯一id,到msg_forward注册后，就可以互相之间通讯了。
		客户端 之间通讯，不需要配置彼此之间的地址,只需要配置msg_forward的地址就能实现通讯。
	
编译方法：
	整个文件夹放到linux目录，安装cmake gcc git等。

	git submodule init      --更新子模块
	git submodule update			
	git checkout -b	master		--强制删掉本地分支， track远程master分支
	主目录执行：sh clearBuild.sh 完成编译

vs浏览代码：
	执行.\vs\run.bat,生成sln文件

功能说明参考： doc.lua文件
	
目录结构：
	mf_svr      ==msg forward server
	mf_driver   ==msg forward client driever。 客户端驱动库
	mf_proto	==msg forward 协议库




