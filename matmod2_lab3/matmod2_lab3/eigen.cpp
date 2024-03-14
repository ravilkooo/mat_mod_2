#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <iomanip>
#include <chrono>
#include <Eigen\Sparse>


#define Pi 3.14159265

typedef Eigen::SparseMatrix<double> SpMat; // declares a column-major sparse matrix type of double
typedef Eigen::Triplet<double> Trip;

class System
{
public:
	unsigned N = 50;
	unsigned M = 50;
	unsigned N1;
	double L = 5;
	double x_PML;
	double eps = 1;
	double k_0 = 10;
	double y_0 = 0.35;
	double hx;
	double hy;

	std::vector<Trip> coefficients;	// list of non-zeros coefficients
	Eigen::VectorXd B;	// the right hand side-vector resulting from the constraints
	SpMat A;

	System()
	{}

	System(unsigned N1, unsigned M, double L, double x_PML,
		double y_0, double k_0, double eps)
		: N1(N1), M(M), L(L),
		x_PML(x_PML), y_0(y_0), k_0(k_0), eps(eps)
	{
		/*hx = x_PML / double(N);
		inv_hx = double(N) / x_PML;
		inv2_hx = inv_hx * inv_hx;
		hy = 1. / double(M);
		inv_hy = double(M);
		inv2_hy = inv_hy * inv_hy;
		N1 = std::ceil(L / hx);*/
		hx = L / double(N1);
		inv_hx = double(N1) / L;
		inv2_hx = inv_hx * inv_hx;
		hy = 1. / double(M);
		inv_hy = double(M);
		inv2_hy = inv_hy * inv_hy;
		N = std::floor(x_PML / hx);                 // the right hand side-vector resulting from the constraints
	}

	unsigned ReIdx(unsigned i, unsigned j)
	{
		return 2 * ((i - 1) + (j - 1) * (N - 1));
	}
	unsigned ImIdx(unsigned i, unsigned j)
	{
		return 2 * ((i - 1) + (j - 1) * (N - 1)) + 1;
	}

	Eigen::VectorXd solve()
	{
		Eigen::SparseLU<SpMat> solver;
		solver.analyzePattern(A);
		solver.factorize(A);
		return solver.solve(B);
		//Eigen::SimplicialCholesky<SpMat> chol(A);  // performs a Cholesky factorization of A
		//return chol.solve(B);         // use the factorization to solve for the given right hand side
	}

