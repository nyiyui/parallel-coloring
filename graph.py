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

def make_strong_scaling_threads(df, n_tasks=1):
    df = df[df['n_tasks'] == n_tasks]
    df = df.drop_duplicates(subset=['n_threads'], keep='first')
    time_one = df[df['n_threads'] == 1]['total'].values[0]
    print('time_one', time_one)
    df['speedup'] = time_one / df['total']
    df['efficiency'] = df['speedup'] / df['n_threads']
    print(df)
    plt.figure(figsize=FIGSIZE)
    plt.title(f'Strong Scaling: Speedup v. Threads (Tasks={n_tasks})')
    plt.xlabel('Number of Threads')
    plt.ylabel('Speedup')
    plt.xscale('log')
    plt.xticks([1, 2, 4, 8, 16, 32, 64], ["1", "2", "4", "8", "16", "32", "64"])
    plt.plot(df['n_threads'], df['speedup'], label='Actual', marker='o')
    ideal = [n_threads for n_threads in df['n_threads']]
    plt.plot(df['n_threads'], ideal, label='Ideal', linestyle='--')
    plt.legend()
    plt.savefig(f'strong_scaling_threads_{n_tasks}tasks.png', dpi=300, bbox_inches='tight')
    plt.clf()
    # efficiency
    plt.title(f'Strong Scaling: Efficiency v. Threads (Tasks={n_tasks})')
    plt.xlabel('Number of Threads')
    plt.ylabel('Efficiency')
    plt.xscale('log')
    plt.xticks([1, 2, 4, 8, 16, 32, 64], ["1", "2", "4", "8", "16", "32", "64"])
    plt.plot(df['n_threads'], df['efficiency'], label='Actual', marker='o')
    ideal = [1 for n_threads in df['n_threads']]
    plt.plot(df['n_threads'], ideal, label='Ideal', linestyle='--')
    plt.legend()
    plt.savefig(f'strong_scaling_efficiency_threads_{n_tasks}tasks.png', dpi=300, bbox_inches='tight')

def make_strong_scaling_tasks(df, n_threads=1):
    df = df[df['n_threads'] == n_threads]
    df = df.drop_duplicates(subset=['n_tasks'], keep='first')
    time_one = df[df['n_tasks'] == 1]['total'].values[0]
    print('time_one', time_one)
    df['speedup'] = time_one / df['total']
    df['efficiency'] = df['speedup'] / df['n_tasks']
    print(df)
    plt.figure(figsize=FIGSIZE)
    plt.title(f'Strong Scaling: Speedup v. MPI Tasks (Threads={n_threads})')
    plt.xlabel('Number of MPI Tasks')
    plt.ylabel('Speedup')
    plt.xscale('log')
    plt.xticks([1, 2, 4, 8, 16, 32, 64], ["1", "2", "4", "8", "16", "32", "64"])
    plt.plot(df['n_tasks'], df['speedup'], label='Actual', marker='o')
    ideal = [n_tasks for n_tasks in df['n_tasks']]
    plt.plot(df['n_tasks'], ideal, label='Ideal', linestyle='--')
    plt.legend()
    plt.savefig(f'strong_scaling_tasks_{n_threads}threads.png', dpi=300, bbox_inches='tight')
    plt.clf()
    # efficiency
    plt.title(f'Strong Scaling: Efficiency v. MPI Tasks (Threads={n_threads})')
    plt.xlabel('Number of MPI Tasks')
    plt.ylabel('Efficiency')
    plt.xscale('log')
    plt.xticks([1, 2, 4, 8, 16, 32, 64], ["1", "2", "4", "8", "16", "32", "64"])
    plt.plot(df['n_tasks'], df['efficiency'], label='Actual', marker='o')
    ideal = [1 for n_tasks in df['n_tasks']]
    plt.plot(df['n_tasks'], ideal, label='Ideal', linestyle='--')
    plt.legend()
    plt.savefig(f'strong_scaling_efficiency_tasks_{n_threads}threads.png', dpi=300, bbox_inches='tight')

