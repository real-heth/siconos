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
#include <stdlib.h>  // for free, malloc

#include "LCP_Solvers.h"                   // for lcp_nsqp
#include "LinearComplementarityProblem.h"  // for LinearComplementarityProblem
#include "NumericsFwd.h"                   // for LinearComplementarityProblem
#include "NumericsMatrix.h"                // for NumericsMatrix
#include "QP_Solvers.h"                    // for ql0001_
#include "SiconosConfig.h"                 // for HAS_FORTRAN, HAVE_QL0001
#include "SolverOptions.h"                 // for SICONOS_DPARAM_TOL, Solver...
#include "numerics_verbose.h"

void lcp_nsqp(LinearComplementarityProblem *problem, double *z, double *w, int *info,
              SolverOptions *options) {
  /* matrix M/vector q of the lcp */
  double *M = problem->M->matrix0;
  double *q = problem->q;
  /* size of the LCP */
  int n = problem->size;

  int i, j;

  int nmax;
  int m, me, mmax, mnn;

  double *Q, *A;
  double *p, *b, *xl, *xu;

  double *lambda;

  int lwar, liwar, iout, un;
  int *iwar;
  double *war;

  double tol = options->dparam[SICONOS_DPARAM_TOL];

  /* / m :        total number of constraints.*/
  m = n;
  /* / me :       number of equality constraints.*/
  me = 0;
  /* /  mmax :     row dimension of a. mmax must be at least one and greater than m.*/
  mmax = m + 1;
  /* /n :        number of variables.
  //nmax :     row dimension of C. nmax must be greater or equal to n.*/
  nmax = n;
  /* /mnn :      must be equal to m + n + n. */
  mnn = m + n + n;

  for (i = 0; i < n; i++) {
    z[i] = 0.0;
    w[i] = 0.0;
  }

  /* / Creation of objective function matrix Q and the the constant vector of the objective
   * function p*/

  Q = (double *)malloc(nmax * nmax * sizeof(double));
  for (i = 0; i < n; i++) {
    for (j = 0; j < n; j++) Q[j * n + i] = (M[j * n + i] + M[i * n + j]);
  }
  /* /for (i=0;i<n*n;i++) printf("Q[%i] = %g\n",i,Q[i]);*/

  p = (double *)malloc(nmax * sizeof(double));
  for (i = 0; i < n; i++) p[i] = q[i];
  /* /for (i=0;i<n;i++) printf("p[%i] = %g\n",i,p[i]);*/

  /* / Creation of the data matrix of the linear constraints, A and  the constant data of the
   * linear constraints b*/
  A = (double *)malloc(mmax * nmax * sizeof(double));
  for (i = 0; i < m; i++) {
    for (j = 0; j < n; j++) A[j * mmax + i] = M[j * n + i];
  }

  /* /for (i=0;i<mmax*mmax;i++) printf("A[%i] = %g\n",i,A[i]);*/

  b = (double *)malloc(mmax * sizeof(double));
  for (i = 0; i < m; i++) b[i] = q[i];

  /* /for (i=0;i<m;i++) printf("b[%i] = %g\n",i,b[i]);*/

  /* / Creation of the the lower and upper bounds for the variables.*/
  xu = (double *)malloc(n * sizeof(double));
  for (i = 0; i < n; i++) xu[i] = 1e300;
  xl = (double *)malloc(n * sizeof(double));
  for (i = 0; i < n; i++) xl[i] = 0.0;

  /* / on return, lambda contains the lagrange multipliers.*/
  lambda = (double *)malloc(mnn * sizeof(double));
  for (i = 0; i < mnn; i++) lambda[i] = 0.0;

  /* /   integer indicating the desired output unit number,*/
  iout = 6;

  /* /   output control.*/
  un = 1;

  /* / real working array. */
  lwar = 3 * nmax * nmax / 2 + 10 * nmax + 2 * mmax;
  war = (double *)malloc(lwar * sizeof(double));
  /* / integer working array. */
  liwar = n;
  iwar = (int *)malloc(liwar * sizeof(int));
  iwar[0] = 1;

#ifdef HAVE_QL0001
#ifdef HAS_FORTRAN
  /* / call ql0001_*/
  /*   F77NAME(ql0001)(m, me, mmax, n, nmax, mnn, Q, p, A, b, xl, xu, */
  /*    z, lambda, iout, *info , un, war, lwar, iwar, liwar, tol); */
  ql0001(&m, &me, &mmax, &n, &nmax, &mnn, Q, p, A, b, xl, xu, z, lambda, &iout, info, &un, war,
         &lwar, iwar, &liwar, &tol);
#else
  numerics_error("lcp_qp", "Fortran language is not enabled in siconos numerics");
#endif
#else
  numerics_error("lcp_qp", "ql0001 is not available in siconos numerics");
#endif
  /* /    printf("tol = %10.4e\n",*tol);
  // for (i=0;i<mnn;i++)printf("lambda[%i] = %g\n",i,lambda[i]);
  // for (i=0;i<n;i++)printf("z[%i] = %g\n",i,z[i]);

  // getting the multiplier due to the lower bounds*/
  for (i = 0; i < n; i++) w[i] = lambda[m + i];

  /* / memory freeing*/

  free(Q);
  free(p);
  free(A);
  free(b);
  free(xu);
  free(xl);
  free(lambda);
  free(iwar);
  free(war);
}
