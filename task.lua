
测试多个mf_svr,高可用

实现测试用例，根据doc功能来写。

protobuf 3
{
	配置库： 写配置文本， 定义配置proto结构， 工具生成 cpp配置类。 用户直接使用配置对象。 
		用户加配置需要做： 1）proto文件加字段，2）配置文本加内容，3）（工具自动更新类）代码直接读取对象字段。
	
}
	考虑更好的配置库： 用户加配置需要做： 1）配置文本加内容，3）（工具自动更新类）代码直接读取对象字段。
		不用protobuf,工具做到精简。 参考帝华的xml生成cpp