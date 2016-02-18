# Introduction #

本项目基于[S.Huber and M.Held,11](http://cccg.ca/proceedings/2010/paper50.pdf)的算法进行实现。

算法首先计算出平面直线图对应的摩托图，利用原有的平面直线图和摩托图生成一个新的平面直线图，然后模拟新的平面直线图的波前传播，得到新图的直骨架，也即是原图的直骨架。摩托图的引入降低了 split event 的时间消耗，从而保证该算法有较高的运行效率。

我们实现了一个交互友好的界面以展示算法的实现过程以及计算结果。我们还实现了基于直骨架构造三维地形图的功能。

---

项目成员及分工：
  * 曹一溪：实现基于多边形构建摩托图的算法
  * 吴敏：实现基于摩托图构建直骨架的算法
  * 杨昊：设计程序界面与交互方式

This project implements an algorithm proposed by [S.Huber and M.Held,11](http://cccg.ca/proceedings/2010/paper50.pdf).

A motorcycle graph is firstly generated in order to build a new straight-line graph based on the original one. Then the straight skeleton is calculated by emulating a wave-front propagation triggered by the new straight-line graph. The relative efficiency of this algorithm is guaranteed by the decreased time-cost of "split events", which benefits mainly from the use of motorcycle graph.

Our implementation provides a friendly user interface, and an application in terrain model construction.

See [Stefan Huber's website](http://www.cosy.sbg.ac.at/~shuber/research/straightskeleton.html) for more information about straight skeleton.

# Results #

From Tudou

&lt;wiki:gadget url="http://www.tudou.com/programs/view/jbV5F7Gi95s/?resourceId=0\_06\_05\_99"/&gt;

