import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation
from matplotlib import gridspec


class Individual:

    def __init__(self, genome: np.array, best_fitness: float):
        self.genome = genome
        self.best_fitness = best_fitness

    @property
    def x(self):
        return self.genome[0]

    @property
    def y(self):
        return self.genome[1]

    @property
    def z(self):
        return self.best_fitness

    @staticmethod
    def from_text(text: str):
        split_line = text.split(',')
        if len(split_line) > 1:
            xs, ys, zs = split_line
            return Individual(np.array([[float(xs)], [float(ys)]]), float(zs))
        return None

    @staticmethod
    def population_from_text(line: str):
        population = []
        for text in line.split(';'):
            individual = Individual.from_text(text)
            if individual:
                population.append(individual)
        return population

    @staticmethod
    def evolution_from_text(filename):
        evolution = []
        with open(filename, 'r') as f:
            for line in f:
                evolution.append(Individual.population_from_text(line))
        return evolution


class EvolutionData:

    def __init__(self, evolution):
        self.evolution = evolution

    def best(self, generation: int):
        return min(self.evolution[generation], key=lambda obj: obj.z)

    def population(self, generation: int):
        return self.evolution[generation]

    def population_coordinates(self, generation: int):
        population = self.population(generation)
        xdata = np.ones(len(population)) * np.nan
        ydata = np.ones(len(population)) * np.nan
        zdata = np.ones(len(population)) * np.nan
        for i in range(len(population)):
            xdata[i] = population[i].x
            ydata[i] = population[i].y
            zdata[i] = population[i].z
        return (xdata, ydata, zdata)

    def __len__(self):
        return len(self.evolution)


