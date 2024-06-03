import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
import sys
import os


def get_filter_values(df):
    try:
        df = df.dropna(subset=['vectime', 'vecvalue'])
        return df['name'].unique()
    except Exception as e:
        print(f"An error occurred while retrieving filter values: {e}")
        return []

def plot_data(df, filter_value, filepath):
    try:
        df = df[['run', 'module', 'name', 'vectime', 'vecvalue']].dropna(subset=['vectime', 'vecvalue'])
        df = df[df['name'] == filter_value]

        if df.empty:
            print(f"No data found for '{filter_value}'.")
            return -1

        grouped = df.groupby('run')
        path = os.path.dirname(filepath)
        plots_path = os.path.join(path, 'plots')

        if not os.path.exists(plots_path):
            os.makedirs(plots_path)

        for i, (name, subtable) in enumerate(grouped):
            plt.figure(figsize=(10, 6))
            for j, (_, module_data) in enumerate(subtable.iterrows()):
                x_values = np.array([float(value) for value in module_data['vectime'].strip('"').split()])
                y_values = np.array([float(value) for value in module_data['vecvalue'].strip('"').split()])
                plt.plot(x_values, y_values, label=f"Node {j + 1}")

            plt.xlabel('time')
            plt.ylabel(f'{filter_value}')
            plt.title(f'Plot for {name}')
            plt.grid(True)
            plt.legend()
            plt.tight_layout()
            plt.savefig(os.path.join(plots_path, f'{filter_value}_plot_{i}.png'))
            plt.close()

    except Exception as e:
        print(f"An error occurred: {e}")
        return -1

def main():
    if len(sys.argv) < 2:
        print("Usage: python script.py <filepath> [filter_value]")
        return -1

    filepath = sys.argv[1]

    try:
        df = pd.read_csv(filepath)
    except FileNotFoundError:
        print("Error: File not found.")
        return -1
    except pd.errors.EmptyDataError:
        print("Error: File is empty.")
        return -1
    except pd.errors.ParserError:
        print("Error: Unable to parse file.")
        return -1

    if len(sys.argv) > 2:
        filter_value = sys.argv[2]
        return plot_data(df, filter_value, filepath)

    filter_values = get_filter_values(df)
    
    if len(filter_values) == 0:
        print("No filter values found.")
        return -1

    print("Available filter values:")
    for i, value in enumerate(filter_values, 1):
        print(f"{i}. {value}")

    try:
        choice = int(input("Please select a filter value by entering the corresponding number: "))
        if choice < 1 or choice > len(filter_values):
            print("Invalid choice.")
            return -1
    except ValueError:
        print("Invalid input. Please enter a number.")
        return -1

    filter_value = filter_values[choice - 1]
    return plot_data(df, filter_value, filepath)

if __name__ == "__main__":
    main()