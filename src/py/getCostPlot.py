import matplotlib.pyplot as plt
import pandas as pd

# Read the CSV file
df = pd.read_csv('data/cost/sa_a280.tsp_00.csv', sep=';', header=None)

# Get the data for the x and y axes
x = df[0]
y = df[1]

# Plot the number of iterations vs cost
plt.plot(x, y, color='green', label='Cost')

# Plot the absolute minimum found
min_y = y.cummin()
 # Plot the minimum cost found up to each iteration
plt.plot(x, min_y, color='blue', linestyle='--', label='Minimum Cost')

# Add labels and title
#plt.ylim(2700,3250)
plt.xlabel('Number of Iterations')
plt.ylabel('Cost')
plt.legend()

# Show the plot
plt.show()