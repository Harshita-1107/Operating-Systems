import os
import matplotlib.pyplot as plt
plt.style.use("seaborn")

os.system("./generate > result.txt")
data = open("result.txt","r")
fig, axs = plt.subplots(2, 2,figsize=(10,10))

n = [10,20,30,40,50]
label = ["Round robin","Priority based RR scheduling"]
for i in range(5):
    avg_WT = []
    avg_TT = []
    avg_RT = []
    for j in range(5):
        x1,x2,x3 = data.readline().split(" ")
        x1 = float(x1)
        x2 = float(x2)
        x3 = float(x3)
        avg_WT.append(x1)
        avg_TT.append(x2)
        avg_RT.append(x3)
    axs[0,0].plot(n,avg_WT,label=label[i])
    axs[0,1].plot(n,avg_TT,label=label[i])
    axs[1,0].plot(n,avg_RT,label=label[i])
    
axs[0,0].set(xlabel="N",ylabel="Average waiting time")
axs[0,1].set(xlabel="N",ylabel="Average Turn around time")
axs[1,0].set(xlabel="N",ylabel="Average response time")
axs[0,0].legend()
axs[0,1].legend()
axs[1,0].legend()
plt.grid(True)
plt.show()
