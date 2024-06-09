![image](https://github.com/fudanrobotuser/igh/assets/171946866/c590122b-336e-45cb-a2ca-15010b9e334e)使用 ethercat igh 做伺服电机驱动，

其中， 左腿6个关节，右腿6个关节，腰部3个关节，左臂7个关节，右臂7个关节，

左臂右臂为开璇一体化关节电机，使用 ethercat 总线，

整个下肢为 小象电机+elmo 伺服驱动， 组合

基于 igh 源码中的  example/dc_user/main.c 文件来直接修改


igh 控制器层的 的状态变化流程如下图

