# Generate LUT
import numpy as np
import math

#=================== for one-input functions ===== d-bit ==========
# slot = 32768 # slot
# colSize = pow(2,10)     # <= you need to change the bit length d
# rowSize = 1             # <= the row size is 2 only for 16-bit integers
# input_lut = []
# output_lut = []
# s=-32768
# for i in range(0, rowSize * colSize):
#     input_lut.append(s)
#     output_lut.append(s)
#     s=s+1

# fi = open("build/Table/128bit/one/vectorOfInLUT_one_10.txt","w") # <= you need to change the bit length d
# fo = open("build/Table/128bit/one/vectorOfOutLUT_one_10.txt","w") # <= you need to change the bit length d
# k = (int)(slot/colSize)

# for i in range(0,rowSize):
#     # t=0
#     for j in range(0,slot):
#         if(j%k==0):
#             fi.write(str(input_lut[t])+' ')
#             fo.write(str(output_lut[t])+' ')
#             t=t+1
#             # fi.write(str(input_lut[i*slot+j])+' ')
#             # fo.write(str(output_lut[i*slot+j])+' ')
#         else:
#             fi.write(str(0)+' ')
#             fo.write(str(0)+' ')
#     fi.write('\n')
#     fo.write('\n')
# fi.close()
# fo.close()

# for i in range(0,rowSize):
#     for j in range(0,slot):
#         fi.write(str(input_lut[i*slot+j])+' ')
#         fo.write(str(output_lut[i*slot+j])+' ')
#     fi.write('\n')
#     fo.write('\n')
# fi.close()
# fo.close()

# =================== for two-input functions ===== d-bit ==========
# slot = 32768 # slot
# colSize = pow(2,12) # input data point # <= you need to change the bit length d
# rowSize = 1 # always one row
# input_lut = []
# output_lut = []
# s = 0
# for i in range(0, rowSize * colSize):
#     input_lut.append(s)
#     s=s+1

# for i in range(0, colSize):
#     f = 0
#     for j in range(0, colSize):
#         output_lut.append(f)
#         f=f+1

# fi = open("build/Table/128bit/vectorOfInLUT_two_12.txt","w") # <= you need to change the bit length d
# fo = open("build/Table/128bit/vectorOfOutLUT_two_12.txt","w") # <= you need to change the bit length d
# fd = open("build/Table/128bit/IndexLut_two_12.txt","w") # <= you need to change the bit length d
# k = (int)(slot/colSize)
# t=0
# for i in range(0,slot):
#     if(i%k==0):
#         fi.write(str(input_lut[t])+' ')
#         t=t+1
#     else:
#         fi.write(str(0)+' ')
# fi.close()
# for i in range(0,int(len(output_lut)/colSize)):
#     t=0
#     for j in range(0,slot):
#         if(j%k==0):
#             fo.write(str(output_lut[i*colSize+t])+' ')
#             t=t+1
#         else:
#             fo.write(str(0)+' ')
#     fo.write('\n')
# fo.close()
# for i in range(0,int(len(input_lut))):
#     for j in range(0,slot):
#         if(j==k*i):
#             fd.write(str(1)+' ')
#         else:
#             fd.write(str(0)+' ')
#     fd.write('\n')
# fd.close()

# =================== for three-input functions ===== d-bit ==========
# slot = 32768 # slot
# colSize = pow(2,6) # input data point # <= you need to change the bit length d
# rowSize = 1 # always one row
# input_lut = []
# output_lut = []
# s = 0
# for i in range(0, rowSize * colSize):
#     input_lut.append(s)
#     s=s+1

# for i in range(0, colSize):
#     for j in range(0, colSize):
#         # f = 0
#         for z in range(0, colSize):
#             output_lut.append(1)
#             # f=f+1

# fi = open("build/Table/128bit/three/vectorOfInLUT_three_6.txt","w") # <= you need to change the bit length d
# fo = open("build/Table/128bit/three/vectorOfOutLUT_three_6.txt","w") # <= you need to change the bit length d
# fd = open("build/Table/128bit/three/IndexLut_three_6.txt","w") # <= you need to change the bit length d
# k = (int)(slot/colSize)
# t=0
# for i in range(0,slot):
#     if(i%k==0):
#         fi.write(str(input_lut[t])+' ')
#         t=t+1
#     else:
#         fi.write(str(0)+' ')
# fi.close()
# for i in range(0,int(len(output_lut)/colSize)):
#     t=0
#     for j in range(0,slot):
#         if(j%k==0):
#             fo.write(str(output_lut[i*colSize+t])+' ')
#             t=t+1
#         else:
#             fo.write(str(0)+' ')
#     fo.write('\n')
# fo.close()
# for i in range(0,int(len(input_lut))):
#     for j in range(0,slot):
#         if(j==k*i):
#             fd.write(str(1)+' ')
#         else:
#             fd.write(str(0)+' ')
#     fd.write('\n')
# fd.close()

# =================== for related works ===== just a sample table ==========
# ===The table sizes are the same as related work, but the content is random ===
InputSize = pow(2,1) # number of slots set as 2^15=32768 # <= you need to change the bit length d
input_lut = []
s = 0
for i in range(0,InputSize):
    input_lut.append(s)
    s=s+1
coeff=[]

for i in range(0,InputSize):
    y = 0
    for j in range(0,InputSize):
        coeff.append(y)
        y=y+1

fi = open("build/Table/RelatedWork/relatedwork_in_1.txt","w") # <= you need to change the bit length d
fc = open("build/Table/RelatedWork/relatedwork_coeff_1.txt","w") # <= you need to change the bit length d
for i in range(0,InputSize):
    fi.write(str(input_lut[i])+' ')
    for j in range(0,InputSize):
        fc.write(str(coeff[i*InputSize+j])+' ')
    fc.write('\n')
fi.close()
fc.close()
