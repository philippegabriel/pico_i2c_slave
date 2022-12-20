#!/usr/bin/env python

# generate 32bit Big endian sine function
# lookup table

import math
def swap32(x):
    return (((x << 24) & 0xff000000) |
            ((x <<  8) & 0x00ff0000) |
            ((x >>  8) & 0x0000ff00) |
            ((x >> 24) & 0x000000ff))
    
def scale(X,A,B,C,D,force_float=False):
    retval = ((float(X - A) / (B - A)) * (D - C)) + C
    if not force_float and all(map(lambda x: type(x) == int, [X,A,B,C,D])):
        return int(round(retval))
    return retval

SAMPLES = 2048
SCALE = 0,65535 ## Output scale range

def hexOutput(table):
    index=0
    while index < len(table):
#        output = ''
        outputBE = ''
        for i in range(8):
#            output += '0x{0:0{1}x},'.format(table[index],8)
            outputBE += '0x{0:0{1}x},'.format(swap32(table[index]),8)
            index += 1
            if index == len(table):
                break
#        print(output)
        print(outputBE)

def decOutput(table):
    print(table)

if __name__ == '__main__':
    angles = [scale(i,0,SAMPLES,0,360,1) for i in range(SAMPLES)]
    sin_table = [int(round(scale(s,-1,1,SCALE[0],SCALE[1]))) for s in [
        math.sin(math.radians(a)) for a in angles
    ]]
    hexOutput(sin_table)
    #decOutput(sin_table)
