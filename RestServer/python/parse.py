import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
import sys
import os
import math

def main():
    if len(sys.argv) < 6:
        print("Usage: python script.py <filename> <filter_value> <save> <plotTogether> <width> <height>")
        return -1

    filename = sys.argv[1]
    filter_value = sys.argv[2]
    save_plots = True if sys.argv[3] == 'save' else False
    plot_together = True if sys.argv[4] == 'plotTogether' else False
    width = int(sys.argv[5])
    height = int(sys.argv[6])

    try:
        df = pd.read_csv(filename)
    except FileNotFoundError:
        print("Python error: File not found:" + filename)
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

        dpi = calculate_dpi(width, height)

        if plot_together:
            fig, ax = plt.subplots(dpi=dpi)

        for i, (name, subtable) in enumerate(filtered_subtable.items()):
            if not plot_together:
                fig, axs = plt.subplots(1, 1, dpi=dpi)

            row = i // num_cols
            col = i % num_cols
            for j, (_, module_data) in enumerate(subtable.iterrows()):
                x_str_values = module_data['vectime'].strip('"').split()
                y_str_values = module_data['vecvalue'].strip('"').split()
                x_values = np.array([float(value) for value in x_str_values])
                y_values = np.array([float(value) for value in y_str_values])
                if plot_together:
                    ax.plot(x_values, y_values, label=f"Node {j + 1}")
                else:
                    axs.plot(x_values, y_values, label=f"Node {j + 1}")

            if not plot_together:
                axs.set_xlabel('time')
                axs.set_ylabel(f'{filter_value}')
                axs.set_title(f'Plot for {name}')
                axs.grid(True)
                axs.legend()
                plt.tight_layout()

            if save_plots:
                save_path = os.path.dirname(filename)
                if plot_together:
                    plt.savefig(os.path.join(save_path, f'{filter_value}_plots.png'), dpi=dpi)
                else:
                    plt.savefig(os.path.join(save_path, f'{filter_value}_plot_{name}.png'), dpi=dpi)
                return 0
            else:
                if not plot_together:
                    plt.show()
                return 0
        if plot_together:
            plt.xlabel('time')
            plt.ylabel(f'{filter_value}')
            plt.title(f'Plots for {filter_value}')
            plt.grid(True)
            plt.legend()
            plt.tight_layout()
            if save_plots:
                save_path = os.path.dirname(filename)
                plt.savefig(os.path.join(save_path, f'{filter_value}_plots.png'), dpi=dpi)
                return 0
            else:
                plt.show()
                return 0
    except Exception as e:
        print("An error occurred:", e)
        return -1

def calculate_dpi(width, height, screen_width=1920, screen_height=1080):
    diagonal_screen_size = math.sqrt(screen_width**2 + screen_height**2)
    diagonal_image_size = math.sqrt(width**2 + height**2)
    dpi = (diagonal_image_size / diagonal_screen_size) * 96
    return dpi

if __name__ == "__main__":
    main()
