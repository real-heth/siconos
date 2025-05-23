/* Siconos is a program dedicated to modeling, simulation and control
 * of non smooth dynamical systems.
 *
 * Copyright 2024 INRIA.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef MOHRCOULOMB2D_local_problem_tools_H
#define MOHRCOULOMB2D_local_problem_tools_H

/*!\file

 */
#include "NumericsFwd.h"    // for MohrCoulomb2DProblem
#include "SiconosConfig.h"  // for BUILD_AS_CPP // IWYU pragma: keep

#if defined(__cplusplus) && !defined(BUILD_AS_CPP)
extern "C" {
#endif

/** pointer to function used to call local solver */
typedef int (*SolverPtr)(MohrCoulomb2DProblem *, double *, SolverOptions *);

/** pointer to function used to update local problem */
typedef void (*UpdatePtr)(int, MohrCoulomb2DProblem *, MohrCoulomb2DProblem *, double *,
                          SolverOptions *);

/** pointer to function used to post-processed results after a call to the
 * (local) solver */
typedef void (*PostSolverPtr)(int, double *);

/** pointer to function used to free memory for objects used in nsgs solvers */
typedef void (*FreeLocalSolverPtr)(MohrCoulomb2DProblem *, MohrCoulomb2DProblem *,
                                   SolverOptions *);

typedef void (*CopyLocalReactionPtr)(double *, double *);

typedef void (*PerformRelaxationPtr)(double *, double *, double);

typedef double (*LightErrorSquaredPtr)(double *, double *);

typedef double (*SquaredNormPtr)(double *);

struct LocalMC2DProblemFunctionToolkit {
  SolverPtr local_solver;
  UpdatePtr update_local_problem;
  PostSolverPtr post_processed_local_result;
  FreeLocalSolverPtr free_local_solver;
  CopyLocalReactionPtr copy_local_reaction;
  PerformRelaxationPtr perform_relaxation;
  LightErrorSquaredPtr light_error_squared;
  SquaredNormPtr squared_norm;
};

struct LocalMC2DProblemFunctionToolkit *localMC2DProblemFunctionToolkit_new();

void localMC2DProblemFunctionToolkit_display(struct LocalMC2DProblemFunctionToolkit *);

MohrCoulomb2DProblem *mc2d_local_problem_allocate(MohrCoulomb2DProblem *problem);

void mc2d_local_problem_free(MohrCoulomb2DProblem *localproblem,
                             MohrCoulomb2DProblem *problem);
void mc2d_local_problem_compute_q(MohrCoulomb2DProblem *problem,
                                  MohrCoulomb2DProblem *localproblem, double *reaction,
                                  int contact);
void mc2d_local_problem_fill_M(MohrCoulomb2DProblem *problem,
                               MohrCoulomb2DProblem *localproblem, int contact);

#if defined(__cplusplus) && !defined(BUILD_AS_CPP)
}
#endif

#endif
