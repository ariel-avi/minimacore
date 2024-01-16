
import numpy as np
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation


class Individual:

    def __init__(self, genome : np.array, best_fitness : float):
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
    def from_text(text : str):
        split_line = text.split(',')
        if len(split_line) > 1:
            xs, ys, zs = split_line
            return Individual(np.array([[float(xs)], [float(ys)]]), float(zs))
        return None

    @staticmethod
    def population_from_text(line : str):
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

    def best(self, generation : int):
        return min(self.evolution[generation], key=lambda obj: obj.z)

    def population(self, generation : int):
        return self.evolution[generation]

    def population_coordinates(self, generation : int):
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

    def __init__(self, evolution_data, title : str, *baseline_data, **kwargs):
        self.figure = plt.figure(figsize=(16,9))
        self.figure.suptitle(title, size=39)
        self.init_axis_3d(**kwargs)
        (x, y, z) = baseline_data
        self.axis_3d.plot_surface(x, y, z, cmap='viridis', alpha=0.3)
        self.init_axis_2d(**kwargs)
        self.axis_2d.contour(x, y, z, cmap='viridis')
        self.evolution_data = evolution_data
        xdata, ydata, zdata = self.evolution_data.population_coordinates(0)
        best = self.evolution_data.best(0)
        self.population_scatter_3d = self.axis_3d.scatter(xdata, ydata, zdata, marker='x', c='red', zorder=1)
        self.best_scatter_3d = self.axis_3d.scatter([best.x], [best.y], [best.z], marker='x', c='black', zorder=1)
        self.population_scatter_2d = self.axis_2d.scatter(xdata, ydata, marker='x', c='red', zorder=1)
        self.best_scatter_2d = self.axis_2d.scatter([best.x], [best.y], marker='x', c='black', zorder=1)

    def init_axis_3d(self, **kwargs):
        self.axis_3d = self.figure.add_subplot(121, projection='3d')
        self.axis_3d.set_xlabel("X")
        self.axis_3d.set_ylabel("Y")
        self.axis_3d.set_zlabel("Z")
        if "xlim" in kwargs:
            self.axis_3d.set_xlim(*kwargs["xlim"])
        if "ylim" in kwargs:
            self.axis_3d.set_ylim(*kwargs["xlim"])
        self.axis_3d.set_zticks(np.linspace(0, 100, 3))
        self.axis_3d.set_box_aspect([3, 3, 1])

    def init_axis_2d(self, **kwargs):
        self.axis_2d = self.figure.add_subplot(122)
        self.axis_2d.set_xlabel("X")
        self.axis_2d.set_ylabel("Y")
        if "xlim" in kwargs:
            self.axis_2d.set_xlim(*kwargs["xlim"])
        if "ylim" in kwargs:
            self.axis_2d.set_ylim(*kwargs["ylim"])
        self.axis_2d.set_aspect('equal')

    def update(self, frame : int):
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

    def animate(self, **kwargs):
        if "frames" not in kwargs:
            kwargs['frames'] = range(len(self.evolution_data))
        return FuncAnimation(self.figure, self.update, interval=200, repeat=True, **kwargs)


if __name__ == "__main__":
    evolution = EvolutionData(Individual.evolution_from_text('rastrigin_population_evolution.txt'))
    plot = ComparisonPlot(evolution, "Rastrigin Function", x, y, z, xlim=(-5.12, 5.12), ylim=(-5.12, 5.12))
    animation = plot.animate()
    animation.save("rastrigin.gif")
