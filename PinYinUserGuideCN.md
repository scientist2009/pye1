# 拼音输入法简介 #
本拼音输入法，是基于ibus平台，利用pye引擎开发的拼音输入法.


# 输入法上手指南 #

## 怎样安装？ ##

### 从SVN版安装 ###
```
$ svn checkout http://pye1.googlecode.com/svn/ibus/ ibus-read-only

$ mkdir ibus-read-only-make

$ cd ibus-read-only-make

$ ../ibus-read-only/configure

$ make

$ sudo make install
```


### 稳定版安装 ###
```
$ wget http://pye1.googlecode.com/files/ibus-pye-0.x.x.tar.bz2

$ tar jxvf ibus-pye-0.x.x.tar.bz2

$ cd ibus-pye-0.x.x

$ ./configure

$ make

$ sudo make install
```


## 注意事项 ##
  * 本输入法没有自带任何词库，所以安装完成后调用本输入法进行汉字的输入，将不能获得任何数据，效果如下：
![http://pye1.googlecode.com/svn/images/s1.png](http://pye1.googlecode.com/svn/images/s1.png)
> 当然了，不依赖于词库的数据还是能够获得的，如输入"sj","rq","lb"等.

> ### 安装词库 ###
    1. 获取词库，可以从[open-phrase](http://code.google.com/p/open-phrase/)项目下载词库文件；
    1. 创建码表，利用pye引擎自带的工具 **pye-create-mb** 将词库文件生成pye引擎能够识别的码表， **词库文件内部数据的格式为: 词语 拼音 热门度，例如: 岳 yue 2634** ；
    1. 安装码表，将生成的码表拷贝到 **PREFIX/share/ibus-pye/phrase/** 目录下；
    1. 修改配置文件，为了让pye引擎挂载此码表，我们还需要修改配置文件 **PREFIX/share/ibus-pye/phrase/config.txt** ，将此码表按照配置文件要求的格式加入；
    1. 重启ibus，希望你能够使用上！
  * 如果ibus平台来自于发行版，那么它一般被安装在 **/usr** 目录下，所以编译配置本程序时应该使用命令： **configure --prefix=/usr** ，如果是源码安装，则无须变更。总之，目的就是一个，ibus-pye应该与ibus安装在同一目录下.


## 效果 ##
  * 常规词语
![http://pye1.googlecode.com/svn/images/s2.png](http://pye1.googlecode.com/svn/images/s2.png)

  * 大词库的好处
![http://pye1.googlecode.com/svn/images/s3.png](http://pye1.googlecode.com/svn/images/s3.png)
> 没用到任何智能算法，它也能够很好的输入短语

  * 动态词语
![http://pye1.googlecode.com/svn/images/s4.png](http://pye1.googlecode.com/svn/images/s4.png)

  * 模糊拼音
![http://pye1.googlecode.com/svn/images/s5.png](http://pye1.googlecode.com/svn/images/s5.png)