class ComparisonPlot:

    def __init__(self, evolution_data, statistics_file: str, title: str, *baseline_data, **kwargs):
        self.evolution_data = evolution_data
        self.statistics = pd.read_csv(statistics_file)
        self.figure = plt.figure(figsize=(16, 9))
        overlay_text = f"Population Size: {len(self.evolution_data.population(0))}\n" \
                       f"Maximum generations: {len(self.evolution_data)}\n"
        if "extra_text" in kwargs:
            overlay_text += kwargs["extra_text"]
        self.specification_text = self.figure.text(0.1, 0.9, overlay_text)
        self.figure.suptitle(title, size=39)
        self.gridspec = gridspec.GridSpec(2, 2, height_ratios=[3, 1])
        self.axis_3d = self.init_axis_3d(**kwargs)
        (x, y, z) = baseline_data
        self.axis_3d.plot_surface(x, y, z, cmap='viridis', alpha=0.3)
        self.axis_2d = self.init_axis_2d(**kwargs)
        self.axis_2d.contour(x, y, z, cmap='viridis')
        self.axis_2d_bottom, self.axis_2d_bottom_twin = self.init_axis_2d_bottom()
        self.average_fitness_line = self.init_statistics_line(self.axis_2d_bottom, "average_fitness", "Average Fitness",
                                                              1, "gray")
        self.best_fitness_line = self.init_statistics_line(self.axis_2d_bottom, "best_fitness", "Best Fitness", 1,
                                                           "black")
        self.selection_pressure_line, = self.init_statistics_line(self.axis_2d_bottom_twin, "selection_pressure",
                                                                  "Selection Pressure", 1, "blue")
        xdata, ydata, zdata = self.evolution_data.population_coordinates(0)
        best = self.evolution_data.best(0)
        self.population_scatter_3d = self.axis_3d.scatter(xdata, ydata, zdata, marker='x', c='red', zorder=1)
        self.best_scatter_3d = self.axis_3d.scatter([best.x], [best.y], [best.z], marker='x', c='black', zorder=1)
        self.population_scatter_2d = self.axis_2d.scatter(xdata, ydata, marker='x', c='red', zorder=1)
        self.best_scatter_2d = self.axis_2d.scatter([best.x], [best.y], marker='x', c='black', zorder=1)

    def init_axis_3d(self, **kwargs):
        axis = plt.subplot(self.gridspec[0, 0], projection='3d')
        axis.set_xlabel("X")
        axis.set_ylabel("Y")
        axis.set_zlabel("Z")
        if "xlim" in kwargs:
            axis.set_xlim(*kwargs["xlim"])
        if "ylim" in kwargs:
            axis.set_ylim(*kwargs["xlim"])
        axis.set_zticks(np.linspace(0, 100, 3))
        axis.set_box_aspect([3, 3, 1])
        return axis

    def init_axis_2d(self, **kwargs):
        axis = plt.subplot(self.gridspec[0, 1])
        axis.set_xlabel("X")
        axis.set_ylabel("Y")
        if "xlim" in kwargs:
            axis.set_xlim(*kwargs["xlim"])
        if "ylim" in kwargs:
            axis.set_ylim(*kwargs["ylim"])
        axis.set_aspect('equal')
        return axis

    def init_axis_2d_bottom(self, **kwargs):
        axis = plt.subplot(self.gridspec[1, :])
        twin = axis.twinx()
        twin.set_ylim(0., 1.)
        twin.set_ylabel("Selection Pressure", c='blue')
        twin.tick_params(axis='y', labelcolor='blue')
        axis.grid(True)
        axis.set_xlabel("Generations")
        axis.set_ylabel("Fitness Value")
        axis.set_xlim(0, len(self.statistics))
        axis.set_ylim(0, self.statistics["average_fitness"].max())
        axis.legend()
        return axis, twin

    def init_statistics_line(self, axis, channel: str, description: str, length: int, color: str):
        return axis.plot(self.get_statistics_data(channel, length), label=description, c=color)

    def get_statistics_data(self, channel: str, length: int):
        data = np.ones(len(self.statistics)) * np.nan
        data[:length] = self.statistics[channel][:length]
        return data

    def update(self, frame: int):
        xdata, ydata, zdata = self.evolution_data.population_coordinates(frame)
        best = self.evolution_data.best(frame)
        self.best_scatter_3d._offsets3d = ([], [], [])
        self.best_scatter_3d.remove()
        self.best_scatter_3d = self.axis_3d.scatter([best.x], [best.y], [best.z], marker='x', c='black', zorder=1)
        self.best_scatter_2d.remove()
        self.best_scatter_2d = self.axis_2d.scatter([best.x], [best.y], marker='x', c='black', zorder=1)
        self.population_scatter_3d.remove()
        self.population_scatter_2d.remove()
        self.population_scatter_3d = self.axis_3d.scatter(xdata, ydata, zdata, marker='x', c='red', zorder=1)
        self.population_scatter_2d = self.axis_2d.scatter(xdata, ydata, marker='x', c='red', zorder=1)
        self.average_fitness_line.set_ydata(self.get_statistics_data("average_fitness", frame + 1))
        self.best_fitness_line.set_ydata(self.get_statistics_data("best_fitness", frame + 1))
        self.selection_pressure_line.set_ydata(self.get_statistics_data("selection_pressure", frame + 1))

    def animate(self, filename: str, **kwargs):
        if "frames" not in kwargs:
            kwargs['frames'] = range(len(self.evolution_data))
        animation = FuncAnimation(self.figure, self.update, **kwargs)
        animation.save(filename)


if __name__ == "__main__":
    x = np.linspace(-5.12, 5.12, 500)
    y = np.linspace(-5.12, 5.12, 500)
    x, y = np.meshgrid(x, y)
    z = np.genfromtxt("rastrigin.csv", delimiter=',')
    evolution = EvolutionData(Individual.evolution_from_text('rastrigin_population_evolution.txt'))
    plot = ComparisonPlot(
        evolution, "rastrigin_statistics.csv", "Rastrigin Function", x, y, z, xlim=(-5.12, 5.12),
        ylim=(-5.12, 5.12),
        extra_text="Selection for Reproduction: Tournament (Tournament Size = 5, Selection Size = 5)\n"
                   "Selection for Replacement: Truncation (Selection Size = 40\n"
                   "Crossover: Uniform Voluminal Crossover (Alpha = 1.5)\n"
                   "Mutation: Gaussian Mutation (Rate = 0.05, Standard Deviation = 0.1)\n"
    )
    plot.animate("rastrigin.gif", interval=200, repeat=True)
