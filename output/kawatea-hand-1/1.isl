cut [0] [X] [40]
color [0.1] [0, 0, 0, 255]
merge [0.0] [0.1]

cut [1] [X] [80]
color [1.1] [255, 255, 255, 255]
merge [1.0] [1.1]

cut [2] [X] [119]
color [2.1] [0, 0, 0, 255]
merge [2.0] [2.1]

cut [3] [X] [159]
color [3.1] [255, 255, 255, 255]
merge [3.0] [3.1]

cut [4] [X] [198]
color [4.1] [0, 0, 0, 255]
merge [4.0] [4.1]

cut [5] [X] [238]
color [5.1] [255, 255, 255, 255]
merge [5.0] [5.1]

cut [6] [X] [277]
color [6.1] [0, 0, 0, 255]
merge [6.0] [6.1]

cut [7] [X] [317]
color [7.1] [0, 74, 173, 255]
merge [7.0] [7.1]

cut [8] [Y] [83]
color [8.1] [0, 0, 0, 255]
merge [8.0] [8.1]

cut [9] [Y] [123]
color [9.1] [255, 255, 255, 255]
merge [9.0] [9.1]

cut [10] [Y] [162]
color [10.1] [0, 0, 0, 255]
merge [10.0] [10.1]

cut [11] [Y] [202]
color [11.1] [255, 255, 255, 255]
merge [11.0] [11.1]

cut [12] [Y] [241]
color [12.1] [0, 0, 0, 255]
merge [12.0] [12.1]

cut [13] [Y] [281]
color [13.1] [255, 255, 255, 255]
merge [13.0] [13.1]

cut [14] [Y] [320]
color [14.1] [0, 0, 0, 255]
merge [14.0] [14.1]

cut [15] [Y] [360]
color [15.1] [255, 255, 255, 255]
merge [15.0] [15.1]

cut [16] [X] [357]
color [16.1] [0, 74, 173, 255]
merge [16.0] [16.1]

cut [17] [Y] [43]
color [17.0] [0, 74, 173, 255]
merge [17.0] [17.1]

cut [18] [40, 83]
cut [18.2] [80, 202]
merge [18.2.1] [18.2.2]
cut [19] [X] [119]
cut [19.1] [159, 281]
merge [19.1.1] [19.1.2]
swap [18.2.0] [19.1.3]
swap [18.2.3] [19.1.0]
merge [18.2.0] [18.2.3]
merge [19.1.0] [19.1.3]
merge [20] [21]
merge [23] [19.0]
merge [24] [22]
merge [25] [18.3]

cut [26] [X] [317]
cut [26.0] [277, 202]
merge [26.0.0] [26.0.3]
cut [27] [X] [238]
cut [27.0] [198, 281]
swap [26.0.1] [27.0.2]
swap [26.0.2] [27.0.1]
