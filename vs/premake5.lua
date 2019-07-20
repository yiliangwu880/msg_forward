--
-- premake5 file to build RecastDemo
-- http://premake.github.io/
--
require "lib"

local action = _ACTION or ""
local outdir = action


WorkSpaceInit  "msg_forward"



Project "mf_svr"
	IncludeFile { 
		"../External/libevent_cpp/External/libevent-2.1.8-stable/out/include/",
		"../External/libevent_cpp/include/",
		"../External/svr_util/include/",
	}

	SrcPath { 
		"../mf_svr/**",  --**递归所有子目录，指定目录可用 "cc/*.cpp" 或者  "cc/**.cpp"
	}
	
Project "test"
	includedirs { 
		"../include/",
		"base/",
	}

	SrcPath { 
		"../test/**",  
	}
	
Project "samples"
	includedirs { 
		"../include/",
		"base/",
	}

	SrcPath { 
		"../samples/**",  
	}
	



    
    
    