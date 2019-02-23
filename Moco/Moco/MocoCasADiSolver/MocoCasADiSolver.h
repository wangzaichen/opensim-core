#ifndef MOCO_MOCOCASADISOLVER_H
#define MOCO_MOCOCASADISOLVER_H
/* -------------------------------------------------------------------------- *
 * OpenSim Moco: MocoCasADiSolver.h                                           *
 * -------------------------------------------------------------------------- *
 * Copyright (c) 2017 Stanford University and the Authors                     *
 *                                                                            *
 * Author(s): Christopher Dembia                                              *
 *                                                                            *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may    *
 * not use this file except in compliance with the License. You may obtain a  *
 * copy of the License at http://www.apache.org/licenses/LICENSE-2.0          *
 *                                                                            *
 * Unless required by applicable law or agreed to in writing, software        *
 * distributed under the License is distributed on an "AS IS" BASIS,          *
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.   *
 * See the License for the specific language governing permissions and        *
 * limitations under the License.                                             *
 * -------------------------------------------------------------------------- */

#include "../MocoDirectCollocationSolver.h"

namespace CasOC {
class Problem;
class Solver;
} // namespace CasOC

namespace OpenSim {

/// This solver uses the CasADi library (https://casadi.org) to convert the
/// MocoProblem into a generic nonlinear programming problem. CasADi efficiently
/// calculcates the derivatives required to solve MocoProblem%s, and may
/// solve your MocoProblem more quickly that MocoTropterSolver. In general,
/// we hope that the feature sets of MocoCasADiSolver and MocoTropterSolver
/// are the same.
/// Note, however, that parameter optimization problems are implemented much
/// less efficiently in this solver; for parameter optimization, first try
/// MocoTropterSolver.
///
/// Parallelization
/// ===============
/// By default, CasADi evaluate the integral cost integrand and the
/// differential-algebraic equations in parallel.
/// This should work fine for almost all models, but if you have custom model
/// components, ensure they are threadsafe. Make sure that threads do not
/// access shared resources like files or global variables at the same time.
///
/// You can turn off or change the number of cores used for individual problems
/// via either the OPENSIM_MOCO_PARALLEL environment variable (see
/// getMocoParallelEnvironmentVariable()) or the `parallel` property of this
/// class. For example, if you plan to solve two problems at the same time on
/// a machine with 4 cores, you could set OPENSIM_MOCO_PARALLEL to 2 to use
/// all 4 cores.
///
/// Note that there is overhead in the parallelization; if you plan to solve
/// many problems, it is better to turn off parallelization here and parallelize the
/// solving of your multiple problems using your system (e.g., invoke the opensim-moco
/// command-line tool in multiple Terminals or Command Prompts).
///
/// Note that the `parallel` property overrides the environment variable, allowing
/// more granular control over parallelization. However, the parallelization setting
/// does not logically belong as a property, as it does not affect the solution.
/// We encourage you to use the environment variable instead, as this allows
/// different users to solve the same problem in their preferred way.
///
/// @note The software license of CasADi (LGPL) is more restrictive than that of
/// the rest of Moco (Apache 2.0).
/// @note This solver currently only supports systems for which \f$ \dot{q} = u
/// \f$ (e.g., no quaternions).
class OSIMMOCO_API MocoCasADiSolver : public MocoDirectCollocationSolver {
    OpenSim_DECLARE_CONCRETE_OBJECT(
            MocoCasADiSolver, MocoDirectCollocationSolver);

public:
    OpenSim_DECLARE_PROPERTY(finite_difference_scheme, std::string,
        "The finite difference scheme CasADi will use to calculate problem "
        "derivatives (default: 'central').");

    OpenSim_DECLARE_OPTIONAL_PROPERTY(parallel, int,
            "Evaluate integral costs and the differential-algebraic "
            "equations in parallel across grid points? "
            "0: not parallel; 1: use all cores (default); greater than 1: use"
            "this number of threads. This overrides the OPENSIM_MOCO_PARALLEL "
            "environment variable.");
    MocoCasADiSolver();

    /// @name Specifying an initial guess
    /// @{

    /// Create a guess that you can edit and then set using setGuess().
    /// The types of guesses available are:
    /// - **bounds**: variable values are the midpoint between the variables'
    ///   bounds (the value for variables with ony one bound is the specified
    ///   bound). This is the default type.
    /// - **random**: values are randomly generated within the bounds.
    /// - **time-stepping**: see MocoSolver::createGuessTimeStepping().
    ///   NOTE: This option does not yet work well for this solver.
    /// @note Calling this method does *not* set an initial guess to be used
    /// in the solver; you must call setGuess() or setGuessFile() for that.
    /// @precondition You must have called resetProblem().
    MocoIterate createGuess(const std::string& type = "bounds") const;

    /// The number of time points in the iterate does *not* need to match
    /// `num_mesh_points`; the iterate will be interpolated to the correct size.
    /// This clears the `guess_file`, if any.
    void setGuess(MocoIterate guess);
    /// Use this convenience function if you want to choose the type of guess
    /// used, but do not want to modify it first.
    void setGuess(const std::string& type) { setGuess(createGuess(type)); }
    /// This clears any previously-set guess, if any. The file is not loaded
    /// until solving or if you call getGuess().
    /// Set to an empty string to clear the guess file.
    void setGuessFile(const std::string& file);

    /// Clear the stored guess and the `guess_file` if any.
    void clearGuess();

    /// Access the guess, loading it from the guess_file if necessary.
    /// This throws an exception if you have not set a guess (or guess file).
    /// If you have not set a guess (or guess file), this returns an empty
    /// guess, and when solving, we will generate a guess using bounds.
    const MocoIterate& getGuess() const;

    /// @}

protected:
    MocoSolution solveImpl() const override;

    std::unique_ptr<CasOC::Problem> createCasOCProblem() const;
    std::unique_ptr<CasOC::Solver> createCasOCSolver(
            const CasOC::Problem&) const;

private:
    void constructProperties();

    // When a copy of the solver is made, we want to keep any guess specified
    // by the API, but want to discard anything we've cached by loading a file.
    MocoIterate m_guessFromAPI;
    mutable SimTK::ResetOnCopy<MocoIterate> m_guessFromFile;
    mutable SimTK::ReferencePtr<const MocoIterate> m_guessToUse;

    // TODO: Move this elsewhere.
    using MocoProblemRepJar = ThreadsafeJar<const MocoProblemRep>;
    using ThreadsafeJarPtr = std::unique_ptr<MocoProblemRepJar>;
    mutable SimTK::ResetOnCopy<ThreadsafeJarPtr> m_jar;
};

} // namespace OpenSim

#endif // MOCO_MOCOCASADISOLVER_H