def make_weak_scaling_tasks(df):
    # df add column based on ratio
    df = df[df['n_threads'] == 1]
    df['ratio'] = df['n'] / (df['n_threads'] * df['n_tasks'])
    df = df[df['ratio'] == 1000]
    total_one = df[df['n'] == 1000]['total'].values[0]
    print('total_one', total_one)
    df['efficiency'] = total_one / df['total']
    print('make_weak_scaling_threads')
    print(df)
    plt.figure(figsize=FIGSIZE)
    plt.title('Weak Scaling Efficiency v. MPI Tasks (Threads=1)')
    plt.xlabel('Number of MPI Tasks')
    plt.ylabel('Weak Scaling Efficiency')
    plt.xscale('log')
    plt.xticks([1, 2, 4, 8, 16, 32], ["1", "2", "4", "8", "16", "32"])
    plt.plot(df['n_tasks'], df['efficiency'], label='Actual', marker='o')
    ideal = [1 for i in range(len(df['n_tasks']))]
    plt.plot(df['n_tasks'], ideal, label='Ideal', linestyle='--')
    plt.legend()
    plt.savefig('weak_scaling_tasks.png', dpi=300, bbox_inches='tight')

def make_weak_scaling_threads(df):
    # df add column based on ratio
    df = df[df['n_tasks'] == 1]
    df['ratio'] = df['n'] / (df['n_threads'] * df['n_tasks'])
    df = df[df['ratio'] == 1000]
    total_one = df[df['n'] == 1000]['total'].values[0]
    print('total_one', total_one)
    df['efficiency'] = total_one / df['total']
    print('make_weak_scaling_threads')
    print(df)
    plt.figure(figsize=FIGSIZE)
    plt.title('Weak Scaling Efficiency v. Threads (Tasks=1)')
    plt.xlabel('Number of Threads')
    plt.ylabel('Weak Scaling Efficiency')
    plt.xscale('log')
    plt.xticks([1, 2, 4, 8, 16, 32], ["1", "2", "4", "8", "16", "32"])
    plt.plot(df['n_threads'], df['efficiency'], label='Actual', marker='o')
    ideal = [1 for i in range(len(df['n_threads']))]
    plt.plot(df['n_threads'], ideal, label='Ideal', linestyle='--')
    plt.legend()
    plt.savefig('weak_scaling_threads.png', dpi=300, bbox_inches='tight')

def make_speedup_threads(df):
    # df add column based on ratio
    df = df[(df['n_tasks'] == 1) & (df['n'] == 10000)]
    total_one = df[df['n_threads'] == 1]['total'].values[0]
    print('total_one', total_one)
    df['speedup'] = total_one / df['total']
    print('make_speedup_threads')
    print(df)

    plt.figure(figsize=FIGSIZE)
    plt.title('Strong Scaling: Speedup v. Threads (Tasks=1)')
    plt.xlabel('Number of Threads')
    plt.ylabel('Speedup')
    plt.xscale('log')
    plt.xticks([1, 2, 4, 8, 16, 32], ["1", "2", "4", "8", "16", "32"])
    plt.plot(df['n_threads'], speedup, label='Actual', marker='o')
    ideal = [1 for i in range(len(df['n_threads']))]
    plt.plot(df['n_threads'], ideal, label='Ideal', linestyle='--')
    plt.legend()
    plt.savefig('speedup_threads.png', dpi=300, bbox_inches='tight')

if __name__ == '__main__':
    make_strong_scaling_threads(parse(open('slurm-2551038.out')), n_tasks=1)
    make_strong_scaling_threads(parse(open('slurm-2551038.out')), n_tasks=2)
    make_strong_scaling_threads(parse(open('slurm-2551038.out')), n_tasks=4)
    make_strong_scaling_tasks(parse(open('slurm-2551038.out')), n_threads=1)
    make_strong_scaling_tasks(parse(open('slurm-2551038.out')), n_threads=2)
    make_strong_scaling_tasks(parse(open('slurm-2551038.out')), n_threads=4)
    make_strong_scaling_tasks(parse(open('slurm-2551038.out')), n_threads=8)
    make_weak_scaling_tasks(parse(open('slurm-2550956.out')))
    make_weak_scaling_threads(parse(open('slurm-2551010.out')))
    # make_speedup_threads(parse(open('slurm-2551010.out')))