	void fillAB()
	{
		B = Eigen::VectorXd(2 * (N - 1) * (M - 1));
		for (unsigned i = 0; i < 2 * (N - 1) * (M - 1); i++)
		{
			B[i] = 0;
		}

		// #
		// X#
		// i = 1, j = 1
		coefficients.push_back(Trip(ReIdx(1, 1), ReIdx(1, 1), k2_val(y(1)) - 2 * inv2_hx - 2 * inv2_hy));
		coefficients.push_back(Trip(ReIdx(1, 1), ReIdx(2, 1), inv2_hx));
		coefficients.push_back(Trip(ReIdx(1, 1), ReIdx(1, 2), inv2_hy));
		B[ReIdx(1, 1)] = -f(y(1)) * inv2_hx;

		coefficients.push_back(Trip(ImIdx(1, 1), ImIdx(1, 1), k2_val(y(1)) - 2 * inv2_hx - 2 * inv2_hy));
		coefficients.push_back(Trip(ImIdx(1, 1), ImIdx(2, 1), inv2_hx));
		coefficients.push_back(Trip(ImIdx(1, 1), ImIdx(1, 2), inv2_hy));
		B[ImIdx(1, 1)] = -f(y(1)) * inv2_hx;

		for (unsigned j = 2; j < M - 1; j++)
		{
			// #
			// X#
			// #
			// i = 1
			coefficients.push_back(Trip(ReIdx(1, j), ReIdx(1, j), k2_val(y(j)) - 2 * inv2_hx - 2 * inv2_hy));
			coefficients.push_back(Trip(ReIdx(1, j), ReIdx(2, j), inv2_hx));
			coefficients.push_back(Trip(ReIdx(1, j), ReIdx(1, j - 1), inv2_hy));
			coefficients.push_back(Trip(ReIdx(1, j), ReIdx(1, j + 1), inv2_hy));
			B[ReIdx(1, j)] = -f(y(j)) * inv2_hx;

			coefficients.push_back(Trip(ImIdx(1, j), ImIdx(1, j), k2_val(y(j)) - 2 * inv2_hx - 2 * inv2_hy));
			coefficients.push_back(Trip(ImIdx(1, j), ImIdx(2, j), inv2_hx));
			coefficients.push_back(Trip(ImIdx(1, j), ImIdx(1, j - 1), inv2_hy));
			coefficients.push_back(Trip(ImIdx(1, j), ImIdx(1, j + 1), inv2_hy));
			B[ImIdx(1, j)] = -f(y(j)) * inv2_hx;
		}

		// X#
		// #
		// i = 1, j = M - 1
		coefficients.push_back(Trip(ReIdx(1, M - 1), ReIdx(1, M - 1), k2_val(y(M - 1)) - 2 * inv2_hx - 2 * inv2_hy));
		coefficients.push_back(Trip(ReIdx(1, M - 1), ReIdx(2, M - 1), inv2_hx));
		coefficients.push_back(Trip(ReIdx(1, M - 1), ReIdx(1, M - 2), inv2_hy));
		B[ReIdx(1, M - 1)] = -f(y(M - 1)) * inv2_hx;

		coefficients.push_back(Trip(ImIdx(1, M - 1), ImIdx(1, M - 1), k2_val(y(M - 1)) - 2 * inv2_hx - 2 * inv2_hy));
		coefficients.push_back(Trip(ImIdx(1, M - 1), ImIdx(2, M - 1), inv2_hx));
		coefficients.push_back(Trip(ImIdx(1, M - 1), ImIdx(1, M - 2), inv2_hy));
		B[ImIdx(1, M - 1)] = -f(y(M - 1)) * inv2_hx;

		//  #
		// #X#
		// i = 2 .. N1-1, j = 1
		for (unsigned i = 2; i < N1; i++)
		{
			coefficients.push_back(Trip(ReIdx(i, 1), ReIdx(i - 1, 1), inv2_hx));
			coefficients.push_back(Trip(ReIdx(i, 1), ReIdx(i, 1), k2_val(y(1)) - 2 * inv2_hx - 2 * inv2_hy));
			coefficients.push_back(Trip(ReIdx(i, 1), ReIdx(i + 1, 1), inv2_hx));
			coefficients.push_back(Trip(ReIdx(i, 1), ReIdx(i, 2), inv2_hy));

			coefficients.push_back(Trip(ImIdx(i, 1), ImIdx(i - 1, 1), inv2_hx));
			coefficients.push_back(Trip(ImIdx(i, 1), ImIdx(i, 1), k2_val(y(1)) - 2 * inv2_hx - 2 * inv2_hy));
			coefficients.push_back(Trip(ImIdx(i, 1), ImIdx(i + 1, 1), inv2_hx));
			coefficients.push_back(Trip(ImIdx(i, 1), ImIdx(i, 2), inv2_hy));
		}

		// #X#
		//  #
		// i = 2 .. N1-1, j = M - 1
		for (unsigned i = 2; i < N1; i++)
		{
			coefficients.push_back(Trip(ReIdx(i, M - 1), ReIdx(i - 1, M - 1), inv2_hx));
			coefficients.push_back(Trip(ReIdx(i, M - 1), ReIdx(i, M - 1), k2_val(y(1)) - 2 * inv2_hx - 2 * inv2_hy));
			coefficients.push_back(Trip(ReIdx(i, M - 1), ReIdx(i + 1, M - 1), inv2_hx));
			coefficients.push_back(Trip(ReIdx(i, M - 1), ReIdx(i, M - 2), inv2_hy));

			coefficients.push_back(Trip(ImIdx(i, M - 1), ImIdx(i - 1, M - 1), inv2_hx));
			coefficients.push_back(Trip(ImIdx(i, M - 1), ImIdx(i, M - 1), k2_val(y(1)) - 2 * inv2_hx - 2 * inv2_hy));
			coefficients.push_back(Trip(ImIdx(i, M - 1), ImIdx(i + 1, M - 1), inv2_hx));
			coefficients.push_back(Trip(ImIdx(i, M - 1), ImIdx(i, M - 2), inv2_hy));
		}

		// #X#
		// i = N1, j = 1 .. M - 1
		for (unsigned j = 1; j < M; j++)
		{
			double a = a_val(x(N1), y(j));
			coefficients.push_back(Trip(ReIdx(N1, j), ReIdx(N1 - 1, j), 1));
			coefficients.push_back(Trip(ReIdx(N1, j), ReIdx(N1, j), -2. + 1. / (a * a + 1.)));
			coefficients.push_back(Trip(ReIdx(N1, j), ImIdx(N1, j), -a / (a * a + 1.)));
			coefficients.push_back(Trip(ReIdx(N1, j), ReIdx(N1 + 1, j), 1. - 1. / (a * a + 1.)));
			coefficients.push_back(Trip(ReIdx(N1, j), ImIdx(N1 + 1, j), a / (a * a + 1.)));

			coefficients.push_back(Trip(ImIdx(N1, j), ImIdx(N1 - 1, j), 1));
			coefficients.push_back(Trip(ImIdx(N1, j), ReIdx(N1, j), a / (a * a + 1.)));
			coefficients.push_back(Trip(ImIdx(N1, j), ImIdx(N1, j), -2. + 1. / (a * a + 1.)));
			coefficients.push_back(Trip(ImIdx(N1, j), ReIdx(N1 + 1, j), -a / (a * a + 1.)));
			coefficients.push_back(Trip(ImIdx(N1, j), ImIdx(N1 + 1, j), 1. - 1. / (a * a + 1.)));
		}

		//  #
		// #X#
		// i = N1+1 .. N-2, j = 1
		for (unsigned i = N1 + 1; i < N - 1; i++)
		{
			double a = a_val(x(i), y(1));
			double k = k_val(y(1));

			int j = 1;

			coefficients.push_back(Trip(ReIdx(i, j), ReIdx(i - 1, j), a / ((a * a + 1) * (a * a + 1)) * inv_hx
				* (a * (a * a - 1) * inv_hx + k * (3 * a * a - 1) / (a * a + 1))));
			coefficients.push_back(Trip(ReIdx(i, j), ImIdx(i - 1, j), a * a / ((a * a + 1) * (a * a + 1)) * inv_hx
				* (2 * a * inv_hx - k * (a * a - 3) / (a * a + 1))));
			coefficients.push_back(Trip(ReIdx(i, j), ReIdx(i, j), -2 * a * a * (a * a - 1) / ((a * a + 1) * (a * a + 1)) * inv2_hx
				- k * a * (3 * a * a - 1) / ((a * a + 1) * (a * a + 1) * (a * a + 1)) * inv_hx
				- 2 * inv2_hy + k * k));
			coefficients.push_back(Trip(ReIdx(i, j), ImIdx(i, j), a * a / ((a * a + 1) * (a * a + 1)) * inv_hx
				* (-4 * a * inv_hx + k * (a * a - 3) / (a * a + 1))));
			coefficients.push_back(Trip(ReIdx(i, j), ReIdx(i + 1, j), a * a * (a * a - 1) * inv2_hx / ((a * a + 1) * (a * a + 1))));
			coefficients.push_back(Trip(ReIdx(i, j), ImIdx(i + 1, j), 2 * a * a * a / ((a * a + 1) * (a * a + 1)) * inv2_hx));
			coefficients.push_back(Trip(ReIdx(i, j), ReIdx(i, j + 1), inv2_hy));


			coefficients.push_back(Trip(ImIdx(i, j), ReIdx(i - 1, j), a * a / ((a * a + 1) * (a * a + 1)) * inv_hx
				* (-2 * a * inv_hx + k * (a * a - 3) / (a * a + 1))));
			coefficients.push_back(Trip(ImIdx(i, j), ImIdx(i - 1, j), a / ((a * a + 1) * (a * a + 1)) * inv_hx
				* (a * (a * a - 1) * inv_hx + k * (3 * a * a - 1) / (a * a + 1))));
			coefficients.push_back(Trip(ImIdx(i, j), ReIdx(i, j), a * a / ((a * a + 1) * (a * a + 1)) * inv_hx
				* (4 * a * inv_hx - k * (a * a - 3) / (a * a + 1))));
			coefficients.push_back(Trip(ImIdx(i, j), ImIdx(i, j), -2 * a * a * (a * a - 1) / ((a * a + 1) * (a * a + 1)) * inv2_hx
				- k * a * (3 * a * a - 1) / ((a * a + 1) * (a * a + 1) * (a * a + 1)) * inv_hx
				- 2 * inv2_hy + k * k));
			coefficients.push_back(Trip(ImIdx(i, j), ReIdx(i + 1, j), -2 * a * a * a / ((a * a + 1) * (a * a + 1)) * inv2_hx));
			coefficients.push_back(Trip(ImIdx(i, j), ImIdx(i + 1, j), a * a * (a * a - 1) * inv2_hx / ((a * a + 1) * (a * a + 1))));
			coefficients.push_back(Trip(ImIdx(i, j), ImIdx(i, j + 1), inv2_hy));
		}

		// #X#
		//  #
		// i = N1+1 .. N-2, j = M - 1
		for (unsigned i = N1 + 1; i < N - 1; i++)
		{
			double a = a_val(x(i), y(M - 1));
			double k = k_val(y(M - 1));

			int j = M - 1;

			coefficients.push_back(Trip(ReIdx(i, j), ReIdx(i, j - 1), inv2_hy));
			coefficients.push_back(Trip(ReIdx(i, j), ReIdx(i - 1, j), a / ((a * a + 1) * (a * a + 1)) * inv_hx
				* (a * (a * a - 1) * inv_hx + k * (3 * a * a - 1) / (a * a + 1))));
			coefficients.push_back(Trip(ReIdx(i, j), ImIdx(i - 1, j), a * a / ((a * a + 1) * (a * a + 1)) * inv_hx
				* (2 * a * inv_hx - k * (a * a - 3) / (a * a + 1))));
			coefficients.push_back(Trip(ReIdx(i, j), ReIdx(i, j), -2 * a * a * (a * a - 1) / ((a * a + 1) * (a * a + 1)) * inv2_hx
				- k * a * (3 * a * a - 1) / ((a * a + 1) * (a * a + 1) * (a * a + 1)) * inv_hx
				- 2 * inv2_hy + k * k));
			coefficients.push_back(Trip(ReIdx(i, j), ImIdx(i, j), a * a / ((a * a + 1) * (a * a + 1)) * inv_hx
				* (-4 * a * inv_hx + k * (a * a - 3) / (a * a + 1))));
			coefficients.push_back(Trip(ReIdx(i, j), ReIdx(i + 1, j), a * a * (a * a - 1) * inv2_hx / ((a * a + 1) * (a * a + 1))));
			coefficients.push_back(Trip(ReIdx(i, j), ImIdx(i + 1, j), 2 * a * a * a / ((a * a + 1) * (a * a + 1)) * inv2_hx));


			coefficients.push_back(Trip(ImIdx(i, j), ImIdx(i, j - 1), inv2_hy));
			coefficients.push_back(Trip(ImIdx(i, j), ReIdx(i - 1, j), a * a / ((a * a + 1) * (a * a + 1)) * inv_hx
				* (-2 * a * inv_hx + k * (a * a - 3) / (a * a + 1))));
			coefficients.push_back(Trip(ImIdx(i, j), ImIdx(i - 1, j), a / ((a * a + 1) * (a * a + 1)) * inv_hx
				* (a * (a * a - 1) * inv_hx + k * (3 * a * a - 1) / (a * a + 1))));
			coefficients.push_back(Trip(ImIdx(i, j), ReIdx(i, j), a * a / ((a * a + 1) * (a * a + 1)) * inv_hx
				* (4 * a * inv_hx - k * (a * a - 3) / (a * a + 1))));
			coefficients.push_back(Trip(ImIdx(i, j), ImIdx(i, j), -2 * a * a * (a * a - 1) / ((a * a + 1) * (a * a + 1)) * inv2_hx
				- k * a * (3 * a * a - 1) / ((a * a + 1) * (a * a + 1) * (a * a + 1)) * inv_hx
				- 2 * inv2_hy + k * k));
			coefficients.push_back(Trip(ImIdx(i, j), ReIdx(i + 1, j), -2 * a * a * a / ((a * a + 1) * (a * a + 1)) * inv2_hx));
			coefficients.push_back(Trip(ImIdx(i, j), ImIdx(i + 1, j), a * a * (a * a - 1) * inv2_hx / ((a * a + 1) * (a * a + 1))));
		}

		{
			//  #
			// #X
			// i = N-1, j = 1

			double a = a_val(x(N - 1), y(1));
			double k = k_val(y(1));

			int i = N - 1;
			int j = 1;

			coefficients.push_back(Trip(ReIdx(i, j), ReIdx(i - 1, j), a / ((a * a + 1) * (a * a + 1)) * inv_hx
				* (a * (a * a - 1) * inv_hx + k * (3 * a * a - 1) / (a * a + 1))));
			coefficients.push_back(Trip(ReIdx(i, j), ImIdx(i - 1, j), a* a / ((a * a + 1) * (a * a + 1)) * inv_hx
				* (2 * a * inv_hx - k * (a * a - 3) / (a * a + 1))));
			coefficients.push_back(Trip(ReIdx(i, j), ReIdx(i, j), -2 * a * a * (a * a - 1) / ((a * a + 1) * (a * a + 1)) * inv2_hx
				- k * a * (3 * a * a - 1) / ((a * a + 1) * (a * a + 1) * (a * a + 1)) * inv_hx
				- 2 * inv2_hy + k * k));
			coefficients.push_back(Trip(ReIdx(i, j), ImIdx(i, j), a* a / ((a * a + 1) * (a * a + 1)) * inv_hx
				* (-4 * a * inv_hx + k * (a * a - 3) / (a * a + 1))));
			coefficients.push_back(Trip(ReIdx(i, j), ReIdx(i, j + 1), inv2_hy));


			coefficients.push_back(Trip(ImIdx(i, j), ReIdx(i - 1, j), a* a / ((a * a + 1) * (a * a + 1)) * inv_hx
				* (-2 * a * inv_hx + k * (a * a - 3) / (a * a + 1))));
			coefficients.push_back(Trip(ImIdx(i, j), ImIdx(i - 1, j), a / ((a * a + 1) * (a * a + 1)) * inv_hx
				* (a * (a * a - 1) * inv_hx + k * (3 * a * a - 1) / (a * a + 1))));
			coefficients.push_back(Trip(ImIdx(i, j), ReIdx(i, j), a* a / ((a * a + 1) * (a * a + 1)) * inv_hx
				* (4 * a * inv_hx - k * (a * a - 3) / (a * a + 1))));
			coefficients.push_back(Trip(ImIdx(i, j), ImIdx(i, j), -2 * a * a * (a * a - 1) / ((a * a + 1) * (a * a + 1)) * inv2_hx
				- k * a * (3 * a * a - 1) / ((a * a + 1) * (a * a + 1) * (a * a + 1)) * inv_hx
				- 2 * inv2_hy + k * k));
			coefficients.push_back(Trip(ImIdx(i, j), ImIdx(i, j + 1), inv2_hy));
		}

		{
			// #X
			//  #
			// i = N-1, j = M - 1
			double a = a_val(x(N - 1), y(M - 1));
			double k = k_val(y(M - 1));
			int i = N - 1;
			int j = M - 1;

			coefficients.push_back(Trip(ReIdx(i, j), ReIdx(i, j - 1), inv2_hy));
			coefficients.push_back(Trip(ReIdx(i, j), ReIdx(i - 1, j), a / ((a * a + 1) * (a * a + 1)) * inv_hx
				* (a * (a * a - 1) * inv_hx + k * (3 * a * a - 1) / (a * a + 1))));
			coefficients.push_back(Trip(ReIdx(i, j), ImIdx(i - 1, j), a* a / ((a * a + 1) * (a * a + 1)) * inv_hx
				* (2 * a * inv_hx - k * (a * a - 3) / (a * a + 1))));
			coefficients.push_back(Trip(ReIdx(i, j), ReIdx(i, j), -2 * a * a * (a * a - 1) / ((a * a + 1) * (a * a + 1)) * inv2_hx
				- k * a * (3 * a * a - 1) / ((a * a + 1) * (a * a + 1) * (a * a + 1)) * inv_hx
				- 2 * inv2_hy + k * k));
			coefficients.push_back(Trip(ReIdx(i, j), ImIdx(i, j), a* a / ((a * a + 1) * (a * a + 1)) * inv_hx
				* (-4 * a * inv_hx + k * (a * a - 3) / (a * a + 1))));


			coefficients.push_back(Trip(ImIdx(i, j), ImIdx(i, j - 1), inv2_hy));
			coefficients.push_back(Trip(ImIdx(i, j), ReIdx(i - 1, j), a* a / ((a * a + 1) * (a * a + 1)) * inv_hx
				* (-2 * a * inv_hx + k * (a * a - 3) / (a * a + 1))));
			coefficients.push_back(Trip(ImIdx(i, j), ImIdx(i - 1, j), a / ((a * a + 1) * (a * a + 1)) * inv_hx
				* (a * (a * a - 1) * inv_hx + k * (3 * a * a - 1) / (a * a + 1))));
			coefficients.push_back(Trip(ImIdx(i, j), ReIdx(i, j), a* a / ((a * a + 1) * (a * a + 1)) * inv_hx
				* (4 * a * inv_hx - k * (a * a - 3) / (a * a + 1))));
			coefficients.push_back(Trip(ImIdx(i, j), ImIdx(i, j), -2 * a * a * (a * a - 1) / ((a * a + 1) * (a * a + 1)) * inv2_hx
				- k * a * (3 * a * a - 1) / ((a * a + 1) * (a * a + 1) * (a * a + 1)) * inv_hx
				- 2 * inv2_hy + k * k));
		}

		//  #
		// #X
		//  #
		// i = N-1, j = 2 .. M - 2
		for (unsigned j = 2; j < M - 1; j++)
		{
			double a = a_val(x(N - 1), y(j));
			double k = k_val(y(j));
			int i = N - 1;

			coefficients.push_back(Trip(ReIdx(i, j), ReIdx(i, j - 1), inv2_hy));
			coefficients.push_back(Trip(ReIdx(i, j), ReIdx(i - 1, j), a / ((a * a + 1) * (a * a + 1)) * inv_hx
				* (a * (a * a - 1) * inv_hx + k * (3 * a * a - 1) / (a * a + 1))));
			coefficients.push_back(Trip(ReIdx(i, j), ImIdx(i - 1, j), a* a / ((a * a + 1) * (a * a + 1)) * inv_hx
				* (2 * a * inv_hx - k * (a * a - 3) / (a * a + 1))));
			coefficients.push_back(Trip(ReIdx(i, j), ReIdx(i, j), -2 * a * a * (a * a - 1) / ((a * a + 1) * (a * a + 1)) * inv2_hx
				- k * a * (3 * a * a - 1) / ((a * a + 1) * (a * a + 1) * (a * a + 1)) * inv_hx
				- 2 * inv2_hy + k * k));
			coefficients.push_back(Trip(ReIdx(i, j), ImIdx(i, j), a* a / ((a * a + 1) * (a * a + 1)) * inv_hx
				* (-4 * a * inv_hx + k * (a * a - 3) / (a * a + 1))));
			coefficients.push_back(Trip(ReIdx(i, j), ReIdx(i, j + 1), inv2_hy));


			coefficients.push_back(Trip(ImIdx(i, j), ImIdx(i, j - 1), inv2_hy));
			coefficients.push_back(Trip(ImIdx(i, j), ReIdx(i - 1, j), a* a / ((a * a + 1) * (a * a + 1)) * inv_hx
				* (-2 * a * inv_hx + k * (a * a - 3) / (a * a + 1))));
			coefficients.push_back(Trip(ImIdx(i, j), ImIdx(i - 1, j), a / ((a * a + 1) * (a * a + 1)) * inv_hx
				* (a * (a * a - 1) * inv_hx + k * (3 * a * a - 1) / (a * a + 1))));
			coefficients.push_back(Trip(ImIdx(i, j), ReIdx(i, j), a* a / ((a * a + 1) * (a * a + 1)) * inv_hx
				* (4 * a * inv_hx - k * (a * a - 3) / (a * a + 1))));
			coefficients.push_back(Trip(ImIdx(i, j), ImIdx(i, j), -2 * a * a * (a * a - 1) / ((a * a + 1) * (a * a + 1)) * inv2_hx
				- k * a * (3 * a * a - 1) / ((a * a + 1) * (a * a + 1) * (a * a + 1)) * inv_hx
				- 2 * inv2_hy + k * k));
			coefficients.push_back(Trip(ImIdx(i, j), ImIdx(i, j + 1), inv2_hy));
		}

		//  #
		// #X#
		//  #
		// i = 2 .. N1 - 1, j = 2 .. M - 2
		for (unsigned i = 2; i < N1; i++)
			for (unsigned j = 2; j < M - 1; j++)
			{
				double a = a_val(x(i), y(j));
				double k = k_val(y(j));

				coefficients.push_back(Trip(ReIdx(i, j), ReIdx(i, j - 1), inv2_hy));
				coefficients.push_back(Trip(ReIdx(i, j), ReIdx(i - 1, j), inv2_hx));
				coefficients.push_back(Trip(ReIdx(i, j), ReIdx(i, j), k2_val(y(j)) - 2 * inv2_hx - 2 * inv2_hy));
				coefficients.push_back(Trip(ReIdx(i, j), ReIdx(i + 1, j), inv2_hx));
				coefficients.push_back(Trip(ReIdx(i, j), ReIdx(i, j + 1), inv2_hy));


				coefficients.push_back(Trip(ImIdx(i, j), ImIdx(i, j - 1), inv2_hy));
				coefficients.push_back(Trip(ImIdx(i, j), ImIdx(i - 1, j), inv2_hx));
				coefficients.push_back(Trip(ImIdx(i, j), ImIdx(i, j), k2_val(y(j)) - 2 * inv2_hx - 2 * inv2_hy));
				coefficients.push_back(Trip(ImIdx(i, j), ImIdx(i + 1, j), inv2_hx));
				coefficients.push_back(Trip(ImIdx(i, j), ImIdx(i, j + 1), inv2_hy));
			}

		//  #
		// #X#
		//  #
		// i = N1 + 1 .. N - 2, j = 2 .. M - 2
		for (unsigned i = N1 + 1; i < N - 1; i++)
			for (unsigned j = 2; j < M - 1; j++)
			{
				double a = a_val(x(i), y(j));
				double k = k_val(y(j));

				coefficients.push_back(Trip(ReIdx(i, j), ReIdx(i, j - 1), inv2_hy));
				coefficients.push_back(Trip(ReIdx(i, j), ReIdx(i - 1, j), a / ((a * a + 1) * (a * a + 1)) * inv_hx
					* (a * (a * a - 1) * inv_hx + k * (3 * a * a - 1) / (a * a + 1))));
				coefficients.push_back(Trip(ReIdx(i, j), ImIdx(i - 1, j), a* a / ((a * a + 1) * (a * a + 1)) * inv_hx
					* (2 * a * inv_hx - k * (a * a - 3) / (a * a + 1))));
				coefficients.push_back(Trip(ReIdx(i, j), ReIdx(i, j), -2 * a * a * (a * a - 1) / ((a * a + 1) * (a * a + 1)) * inv2_hx
					- k * a * (3 * a * a - 1) / ((a * a + 1) * (a * a + 1) * (a * a + 1)) * inv_hx
					- 2 * inv2_hy + k * k));
				coefficients.push_back(Trip(ReIdx(i, j), ImIdx(i, j), a* a / ((a * a + 1) * (a * a + 1)) * inv_hx
					* (-4 * a * inv_hx + k * (a * a - 3) / (a * a + 1))));
				coefficients.push_back(Trip(ReIdx(i, j), ReIdx(i + 1, j), a* a* (a* a - 1)* inv2_hx / ((a * a + 1) * (a * a + 1))));
				coefficients.push_back(Trip(ReIdx(i, j), ImIdx(i + 1, j), 2 * a * a * a / ((a * a + 1) * (a * a + 1)) * inv2_hx));
				coefficients.push_back(Trip(ReIdx(i, j), ReIdx(i, j + 1), inv2_hy));


				coefficients.push_back(Trip(ImIdx(i, j), ImIdx(i, j - 1), inv2_hy));
				coefficients.push_back(Trip(ImIdx(i, j), ReIdx(i - 1, j), a* a / ((a * a + 1) * (a * a + 1)) * inv_hx
					* (-2 * a * inv_hx + k * (a * a - 3) / (a * a + 1))));
				coefficients.push_back(Trip(ImIdx(i, j), ImIdx(i - 1, j), a / ((a * a + 1) * (a * a + 1)) * inv_hx
					* (a * (a * a - 1) * inv_hx + k * (3 * a * a - 1) / (a * a + 1))));
				coefficients.push_back(Trip(ImIdx(i, j), ReIdx(i, j), a* a / ((a * a + 1) * (a * a + 1)) * inv_hx
					* (4 * a * inv_hx - k * (a * a - 3) / (a * a + 1))));
				coefficients.push_back(Trip(ImIdx(i, j), ImIdx(i, j), -2 * a * a * (a * a - 1) / ((a * a + 1) * (a * a + 1)) * inv2_hx
					- k * a * (3 * a * a - 1) / ((a * a + 1) * (a * a + 1) * (a * a + 1)) * inv_hx
					- 2 * inv2_hy + k * k));
				coefficients.push_back(Trip(ImIdx(i, j), ReIdx(i + 1, j), -2 * a * a * a / ((a * a + 1) * (a * a + 1)) * inv2_hx));
				coefficients.push_back(Trip(ImIdx(i, j), ImIdx(i + 1, j), a* a* (a* a - 1)* inv2_hx / ((a * a + 1) * (a * a + 1))));
				coefficients.push_back(Trip(ImIdx(i, j), ImIdx(i, j + 1), inv2_hy));

			}

		A = SpMat(2 * (N - 1) * (M - 1), 2 * (N - 1) * (M - 1));;
		A.setFromTriplets(coefficients.begin(), coefficients.end());
	}

