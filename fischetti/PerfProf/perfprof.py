from argparse import ArgumentParser
import numpy as np
import matplotlib
matplotlib.use('PDF')
import matplotlib.pyplot as plt
import sys

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


class CmdLineParser(object):
	def __init__(self):
		self.parser = ArgumentParser(description='Plot performance profiles.')
		# default options
		self.parser.add_argument(dest="input", metavar="INPUT",
			help="CSV file with performance profile specification")
		self.parser.add_argument(dest="output", metavar="OUTPUT",
			help="output file (PDF format)")
		self.parser.add_argument("-D", "--delimiter", dest="delimiter", default=',',
			help="delimiter for input files")
		self.parser.add_argument("-M", "--maxratio", dest="maxratio", default=4, type=float,
			help="maxratio for perf. profile")
		self.parser.add_argument("-S", "--shift", dest="shift", default=0, type=float,
			help="shift for data")
		self.parser.add_argument("-L", "--logplot", dest="logplot", action="store_true", default=False,
			help="log scale for x")
		self.parser.add_argument("-T", "--timelimit", dest="timelimit", default=1e99, type=float,
			help="time limit for runs")
		self.parser.add_argument("-P", "--plot-title", dest="plottitle", default=None,
			help="plot title")
		self.parser.add_argument("-X", "--x-label", dest="xlabel", default='Time Ratio',
			help="x axis label")
		self.parser.add_argument("-B", "--bw", dest="bw", action="store_true", default=False,
			help="plot B/W")

	def parseArgs(self):
		return self.parser.parse_args()

def readTable(fp, delimiter):
	"""
	read a CSV file with performance profile specification
	the format is as follows:
	ncols algo1 algo2 ...
	nome_istanza tempo(algo1) tempo(algo2) ...
	...
	"""
	firstline = fp.readline().strip().split(delimiter)
	ncols = int(firstline[0])
	assert(ncols <= len(markers))
	cnames = firstline[1:]
	rnames = []
	rows = []
	for row in fp:
		row = row.strip().split(delimiter)
		rnames.append(row[0])
		rdata = np.empty(ncols)
		for j in range(ncols):
			rdata[j] = float(row[j + 1])
		rows.append(rdata)
	data = np.array(rows)
	return (rnames, cnames, data)


def main():
	parser = CmdLineParser()
	opt = parser.parseArgs()
	print(opt)
	# read data
	rnames, cnames, data = readTable(open(opt.input, 'r'), opt.delimiter)
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
			if data[i,j] >= opt.timelimit:
				ratio[i,j] = opt.maxratio + 1e6
	# sort ratios
	ratio.sort(axis=0)
	# plot first
	y = np.arange(nrows, dtype=np.float64) / nrows
	print(f'y = {y}, len(y) = {len(y)}')
	print(f'ratio = {ratio}, len(ratio) = {len(ratio)}')
	for j in range(ncols):
		options = dict(label=cnames[j],
				linewidth=defLW, linestyle = dashes[j],
				marker=markers[j], markeredgewidth=defLW, markersize=defMS)
		#plt.step(ratio[:,j], y, label=cnames[j], linewidth=defLW, marker=markers[j], markersize=defMS)
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

if __name__ == '__main__':
	main()