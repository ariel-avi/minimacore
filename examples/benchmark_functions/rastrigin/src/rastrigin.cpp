
#include <minimacore_concepts.h>
#include <selection_operators.h>
#include <setup.h>
#include <runner.h>
#include <iostream>
#include <fstream>

using namespace minimacore::genetic_algorithm;

template<floating_point_type T>
inline T square(T value)
{
  return value * value;
}

template<floating_point_type T>
inline T rastrigin(const Eigen::VectorX<T>& input)
{
  T a = 10.;
  T result = a * input.size();
  for (std::size_t i = 0; i < input.size(); i++) result += (square(input(i)) - a * std::cos(2. * EIGEN_PI * input(i)));
  return result;
}

template<floating_point_type F = double>
class chromosome_generator_impl : public base_chromosome_generator<F> {
public:
  void generate_chromosome(const individual_ptr<F>& individual) const override
  {
    std::random_device device;
    std::mt19937_64 generator(device());
    std::uniform_real_distribution<F> dist(lower_limit, upper_limit);
    for (auto i{0}; i < individual->genome().size(); i++) individual->genome()(i) = dist(generator);
  }

  chromosome_generator_impl(F lower_limit, F upper_limit)
          :lower_limit(lower_limit), upper_limit(upper_limit) { }

private:
  F lower_limit;
  F upper_limit;
};

template<floating_point_type F>
class rastrigin_evaluation_function : public base_evaluation<F> {
public:
  size_t operator()(base_individual<F>& individual, size_t objective_index) const override
  {
    individual.set_objective_fitness(objective_index, std::abs(rastrigin(individual.genome())));
    return ++objective_index;
  }

  size_t objective_count() const override
  {
    return 1;
  }
};

int main(int argc, char** argv)
{
  auto input = Eigen::VectorXd::LinSpaced(500, -5.12, 5.12);
  auto output = Eigen::MatrixXd(500, 500);
  for (size_t i = 0; i < input.size(); i++) {
    for (size_t j = 0; j < input.size(); j++) {
      Eigen::VectorXd v(2);
      v << input.coeff(i), input.coeff(j);
      output(i, j) = rastrigin(v);
    }
  }
  std::ofstream ofs("rastrigin.csv");
  Eigen::IOFormat fmt(6, 0, ",");
  ofs << output.format(fmt);
  ofs.close();

  Eigen::Vector2d initial_genome(-5.12, -5.12); // Initializes genome at the edge of the boundaries
  auto genome_gen = std::make_unique<genome_generator<double>>(initial_genome);
  genome_gen->append_chromosome_generator(std::make_unique<chromosome_generator_impl<double>>(-5.12, 5.12));
  setup<double> s;
  s.set_population_size(100)
          .set_generations(50)
          .set_selection_for_replacement(std::make_unique<truncation_selection_for_replacement<double>>(50))
          .set_selection_for_reproduction(std::make_unique<tournament_selection_for_reproduction<double>>(5, 10))
          .set_crossover(std::make_unique<uniform_voluminal_crossover<double>>(2))
          .set_mutation(std::make_unique<gaussian_mutation<double>>(0.05, .5))
          .set_genome_generator(std::move(genome_gen))
          .add_evaluation(std::make_unique<rastrigin_evaluation_function<double>>());
  runner<double> r(std::move(s));
  std::ofstream individuals_ofs("rastrigin_population_evolution.txt");
  auto save_individuals = [&r, &individuals_ofs]() {
    Eigen::IOFormat fmt(6, 0, ",");
    for (auto& individual : r.get_population())
      individuals_ofs << individual->genome().transpose().format(fmt) << "," << individual->objective_fitness(0) << ";";
    individuals_ofs << '\n';
  };
  r.get_setup().add_callback(save_individuals);
  r.add_log_stream(std::cout);
  if (r.run() == runner<double>::success) {
    r.export_statistics("rastrigin_statistics.csv", ',');
    return 0;
  }
  return -1;
}