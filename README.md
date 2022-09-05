# ICFPC2022 Team manarimo
================================

# Team Members
* Keita Komuro
* Kenkou Nakamura (@kenkoooo)
* mkut
* Osamu Koga (@osa_k)
* Shunsuke Ohashi (@pepsin_amylase)
* Yu Fujikake (@yuusti)
* Yuki Kawata (@kawatea03)

# GitHub
* Organization icon "manarimo" illustration by @yuusti
* GitHub repository: https://github.com/manarimo/ICFPC2022/

# Portal site
* Solutions listing: http://icfpc2022-manarimo.s3-website-us-east-1.amazonaws.com/portal/#/solutions

# AI
* kawatea/dp.cpp - Dynamic programming based solver
* object-object-visualizer/tools - problem processing pipeline to run AI and applying pre/post processings

## Strategy
Our AI splits the image into thin strips then paints them one by one. Strips are divided into several segments
where each segment is painted by single color. To find the optimal division within a strip, we use dynamic programming.

