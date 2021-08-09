# super-proxy

#### 编译环境准备

```sudo apt install g++ make libssl-dev zlib1g-dev```



#### 编译server

```make server```


#### 运行

```./server```

#### 请求测试

```
增加配置：
http post 方式访问 http://local_host:8080/config，
参数：
        "time": 请求目标地址连接的时间戳 ,
        "host": 目标host,
        "url": 目标url,
        "scheme": http或https,
        "headers": None,
        "port": 目标端口号,
        "mothed": 目标地址访问方式 POST/GET,
        "contentType": "",
        "body": ""
返回:
    增加的配置id


删除配置:
http post 方式访问 http://local_host:8080/config/del，
参数：
        "id": 配置id,

```

### 安装
`sudo ./install.sh`



### 运行说明

安装成功后，服务会已docker实例的方式运行，执行记录默认保存在 /var/log/httpjob 目录下
