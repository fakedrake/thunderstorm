import matplotlib.pyplot as plt
import numpy as np
import json
from collections import defaultdict

def get_data(json_file):
    data = json.load(open(json_file,'r'))
    res = defaultdict(lambda : [])
    for bm in data['benchmarks']:
        name, args_str = bm['name'].split('/')
        res[name].append((int(args_str),bm['real_time']))

    return res

def plot_data(data, title, image_file):
    fig, ax = plt.subplots()
    ax.set_xscale('log')
    ax.set_yscale('log')
    for name,dat in data.items():
        ax.plot(*zip(*dat),label=name)

    ax.set_xlabel('#elements')
    ax.set_ylabel('ns')
    ax.set_title(title)
    ax.legend()
    plt.savefig(image_file)

if __name__ == '__main__':
    plot_data(get_data(sys.argv[1]))
