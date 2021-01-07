/* 
 * Developped by Alexis TOULLAT (alexis.toullat@at-consulting.fr)
 * solver.h: implement the Strategy pattern for launching different
 * solving algorithm
 */

#pragma once

// Structure holding the Strategy pattern algorithm
class Algorithm {
public:
    virtual Solution* solve(Data& data) = 0;
};

// Sub class of Algorithm that launches the GLOP solver
class AlgorithmMIP: public Algorithm {
public:
    Solution* solve(Data& data);
};

// Sub class of Algorithm that solves the problem using a heuristic
class AlgorithmHeuristic: public Algorithm {
public:
    Solution* solve(Data& data);
};

// Strategy pattern for the solving algorithm
class AlgorithmStrategy {
public:
    enum AlgoType { MIP, Heuristic };
    AlgorithmStrategy() { algo_ = NULL; };
    ~AlgorithmStrategy() { algo_ = NULL; };
    void setAlgorithm(int type);
    Solution* solve(Data& data);
private:
    Algorithm* algo_;
};