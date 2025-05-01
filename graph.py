import pandas as pd
from matplotlib import pyplot as plt

FIGSIZE = (8, 6)

def parse(lines):
    rows = []
    
    n_threads = None
    n_tasks = None
    n = None
    nnz = None
    detect_subgraph = None
    color_cliquelike = None
    
    for line in lines:
        line = line.strip()
        if not line:
            continue
        if line.startswith('n_threads='):
            if n_threads is not None:
                total = (detect_subgraph if detect_subgraph else 0) + (color_cliquelike if color_cliquelike else 0)
                rows.append((n_threads, n_tasks, n, nnz, detect_subgraph, color_cliquelike, total))
            n_threads = int(line.split('=')[1])
        elif line.startswith('n_tasks='):
            n_tasks = int(line.split('=')[1])
        elif line.startswith('n='):
            n = int(line.split('=')[1])
        elif line.startswith('nnz='):
            nnz = int(line.split('=')[1])
        elif line.startswith('detect_subgraph:'):
            detect_subgraph = float(line.split(':')[1][:-1])
        elif line.startswith('color_cliquelike:'):
            color_cliquelike = float(line.split(':')[1][:-1])
    if n_threads is not None:
        total = (detect_subgraph if detect_subgraph else 0) + (color_cliquelike if color_cliquelike else 0)
        rows.append((n_threads, n_tasks, n, nnz, detect_subgraph, color_cliquelike, total))

    columns = ('n_threads', 'n_tasks', 'n', 'nnz', 'detect_subgraph', 'color_cliquelike', 'total')
    return pd.DataFrame(rows, columns=columns)

def make_strong_scaling(df):
    plt.figure(figsize=FIGSIZE)
    plt.title('Strong Scaling: Time v. Threads')
    plt.xlabel('Number of threads')
    plt.ylabel('Time (s)')
    plt.xscale('log')
    plt.xticks([1, 2, 4, 8, 16, 32], ["1", "2", "4", "8", "16", "32"])
    plt.plot(df['n_threads'], df['total'], label='Actual', marker='o')
    print(df)
    ideal = [df['total'].values[0] / i for i in df['n_threads']]
    plt.plot(df['n_threads'], ideal, label='Ideal', linestyle='--')
    plt.legend()
    plt.savefig('strong_scaling.png', dpi=300, bbox_inches='tight')

def make_weak_scaling(df):
    df = df[((df['n'] == 100) & (df['n_threads'] == 1)) | (df['n'] != 100)]
    plt.figure(figsize=FIGSIZE)
    plt.title('Weak Scaling: Efficiency v. Threads')
    plt.xlabel('Number of threads')
    plt.ylabel('Efficiency')
    plt.xscale('log')
    plt.xticks([1, 2, 4, 8, 16, 32], ["1", "2", "4", "8", "16", "32"])
    total_one = df[df['n'] == 100]['total'].values[0]
    print('total_one', total_one)
    plt.plot(df['n_threads'], total_one/df['total'], label='Actual', marker='o')
    ideal = [1 for i in range(len(df['n_threads']))]
    plt.plot(df['n_threads'], ideal, label='Ideal', linestyle='--')
    plt.legend()
    plt.savefig('weak_scaling.png', dpi=300, bbox_inches='tight')

def make_thread_to_thread_speedup(df):
    df = df[((df['n'] == 100) & (df['n_threads'] == 1)) | (df['n'] != 100)]
    plt.figure(figsize=FIGSIZE)
    plt.title('Thread-to-Thread Speedup: Speedup v. Threads')
    plt.xlabel('Number of threads')
    plt.ylabel('Speedup')
    plt.xscale('log')
    plt.xticks([1, 2, 4, 8, 16, 32], ["1", "2", "4", "8", "16", "32"])
    total_one = df[df['n'] == 100]['total'].values[0]
    print('total_one', total_one)
    plt.plot(df['n_threads'], total_one/df['total'], label='Actual', marker='o')
    ideal = df['n_threads'].values
    plt.plot(df['n_threads'], ideal, label='Ideal', linestyle='--')
    plt.legend()
    plt.savefig('thread_to_thread_speedup.png', dpi=300, bbox_inches='tight')

if __name__ == '__main__':
    study_strong = parse(open('slurm-2549836.out'))
    print('=== Strong Scaling ===')
    print(study_strong)
    make_strong_scaling(study_strong)
    study_weak2 = parse(open('slurm-2549856.out'))
    print('=== Weak Scaling ===')
    print(study_weak2)
    make_weak_scaling(study_weak2)
    make_thread_to_thread_speedup(study_weak2)
