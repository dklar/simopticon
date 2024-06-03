import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
import sys
import os

def main():
    if len(sys.argv) < 4:
        print("Usage: python script.py <filename> <filter_value> <save>")
        return -1

    filename = sys.argv[1]
    filter_value = sys.argv[2]
    save_plots = True if sys.argv[3] == 'save' else False

    try:
        df = pd.read_csv(filename)
    except FileNotFoundError:
        print("Error: File not found.")
        return -1
    except pd.errors.EmptyDataError:
        print("Error: File is empty.")
        return -1
    except pd.errors.ParserError:
        print("Error: Unable to parse file.")
        return -1
    try:
        df = df[['run', 'module', 'name', 'vectime', 'vecvalue']]
        df = df.loc[df['name'] == filter_value]

        if df.empty:
            print(f"No data found for '{filter_value}'.")
            return -1

        grouped = df.groupby('run')
        filtered_subtable = {}

        for name, group in grouped:
            if filter_value in group['name'].values:
                filtered_subtable[name] = group[group['name'] == filter_value]

        num_plots = len(filtered_subtable)
        num_cols = 2
        num_rows = (num_plots + num_cols - 1) // num_cols

        fig, axs = plt.subplots(num_rows, num_cols, figsize=(15, 6 * num_rows))

        for i, (name, subtable) in enumerate(filtered_subtable.items()):
            row = i // num_cols
            col = i % num_cols
            for j, (_, module_data) in enumerate(subtable.iterrows()):
                x_str_values = module_data['vectime'].strip('"').split()
                y_str_values = module_data['vecvalue'].strip('"').split()
                x_values = np.array([float(value) for value in x_str_values])
                y_values = np.array([float(value) for value in y_str_values])
                axs[row, col].plot(x_values, y_values, label=f"Node {j + 1}")

            axs[row, col].set_xlabel('time')
            axs[row, col].set_ylabel(f'{filter_value}')
            axs[row, col].set_title(f'Plot for {name}')
            axs[row, col].grid(True)
            axs[row, col].legend()

        plt.tight_layout()

        if save_plots:
            if not os.path.exists('plots'):
                os.makedirs('plots')
            plt.savefig(os.path.join('plots', f'{filter_value}_plots.png'))
            return 0
        else:
            plt.show()
            return 0
    except Exception as e:
        print("An error occurred:", e)
        return -1

if __name__ == "__main__":
    main()