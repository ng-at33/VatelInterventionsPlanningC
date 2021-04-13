/* 
 * Developped by Alexis TOULLAT (alexis.toullat@at-consulting.fr)
 * solver.h: implement the Strategy pattern for launching different
 * solving algorithms
 */

#pragma once

// Structure holding the Strategy pattern algorithm
class Algorithm {
public:
    virtual Solution* solve(Data* pData) = 0;
    virtual ~Algorithm() = default;
};

// Sub class of Algorithm that launches the GLOP solver
class AlgorithmMIP: public Algorithm {
public:
    Solution* solve(Data* pData);
};

// Sub class of Algorithm that solves the problem using a heuristic
class AlgorithmHeuristic: public Algorithm {
public:
    Solution* solve(Data* pData);
};

// Strategy pattern for the solving algorithm
class AlgorithmStrategy {
public:
    enum AlgoType { MIP, Heuristic };
    AlgorithmStrategy() { algo_ = nullptr; };
    void setAlgorithm(int type);
    Solution* solve(Data* pData);
private:
    Algorithm* algo_;
};