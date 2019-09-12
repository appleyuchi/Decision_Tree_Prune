注：
该说明使用markdown书写，下载后不可直接观看，
需要借助haroopad等可视化工具。

环境：

要求 | 查询命令 |我的版本
:-: | :-: | :-: 
make版本|make --version| 4.2.1
glibc版本 | ldd --version| 2.28
Xubuntu|uname -a|4.18.0-25-generic


make命令：

make命令 | 生成文件 | 生成文件的作用 | 
:-: | :-: | :-: 
make c4.5 | c4.5 | 可用来建模
make c4.5gt | c4.5gt.c| 所有源代码合并到一个文件中,<br>该文件为建模而服务
make c4.5rules|c4.5rules|分类树的规则
make c4.5rulesgt|c4.5rulesgt.c| 所有源代码合并到一个文件中,<br>该文件为生成分类规则而服务

软件使用命令（以crx为例）：

使用命令 | 需要哪些文件| 生成哪些文件 | 作用 |
:-: | :-: | :-: | :-: 
./c4.5 crx|crx.names,crx.data|crx.tree,crx.unpruned|建立crx训练后的模型
./c4.5rules|crx.tree,crx.unpruned|-|直接终端输出分类规则

这个软件的配套使用说明书是：《C4.5: Programs for Machine Learning》，
整本书我都看了，只讲了怎么建模、验证（带有类别tag的数据集），但是没有讲测试（不带类别tag的数据集）

注意，根据作者本人的邮件回复，C4.5并不能用来"批量预测"数据集，除非你自己编写额外的代码。
所以，要么使用python接口，要么使用C5.0
C5.0的使用在另外一个文件夹中