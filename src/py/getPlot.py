import pathlib 
import pandas
import numpy as np
import matplotlib.pyplot as plt

# parameters
defLW = 1.2  # default line width
defMS = 7  # default marker size
dashes = ['-',  # solid line
          '--',  # dashed line
          '-.',  # dash-dot line
          ':',  # dotted line
          '-',
          '--']

markers = ['+', 'x', 's', '^', 'o', 'd']
colors = ['r', 'b', 'y', 'g', 'm', 'c']
# construct structu opt
class opt:
  delimiter=';'
  maxratio=4
  shift=0
  logplot=False
  timelimit=1e99
  plottitle=None
  xlabel='Time Ratio'
  bw=False
  output='data/test.pdf'

if __name__=='__main__':
  folder = pathlib.Path("data/out")
  
  # make table columns are "time" columns of each single file.
  # all files have the same number of rows

  data=pandas.DataFrame()
  for f in folder.glob("*.csv"):
    f=pathlib.Path(f)
    print(f.name.split("_")[0])
    df = pandas.read_csv(f,delimiter=';')
    # Index(['iteration; tlstart; n_passagges; passagge 0; tlfinal; time'], dtype='object')
    # add row to table
    data[f.name.split("_")[0]]=df['time']
  print(data.head())
  cnames = data.columns
  rnames = data.index
  # transform data to matrix
  data = data.to_numpy()

  nrows, ncols = data.shape
  # add shift
  data = data + opt.shift
  # compute ratios
  minima = data.min(axis=1)
  ratio = data
  for j in range(ncols):
      ratio[:, j] = data[:, j] / minima
  # compute maxratio
  if opt.maxratio == -1:
      opt.maxratio = ratio.max()
  # any time >= timelimit will count as maxratio + bigM (so that it does not show up in plots)
  for i in range(nrows):
      for j in range(ncols):
          if data[i, j] >= opt.timelimit:
              ratio[i, j] = opt.maxratio + 1e6
  # sort ratios
  ratio.sort(axis=0)
  # plot first
  y = np.arange(nrows, dtype=np.float64) / nrows
  for j in range(ncols):
      options = dict(label=cnames[j],
                     linewidth=defLW, linestyle=dashes[j],
                     marker=markers[j], markeredgewidth=defLW, markersize=defMS)
      if opt.bw:
          options['markerfacecolor'] = 'w'
          options['markeredgecolor'] = 'k'
          options['color'] = 'k'
      else:
          options['color'] = colors[j]
      if opt.logplot:
          plt.semilogx(ratio[:, j], y, **options)
      else:
          plt.plot(ratio[:, j], y, **options)
  plt.axis([1, opt.maxratio, 0, 1])
  plt.legend(loc='lower right')
  if opt.plottitle is not None:
      plt.title(opt.plottitle)
  plt.xlabel(opt.xlabel)
  plt.savefig(opt.output)





  
  