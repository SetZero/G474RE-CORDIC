import matplotlib.pyplot as plt
from pandas import read_csv

data = read_csv("performance.csv", sep=',')
part = data.iloc[:,[0, 1, 4]]
group_by_subj = part.groupby(["SUBJ"])
group_by_subj.boxplot()
plt.plot(subplots=True)
plt.figure(figsize=(4, 3))
plt.show()
# plt.boxplot(column=['GCC', "CORDIC"])
