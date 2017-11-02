# linux-群聊系统
linux 下的命令行下群聊系统

第一步执行make命令
```shell
make
```
执行完后会生成client和server两个程序
启动服务端,绑定端口
```shell
./server 8080
```
分别启动几个客户端
```shell
./client 服务端地址 端口  当前用户名

```
例如 ./client 192.168.1.111 8080 jack

### 然后就可以聊天啦

![Alt text](https://github.com/cmniefei/linux-chat/blob/master/images%E6%95%88%E6%9E%9C%E5%9B%BE/1.png)
![Alt text](https://github.com/cmniefei/linux-chat/blob/master/images%E6%95%88%E6%9E%9C%E5%9B%BE/2.png)
