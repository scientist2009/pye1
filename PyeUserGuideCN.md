# Pye引擎简介 #
一个运行效率高、占用资源少的拼音输入法引擎.


# Pye引擎上手指南 #

## 怎样安装？ ##

### 从SVN版安装 ###
```
$ svn checkout http://pye1.googlecode.com/svn/trunk/ pye-read-only

$ cd pye-read-only

$ ./configure

$ make

$ sudo make install
```


### 稳定版安装 ###
```
$ wget http://pye1.googlecode.com/files/pye-0.x.x.tar.bz2

$ tar jxvf pye-0.x.x.tar.bz2

$ cd pye-0.x.x

$ ./configure

$ make

$ sudo make install
```


## 内置工具 ##
  * pye-create-mb 创建pye引擎能够识别的系统码表文件.
  * pye-create-umb 创建pye引擎能够识别的用户码表文件.
  * pye-parse-umb 分析由pye引擎生成的用户码表文件.

## API说明 ##
遥遥无期.