	void clear()
	{
		//free(A);
		//free(B);
	}


	void print_matrix()
	{
		/*for (unsigned i = 0; i < 2 * (N - 1) * (M - 1); i++)
		{
			for (unsigned j = 0; j < (N - 1) * (M - 1); j++)
			{
				std::cout << std::fixed << std::setprecision(2) << A[i][2 * j] << " (" << A[i][2 * j + 1] << ")\t";
			}
			std::cout << "  | " << B[i] << "\n";
		}*/

	}
	double f(double y)
	{
		return (std::abs(y - y_0) < 2 * hy) ? 0.25 * (1 + std::cos(0.5 * Pi * (y - y_0) / hy)) : 0;
	}
	double x(unsigned i)
	{
		return i * hx;
	}
	double y(unsigned j)
	{
		return j * hy;
	}

private:
	double inv_hx;
	double inv2_hx;
	double inv_hy;
	double inv2_hy;

	double phi(double y)
	{
		 return std::cosh(0.5) - std::cosh(y - 0.5);
		//return y * (1 - y) * (0.5 - y) * (0.5 - y);
	}
	double k2_val(double y)
	{
		return k_0 * k_0 * (1 + eps * phi(y));
	}
	double k_val(double y)
	{
		return k_0 * std::sqrt(1 + eps * phi(y));
	}
	double a_val(double x, double y)
	{
		return k_val(y) * (x_PML - x);
	}

};

