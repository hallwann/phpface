# phpface

本程序为c+php编写，可以检测是否活体，人脸位置，年龄，角度等。基础库采用Opencv 3.4.16和Arcface3.0。

###### 发行版：

​	dest目录可直接应用于linux，运行前先需运行install.sh安装依赖。

​	config.ini 填入从ai.arcsoft.com.cn申请的appkey和sdkkey。

###### 开发版：

​	主程序为demo.cpp，进入build运行“**cmak ..** ”再运行“**make**”。

​	dev_install.sh为开发环境安装脚本。

运行phpface -h（--help)查看用法