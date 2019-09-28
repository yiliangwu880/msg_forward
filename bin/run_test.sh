#!/bin/sh
#启动测试，终端输出测试错误信息。
#启动关闭进程，把进程启动为后台 

user_name=`whoami`

#$1 进程关键字，会用来grep
function KillProcess(){
    echo "KillProcess $1"
	ps -ef|grep $user_name|grep -v "grep"|grep -v $0|grep $1|awk '{print $2}' |xargs kill -9 &>/dev/null
	
}

#$1 start cmd
function StartDaemon()
{
	if [ $# -lt 1 ];then
		echo "StartDaemon miss para 1"
	fi
	echo StartDaemon $1
	nohup $1 &>/dev/null &
}

function Restart()
{
	KillProcess $1
	StartDaemon $1
}

function Init()
{
	#复制执行文件
	cp mf_svr ./svr1 -rf
	cp mf_svr ./svr2 -rf
	cp mf_svr ./svr3 -rf
	cp mf_svr ./combine_svr -rf
	
	rm log.txt
	rm ./svr1/log.txt
	rm ./svr2/log.txt
	rm ./svr3/log.txt
	rm ./combine_svr/log.txt
}

function TestCombine()
{
	cd combine_svr
	./mf_svr 
	cd -
	
	sleep 3
	./test_combine
	
	KillProcess "./mf_svr"
	echo CombineTest end
}

function TestRecon()
{
	cd svr1
	./mf_svr 
	cd -
	
	sleep 3
	StartDaemon ./test_recon
	
	echo cd svr1
	cd svr1
	
	echo restart1
	sleep 2
	KillProcess "./mf_svr"
	sleep 2
	./mf_svr 
	
	echo restart2
	sleep 2
	KillProcess "./mf_svr"
	sleep 2
	./mf_svr 
	
	cd -
	
	echo TestRecon end
}


#main follow
########################################################################################################
Init
#TestCombine
TestRecon
exit
StartDaemon "./mf_svr"

sleep 3
KillProcess "./mf_svr"