void do_exp_eigen(unsigned N1, unsigned M, double L, double x_PML,
	double y_0, double k_0, double eps, std::string fname)
{
	System s(N1, M, L, x_PML, y_0, k_0, eps);
	unsigned N = s.N;
	std::cout << s.N1 << "(" << s.N << ") " << s.M << "\n";
	//s.print_matrix();

	/*for (int j = 1; j < s.M; j++)
	{
		for (int i = 1; i < s.N; i++)
		{
			std::cout << i << " " << j << " -> " << s.ReIdx(i, j) << "/" << s.ImIdx(i, j) << "\n";
		}
	}*/

	s.fillAB();
	//s.print_matrix();

	// Solving
	Eigen::VectorXd X = s.solve();

	std::ofstream fout;
	fout.open(fname + "_eigen_data.txt");
	fout << N1 << " " << M << " " << L << " " << x_PML << " " << y_0 << " " << k_0 << " " << eps << "\n";
	fout.close();

	fout.open(fname + "_eigen_Z.txt");
	int iter = 0;

	for (int i = 0; i <= N; i++)
	{
		fout << "0";
		if (i != N)
		{
			fout << " ";
		}
	}
	fout << std::endl;
	for (int j = 1; j < M; j++)
	{
		fout << s.f(s.y(j)) << " ";
		for (int i = 1; i < N; i++)
		{
			fout << X[iter] << " ";

			iter += 2;
		}
		fout << "0\n";
	}
	for (int i = 0; i <= N; i++)
	{
		fout << "0";
		if (i != N)
		{
			fout << " ";
		}
	}
	fout.close();

	fout.open(fname + "_eigen_X.txt");
	for (int i = 0; i <= N; i++)
	{
		fout << s.x(i) << std::endl;
	}
	fout.close();

	fout.open(fname + "_eigen_Y.txt");
	for (int i = 0; i <= M; i++)
	{
		fout << s.y(i) << std::endl;
	}
	fout.close();

	s.clear();
	
}

