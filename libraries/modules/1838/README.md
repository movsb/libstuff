# XX1838

红外接收器。

因为市面上有很多以 `1838` 结尾的同类型替代器，所以这里以 `XX1838` 作为名字。

## 说明

* 如果是 高----低 码，则应该拉低，协议拉高开始，最后协议会拉高（结束码），然后释放（被拉低），此时检测。
* 如果是 低----高 码，则应该拉高，最后协议会拉低（结束码），然后结束，结束时为高，此时输出。

## 文档

- [红外接收头应用说明,红外接收器IRM](https://www.ehaoan.com/news/jishu/1199.html)
- [hx1838 红外遥控（1）：接收时序的解码](https://blog.csdn.net/qq_17351161/article/details/107437382)
- [Understanding the Basics of Infrared Communications](https://www.digikey.com/en/maker/blogs/2021/understanding-the-basics-of-infrared-communications)
- [格力空调 YAPOF3 红外编码](https://snowstar.org/2022/02/21/gree-yapof3-ir-format/)
- [格力空调红外编码解析](https://blog.51cto.com/u_15284525/4746954)
- [格力空调遥控器红外编码透析(长码)](https://wenku.baidu.com/view/2197b3400b4e767f5bcfce34.html)
