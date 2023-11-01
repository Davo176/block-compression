# Import libraries
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D
import numpy as np
import sys
import argparse
import os


### update when running script
## update when running
parser = argparse.ArgumentParser(
    formatter_class=argparse.RawDescriptionHelpFormatter)
parser.add_argument(
    "path_to_item_under_test",
    metavar="<./input/my_data.txt or ./input/mydata.csv>",
    help="script or executable to run")
parser.add_argument(
    "input_data_format_line",
    metavar="<[x,y,z,px,py,pz]>",
    help="[x,y,z,px,py,pz]")

args = parser.parse_args()


if not os.path.isfile(args.path_to_item_under_test):
    print("File Not found at '{}'".format(
        args.path_to_item_under_test))
    sys.exit(10)

file_path=args.path_to_item_under_test 
if isinstance(args.input_data_format_line,str):
     temp=args.input_data_format_line
     temp=temp[1:-1].split(',')

     temp=list(map(int,temp))
     args.input_data_format_line=temp
input_data_first_line=args.input_data_format_line
# [64,64,8,1,1,1]


fig = plt.figure(figsize=(10, 10))
ax = plt.axes(projection='3d')

alpha = 0.9



with open(file_path,'r') as f:
    inputs_from_file=f.read()

inputs_from_file=inputs_from_file.split('\n')
x_count, y_count, z_count, parent_x, parent_y, parent_z=input_data_first_line
axes = [x_count, y_count, z_count]

visualize_data=False

initial_color=[0,0,0,alpha]
n=0
color_coding={
    'sea':[0, 0, 1, alpha]
}

# Create Data
data = np.ones(axes)

# Control colour
colors = np.empty(axes+[4] )
k=0
axis_value=0

print(data.shape,colors.shape)

for i in inputs_from_file:
    if i=='':
        continue
    input_data_block=i.split(',')
    if input_data_block[-1] not in color_coding:
        if initial_color[n]+0.2<1:
                initial_color=initial_color[:n]+[initial_color[n]+0.2]+initial_color[n+1:]
        else:
            initial_color=initial_color[:n]+[0]+initial_color[n+1:]
        color_coding[input_data_block[-1]]=initial_color
        n+=1
        n%=3
    colors[int(input_data_block[0])][int(input_data_block[1])][int(input_data_block[2])]=color_coding[input_data_block[-1]]
        

# turn off/on axis
plt.axis('off')
 
# Voxels is used to customizations of
# the sizes, positions and colors.
ax.voxels(data, facecolors=colors, edgecolors='grey')

plt.show()