int main()
{
	double eps = 100;
	double L = 5;
	double k_0 = 10;
	double y_0 = 0.3;

	double x_PML = 5 * L;
	unsigned N1 = 40;
	unsigned M = 50;

	unsigned iter = 0;
	auto start = std::chrono::high_resolution_clock::now();
	do_exp_eigen(N1, M, L, x_PML, y_0, k_0, eps, std::to_string(iter));
	auto stop = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::seconds>(stop - start);
	std::cout << ":: " << duration.count() << " seconds\n";
	iter++;

	{
		std::vector<double> L_vals{ 0.5, 1, 5, 10 };
		for (int i = 0; i < L_vals.size(); i++)
		{
			x_PML = 15;
			start = std::chrono::high_resolution_clock::now();
			do_exp_eigen((unsigned) (N1 * L_vals[i] / L_vals[0]), M, L_vals[i], x_PML, y_0, k_0, eps, std::to_string(iter));
			stop = std::chrono::high_resolution_clock::now();
			duration = std::chrono::duration_cast<std::chrono::seconds>(stop - start);
			std::cout << "L = " << L_vals[i] << " :: " << duration.count() << " seconds\n";
			iter++;
		}
	}
	x_PML = 5 * L;
	{
		std::vector<double> eps_vals{ 100, 1, 0.1, 5e-2, 1e-2, 1e-3, 1e-4 };
		for (int i = 0; i < eps_vals.size(); i++)
		{
			start = std::chrono::high_resolution_clock::now();
			do_exp_eigen(N1, M, L, x_PML, y_0, k_0, eps_vals[i], std::to_string(iter));
			stop = std::chrono::high_resolution_clock::now();
			duration = std::chrono::duration_cast<std::chrono::seconds>(stop - start);
			std::cout << "eps = " << eps_vals[i] << " :: " << duration.count() << " seconds\n";
			iter++;
		}
	}

	{
		std::vector<double> k_vals{ 0.5, 1., 5., 10., 15 };
		for (int i = 0; i < k_vals.size(); i++)
		{
			start = std::chrono::high_resolution_clock::now();
			do_exp_eigen(N1, M, L, x_PML, y_0, k_vals[i], eps, std::to_string(iter));
			stop = std::chrono::high_resolution_clock::now();
			duration = std::chrono::duration_cast<std::chrono::seconds>(stop - start);
			std::cout << "k = " << k_vals[i] << " :: " << duration.count() << " seconds\n";
			iter++;
		}
	}

	{
		std::vector<double> y_vals{ 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9 };
		for (int i = 0; i < y_vals.size(); i++)
		{
			start = std::chrono::high_resolution_clock::now();
			do_exp_eigen(N1, M, L, x_PML, y_vals[i], k_0, eps, std::to_string(iter));
			stop = std::chrono::high_resolution_clock::now();
			duration = std::chrono::duration_cast<std::chrono::seconds>(stop - start);
			std::cout << "y = " << y_vals[i] << " :: " << duration.count() << " seconds\n";
			iter++;
		}
	}
}