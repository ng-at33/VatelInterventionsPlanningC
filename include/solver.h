/*
 * Developped by Alexis TOULLAT (alexis.toullat@at-consulting.fr)
 * solver.h: implement the Strategy pattern for launching different
 * solving algorithms
 */

#pragma once

// namespace vtl
// Structure holding the Strategy pattern algorithm
class Algorithm {
public:
    virtual std::unique_ptr<Solution> solve(std::unique_ptr<Data>& p_data) = 0;
    virtual ~Algorithm() = default;
};

// Sub class of Algorithm that launches the GLOP solver
class AlgorithmMIP: public Algorithm {
public:
    std::unique_ptr<Solution> solve(std::unique_ptr<Data>& p_data);
};

// Sub class of Algorithm that solves the problem using a heuristic
class AlgorithmHeuristic: public Algorithm {
public:
    std::unique_ptr<Solution> solve(std::unique_ptr<Data>& p_data);
};

// Strategy pattern for the solving algorithm
class AlgorithmStrategy {
public:
    enum AlgoType { MIP, Heuristic };
    AlgorithmStrategy() { algo_ = nullptr; };
    void setAlgorithm(int type);
    std::unique_ptr<Solution> solve(std::unique_ptr<Data>& p_data);
private:
    Algorithm* algo_;
};