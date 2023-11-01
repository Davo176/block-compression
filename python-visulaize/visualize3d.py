# Import libraries
import matplotlib.pyplot as plt
import sys
import os
from mpl_toolkits.mplot3d import Axes3D
import numpy as np
import argparse

## update when running
parser = argparse.ArgumentParser(
    formatter_class=argparse.RawDescriptionHelpFormatter)
parser.add_argument(
    "path_to_item_under_test",
    metavar="<./input/my_data.txt or ./input/mydata.csv>",
    help="script or executable to run")

args = parser.parse_args()


if not os.path.isfile(args.path_to_item_under_test):
    print("File Not found at '{}'".format(
        args.path_to_item_under_test))
    sys.exit(10)

file_path=args.path_to_item_under_test 

##  './BLOCK6/Inputs/the_intro_one_32768_4x4x4.csv'

fig = plt.figure(figsize=(10, 10))
ax = plt.axes(projection='3d')

alpha = 0.7

with open(file_path,'r') as f:
    inputs_from_file=f.read()

inputs_from_file=inputs_from_file.split('\n')
x_count, y_count, z_count, parent_x, parent_y, parent_z=list(map(int,inputs_from_file[0].split(',')))
axes = [x_count, y_count, z_count]

visualize_data=False

## color to be used
color_coding={}
initial_color=[0,0,0,alpha]
n=0
# Create Data
data = np.ones(axes)

# Control colour
colors = np.empty(axes+[4] )
k=0
axis_value=0

print('Preparing input for the 3d visual')
for i in inputs_from_file[1:]:
    if not visualize_data:
        if i=='':
            visualize_data=True
        else:
            ## inputs available in file
            data_code=i.split(',')[0]
            if initial_color[n]+0.2<1:
                initial_color=initial_color[:n]+[initial_color[n]+0.2]+initial_color[n+1:]
            else:
                initial_color=initial_color[:n]+[0]+initial_color[n+1:]
            color_coding[data_code]=initial_color

            n+=1
            n%=3
            if (n%8==0):
                alpha+=0.1
                n=0

    else:
        ## create the data for the visualization
        if i=='':
            axis_value+=1
            k=0
            continue
        for j in range(y_count):
            colors[k][j][axis_value]=color_coding[i[j]]
        k+=1
        
print('Creating the 3d visual')
# turn off/on axis
plt.axis('off')
 
# Voxels is used to customizations of
# the sizes, positions and colors.
ax.voxels(data, facecolors=colors)

plt.show()