#!/bin/sh
#一步测试全部，错误信息输出到 error.txt 

user_name=`whoami`

#$1 进程关键字，会用来grep
function KillProcess(){
    echo "KillProcess $1"
	ps -ef|grep $user_name|grep -v "grep"|grep -v $0|grep $1|awk '{print $2}' |xargs kill -10 &>/dev/null
	
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
	mkdir f_test_combine
	cp test_combine ./f_test_combine -rf
	
	rm error.txt
	rm log.txt
	rm ./svr1/log.txt
	rm ./svr2/log.txt
	rm ./svr3/log.txt
	rm ./combine_svr/log.txt
	rm ./f_test_combine/log.txt
}

function TestCombine()
{
	cd combine_svr
	./mf_svr 
	cd -
	
	sleep 3
	cd f_test_combine
	./test_combine > OutLog.txt
	cd -
	
	KillProcess "./mf_svr"
	echo CombineTest end
	
	grep "ERROR\|error" ./f_test_combine/OutLog.txt >>  error.txt  #追加
	grep "ERROR\|error" ./combine_svr/log.txt >>  error.txt 
}

function TestRecon()
{
	echo restart1
	cd svr1
	StartDaemon ./mf_svr 
	cd -
	
	sleep 2
	StartDaemon ./test_recon
	sleep 2
	
	#reconnect 1
	echo cd svr1
	cd svr1
	echo restart2
	KillProcess "./mf_svr"
	sleep 1
	StartDaemon ./mf_svr 
	

	#reconnect 2
	echo restart3
	sleep 3
	KillProcess "./mf_svr"
	sleep 1
	StartDaemon ./mf_svr 
	
	
	sleep 1
	KillProcess mf_svr
	KillProcess test_recon
	echo TestRecon end
	cd -
	grep "ERROR\|error" log.txt >>  error.txt 
}


#main follow
########################################################################################################
Init
#TestCombine
TestRecon
cat error.txt

