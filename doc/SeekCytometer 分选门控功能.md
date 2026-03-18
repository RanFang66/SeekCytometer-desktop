# SeekCytometer 上位机分选门控功能

## 需求概述

SeekCytometer分选上位机需要在分析结果图上进行可视化的分选门控框绘制操作，通过鼠标绘制不同的门控框。门控框对应到直方图或者三点图中坐标系的范围，并将门控框数据下发到Zynq SoC中，作为实时分选的判据。不同的可视化图形支持不同的门控框。具体对应如下：

| 门控类型                  | 支持数据图类型    | 门控含义                                                     | 对应门控数据                                                 | 开发状态     |
| ------------------------- | ----------------- | ------------------------------------------------------------ | ------------------------------------------------------------ | ------------ |
| 区间门控(Interval Gate)   | 直方图(histogram) | X轴对应的区间范围                                            | （X_min,X_max)                                               | 开发中       |
| 矩形门控(Rectangle Gate)  | 散点图(scatter)   | 坐标系中矩形框对应的范围                                     | 矩形的左下角顶点(x_min, y_min)和矩形的右上角顶点(x_max, y_max) | 开发中       |
| 多边形门控(Polygon Gate)  | 散点图(scatter)   | 坐标系中多边形所包围的范围(多边形的边不能有交叉，且最多支持12个顶点) | 多边形的顶点(x1, y1),(x2, y2)...(xn,yn)                      | 计划后续实现 |
| 椭圆门控(Ellipse Gate)    | 散点图(scatter)   | 坐标系中椭圆所包含的范围                                     | 椭圆外切矩形的左下角顶点(x_min,y_min)和右上角顶点(x_max, y_max) | 计划后续实现 |
| 四象限门控(Quadrant Gate) | 散点图(scatter)   | 通过一个十字线将整个散点图分为4个象限对应的区域，            | 十字线的中心位置（x0, y0)                                    | 计划后续实现 |







## 门控可视化交互实现

在SeekCytometer的worksheet模块中，实现不同门控的可视化交互操作，包括门控的绘制添加，删除。后续还会开发编辑修改的功能。

| 门控类型                  | 绘制方式                                                     | 绘制过程显示                                  | 绘制完成显示                           |
| ------------------------- | ------------------------------------------------------------ | --------------------------------------------- | -------------------------------------- |
| 区间门控(Interval Gate)   | 点击添加区间门控按钮，然后鼠标左键点击到直方图的PLOT区域开始绘制，如果不是在直方图的PLOT区域内则进行提示。拖动鼠标并再次点击左键后完成绘制。两次点击的X轴范围表示最终的区间范围。 | 蓝色虚线绘制的区间（两条竖线夹一条横线\|-\|） | 红色实线绘制的区间（两条竖线夹一条横线 |
| 矩形门控(Rectangle Gate)  | 点击添加矩形门控按钮，然后鼠标左键点击到散点图的PLOT区域开始绘制，如果不是在散点图的PLOT区域内则进行提示。拖动鼠标并再次点击左键后完成绘制。两次点击的点作为顶点确定唯一的矩形区域。 | 蓝色虚线绘制的矩形                            | 红色实线绘制的矩形                     |
| 多边形门控(Polygon Gate)  | 点击添加多边形门控按钮，然后鼠标左键点击到散点图的PLOT区域开始绘制，如果不是在散点图的PLOT区域内则进行提示。拖动鼠标再次点击左键绘制多边形的下一个顶点，一直到双击鼠标左键表示完成多边形的最后一个顶点绘制。所有顶点依次连接确定唯一多边形，如果中间出现边相交或者顶点超过12个则提示绘制失败。 | 蓝色虚线依次连接各个顶点的开放折线            | 红色实线依次连接各个顶点的开放折线     |
| 椭圆门控(Ellipse Gate)    | 点击添加椭圆形门控按钮，然后鼠标左键点击到散点图的PLOT区域开始绘制，如果不是在散点图的PLOT区域内则进行提示。拖动鼠标并再次点击左键后完成绘制。两次点击的点作为顶点确定唯一矩形，该矩形的内切椭圆即为最终绘制的椭圆。 | 蓝色虚线绘制的椭圆                            | 红色实线绘制的椭圆                     |
| 四象限门控(Quadrant Gate) | 点击添加四象限门控按钮，然后鼠标左键点击到散点图的PLOT区域开始绘制，如果不是在散点图的PLOT区域内则进行提示。鼠标左键点击的位置即为四象限门控的原点，以该点为中心将整个散点图区域划分为四象限区域。 | 蓝色虚线绘制的十字线                          | 红色实线绘制的十字线                   |

门控的显示要注意门控对应的图像中的坐标系和绘图坐标系的转换，尤其当门控所处的散点图或者直方图的坐标系范围发生变化时，对应的门控也需要重新绘制。







## 门控数据管理

门控数据通过POSTGRESQL数据库进行管理，GATE表的定义如下：

```sql
CREATE TYPE GateType AS ENUM('rectangle', 'polygon', 'ellipse', 'interval', 'quadrant');
CREATE TABLE Gates (
    gate_id                     SERIAL PRIMARY KEY NOT NULL,
    worksheet_id                INT NOT NULL,
    gate_name                   VARCHAR(64) NOT NULL,
    gate_type                   GateType NOT NULL,
    -- parent_population_id        INT,
    x_axis_id                   INT NOT NULL,
    y_axis_id                   INT,
    x_mearsure_type             MeasureType NOT NULL DEFAULT 'Height',
    y_mearsure_type             MeasureType,
    gate_data                   JSONB NOT NULL,
    FOREIGN KEY (worksheet_id) REFERENCES WorkSheets(worksheet_id) ON DELETE CASCADE,
    FOREIGN KEY (x_axis_id) REFERENCES DetectorSettings(detector_setting_id) ON DELETE CASCADE,
    FOREIGN KEY (y_axis_id) REFERENCES DetectorSettings(detector_setting_id) ON DELETE CASCADE,
    UNIQUE (worksheet_id, gate_name)
);
```



其中gate_data表示不同类型门控对应的一个或多个坐标点数据。

